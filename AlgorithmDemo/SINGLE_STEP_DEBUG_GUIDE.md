# AlgorithmDemo 单步调试说明

## 目标

本文档说明如何用当前项目里的 `AlgorithmDemo` 单步调试算法库源码。

推荐调试方式是：

```text
PySide6 GUI 负责生成 profile.json
CLion 负责 Debug AlgorithmDemo.exe --profile profile.json
AlgorithmDemo 再调用 AlgorithmLibraryd.dll
```

这样既可以保留 GUI 配置的便利性，又可以在 CLion 中单步进入 C++ Demo 和 DLL 源码。

## 前置条件

确认使用的是当前工程源码构建出来的 Debug 版本，而不是 third_lib 中的预编译 DLL。

当前 CMake 顶层工程包含：

```text
AlgorithmLibrary
AlgorithmDemo
```

`AlgorithmDemo` 会链接本地 `AlgorithmLibrary` target，并在构建后把本地 DLL 拷贝到 Demo 输出目录。

关键输出文件一般位于：

```text
E:\test\algorithmlibrary_ALP\BUILD\ReleaseAlgorithmDemo\x64\bin\AlgorithmDemod.exe
E:\test\algorithmlibrary_ALP\BUILD\ReleaseAlgorithmDemo\x64\bin\AlgorithmLibraryd.dll
E:\test\algorithmlibrary_ALP\BUILD\ReleaseAlgorithmDemo\x64\bin\AlgorithmDemod.pdb
E:\test\algorithmlibrary_ALP\BUILD\AlgorithmLibrary2.0.1.0\CppTypeInterface\x64\Debug\bin\AlgorithmLibraryd.pdb
```

如果缺少 `pdb`，一般就无法顺利单步进入源码。

## 第一步：用 GUI 生成 profile

先打开 PySide6 GUI，配置测试参数：

- Raw 文件
- Raw 尺寸
- frame 数
- APS 测试项，例如 `snoise`、`tnoise`、`badpixel`
- ROI / active area
- 多线程开关
- 输出目录

保存后会生成：

```text
E:\test\algorithmlibrary_ALP\AlgorithmDemo\config\aps_gui_profile.json
```

注意：GUI 只负责生成配置。真正单步调试时，不建议从 GUI 点击开始，因为 GUI 会用 `QProcess` 启动 exe，不方便在 CLion 里从程序入口开始断住。

## 第二步：配置 CLion Debug

在 CLion 中选择 Debug 配置。

建议新建一个 Run/Debug Configuration：

```text
Target:
AlgorithmDemo

Executable:
E:\test\algorithmlibrary_ALP\BUILD\ReleaseAlgorithmDemo\x64\bin\AlgorithmDemod.exe

Working directory:
E:\test\algorithmlibrary_ALP

Program arguments:
--profile E:\test\algorithmlibrary_ALP\AlgorithmDemo\config\aps_gui_profile.json
```

然后点击 Debug，而不是普通 Run。

## 第三步：推荐断点

### 程序入口

文件：

```text
AlgorithmDemo\src\main.cpp
```

推荐断点：

```cpp
return RunApsRawTestProfile(argv[i + 1]);
```

这个断点用于确认当前确实走的是 profile 模式，而不是普通 TUI 模式。

### Profile 总入口

文件：

```text
AlgorithmDemo\src\ApsRawTestRunner.cpp
```

推荐断点：

```cpp
int RunApsRawTestProfile(const std::string& profilePath)
```

这里可以看到传入的 JSON 路径。

### JSON 配置解析

推荐断点：

```cpp
config = LoadProfileConfig(profilePath);
```

进入 `LoadProfileConfig` 后重点看：

```text
mode
sensor
raw.path
raw.width
raw.height
raw.frames
raw.raw_type
raw.pixel_format
multi_thread
roi
active_area
tests
output
```

如果出现 `exit=1`，通常是在这里或后面的异常处理中失败。

### 算法执行入口

推荐断点：

```cpp
const bool ok = RunSelectedTests(config, &result);
```

进入 `RunSelectedTests` 后，核心流程是：

```text
检查测试项
检查 raw 文件是否存在
CreateAPSAlgoInterface
SetLogEnable
SetMultiThreadEnable
SetRawDataSize
SetActiveArea
读取 raw
检查 raw 文件大小
ImportRawData
逐个执行测试项
```

