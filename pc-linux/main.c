#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "input-debug.h"


static volatile sig_atomic_t interrupted = 0;

static void interrupt_handler(int sig)
{
    interrupted = 1;
}

int event_len, sock_len;

int choose_device(int default_select)
{
    char buf[1024], byte, *str;
    ssize_t len = 0;
    int option[99], option_count = 0, selected = 0;

    int fd = open("/proc/bus/input/devices", O_RDONLY);
    if (fd < 0)
    {
        perror("fail /proc/bus/input/devices");
        exit(EXIT_FAILURE);
    }

    while (read(fd, &byte, 1) > 0)
    {
        buf[len++] = byte;
        if (len > 1 && byte == '\n' && buf[len - 2] == '\n')
        {
            buf[len - 1] = 0;
            str = strstr(buf, "H: Handlers=");
            if (str != NULL)
            {
                str = strtok(str + 12, "\n");
                if (strstr(str, "mouse") != NULL && strstr(str, "event") != NULL)
                {
                    str = strstr(str, "event");
                    str = strtok(str + 5, " \n");
                    option[option_count++] = atoi(str);
                    printf("%2d : ", option_count);
                    // find the name
                    str = strstr(buf, "N: Name=");
                    if (str != NULL)
                    {
                        if (str[8] == '"')
                            str = strtok(str + 9, "\"\n");
                        else
                            str = strtok(str + 8, "\n");
                        printf("%s\n", str);
                    }
                    else
                    {
                        printf("/dev/input/event%d\n\n", option[option_count - 1]);
                    }
                }
            }
            len = 0;
        }
    }
    if (option_count == 0)
        return -1;
    if (option_count == 1)
        return option[0];
    do
    {
        printf("Choose [1-%d]: ", option_count);
        if (default_select > 0 && default_select <= option_count)
        {
            printf("%d\n", default_select);
            return option[default_select - 1];
        }
        fflush(stdout);
        fgets(buf, 100, stdin);
        selected = atoi(buf);
    } while (selected < 1 || selected > option_count);
    return option[selected - 1];
}

void capture_mouse(int input_event_no, struct sockaddr_in target)
{
    char dev_name[1024], *buf, *buf_send;
    int fd, i, event_count, recv_len, event_len,
        sock_fd, buf_len, send_len, event_time_len,
        android_event_size = 16;
    struct input_event *ev;
    fd_set rdfs;

    event_len = (int)sizeof(struct input_event);
    event_time_len = (int)sizeof(ev->time);
    buf_len = event_len * 64;

    sprintf(dev_name, "/dev/input/event%d", input_event_no);
    printf("Opening %d %s\n", event_time_len, dev_name);
    fd = open(dev_name, O_RDONLY);
    if (fd < 0)
    {
        perror("Failed");
        exit(EXIT_FAILURE);
    }
    if (ioctl(fd, EVIOCGRAB, (void *)1) != 0)
    {
        close(fd);
        printf("Device already grabbed by another process\nCheck with fuser -v %s\n", dev_name);
        exit(EXIT_FAILURE);
    }
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
    {
        perror("net");
        goto err;
    }

    signal(SIGINT, interrupt_handler);
    signal(SIGTERM, interrupt_handler);
    FD_ZERO(&rdfs);
    FD_SET(fd, &rdfs);
    buf = malloc(buf_len);
    buf_send = malloc(buf_len);
    while (!interrupted)
    {
        select(fd + 1, &rdfs, NULL, NULL, NULL);
        if (interrupted)
            break;
        recv_len = read(fd, buf, 2048);

        if (recv_len % event_len != 0)
        {
            printf("Corrupted event. %d %% %d != 0\n", recv_len, event_len);
            continue;
        }
        // linux x86_64 = 24 byte, Android Arm 16 byte
        // sizeof(input_event.time), linux x86_64 = 16 byte, Arm = 8 byte
        event_count = recv_len / event_len;
        ev = (struct input_event *)buf;
        for (i = 0; i < event_len; i++)
        {
            ev->time.tv_sec = 0;
            ev->time.tv_usec = 0;
            // translate button
            if (ev->type == EV_KEY)
            {
                switch (ev->code)
                {
                case BTN_MIDDLE:
                    ev->code = KEY_POWER;
                    break;
                case BTN_RIGHT:
                    ev->code = KEY_BACK;
                    break;
                }
            }
            memcpy(buf_send + android_event_size * i, ((char *)ev) + 8, android_event_size);
            ev++;
        }
        sendto(sock_fd, buf_send, android_event_size * event_count, 0, (struct sockaddr *)&target, sock_len);
#ifdef DEBUG
        ev = (struct input_event *)buf;
        for (i = 0; i < recv_len / event_len; i++)
        {
            print_event(ev);
            ev++;
        }
        printf("---> %2d event - %d bytes\n", recv_len / event_len, recv_len);
#endif
    }
err:
    free(buf);
    free(buf_send);
    ioctl(fd, EVIOCGRAB, (void *)0);
    close(fd);
}
void *atos(struct sockaddr_in *addr)
{
    uint32_t ip = ntohl(addr->sin_addr.s_addr);
    uint16_t port = ntohs(addr->sin_port);
    // maks: xxx.xxx.xxx.xxx:xxxxx
    char *buff = malloc(22);
    sprintf(buff, "%d.%d.%d.%d:%d\n",
            ip >> 24 & 0xff,
            ip >> 16 & 0xff,
            ip >> 8 & 0xff,
            ip >> 0 & 0xff,
            port);
    printf(buff);
    free(buff);
}

int main(int argc, char **argv)
{
    int input_event_no, default_select = -1;
    struct sockaddr_in sockbc;
    char *ip, *port;
    sock_len = sizeof(struct sockaddr_in);

    if (argc < 2)
    {
        printf("%s ip[:port 1567] [mouse]");
        return 1;
    }

    sockbc.sin_family = AF_INET;
    sockbc.sin_addr.s_addr = INADDR_BROADCAST;
    ip = strtok(argv[1], ":");
    port = strtok(NULL, "");
    if (port != NULL)
        sockbc.sin_port = htons(atoi(port));
    else
        sockbc.sin_port = htons(1567);

    if (inet_aton(argv[1], &sockbc.sin_addr) == 0)
    {
        printf("Invalid IPv4\n");
        return 1;
    }

    printf("Your mouse will redirected to UDP ");
    atos(&sockbc);

    if (argc > 2)
        default_select = atoi(argv[2]);

    input_event_no = choose_device(default_select);
    if (input_event_no < 0)
    {
        perror("Require working mouse.");
        return 1;
    }
    capture_mouse(input_event_no, sockbc);

    return 0;
}