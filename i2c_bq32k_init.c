/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-23    PiscesLyn   add bq32k port file
 */

#include <rtdevice.h>
#include "bq32k.h"

#if defined(BSP_USING_RTC_BQ32K)
int rt_hw_bq32k_init(void)
{
    rt_err_t result;
    result = bq32k_register("bq32k", "i2c3");
    if (result != RT_EOK)
    {
        rt_kprintf("bq32k register err code: %d\n", result);
        return result;
    }
    rt_kprintf("bq32k init success\n");
    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_bq32k_init);
#endif

