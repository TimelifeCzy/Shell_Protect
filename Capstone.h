#pragma once
#pragma once
#ifndef _CAPSTONE_H_
#define _CAPSTONE_H_

#include "capstone/include/capstone/capstone.h"

#ifdef _WIN64 // 64位平台编译器会自动定义这个宏
#pragma comment(lib,"capstone\\64\\capstone.lib")
#else
#pragma comment(lib,"capstone\\32\\capstone.lib")
#endif

class Capstone
{
public:
	Capstone();
	virtual ~Capstone();

public:
	// 初始化反汇编引擎
	void InitCapstone();
	// 显示反汇编信息
	void ShowAssembly(const void* pAddr, int nLen);

	void AnalyOpcodeHlper(const void* pAddr, int nLen);

	int AnalyencodeVmHlper(cs_insn* ins, unsigned int rankey);

private:
	// 反汇编引擎句柄
	csh Handle;
	// 错误信息
	cs_err err;
	// 保存反汇编得到的指令的缓冲区首地址
	cs_insn* pInsn;
	// 结构体内存
	cs_opt_mem OptMem;
};

#endif