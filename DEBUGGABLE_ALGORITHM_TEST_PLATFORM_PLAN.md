# 可调试算法测试平台方案

## 1. 背景

当前项目里有两个相关入口：

- `AlgorithmLibrary`：算法库源码与 DLL 目标。
- `AlgorithmDemo`：轻量 C++ Demo，可作为调试启动入口。
- `AlgorithmTest`：Qt GUI 离线测试工具，界面直观，但依赖 Qt，且不适合作为当前源码联调入口。

用户希望测试 APS、EVS/DVS 等算法项，例如 `snoise`、`tnoise`、`badpixel`、`hotpixel` 等，同时希望：

- 不安装完整 Qt SDK。
- 有比 TUI 更直观的 GUI 选择方式。
- 保留从 Demo 单步进入 `AlgorithmLibraryd.dll` 源码的能力。
- 能记住上次选择。
- 能逐步替代 `AlgorithmTest` 的测试流程。

因此，本方案采用：

```text
PySide6 GUI + AlgorithmDemo C++ 执行器 + JSON 配置协议
```

## 2. 总体目标

构建一个无 Qt SDK 依赖、界面直观、可调试 C++ 算法源码的测试平台。

最终形态：

```text
PySide6 GUI
  负责选择 raw、sensor、APS/EVS、测试项、ROI、阈值、帧范围
        |
        v
profile.json / last_session.json
        |
        v
AlgorithmDemod.exe
  读取 JSON，调用 AlgorithmLibraryd.dll
        |
        v
result.json / GUI 导出的 CSV / run.log
        |
        v
PySide6 GUI 展示运行结果
```

## 3. 角色划分

### 3.1 PySide6 GUI

PySide6 只负责界面、配置和结果展示，不直接调用算法 DLL。

主要职责：

- 选择 APS 或 EVS/DVS。
- 选择 sensor type。
- 选择 raw type、pixel format。
- 选择 raw 文件或 raw 文件组。
- 配置 raw 宽、高、帧数、bit depth。
- 勾选测试项。
- 配置每个测试项的 ROI、阈值、帧范围等参数。
- 保存 `profile.json`。
- 维护 `last_session.json`，记住上次选择。
- 启动 `AlgorithmDemod.exe`。
- 读取 `result.json` 并展示，后续可由 GUI 导出 CSV。

PySide6 不负责：

- 不解析大体积 raw 并逐像素计算。
- 不直接管理 C++ 算法对象生命周期。
- 不通过 `ctypes` 直接调用 C++ 类接口。
- 不承担核心算法计算。

### 3.2 AlgorithmDemo

`AlgorithmDemo` 从普通 Demo 升级为可调试算法执行器。

主要职责：

- 支持 TUI 手动操作。
- 支持 `--profile xxx.json` 自动执行。
- 读取 `profile.json`。
- 初始化 APS 或 EVS/DVS 算法接口。
- 导入 raw 数据。
- 按 JSON 中选择的测试项依次执行。
- 输出 `result.json` 和 `run.log`，后续可扩展 CSV 导出。
- 保证 Debug 时可以单步进入 `AlgorithmLibraryd.dll` 源码。

推荐运行方式：

```text
AlgorithmDemod.exe --profile E:/test/algorithmlibrary_ALP/profiles/aps_snoise.json
```

### 3.3 AlgorithmLibrary

`AlgorithmLibrary` 保持算法库职责。

主要职责：

- 提供 APS 算法接口。
- 提供 EVS/DVS 算法接口。
- 输出 Debug DLL 和 PDB。
- 供 `AlgorithmDemo` 调用和单步调试。

## 4. 为什么不用 PySide6 直接调用 DLL

不建议 PySide6 直接调用算法 DLL，原因如下：

- 当前算法库是 C++ 类接口，不是稳定的纯 C ABI。
- 接口涉及虚函数、`std::string`、对象指针和生命周期管理。
- Python 直接调用 C++ DLL 容易遇到 ABI 和内存释放问题。
- 单步调试链路会变复杂。

更稳妥的方式是：

```text
Python 负责 GUI
C++ Demo 负责算法对象和 DLL 调用
JSON 负责进程间配置交换
```

## 5. JSON 配置方案

### 5.1 为什么选择 JSON

INI 适合简单扁平配置，例如上次 raw 路径、线程数、是否启用某个开关。

但当前需求包含：

- APS / EVS/DVS 多模式。
- 多 sensor。
- 多 raw 或 raw 分组。
- 多测试项。
- 每个测试项有独立参数。
- ROI、阈值、帧范围等嵌套结构。
- GUI 与 C++ 执行器之间交换配置。

因此 JSON 更合适。

### 5.2 推荐文件

