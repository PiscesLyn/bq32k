/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-13     PiscesLyn    first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "bq32k.h"

//#define DRV_DEBUG
#define LOG_TAG             "bq32k"
#include <drv_log.h>

struct bq32k_device
{
    struct rt_device         parent;
    struct rt_i2c_bus_device *bus;
};

#define BQ32K_ADDRESS       0x68

#define BQ32K_SECONDS       0x00    /* Seconds register address */
#define BQ32K_SECONDS_MASK  0x7F    /* Mask over seconds value */
#define BQ32K_STOP          0x80    /* Oscillator Stop flat */

#define BQ32K_MINUTES       0x01    /* Minutes register address */
#define BQ32K_MINUTES_MASK  0x7F    /* Mask over minutes value */
#define BQ32K_OF            0x80    /* Oscillator Failure flag */

#define BQ32K_HOURS_MASK    0x3F    /* Mask over hours value */
#define BQ32K_CENT          0x40    /* Century flag */
#define BQ32K_CENT_EN       0x80    /* Century flag enable bit */

#define BQ32K_CALIBRATION   0x07    /* CAL_CFG1, calibration and control */
#define BQ32K_TCH2          0x08    /* Trickle charge enable */
#define BQ32K_CFG2          0x09    /* Trickle charger control */
#define BQ32K_TCFE          BIT(6)  /* Trickle charge FET bypass */

#define MAX_LEN             10      /* Maximum number of consecutive
                                    * register for this particular RTC.
                                    */

#define bcd2bin(x)    (((x) & 0x0f) + ((x) >> 4) * 10)
#define bin2bcd(x)    ((((x) / 10) << 4) + (x) % 10)
  
struct bq32k_regs {  
    rt_uint8_t     seconds;  
    rt_uint8_t     minutes;  
    rt_uint8_t     cent_hours;  
    rt_uint8_t     day;  
    rt_uint8_t     date;  
    rt_uint8_t     month;  
    rt_uint8_t     years;  
};

static rt_err_t bq32k_read(rt_device_t dev, void *data, rt_off_t off, rt_size_t len)  
{  
    struct bq32k_device *bq32k;
    RT_ASSERT(dev != 0);

    bq32k = (struct bq32k_device *) dev;

    struct rt_i2c_msg msgs[] = {
        {  
            .addr = BQ32K_ADDRESS,
            .flags = 0,  
            .len = 1,  
            .buf = (rt_uint8_t *)&off,
        }, {  
            .addr = BQ32K_ADDRESS,
            .flags = RT_I2C_RD,
            .len = len,
            .buf = (rt_uint8_t *)data,
        }  
    };  
  
    if (rt_i2c_transfer(bq32k->bus, msgs, 2) == 2)
        return RT_EOK;
  
    return -RT_ERROR;
}  
  
static rt_err_t bq32k_write(rt_device_t dev, void *data, rt_off_t off, rt_size_t len)  
{  
    struct bq32k_device *bq32k;
    RT_ASSERT(dev != 0);

    bq32k = (struct bq32k_device *) dev;

    uint8_t buffer[MAX_LEN + 1];
    
    buffer[0] = off;
    memcpy(&buffer[1], data, len);

    if (rt_i2c_master_send(bq32k->bus, BQ32K_ADDRESS, 0, buffer, len + 1) == len + 1)
        return RT_EOK; 
    return -RT_ERROR;  
}  
  
static rt_err_t bq32k_rtc_read_time(rt_device_t dev, struct tm *tm)  
{  
    struct bq32k_regs regs;
    rt_err_t error;
  
    error = bq32k_read(dev, &regs, 0, sizeof(regs));
    if (error != RT_EOK)
        return error;
  
    tm->tm_sec = bcd2bin(regs.seconds & BQ32K_SECONDS_MASK);
    tm->tm_min = bcd2bin(regs.minutes & BQ32K_SECONDS_MASK);
    tm->tm_hour = bcd2bin(regs.cent_hours & BQ32K_HOURS_MASK);
    tm->tm_mday = bcd2bin(regs.date);
    tm->tm_wday = bcd2bin(regs.day) - 1;
    tm->tm_mon = bcd2bin(regs.month) - 1;
    tm->tm_year = bcd2bin(regs.years) +
                ((regs.cent_hours & BQ32K_CENT) ? 100 : 0);
  
    return RT_EOK;
}  
  
