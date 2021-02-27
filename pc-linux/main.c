#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <linux/input.h>

static volatile sig_atomic_t interrupted = 0;

static void interrupt_handler(int sig)
{
    interrupted = 1;
}

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

void capture_mouse(int input_event_no)
{
    char dev_name[1024];
    int fd, i, rd, event_size;
    struct input_event ev[64];
    fd_set rdfs;

    event_size = (int)sizeof(struct input_event);

    sprintf(dev_name, "/dev/input/event%d", input_event_no);
    printf("Opening %s\n", dev_name);
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
    signal(SIGINT, interrupt_handler);
    signal(SIGTERM, interrupt_handler);
    FD_ZERO(&rdfs);
    FD_SET(fd, &rdfs);
    while (!interrupted)
    {
        select(fd + 1, &rdfs, NULL, NULL, NULL);
        if (interrupted)
            break;
        rd = read(fd, ev, sizeof(ev));

        if (rd < event_size)
        {
            printf("expected %d bytes, got %d\n", event_size, rd);
            perror("\nevtest: error reading");
            goto err;
        }
        for (i = 0; i < rd / event_size; i++)
        {
            printf("%d\t%d\t%d\n", ev[i].type, ev[i].code, ev[i].value);
        }
    }
err:
    ioctl(fd, EVIOCGRAB, (void *)0);
    close(fd);
}

int main(int argc, char **argv)
{
    int input_event_no, default_select = -1;
    if (argc > 1)
    {
        default_select = atoi(argv[1]);
    }
    input_event_no = choose_device(default_select);
    if (input_event_no < 0)
    {
        perror("Require working mouse.");
        return 1;
    }
    capture_mouse(input_event_no);

    return 0;
}