如果出现 `exit=2`，通常说明 profile 能解析，但测试执行失败，例如 raw 尺寸不匹配、raw 文件不存在、没有选择测试项、ROI 不合法等。

### 创建算法接口

推荐断点：

```cpp
CreateAPSAlgoInterface(config.sensor, config.rawType, "", config.pixelFormat, 0)
```

这里会进入算法库 DLL 的导出函数。

对应源码通常在：

```text
AlgorithmLibrary\src\AlpMPAlgoInterface.cpp
```

可以继续单步进入具体 sensor 对应的算法实现。

### 导入 raw

推荐断点：

```cpp
aps->ImportRawData(&raw[0], raw.size(), config.frameStart, config.frames, config.headerFooter)
```

这里可以检查：

```text
raw 指针是否有效
raw.size() 是否符合预期
frameStart
frames
headerFooter
```

当前常用 raw 的正确尺寸是：

```text
3264 x 2448 x 5 frames x 2 bytes = 79902720 bytes
```

如果高度误填为 `2440`，会得到：

```text
expected = 3264 x 2440 x 5 x 2 = 79641600 bytes
```

此时会报：

```text
Raw size mismatch
```

### 单个测试项

常用断点：

```cpp
aps->SNoise(config.frameStart, config.frames, roi, result)
aps->TNoise(config.frameStart, config.frames, roi, result)
aps->BadPixel(config.frameStart, config.frames, roi, result)
aps->HotPixel(config.frameStart, config.frames, roi, result)
aps->DataMean(config.frameStart, config.frames, roi, result)
aps->DSNU(config.frameStart, config.frames, roi, result)
```

如果 Step Into 没有进入算法实现，可以直接在算法库源码中搜索对应函数名并打断点。

例如：

```powershell
rg -n "SNoise\(|TNoise\(|BadPixel\(" AlgorithmLibrary\src
```

## 调用链

```text
main
  -> RunApsRawTestProfile
    -> LoadProfileConfig
    -> RunSelectedTests
      -> CreateAPSAlgoInterface
      -> aps->SetLogEnable
      -> aps->SetMultiThreadEnable
      -> aps->SetRawDataSize
      -> aps->SetActiveArea
      -> aps->ImportRawData
      -> aps->SNoise
      -> aps->TNoise
      -> aps->BadPixel
      -> aps->HotPixel
      -> aps->DataMean
      -> aps->DSNU
    -> WriteResultJson
```

## 退出码含义

`RunApsRawTestProfile` 的返回值有三种：

```text
0 = 成功
1 = profile 解析或运行过程发生异常
2 = profile 成功解析，但测试执行失败
```

区别如下：

```text
exit=1
通常是异常，例如 JSON 格式错误、mode 不支持、写结果文件异常等。

exit=2
通常是测试失败，例如 raw 尺寸不匹配、raw 文件不存在、没有选择测试项、ROI 不合法等。
```

失败时仍会尽量写出：

```text
E:\test\algorithmlibrary_ALP\output\aps_gui_run\result.json
```

可以从其中的 `error` 字段查看具体失败原因。

## 常用单步操作

CLion 常用快捷键：

```text
F7        Step Into，进入函数
F8        Step Over，跳过函数
Shift+F8  Step Out，跳出当前函数
F9        Resume，继续运行到下一个断点
```

建议调试顺序：

```text
main
RunApsRawTestProfile
LoadProfileConfig
RunSelectedTests
CreateAPSAlgoInterface
ImportRawData
目标测试项，例如 SNoise 或 BadPixel
```

## 进不去 DLL 源码时的排查

### 1. 确认使用 Debug 配置

必须使用 Debug 构建。Release 或优化后的构建可能导致断点失效、变量不可见、单步跳动异常。

### 2. 确认加载的是本地 DLL

Demo 运行目录中应该存在：

```text
E:\test\algorithmlibrary_ALP\BUILD\ReleaseAlgorithmDemo\x64\bin\AlgorithmLibraryd.dll
```

如果实际加载的是 `third_lib` 里的 DLL，就无法单步进当前源码。

### 3. 确认 PDB 存在

应存在：

```text
E:\test\algorithmlibrary_ALP\BUILD\AlgorithmLibrary2.0.1.0\CppTypeInterface\x64\Debug\bin\AlgorithmLibraryd.pdb
```

没有 PDB 时，调试器通常只能进入反汇编或直接 Step Over。

### 4. 直接在实现函数上打断点

虚函数调用有时 Step Into 不一定稳定。比如：

