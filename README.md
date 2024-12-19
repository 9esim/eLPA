# eLPA

English|[简体中文](README_CN.md)

The 9eSIM mini tool is a external (or we called extension) LPA on a board, which based on ESP32S3.

9eSIM mini tool also named 9eSIM BLE smartcard reader。It based on [estkme-group/lpac: C-based eUICC LPA](https://github.com/estkme-group/lpac) (we call it lpac below) and ported to the ESP32S3 board.

**Note:**

About the license:
`lpac` is a multi-license project. If you need to use the code related to `lpac`, please strictly follow the relevant license declaration. Due to development needs, some `lpac` header files are placed in the `include` directory.

The file names are as follows:

- `applet.h`
- `driver.h`
- `jprint.h`
- `main.h`

The above files need to comply with the `lpac` license.

The code under the `src/app`, `src/driver`, and `src/euicc` directories is the ported code of `lpac`, which needs to comply with the `lpac` license.

The code under the `src/cjson` directory is the ported code of `cJSON`, which needs to comply with the `cJSON` license. All other code is licensed under the MIT license.