```text
profile.json
  一次测试任务的完整配置。

last_session.json
  GUI 上次选择，用于恢复界面状态。

result.json
  结构化测试结果，便于 GUI 展示。

summary.csv
  可选表格结果，建议由 GUI 从 result.json 导出，便于 Excel 或其他工具查看。

run.log
  执行日志和错误信息。
```

### 5.3 profile.json 示例

```json
{
  "version": 1,
  "mode": "APS",
  "sensor": "ALP_003CA",
  "thread_count": 14,
  "log_enabled": true,
  "raw": {
    "path": "E:/test/algorithmlibrary_ALP/test_rawdata/testdata_center_crop_3264x2448_5frames_u16_from_3280x2464.raw",
    "width": 3264,
    "height": 2448,
    "frames": 5,
    "raw_type": "UNPACK10",
    "pixel_format": "QuadBayerGBRG",
    "header_footer": false
  },
  "active_area": {
    "row_start": 0,
    "row_end": 1223,
    "col_start": 0,
    "col_end": 1631
  },
  "tests": [
    {
      "name": "snoise",
      "enabled": true,
      "roi": {
        "x": 0,
        "y": 0,
        "width": 3264,
        "height": 2448
      }
    },
    {
      "name": "tnoise",
      "enabled": true,
      "frame_start": 0,
      "frame_count": 5
    },
    {
      "name": "badpixel",
      "enabled": true,
      "thresholds": {
        "pixel": 20,
        "line": 20
      }
    }
  ],
  "output": {
    "directory": "E:/test/algorithmlibrary_ALP/output/aps_debug_run",
    "result_json": "result.json",
    "log": "run.log"
  }
}
```

## 6. 测试项范围

### 6.1 第一阶段 APS

优先支持高频、单 raw 或少量帧即可执行的测试项：

- `snoise`
- `tnoise`
- `datamean`
- `badpixel`
- `hotpixel`
- `dsnu`

### 6.2 第一阶段 EVS/DVS

优先支持常用事件测试项：

- `count_events`
- `stationary_noise`
- `stationary_uniformity`
- `hotpixel`
- `find_peak`
- `badpixel`

### 6.3 第二阶段 APS

补齐更多 AlgorithmTest 中已有的普通测试项：

- `blc`
- `shading`
- `pedestal`
- `read_noise`
- `saturation`
- `show/export`

### 6.4 第二阶段 EVS/DVS

补齐更多事件算法项：

- `image_contrast_sensitivity`
- `accompanied_peak_and_delayed_peak`
- `spatial_response_uniformity`
- `show/export`

### 6.5 第三阶段复杂实验流

复杂项通常需要多组 raw、曝光时间表或多阶段计算：

- `dark_current`
- `linearity`
- `oetc`
- `overall_system_gain`
- `ptc`

这些测试项需要单独设计数据集配置，例如：

```text
dataset.csv
  group,raw_path,width,height,frames,exposure_ms,gain,tag
```

## 7. 单步调试方案

单步调试时，推荐不从 PySide6 启动，而是：

1. 先用 PySide6 生成或修改 `profile.json`。
2. 在 CLion 或 Visual Studio 中启动 `AlgorithmDemod.exe --profile xxx.json`。
3. 在 `AlgorithmDemo` 调用处下断点。
4. 在 `AlgorithmLibrary/src/*.cpp` 中下断点。
5. 使用 Step Into 进入 DLL 源码。

关键调试链路：

```text
AlgorithmDemod.exe
  -> 本工程 Debug 版 AlgorithmLibraryd.dll
  -> 对应 AlgorithmLibraryd.pdb
  -> AlgorithmLibrary/src/*.cpp
```

必须确认：

- 运行时加载的是 `AlgorithmLibraryd.dll`。
- 符号文件 `AlgorithmLibraryd.pdb` 已加载。
- DLL 路径来自本工程 Debug 构建产物，而不是 third_lib 里的预编译 DLL。

Visual Studio 中可通过：

```text
Debug -> Windows -> Modules
```

查看 `AlgorithmLibraryd.dll` 的实际加载路径和符号状态。

## 8. 与 AlgorithmTest 的关系

`AlgorithmTest` 保留为 Qt 离线参考工具。

新平台不直接复刻 Qt UI，而是迁移其中的算法调用流程：

- APS 初始化流程。
- APS raw 导入流程。
- APS 各测试项调用流程。
- EVS/DVS 初始化流程。
- EVS/DVS raw 导入流程。
- EVS/DVS 各测试项调用流程。

最终目标是让常用测试不再依赖 `AlgorithmTest`。

## 9. 与当前 AlgorithmDemo 的关系

当前 `AlgorithmDemo` 已经具备作为 C++ 调试入口的基础。

后续不建议把所有逻辑继续堆在一个文件中，而应逐步拆分：

