# Shell_Protect

PE虚拟壳框架，项目意图给学习软件保护加壳的初学者提供一些思路和引导。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/dlg.png)

支持一键加壳/脱壳，全压缩/加密，IAT加密等。

1. 压缩库支持lz4/quicklz，以保证x32/x64压缩稳定。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/1.png)

2. 重点说一下虚拟机：虚拟壳目前仅支持x64加代码片段(需要稍作修改)，这只是一个示例和思路，如何自己编写虚拟机，运行态保存上下文环境及维护堆栈。

- 对需要加密代码段进行Vmcode，简单点说加密代码，dll中壳代码执行vmentry进入虚拟机，默认x32直接进入壳main函数未Vm。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/4.png)

- 虚拟机读取加密的代码段进行解密，变成正常的Opcode或者直接识别Vmcode都可以，解析器负责对Vmcode进行语义分析。 如mov eax, 23h  Vmcode: xx xx xx xx 进入Vmentry读取Vmcode：xx xx xx xx，Vm虚拟机转换成(分析后)知道他是mov eax,23。
- 解析完成之后指令分发处理，如mov类的指令分发至mov相关处理函数，mov又可以细化为不同的操作数，如mov eax/rax/rbx/imm/xxx等，处理函数再进行进一步字节码筛选匹配。
- mov rax/rbx/rcx/r8/mm/imm等等，分别挂钩不同的handle(混淆执行器)来执行等同于mov eax,23h代码。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/2.png)

- 退出和进入Vmhandle时候会出现寄存器数据丢失，栈数据丢失，上下代码执行就会导致当前运行环境不完整。因为是在虚拟机中操作，**开辟对应的代码段的栈空间及寄存器环境，这里有个概念，当前代码段或者函数的栈及全局寄存器变量。如下，每次传入属于当前代码段的寄存器环境：**。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/3.png)

项目中的虚拟机没有高级算法，只是简单加密用来阐述过程，Vmcode分析引擎因为能力/精力有限，没有去构造Vmcode代码分析引擎，只是构造了解密后代码分析引擎挂钩handle处理。再加壳时候记录了加密汇编大小/长度/基于该代码段的起始偏移，用来做为Vmcode分析引擎使用，快速解密和处理分发。

**注意：本项目仅支持加壳器中的Main函数，只对Main汇编映射指令进行了处理，未处理其它指令。**

造轮子的意义在于学习理解：理解虚拟机和指令集映射，理解虚拟机结构和协同工作。

推荐专业的虚拟机分析引擎(看雪版主玩命)： https://github.com/devilogic/cerberus.git ，加密解密书籍有配套虚拟壳代码(没有看过)，也可以参考。

软件保护技术还可应用于免杀，当然可以扩展更深层次了解杀软检测及反检测。

![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/5.png)
![image](https://github.com/TimelifeCzy/Shell_Protect/blob/main/readmepng/6.png)

项目仅用于学习，请大家重视。

## Stargazers over time

[![Stargazers over time](https://starchart.cc/TimelifeCzy/Shell_Protect.svg)](https://starchart.cc/TimelifeCzy/Shell_Protect)