```cpp
aps->SNoise(...)
```

如果没有进去，可以在具体实现类的 `SNoise` 函数上直接打断点，然后按 `F9` 继续运行。

### 5. 确认测试项真的被选中

如果 profile 中某个测试项是：

```json
{
  "name": "snoise",
  "enabled": false
}
```

那相关算法函数不会执行，对应断点也不会命中。

## GUI 与单步调试的关系

GUI 启动 C++ 的方式是：

```text
QProcess
  -> AlgorithmDemod.exe --profile aps_gui_profile.json
```

所以 GUI 可以看到日志，但它不是最适合单步调试 C++ 的入口。

推荐习惯是：

```text
修改配置时：用 GUI
单步调试时：用 CLion Debug AlgorithmDemo --profile xxx.json
查看最终结果：看 result.json 或 GUI 日志
```

## CLion 调试前端崩溃时

如果 CLion 弹出：

```text
Debugger process finished with exit code -1073741819 (0xC0000005)
```

并且 crash dump 里显示崩溃进程是：

```text
LLDBFrontend.exe
```

这通常说明崩溃发生在 CLion 的 LLDB 调试前端，不是 `AlgorithmDemod.exe` 本身。

## 命令行 LLDB 图形界面

如果 CLion 的调试前端不稳定，但命令行 LLDB 可以正常断住，可以使用项目里的 PySide6 调试 GUI：

```text
E:\test\algorithmlibrary_ALP\tools\algorithm_debug_gui.py
```

启动方式是使用已经安装 PySide6 的 Python 解释器运行它，例如：

```powershell
cd E:\test\algorithmlibrary_ALP
<安装了 PySide6 的 python.exe> tools\algorithm_debug_gui.py
```

这个 GUI 做的事情和命令行 LLDB 一样：

```text
启动 lldb.exe
加载 AlgorithmDemod.exe
设置 main.cpp:19 断点
process launch -- --profile aps_gui_profile.json
停在 main 函数的第一行循环附近
```

界面中有三个核心区域：

```text
调试配置：选择 lldb.exe、AlgorithmDemod.exe、aps_gui_profile.json
断点：选择常用断点，或手工输入 file + line
LLDB 命令：手工命令、函数断点、线程、内存
LLDB 输出：显示 LLDB 原始输出
```

推荐使用顺序：

```text
1. 先用 Algorithm Test GUI 保存最新 aps_gui_profile.json
2. 打开 algorithm_debug_gui.py
3. 确认 lldb.exe、AlgorithmDemod.exe、profile 路径正确
4. 点击“启动”，程序会自动停在 main
5. 添加目标断点，例如 Demo SNoise 调用或 DLL SNoise 实现
6. 点击“继续”
7. 命中断点后使用“单步进入 / 单步跳过 / 跳出函数 / 调用栈 / 变量”
```

按钮只有在 LLDB 输出 `Process stopped` 后才适合使用。启动后如果还在 `process launch` 阶段，或者点击“继续”后程序正在运行，此时不要连续发送 `step` / `next`；等命中断点停住后再单步。

如果目标是进入 profile 流程，推荐在断点预设中选择 `RunApsRawTestProfile`，再点击“运行到选中断点”。

等价的手工命令是：

```text
breakpoint set --file ApsRawTestRunner.cpp --line 1309
continue
```

所以当 LLDB 停在 `main.cpp:26`，但点击“单步进入”直接跑到 `main.cpp:34` 时，不要继续纠结 step into，直接用“运行到选中断点”。这是绕开 Windows LLDB 对 MSVC/PDB 跨函数 step into 不稳定问题的更稳方式。

如果要进入其他函数，也按同样思路操作：

```text
1. 在断点预设中选择目标，例如 RunSelectedTests / ImportRawData / Demo SNoise 调用
2. 点击“运行到选中断点”
3. LLDB 命中断点后再查看变量或继续运行
```

这等价于手工执行：

```text
breakpoint set --file <file> --line <line>
continue
```

GUI 按钮和 LLDB 命令对应关系：

```text
继续        -> continue
单步跳过    -> next
单步进入    -> step
跳出函数    -> finish
调用栈      -> bt
变量        -> frame variable
运行到选中断点 -> breakpoint set <file>:<line> + continue
列出断点    -> breakpoint list
停止        -> process kill + quit
```

### 调试 GUI：函数断点

如果不想记文件行号，可以切到 `函数` 页签：

