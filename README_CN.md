# eLPA

[English](README.md)|简体中文

9eSIM小工具是一个基于ESP32S3实现的外部（扩展）的LPA。

9eSIM小工具即9eSIM BLE蓝牙读卡器。它是基于[estkme-group/lpac: C-based eUICC LPA](https://github.com/estkme-group/lpac) (后面称之为`lpac`)移植开发板载LPA工具。

**注意事项：**

关于许可证：
`lpac`是多许可证项目，若您需要使用`lpac`相关的代码，请严格遵守相关许可声明。本项目因开发需要，对将部分`lpac`的头文件放在了`include`目录下。
此部分文件名如下：

- `applet.h`
- `driver.h`
- `jprint.h`
- `main.h`

以上文件需要遵守`lpac`的许可证。

`src/app`、`src/driver`、`src/euicc`目录下的代码为`lpac`的移植代码，需要遵守`lpac`的许可声明。

`src/cjson`目录下的代码为`cJSON`的移植代码，需要遵守`cJSON`的许可声明。

其余代码遵守MIT许可。
