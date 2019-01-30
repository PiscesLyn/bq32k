# bq32k 软件包

## 介绍

`bq32k` 软件包是 RT-Thread 针对 I2C BQ32k RTC 推出的一个软件包。使用这个软件包，可以在 RT-Thread 上非常方便的使用该器件。

本文主要介绍该软件包的使用方式、API，以及 `MSH` 测试命令。

### 目录结构

```
bq32k
│   README.md                       // 软件包说明
│   bq32k.c                         // 源文件
│   bq32k.h                         // 头文件
│   bq32k_sample.c                  // 软件包使用示例代码
│   SConscript                      // RT-Thread 默认的构建脚本
│   LICENSE                         // 许可证文件
```

### 许可证

bq32k 遵循 Apache-2.0 许可，详见 `LICENSE` 文件。

### 依赖

- RT_Thread 3.0+
- i2c 设备驱动

## 获取方式

使用 `bq32k package` 需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
RT-Thread online packages
    peripheral libraries and drivers  --->
        bq32k: An I2C-bus RTC --->
```

进入 pcf8574 软件包的配置菜单按自己的需求进行具体的配置

```
    --- bq32k: An I2C-bus RTC                           
        [*]   Enable bq32k sample
           Version (latest)  --->
```

**Enable bq32k sample** ：开启 bq32k  使用示例

配置完成后让 RT-Thread 的包管理器自动更新，或者使用 pkgs --update 命令更新包到 BSP 中。

## 使用方法

bq32k 软件包的使用流程一般如下：

1. 初始化 bq32k 设备 `bq32k_register`
2. 使用 RT_Thread device接口进行bq32k和soft rtc同步 的操作

详细的使用方法可以参考[bq32k 示例程序](bq32k_sample.c) 。

## MSH 测试命令

如果开启了 bq32k 软件包的示例程序，就会导出 `bq32k_sync` 命令到控制台，并默认会在 `i2c3`总线注册 BQ32K 设备。运行结果如下：

```
msh >bq32k_sync to
msh >bq32k_sync from
```

## 注意事项

暂无。

## 联系方式

- 维护：[PiscesLyn](https://github.com/PiscesLyn)
- 主页：<https://github.com/PiscesLyn/bq32k>
