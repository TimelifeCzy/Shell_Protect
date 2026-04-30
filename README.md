# Shell_Protect

PE 虚拟壳框架，项目用于学习软件保护、PE 加壳、压缩脱壳、IAT 修复和简单虚拟机保护流程。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/dlg.png)

## 功能概览

- 支持一键加壳和一键脱壳。
- 支持 x32/x64，不同平台使用不同压缩库：x32 使用 LZ4，x64 使用 QuickLZ。
- 支持压缩原始区段、清理数据目录和区段文件偏移、运行时解压并修复 IAT。
- x64 包含简单 VM 示例，用于演示代码片段加密、指令解析、分发和上下文维护思路。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/1.png)

## 加壳流程

代码入口主要参考 `MasterWindows::OnBnClickedButton1()`：

1. 拖入目标 PE，`OnDropFiles()` 记录目标路径，并生成同目录的 `FileName_CombatShellData.dat` 路径。
2. `NewSection()` 调用 `AddSection` 添加 `.VMP` 区段，用于放入壳代码。
3. `CompressionData::CompressSectionData()` 压缩原 PE 区段数据，新增 `.UPX` 区段保存压缩数据，并记录原始数据目录、区段大小、区段偏移和 OEP。
4. `studData::LoadLibraryStud()` 加载 `CombatShell.dll`，定位壳入口。
5. `studData::RepairReloCationStud()` 修复壳代码重定位。
6. `studData::CopyStud()` 将壳代码拷贝到 `.VMP`，并把入口点改到壳入口。
7. 成功后生成加壳后的目标文件，同时保留 `old_原文件名` 作为备份。

一键加壳成功后，目标文件同目录会生成：

```text
FileName_CombatShellData.dat
```

该文件用于脱壳恢复，内容包含压缩区段记录、被清理的数据目录、原区段文件信息和原始 OEP。当前实现为了简化流程将这些数据保存到本地文件，后续也可以扩展为写入新增区段。

```text
压缩区段大小记录 | 数据目录记录(16项) | 原区段 SizeOfRawData/PointerToRawData | 原始 OEP
```

## 运行时流程

壳入口在 `CombatShell/CombatShell.cpp` 中：

1. `CombatShellEntry()` 解析 `kernel32/user32` 等必要 API。
2. `CreateWind()` 创建隐藏窗口和辅助线程。
3. `ProcessCallBack()` 触发解压流程；窗口不可用时直接执行解压和跳转。
4. `UnCompression()` 恢复数据目录、区段文件信息，并解压 `.UPX` 中的原始区段数据。
5. `RepairTheIAT()` 重新加载导入模块并修复 IAT。
6. 最后跳转到 `ImageBase + 原始 OEP` 继续执行原程序。

## 脱壳流程

代码入口主要参考 `MasterWindows::OnBnClickedButton2()`：

1. 拖入已加壳程序。
2. `UnShell::UnShellEx()` 读取加壳 PE 和本地 `FileName_CombatShellData.dat`。
3. `UnShell::RepCompressionData()` 根据记录解压 `.UPX` 中的原始区段数据。
4. `UnShell::DeleteSectionInfo()` 删除 `.VMP` 和 `.UPX` 区段信息，恢复数据目录、区段文件偏移、区段大小和 OEP。
5. `UnShell::SaveUnShell()` 重新保存 PE，并替换当前目标文件。

## VM 说明

虚拟机目前主要用于 x64 代码片段保护示例，重点是展示 VM 设计思路，而不是完整商业级虚拟化保护。

- 加壳时记录需要 VM 处理的代码偏移、长度和加密后的指令数据。
- 运行时由 `VmEntry()` 进入 VM，读取加密后的代码片段并解密。
- `VmOpcodeAnalHlper()` 解析指令语义，例如 `mov/xor/add/sub/call/jmp`。
- `VmCodetoExecDispath()` 将解析出的指令分发到对应 handler。
- handler 维护寄存器和栈上下文，模拟原始指令效果。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/4.png)
![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/2.png)
![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/3.png)

项目中的 VM 没有高级算法，只是用于说明 VM 指令解析、分发和上下文维护过程。`CompressionData::VmcodeEntry()` 中仍有硬编码的 VM 指令行数，后续可以通过反汇编自动分析优化。

**注意：当前 VM 示例只覆盖部分指令映射，不是完整指令集。**

## 说明

本项目仅用于学习和研究 PE 结构、软件保护、压缩壳、IAT 修复和虚拟机保护思路，不提供正式 Release 版本。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/5.png)
![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/6.png)

## Stargazers over time

[![Stargazers over time](https://starchart.cc/TimelifeCzy/Shell_Protect.svg)](https://starchart.cc/TimelifeCzy/Shell_Protect)
