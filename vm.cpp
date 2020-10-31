#include "vm.h"
#include "Capstone.h"
//#include "vmtest.h"
#include <vector>

using namespace std;

VM::VM()
{
}

VM::~VM()
{
}

void VM::VmEntry(void* Vmstart, int len)
{
	// 先反汇编，然后针对每条汇编挂钩handler，当eip执行地址则进入虚拟机处理
	Capstone Anasmobj;
	// 当前地址反汇编N行
	Anasmobj.AnalyOpcodeHlper(Vmstart, len);
}