```text
1. 在 Preset 中选择函数，或者在 Name 中输入函数名
2. 点击“添加函数断点”
3. 或者点击“运行到函数”
```

对应 LLDB 命令：

```text
breakpoint set --name RunSelectedTests
breakpoint set --name CAlpAPSMPAlgorithm::SNoise
continue
```

函数名断点适合快速定位，但如果遇到重载、内联、符号名不匹配，还是优先使用文件行号断点。

### 调试 GUI：多线程

切到 `线程` 页签后可以直接执行：

```text
线程列表      -> thread list
选择线程      -> thread select <N>
当前线程栈    -> thread backtrace
所有线程栈    -> thread backtrace all
```

多线程调试建议：

```text
先关多线程看清主流程
再开多线程看线程分发和共享数据
命中断点后先看 thread list，再选择目标线程
```

### 调试 GUI：内存

切到 `内存` 页签后可以输入地址或表达式：

```text
0x0000012345678000
&config
raw.data()
aps
```

常用按钮：

```text
读取内存    -> memory read --format <fmt> --size <size> --count <count> -- <Expr>
读变量内存  -> 对“变量名”输入框中的变量执行 memory read -- &<变量名>
读指针内容  -> 对“变量名”输入框中的指针执行 memory read -- <变量名>
```

例如：

```text
变量名输入 config，点击“读变量内存”
等价于 memory read --format x --size 1 --count 64 -- &config

变量名输入 aps，点击“读指针内容”
等价于 memory read --format x --size 1 --count 64 -- aps
```

注意：这个 GUI 管理的是命令行 LLDB 断点。CLion 编辑器里点出来的红色断点不会自动同步到这个 GUI。

当前项目提供了一个绕开 CLion UI 调试前端的脚本。这个脚本只负责启动命令行 LLDB，并让程序先停在 `main`：

```text
tools\debug_algorithm_demo_lldb.cmd
```

推荐从项目根目录启动：

```powershell
cd /d E:\test\algorithmlibrary_ALP
.\tools\debug_algorithm_demo_lldb.cmd
```

如果当前已经在 `tools` 目录下，则执行：

```powershell
.\debug_algorithm_demo_lldb.cmd
```

启动后会自动完成：

```text
加载 AlgorithmDemod.exe
设置 main.cpp:19 断点
使用 aps_gui_profile.json 启动程序
停在 main 函数附近
```

停住后，终端里会看到类似：

```text
Process stopped
stop reason = breakpoint
-> 18 {
```

这时才可以使用 `n`、`s`、`c` 等单步命令。启动前只创建了 target，还没有 process，此时输入 `n` 或 `c` 会报：

```text
error: invalid process
```

### 脚本查找 LLDB 的规则

脚本会自动查找 CLion 自带的 LLDB，默认扫描：

```text
D:\Program Files\JetBrains\CLion*
C:\Program Files\JetBrains\CLion*
%ProgramFiles%\JetBrains\CLion*
```

也可以通过环境变量指定 LLDB：

```powershell
set LLDB_EXE=D:\Program Files\JetBrains\CLion 2026.1\bin\lldb\win\x64\bin\lldb.exe
.\tools\debug_algorithm_demo_lldb.cmd
```

或者：

```powershell
set CLION_HOME=D:\Program Files\JetBrains\CLion 2026.1
.\tools\debug_algorithm_demo_lldb.cmd
```

脚本支持三个可选参数：

```text
tools\debug_algorithm_demo_lldb.cmd [profile_json] [algorithm_demo_exe] [lldb_exe]
```

例如：

```powershell
tools\debug_algorithm_demo_lldb.cmd ^
  E:\test\algorithmlibrary_ALP\AlgorithmDemo\config\aps_gui_profile.json ^
  E:\test\algorithmlibrary_ALP\BUILD\ReleaseAlgorithmDemo\x64\bin\AlgorithmDemod.exe ^
  "D:\Program Files\JetBrains\CLion 2026.1\bin\lldb\win\x64\bin\lldb.exe"
```

默认 exe 是：

```text
E:\test\algorithmlibrary_ALP\BUILD\ReleaseAlgorithmDemo\x64\bin\AlgorithmDemod.exe
```

默认 profile 是：

```text
E:\test\algorithmlibrary_ALP\AlgorithmDemo\config\aps_gui_profile.json
```

### 与 CLion 断点的关系

