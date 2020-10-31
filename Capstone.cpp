#include "Capstone.h"
#include "windows.h"
#include "iostream"
#include "Stud/Stud.h"
#include <time.h>

using std::cout; using std::endl;

extern FILE* fpVmFile;
extern _VmNode* g_Vm;
extern char* g_dataHlpers;
extern DWORD64 g_dataoffset;

Capstone::Capstone()
{
	srand(time(0));
}

Capstone::~Capstone()
{

}

// 初始化反汇编引擎
void Capstone::InitCapstone()
{
	// 配置堆空间的回调函数
	OptMem.free = free;
	OptMem.calloc = calloc;
	OptMem.malloc = malloc;
	OptMem.realloc = realloc;
	OptMem.vsnprintf = (cs_vsnprintf_t)vsprintf_s;
	// 注册堆空间管理组函数
	cs_option(NULL, CS_OPT_MEM, (size_t)&OptMem);
	// 打开一个句柄
#ifdef _WIN64
	cs_open(CS_ARCH_X86, CS_MODE_64, &Handle);
#else
	cs_open(CS_ARCH_X86, CS_MODE_32, &Handle);
#endif
}

// 反汇编信息输出
void Capstone::ShowAssembly(const void* pAddr, int nLen)
{
	this->InitCapstone();
	// 接收OpCode大小 最大16保存机器指令			
	BYTE* pOpCode = (BYTE *)malloc(nLen * 16);
	memset(pOpCode, 0, (sizeof(BYTE) * 16 * nLen));
	SIZE_T read = 0;
	// 反汇编指定条数的语句
	// 用来读取指令位置内存的缓冲区信息
	cs_insn* ins = nullptr;

	// 读取指定长度的内存空间
	SIZE_T dwCount = 0;
	ReadProcessMemory(NULL, pAddr, pOpCode, nLen * 16, &dwCount);

	int count = cs_disasm(Handle, (uint8_t*)pOpCode, nLen * 16, (uint64_t)pAddr, 0, &ins);

	for (int i = 0; i < nLen; ++i)
	{
		printf("%08X\t", ins[i].address);
		for (uint16_t j = 0; j < 16; ++j)
		{
			if (j < ins[i].size)
				printf("%02X", ins[i].bytes[j]);
			else
				printf(" ");
		}
		printf("\t");
		printf("%s  ", ins[i].mnemonic);
		cout << ins[i].op_str << endl;
	}
	printf("\n");
	// 释放动态分配的空间
	delete[] pOpCode;
	cs_free(ins, count);
}

int Capstone::AnalyencodeVmHlper(cs_insn* ins, unsigned int rankey)
{
	/*
		1. 如果支持全指令，可以不用记录偏移
		2. 如果支持部分指令，需要记录偏移，保证没有进行VMcode加密代码也可以执行
		目前测试代码是全指令，因为都可以识别
		但是用到其它代码段上需要记录偏移
	*/
	/*
	思考了两种方案：
		1. 线性-递归
		2. 精准-模糊
		方案用线性-模糊
		对call-jmp等跳转函数不进行VM加密，模糊含义只对指令判断
		比如mov edi,edi  无论mov后面根什么如果是mov就同一套handler挂钩处理
	*/
	// 目前支持加密的指令
	if (
		0 == _stricmp(ins->mnemonic, "mov")		||
		0 == _stricmp(ins->mnemonic, "push")	||
		0 == _stricmp(ins->mnemonic, "jmp")		||
		0 == _stricmp(ins->mnemonic, "call")	||
		0 == _stricmp(ins->mnemonic, "lea")		||
		0 == _stricmp(ins->mnemonic, "sub")
		)
	{
		for (size_t i = 0; i < ins->size; i++)
		{
			//if (*((char *)ins->address) == (unsigned char)('\x00'))
			//	continue;
			*((char *)ins->address) ^= rankey;
			ins->address += 1;
		}
		return 1;
	}
	return 0;
}

// 指令分析-数据保存
void Capstone::AnalyOpcodeHlper(const void* pAddr, int nLen)
{
	g_Vm->data = (ArrayHlerp *)g_dataHlpers;
	memset(g_Vm->data, 0, nLen * sizeof(ArrayHlerp));

	this->InitCapstone();
	if (!pAddr && !nLen)
		return;
	BYTE* pOpCode = (BYTE *)malloc(nLen * 16);
	memset(pOpCode, 0, (sizeof(BYTE) * 16 * nLen));
	SIZE_T read = 0;
	cs_insn* ins = nullptr;
	SIZE_T dwCount = 0;
	memcpy(pOpCode, pAddr, nLen * 16);
	int count = cs_disasm(Handle, (uint8_t*)pOpCode, nLen * 16, (uint64_t)pAddr, 0, &ins);
	int vmflag = 1;
	unsigned short randnumber = 0;

	for (int i = 0; i < nLen; ++i)
	{
		g_Vm->data->startoffset = ins[i].address - (uint64_t)pAddr;
		// write : 2. 记录每次 异或密码 | byte大小 | 是否成功
		randnumber = rand() % 0xff;
		g_Vm->data->xorKey = randnumber;
		g_Vm->data->bytesize = ins[i].size;
		// fwrite(&randnumber, sizeof(int), 1, fpVmFile);
		// fwrite(&ins[i].size, sizeof(unsigned short), 1, fpVmFile);
		// 进入指令解析和Opcode加密
		if (AnalyencodeVmHlper(&ins[i], randnumber))
		{
			g_Vm->data->encodeflag = 1;
			// fwrite(&vmflag, sizeof(int), 1, fpVmFile);
		}
		else
		{
			g_Vm->data->encodeflag = 0;
			// vmflag = 0;
			// fwrite(&vmflag, sizeof(int), 1, fpVmFile);
		}
		strcpy(g_Vm->data->mnemonic, ins[i].mnemonic);
		// fwrite(ins[i].mnemonic, CS_MNEMONIC_SIZE, 1, fpVmFile);
		// fflush(fpVmFile);
		g_Vm->data++;
	}
	// 恢复指针,否则保存到文件的则是循环后的指针
	g_Vm->Hlperdataoffset = g_dataoffset;
	// fclose(fpVmFile);
	printf("\n");
	// 释放动态分配的空间
	delete[] pOpCode;
	cs_free(ins, count);
}