static rt_err_t bq32k_rtc_set_time(rt_device_t dev, struct tm *tm)
{  
    struct bq32k_regs regs;
  
    regs.seconds = bin2bcd(tm->tm_sec);
    regs.minutes = bin2bcd(tm->tm_min);
    regs.cent_hours = bin2bcd(tm->tm_hour) | BQ32K_CENT_EN;
    regs.day = bin2bcd(tm->tm_wday + 1);
    regs.date = bin2bcd(tm->tm_mday);
    regs.month = bin2bcd(tm->tm_mon + 1);
  
    if (tm->tm_year >= 100) {
        regs.cent_hours |= BQ32K_CENT;
        regs.years = bin2bcd(tm->tm_year - 100);
    } else
        regs.years = bin2bcd(tm->tm_year);
  
    return bq32k_write(dev, &regs, 0, sizeof(regs));
}

static rt_err_t get_bq32k_rtc_timestamp(rt_device_t dev, time_t *time_stamp)
{
    struct tm tm_new = {0};
    if (bq32k_rtc_read_time(dev, &tm_new) != RT_EOK)
    {
        LOG_E("get bq32k rtc time error.");
        return -RT_ERROR;
    }
    LOG_D("get bq32k rtc time.");
    *time_stamp = mktime(&tm_new);
    return RT_EOK;
}

static rt_err_t set_bq32k_rtc_timestamp(rt_device_t dev, time_t time_stamp)
{
    struct tm *p_tm;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return -RT_ERROR;
    }

    if (bq32k_rtc_set_time(dev, p_tm) != RT_EOK)
    {
        LOG_E("set bq32k rtc time error.");
        return -RT_ERROR;
    }
    LOG_D("set bq32k rtc time.");
    return RT_EOK;
}

static rt_err_t bq32k_control(rt_device_t dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK;
    RT_ASSERT(dev != RT_NULL);
    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        if (get_bq32k_rtc_timestamp(dev, (rt_uint32_t *)args) != RT_EOK)
        {
            result = -RT_ERROR;
        }
        LOG_D("RTC: get bq32k rtc_time %x\n", *(rt_uint32_t *)args);
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
        if (set_bq32k_rtc_timestamp(dev, *(rt_uint32_t *)args) != RT_EOK)
        {
            result = -RT_ERROR;
        }
        LOG_D("RTC: set bq32k rtc_time %x\n", *(rt_uint32_t *)args);
        break;
    }

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device bq32k_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    bq32k_control
};
#endif

rt_err_t bq32k_register(const char *name, const char *i2c_bus)
{
    static struct bq32k_device bq32k_drv;
    struct rt_i2c_bus_device *bus;

    bus = rt_i2c_bus_device_find(i2c_bus);
    if (bus == RT_NULL)
    {
        return RT_ENOSYS;
    }

    bq32k_drv.bus = bus;
    bq32k_drv.parent.type      = RT_Device_Class_RTC;
#ifdef RT_USING_DEVICE_OPS
    bq32k_drv.parent.ops       = &bq32k_ops;
#else
    bq32k_drv.parent.init      = RT_NULL;
    bq32k_drv.parent.open      = RT_NULL;
    bq32k_drv.parent.close     = RT_NULL;
    bq32k_drv.parent.read      = RT_NULL;
    bq32k_drv.parent.write     = RT_NULL;
    bq32k_drv.parent.control   = bq32k_control;
#endif

    bq32k_drv.parent.user_data = RT_NULL;

    return rt_device_register(&bq32k_drv.parent, name, RT_DEVICE_FLAG_RDWR);
}
