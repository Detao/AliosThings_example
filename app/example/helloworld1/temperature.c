#include<aos/aos.h>
#include "sensor.h"
int fd_temp = -1;

static int get_temperature_data(float *dataT)
{
    temperature_data_t data = {0};
    ssize_t size = 0;

    size = aos_read(fd_temp, &data, sizeof(data));
    if (size != sizeof(data)) {
        return -1;
    }

    *dataT = (float)data.t / 10;

    return 0;
}

#include <aos/aos.h>

static void app_delayed_action(void *arg)
{
    LOG("helloworld %s:%d %s\r\n", __func__, __LINE__, aos_task_name());
    aos_post_delayed_action(5000, app_delayed_action, NULL);
}

int application_start(int argc, char *argv[])
{
    LOG("application started.");
    aos_post_delayed_action(1000, app_delayed_action, NULL);
    aos_loop_run();

    return 0;
}