/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-25     PiscesLyn   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "bq32k.h"

#if defined(PKG_USING_BQ32K)
#define BQ32K_NAME      "bq32k"
#define BQ32K_I2C_BUS   "i2c3"

int rt_hw_bq32k_init(void)
{
    rt_err_t result;
    result = bq32k_register(BQ32K_NAME, BQ32K_I2C_BUS);
    if (result != RT_EOK)
    {
        rt_kprintf("bq32k register err code: %d\n", result);
        return result;
    }
    rt_kprintf("bq32k init success\n");
    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_bq32k_init);


time_t bq32k_sync_to_rtc(void)
{
#ifdef RT_USING_RTC
    struct tm *cur_tm;
#endif
    time_t cur_time = 0;
    rt_device_t device;

    device = rt_device_find(BQ32K_NAME);
    if (device == RT_NULL)
    {
        return -RT_ERROR;
    }
    
    rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &cur_time);

    if (cur_time)
    {

#ifdef RT_USING_RTC
        cur_tm = localtime(&cur_time);
        set_time(cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec);

        cur_tm = localtime(&cur_time);
        set_date(cur_tm->tm_year + 1900, cur_tm->tm_mon + 1, cur_tm->tm_mday);
#endif /* RT_USING_RTC */

    }

    return cur_time;
}

time_t bq32k_sync_from_rtc(void)
{
    time_t now;
    rt_device_t device;

    /* get current time */
    now = time(RT_NULL);

    device = rt_device_find(BQ32K_NAME);
    if (device == RT_NULL)
    {
        return -RT_ERROR;
    }
    
    rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &now);

    return now;
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void bq32k_sync(int argc, char **argv)
{
    time_t cur_time;
    
    if (argc < 2)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("bq32k_sync to     -sync to rtc\n");
        rt_kprintf("bq32k_sync from   -sync from rtc\n");
        rt_kprintf("\n");
    }
    else
    {
        if (!strcmp(argv[1], "to"))
        {
            cur_time = bq32k_sync_to_rtc();
            rt_kprintf("Get local time from BQ32K: %s", ctime((const time_t*) &cur_time));
        }
        else if (!strcmp(argv[1], "from"))
        {
            cur_time = bq32k_sync_from_rtc();
            rt_kprintf("Get local time from RTC: %s", ctime((const time_t*) &cur_time));
        }
    }
}
MSH_CMD_EXPORT(bq32k_sync, bq32k sync rtc.);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */

#endif
