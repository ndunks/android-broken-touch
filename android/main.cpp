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
    if (ioctl(fd, UI_SET_EVBIT, EV_SYN) < 0)
    {
        printf("Fail ioctl /dev/uinput");
        return 1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_REP);
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(fd, UI_SET_KEYBIT, BTN_SIDE);
    ioctl(fd, UI_SET_KEYBIT, BTN_EXTRA);
    ioctl(fd, UI_SET_KEYBIT, KEY_POWER);
    ioctl(fd, UI_SET_KEYBIT, KEY_BACK);
    ioctl(fd, UI_SET_KEYBIT, KEY_HOME);

    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);
    ioctl(fd, UI_SET_RELBIT, REL_HWHEEL);
    ioctl(fd, UI_SET_RELBIT, REL_WHEEL);

    ioctl(fd, UI_SET_EVBIT, EV_MSC);
    ioctl(fd, UI_SET_MSCBIT, MSC_SCAN);

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

    return 0;
}

static void mice_clear()
{
    sleep(3);
    if (ioctl(fd, UI_DEV_DESTROY) == -1)
    {
        close(fd);
    }
}

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
    if (bind(sock_fd, (struct sockaddr *)&server, sock_len) != 0)
    {
        perror("UDP");
        goto end;
    }

#ifdef DEBUG
    printf("Listening on ");
    atos(&server);
#endif

    // linux x86_64 = 24 byte, Android Arm 16 byte
    event_len = sizeof(struct input_event);
    buf = (char *)malloc(buf_len);

    while (1)
    {
        client_len = sock_len;
        memset(&client, 0, sock_len);
        recv_len = recvfrom(sock_fd, buf, buf_len, 0, (struct sockaddr *)&client, &client_len);

#ifdef DEBUG
        if (recv_len % event_len != 0)
        {
            printf("(!) Some data corrupt\n");
        }
#endif
        //write(fd, buf, recv_len);
        ev = (struct input_event *)buf;
        for (i = 0; i < recv_len / event_len; i++)
        {
            write(fd, ev, event_len);

#ifdef DEBUG
            print_event(ev);
#endif
            ev++;
        }
#ifdef DEBUG
        printf("<--- %2d event - %d bytes\t", recv_len / event_len, recv_len);
        atos(&client);
#endif
    }

    free(buf);
end:
    mice_clear();
}