#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/input-event-codes.h>
#include "main.h"

#define HAS_KEY(v) (key_bits[v / 8] & (1 << (v % 8)))
#define HAS_ABS(v) (abs_bits[v / 8] & (1 << (v % 8)))

static int fd = 0;
static struct input_event *mice_event, *mice_event_sync;
static uint8_t event_size;

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

    mice_event = (input_event *)malloc(sizeof(struct input_event));
    mice_event_sync = (input_event *)calloc(sizeof(struct input_event), 1);
    mice_event_sync->type = EV_SYN;
    mice_event_sync->code = SYN_REPORT;
    return 0;
}

static void mice_clear()
{
    sleep(3);
    if (ioctl(fd, UI_DEV_DESTROY) == -1)
    {
        close(fd);
    }
    free(mice_event);
    free(mice_event_sync);
}

static void mice_write(uint16_t type, uint16_t code, int32_t value)
{
    memset(mice_event, 0, sizeof(struct input_event));
    mice_event->type = type;
    mice_event->code = code;
    mice_event->value = value;
    write(fd, mice_event, sizeof(struct input_event));
}

static void mice_write_sync(uint16_t type, uint16_t code, int32_t value)
{
    mice_write(type, code, value);
    write(fd, mice_event_sync, sizeof(struct input_event));
}

void mice_press(uint16_t code)
{
    mice_write_sync(EV_KEY, code, 1);
}

void mice_release(uint16_t code)
{
    mice_write_sync(EV_KEY, code, 0);
}

void mice_click(uint16_t code)
{
    mice_press(code);
    mice_release(code);
}

void mice_move(int x, int y)
{
    mice_write(EV_REL, REL_X, x);
    mice_write_sync(EV_REL, REL_Y, y);
}

void mice_commander()
{
    char *line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;
    int x, y;
    printf("Command: x y or c\n");
    while (1)
    {
        lineSize = getline(&line, &len, stdin);
        switch (line[0])
        {
        case 'x':
            printf("Bye\n");
            free(line);
            return;
        case 'b':
            printf("BACK\n");
            mice_press(BTN_RIGHT);
            break;
        case 'c':
            printf("CLICK\n");
            mice_press(BTN_LEFT);
            break;
        default:
            strtok(line, " ");
            x = atoi(line);
            strtok(NULL, " ");
            y = atoi(line);
            printf("X = %d, Y = %d\n", x, y);
            mice_move(x, y);
            break;
        }
    }
    free(line);
}
int main()
{

    if (mice_setup() == 0)
    {
        printf("TEST\n");
        mice_commander();
        printf("DONE\n");
        mice_clear();
    };
}