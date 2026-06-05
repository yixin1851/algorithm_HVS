# AlgorithmDemo APS Raw Test Demo

## 目标

这个文档记录当前 `AlgorithmDemo` 的无 Qt 控制台测试方案。

改造后的 Demo 用于测试当前 APS 算法库，支持：

- 使用当前项目下可用的 raw 文件。
- 像 GUI 一样选择测试项后再运行。
- 记住上一次选择，下次启动自动恢复。
- 不依赖 Qt。

## 当前入口

Demo 入口已经改为：

- `AlgorithmDemo/src/main.cpp`
- `AlgorithmDemo/src/ApsRawTestRunner.cpp`
- `AlgorithmDemo/include/ApsRawTestRunner.h`

`main.cpp` 只负责调用：

```cpp
return RunApsRawTestDemo();
```

核心逻辑都在 `ApsRawTestRunner.cpp`。

## 默认 Raw 文件

默认使用裁剪后的 003CA 可用 raw：

```text
E:/test/algorithmlibrary_ALP/test_rawdata/testdata_center_crop_3264x2448_5frames_u16_from_3280x2464.raw
```

该文件由 `image_algo` 的原始测试 raw 裁剪而来：

```text
裁剪前:
E:/test/algorithmlibrary_ALP/test_rawdata/testdata_src_3280x2464_5frames_u16.raw
size = 80819200 bytes

裁剪后:
E:/test/algorithmlibrary_ALP/test_rawdata/testdata_center_crop_3264x2448_5frames_u16_from_3280x2464.raw
size = 79902720 bytes
```

裁剪方式：

- 源尺寸：`3280 x 2464 x 5 frames`
- 目标尺寸：`3264 x 2448 x 5 frames`
- 左右各裁掉 8 列。
- 上下各裁掉 8 行。
- 每像素按 `uint16_t` 存储。

当前 `ALP_003CA` APS 算法固定使用 `3264 x 2448`，因此应使用裁剪后的 raw。

## 默认配置

配置文件路径：

```text
AlgorithmDemo/config/aps_demo_last_config.ini
```

当前默认内容类似：

```ini
[raw]
path=E:/test/algorithmlibrary_ALP/test_rawdata/testdata_center_crop_3264x2448_5frames_u16_from_3280x2464.raw
frames=5

[aps]
sensor=003CA
raw_type=unpack10
pixel_format=QuadBayerGBRG
multi_thread=false
log=false

[items]
selected=snoise,tnoise,badpixel

[roi]
enabled=false
up=0
down=1223
left=0
right=1631

[threshold]
badpixel_threshold=0.19
badpixel_radius=1
hotpixel_threshold=120
```

Demo 启动时会读取该 INI。退出、运行测试、修改配置后都会保存当前选择。

## 菜单功能

启动 Demo 后会出现控制台菜单：

```text
1  Set raw path
2  Set frames
3  Toggle thread mode
4  Select test items
5  Configure ROI
6  Configure thresholds
7  Run selected tests
8  Save config
9  Restore default config
0  Exit
```

常用操作：

- 输入 `4`：进入测试项选择。
- 输入 `7`：运行当前已选择测试项。
- 输入 `0`：退出并保存配置。

测试项选择界面支持：

```text
number/list toggles
a = all
n = none
b = back
```

例如：

```text
1,3
```

表示切换第 1 项和第 3 项的选中状态。

## 当前支持的测试项

| 菜单项 | 配置 ID | 调用接口 | 结果结构 |
|---|---|---|---|
| SNoise | `snoise` | `SNoise(...)` | `APSSNoiseType` |
| TNoise | `tnoise` | `TNoise(...)` | `APSTNoiseType` |
| BadPixel | `badpixel` | `BadPixel(...)` | `APSBadpixelType` |
| HotPixel | `hotpixel` | `HotPixel(...)` | `APSBadpixelType` |
| DataMean | `datamean` | `DataMean(...)` | `APSDataMeanType` |
| DSNU | `dsnu` | `DSNU(...)` | `APSDSNUType` |

暂未放入默认菜单的项：

- `BLC`
- `DPC`

原因：这两类操作会修改导入后的图像数据或依赖前序坏点结果，和普通测量项混跑时容易影响后续测试结果。后续建议作为高级测试项单独加入。

## 运行流程

内部流程如下：

```text
读取 INI 配置
    |
显示当前 raw / sensor / frames / items
    |
用户修改选择
    |
创建 APS 接口
    |
读取 raw 文件
    |
校验 raw 文件大小
    |
ImportRawData
    |
按顺序运行所选测试项
    |
打印摘要结果和耗时
    |
保存 INI
```

默认创建接口：

```cpp
CreateAPSAlgoInterface(ALP_003CA, UNPACK10, "", QuadBayerGBRG, 0);
```

默认参数：

- Sensor: `ALP_003CA`
- RawType: `UNPACK10`
- PixelFormat: `QuadBayerGBRG`
- Frames: `5`
- ROI: disabled
- Thread: single

## 构建方式

已验证 Debug 构建：

```bat
cmd /c "call ""D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && ""D:\Program Files\JetBrains\CLion 2026.1\bin\cmake\win\x64\bin\cmake.exe"" --build cmake-build-debug --target AlgorithmDemo -j 14"
```

输出可执行文件：

```text
E:/test/algorithmlibrary_ALP/BUILD/ReleaseAlgorithmDemo/x64/bin/AlgorithmDemod.exe
```

注意：当前已验证的是 Debug Demo。若要构建 Release Demo，需要确保 `AlgorithmDemo/third_lib/.../Release/bin` 下存在对应的 `AlgorithmLibrary.dll/lib/exp`。

## 验证结果

使用默认配置运行：

```text
items=snoise,tnoise,badpixel
raw=testdata_center_crop_3264x2448_5frames_u16_from_3280x2464.raw
frames=5
thread=single
```

已验证：

- `ImportRawData` 成功。
- `SNoise` 成功。
- `TNoise` 成功。
- `BadPixel` 成功。
- 退出后配置成功保存。

一次 Debug 运行参考耗时：

```text
ImportRawData: about 700 ms
SNoise: about 950-1000 ms
TNoise: about 2300-2500 ms
BadPixel: about 2800-3100 ms
```

Debug 耗时只作为功能验证参考，不建议用于性能结论。

## 后续建议

1. 增加 CSV 导出。
2. 增加 `BLC / DPC` 高级测试项，并明确它们会修改数据。
3. 增加 sensor/rawType/pixelFormat 的菜单选择。
4. 增加 Release Demo 的第三方库目录，方便做真实性能测试。
5. 将结果摘要和详细 bad pixel mask 分开输出。
