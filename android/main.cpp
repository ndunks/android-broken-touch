#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/input-event-codes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "input-debug.h"

#define DEBUG

static int fd = 0, event_len, sock_len;
//static struct input_event *mice_event, *mice_event_sync;

void atos(struct sockaddr_in *addr)
{
    uint32_t ip = ntohl(addr->sin_addr.s_addr);
    uint16_t port = ntohs(addr->sin_port);
    printf("%d.%d.%d.%d:%d\n",
           ip >> 24 & 0xff,
           ip >> 16 & 0xff,
           ip >> 8 & 0xff,
           ip >> 0 & 0xff,
           port);
}

static int mice_setup()
{
    int i;
    uinput_user_dev uidev;

    fd = open("/dev/uinput", O_WRONLY);
    if (fd < 0)
    {
        printf("can't open /dev/uinput");
        return 1;
    }

    // Tell bout this device support for keys
    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
    {
        printf("Fail ioctl /dev/uinput");
        return 1;
    }
    ioctl(fd, UI_SET_KEYBIT, EV_REP);
    ioctl(fd, UI_SET_KEYBIT, EV_SYN);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);

    for (i = REL_X; i < REL_MAX; i++)
    {
        ioctl(fd, UI_SET_RELBIT, i);
    }

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "fixbrokentouch");
    uidev.id.bustype = BUS_VIRTUAL;
    uidev.id.vendor = 1;
    uidev.id.product = 1;
    uidev.id.version = 1;

    if (write(fd, &uidev, sizeof(uidev)) < 0)
    {
        printf("error: write");
        return 1;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0)
    {
        printf("error: UI_DEV_CREATE");
        return 1;
    }

    // mice_event = (input_event *)malloc(event_len);
    // mice_event_sync = (input_event *)calloc(event_len, 1);
    // mice_event_sync->type = EV_SYN;
    // mice_event_sync->code = SYN_REPORT;
    return 0;
}

static void mice_clear()
{
    sleep(3);
    if (ioctl(fd, UI_DEV_DESTROY) == -1)
    {
        close(fd);
    }
    // free(mice_event);
    // free(mice_event_sync);
}

// static void mice_write(uint16_t type, uint16_t code, int32_t value)
// {
//     memset(mice_event, 0, sizeof(struct input_event));
//     mice_event->type = type;
//     mice_event->code = code;
//     mice_event->value = value;
//     write(fd, mice_event, sizeof(struct input_event));
// }

// static void mice_write_sync(uint16_t type, uint16_t code, int32_t value)
// {
//     mice_write(type, code, value);
//     write(fd, mice_event_sync, sizeof(struct input_event));
// }

// void mice_press(uint16_t code)
// {
//     mice_write_sync(EV_KEY, code, 1);
// }

// void mice_release(uint16_t code)
// {
//     mice_write_sync(EV_KEY, code, 0);
// }

// void mice_click(uint16_t code)
// {
//     mice_press(code);
//     mice_release(code);
// }

// void mice_move(int x, int y)
// {
//     mice_write(EV_REL, REL_X, x);
//     mice_write_sync(EV_REL, REL_Y, y);
// }

// void mice_commander()
// {
//     char *line = NULL;
//     size_t len = 0;
//     ssize_t lineSize = 0;
//     int x, y;
//     printf("Command: x y or c\n");
//     while (1)
//     {
//         lineSize = getline(&line, &len, stdin);
//         switch (line[0])
//         {
//         case 'x':
//             printf("Bye\n");
//             free(line);
//             return;
//         case 'b':
//             printf("BACK\n");
//             mice_press(BTN_RIGHT);
//             break;
//         case 'c':
//             printf("CLICK\n");
//             mice_press(BTN_LEFT);
//             break;
//         default:
//             strtok(line, " ");
//             x = atoi(line);
//             strtok(NULL, " ");
//             y = atoi(line);
//             printf("X = %d, Y = %d\n", x, y);
//             mice_move(x, y);
//             break;
//         }
//     }
//     free(line);
// }
int main(int argc, const char **argv)
{
    int sock_fd, buf_len, i;
    socklen_t client_len;
    ssize_t recv_len;
    struct sockaddr_in server, client;
    char *buf;
    struct input_event *ev;

    sock_len = sizeof(struct sockaddr_in);
    event_len = sizeof(struct input_event);
    buf_len = event_len * 64;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (argc > 1)
        server.sin_port = htons(atoi(argv[1]));
    else
        server.sin_port = htons(1567);

    if (server.sin_port <= 0)
    {
        printf("Invalid port");
        return 1;
    }

    if (mice_setup() != 0)
    {
        printf("Fail setup input device\n");
        return 1;
    };

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
    {
        printf("Cannot open fd\n");
        return 1;
    }
    printf("Listening on ");
    if (bind(sock_fd, (struct sockaddr *)&server, sock_len) != 0)
    {
        perror("UDP");
        goto end;
    }
    atos(&server);
        struct input_event utest;
    // linux x86_64 = 24 byte, Android Arm 16 byte
    event_len = sizeof(struct input_event);
    printf("event_len %d, time len %d\n", event_len,sizeof(utest.time));

    buf = (char *)malloc(buf_len);

    while (1)
    {
        client_len = sock_len;
        memset(&client, 0, sock_len);
        recv_len = recvfrom(sock_fd, buf, buf_len, 0, (struct sockaddr *)&client, &client_len);

        if (recv_len % event_len != 0)
        {
            printf("(!) Some data corrupt\n");
        }
        //write(fd, buf, recv_len);
#ifdef DEBUG
        ev = (struct input_event *)buf;
        for (i = 0; i < recv_len / event_len; i++)
        {
            write(fd, ev, event_len);
            print_event(ev);
            ev++;
        }
        printf("<--- %2d event - %d bytes\t", recv_len / event_len, recv_len);
        atos(&client);
#endif
    }

    free(buf);
end:
    mice_clear();
}