```text
AlgorithmDemo/
  include/
    AppConfig.h
    JsonProfile.h
    TestSession.h
    TestRegistry.h
    ApsTestItems.h
    DvsTestItems.h
    ResultWriter.h
  src/
    main.cpp
    ConsoleMenu.cpp
    JsonProfile.cpp
    TestSession.cpp
    TestRegistry.cpp
    ApsTestItems.cpp
    DvsTestItems.cpp
    ResultWriter.cpp
```

推荐保留两种运行模式：

```text
AlgorithmDemod.exe
  进入 TUI 菜单。

AlgorithmDemod.exe --profile xxx.json
  根据 JSON 自动执行。
```

## 10. 构建与 DLL 路径要求

后续实现时，需要特别注意 `AlgorithmDemo` 的链接和拷贝逻辑。

Debug 模式必须优先使用当前工程生成的：

```text
BUILD/AlgorithmLibrary2.0.1.0/CppTypeInterface/x64/Debug/bin/AlgorithmLibraryd.dll
BUILD/AlgorithmLibrary2.0.1.0/CppTypeInterface/x64/Debug/bin/AlgorithmLibraryd.pdb
```

应避免 Debug 调试时误加载：

```text
AlgorithmDemo/third_lib/.../AlgorithmLibraryd.dll
```

否则可能出现断点无法进入源码、符号不匹配或调试到旧 DLL 的问题。

## 11. PySide6 性能判断

PySide6 性能足够满足本平台需求。

原因：

- PySide6 底层仍是 Qt C++。
- Python 只负责 GUI 交互、配置生成、进程启动和结果展示。
- 大体积 raw 导入和算法计算仍由 C++ `AlgorithmDemo` 和 `AlgorithmLibraryd.dll` 完成。

需要避免：

- 不在 Python 中逐像素处理大 raw。
- 不把核心算法迁移到 Python。
- 不在 Python 中直接调用 C++ 类接口。

## 12. 开发阶段规划

### 阶段 1：AlgorithmDemo Profile 执行器

目标：

- 支持 `--profile xxx.json`。
- 读取 APS profile。
- 执行 `snoise`、`tnoise`、`datamean`、`badpixel`、`hotpixel`、`dsnu`。
- 输出 `result.json`。
- 保留 TUI。
- 验证单步进入 DLL 源码。

### 阶段 2：PySide6 GUI 原型

目标：

- 选择 raw。
- 选择 sensor / mode。
- 勾选测试项。
- 配置基础参数。
- 保存 `last_session.json`。
- 生成 `profile.json`。
- 启动 `AlgorithmDemod.exe`。
- 显示运行状态和结果。

### 阶段 3：EVS/DVS 测试项

目标：

- 支持 EVS/DVS 初始化。
- 支持 EVS/DVS raw 导入。
- 支持 `count_events`、`stationary_noise`、`stationary_uniformity`、`hotpixel`、`find_peak`、`badpixel`。

### 阶段 4：复杂测试项与批量数据

目标：

- 支持多 raw 分组。
- 支持曝光时间表。
- 支持 `dark_current`、`linearity`、`oetc`、`overall_system_gain`、`ptc`。
- 支持 `dataset.csv` 或 JSON 数据集描述。

### 阶段 5：结果对齐与替代 AlgorithmTest

目标：

- 与 `AlgorithmTest` 的相同测试项结果做对比。
- 固化输出格式。
- 完善错误提示。
- 补充使用文档。
- 常用测试流程完全不依赖 Qt 工具。

## 13. 验收标准

基础验收：

- 不安装 Qt SDK 也能使用 GUI。
- PySide6 GUI 可生成 profile。
- `AlgorithmDemo` 可按 profile 自动执行。
- APS 高频测试项可运行。
- 结果可保存为 JSON，CSV 由 GUI 导出作为后续增强。
- GUI 能记住上次选择。

调试验收：

- CLion 或 Visual Studio 可启动 `AlgorithmDemod.exe --profile xxx.json`。
- 可在 `AlgorithmDemo` 中命中断点。
- 可单步进入 `AlgorithmLibrary/src/*.cpp`。
- `AlgorithmLibraryd.pdb` 符号正确加载。
- Debug 运行时不误加载 third_lib 的旧 DLL。

替代验收：

- APS 高频项结果与 `AlgorithmTest` 对齐。
- EVS/DVS 高频项结果与 `AlgorithmTest` 对齐。
- 常用 raw 测试流程可不依赖 Qt 工具完成。

## 14. 推荐结论

推荐方案为：

```text
PySide6 GUI 负责直观操作
AlgorithmDemo 负责可调试 C++ 执行
AlgorithmLibraryd.dll 负责算法计算
JSON 负责配置和结果交换
AlgorithmTest 保留为对照工具
```

这套方案兼顾：

- 不安装 Qt SDK。
- GUI 操作直观。
- 配置可记忆。
- 测试项可扩展。
- APS 与 EVS/DVS 都可覆盖。
- 仍然可以单步调试到 DLL 源码。