使用该脚本时，CLion 编辑器里点出来的红色断点不会同步到命令行 LLDB。

需要新增断点时，必须在 LLDB 命令行里输入断点命令，然后输入 `c` 继续运行。

## 命令行 LLDB 如何打断点

### 1. 启动 LLDB

PowerShell 中从项目根目录启动：

```powershell
cd E:\test\algorithmlibrary_ALP
.\tools\debug_algorithm_demo_lldb.cmd
```

如果当前已经在 `tools` 目录下，则执行：

```powershell
.\debug_algorithm_demo_lldb.cmd
```

脚本启动后会自动停在 `main.cpp` 附近。下一步通常是添加你真正关心的算法断点，然后输入 `c`。

例如继续跑到 `SNoise`：

```text
b ApsRawTestRunner.cpp:1035
c
```

### 2. 查看当前断点

```text
breakpoint list
```

简写：

```text
br list
```

### 3. 按文件和行号打断点

完整写法：

```text
breakpoint set --file ApsRawTestRunner.cpp --line 1035
```

简写：

```text
b ApsRawTestRunner.cpp:1035
```

如果断点设置成功，会看到类似：

```text
Breakpoint 5: where = AlgorithmDemod.exe`...
```

如果看到：

```text
no locations (pending)
```

说明该断点暂时没有解析到具体代码位置。常见原因是文件名或行号不对，或者当前 exe/PDB 不是最新构建结果。

### 4. 常用断点位置

Profile 入口：

```text
b main.cpp:19
b main.cpp:26
b ApsRawTestRunner.cpp:1309
```

配置解析：

```text
b ApsRawTestRunner.cpp:594
```

算法执行总入口：

```text
b ApsRawTestRunner.cpp:951
```

创建 APS 算法接口：

```text
b ApsRawTestRunner.cpp:971
```

导入 raw：

```text
b ApsRawTestRunner.cpp:1019
```

APS 测试项调用：

```text
b ApsRawTestRunner.cpp:1035   # SNoise
b ApsRawTestRunner.cpp:1052   # TNoise
b ApsRawTestRunner.cpp:1063   # BadPixel
b ApsRawTestRunner.cpp:1074   # HotPixel
b ApsRawTestRunner.cpp:1085   # DataMean
b ApsRawTestRunner.cpp:1096   # DSNU
```

### 5. 继续运行到断点

```text
c
```

完整写法：

```text
continue
```

程序会一直运行，直到：

```text
命中下一个断点
程序正常结束
程序异常崩溃
```

### 6. 单步调试

```text
n        step over，单步跳过当前函数
s        step into，单步进入当前函数
finish   跳出当前函数
bt       查看调用栈
frame variable
         查看当前栈帧局部变量
q        退出 LLDB
```

例如停在：

```cpp
aps->SNoise(config.frameStart, config.frames, roi, result)
```

如果想进入 `SNoise` 内部，输入：

```text
s
```

如果 `s` 没有进入算法库源码，可以直接在算法库具体实现函数上打断点，再输入 `c`。

### 6.1 查看变量值和地址

查看当前栈帧所有局部变量：

```text
frame variable
```

查看某个变量的值：

```text
frame variable config
frame variable result
frame variable ok
```

查看某个变量的地址：

```text
expression -- &config
expression -- &result
expression -- &ok
```

简写也可以：

```text
p config
p &config
```

如果变量是指针，查看指针本身：

```text
frame variable aps
```

查看指针指向的对象：

```text
p *aps
```

调试 GUI 的“查看值”按钮等价于：

```text
frame variable <变量名>
```

“查看地址”按钮等价于：

```text
expression -- &<变量名>
```

### 7. 删除断点

查看断点编号：

```text
breakpoint list
```

删除某个断点：

```text
breakpoint delete 5
```

删除全部断点：

```text
breakpoint delete
```

### 8. 一个完整例子

目标：停到 `SNoise` 调用处，然后单步进入。

```text
b ApsRawTestRunner.cpp:1035
c
s
bt
```

目标：跳过 raw 导入，直接跑到 `BadPixel`。

```text
b ApsRawTestRunner.cpp:1063
c
```

常用 LLDB 命令：

```text
n    step over
s    step into
c    continue
bt   backtrace
q    quit
```

如果命令行 LLDB 可以正常断住，而 CLion UI 调试仍然崩溃，应优先排查安全软件、文档管控、邮件监控、行为拦截等模块是否注入了 `LLDBFrontend.exe`。
