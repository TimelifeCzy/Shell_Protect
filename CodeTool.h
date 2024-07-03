#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <map>

class MD5
{
public:
	typedef unsigned int size_type; // must be 32bit  

	MD5();
	MD5(const std::string& text);
	void update(const unsigned char* buf, size_type length);
	void update(const char* buf, size_type length);
	MD5& finalize();
	std::string hexdigest() const;
	friend std::ostream& operator<<(std::ostream&, MD5 md5);

private:
	void init();
	typedef unsigned char uint1; //  8bit  
	typedef unsigned int uint4;  // 32bit  
	enum { blocksize = 64 }; // VC6 won't eat a const static int here  

	void transform(const uint1 block[blocksize]);
	static void decode(uint4 output[], const uint1 input[], size_type len);
	static void encode(uint1 output[], const uint4 input[], size_type len);

	bool finalized;
	uint1 buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk  
	uint4 count[2];   // 64bit counter for number of bits (lo, hi)  
	uint4 state[4];   // digest so far  
	uint1 digest[16]; // the result  

	// low level logic operations  
	static inline uint4 F(uint4 x, uint4 y, uint4 z);
	static inline uint4 G(uint4 x, uint4 y, uint4 z);
	static inline uint4 H(uint4 x, uint4 y, uint4 z);
	static inline uint4 I(uint4 x, uint4 y, uint4 z);
	static inline uint4 rotate_left(uint4 x, int n);
	static inline void FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
	static inline void GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
	static inline void HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
	static inline void II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
};

namespace CodeTool
{
	const std::string md5(const std::string& str);

	const std::string wstring2string(const std::wstring& ws);
	const std::wstring string2wstring(const std::string& s);

	const std::string UTF8ToGBK(const char* src_str);
	const std::string GBKToUTF8(const char* src_str);

	const DWORD stringToAddr(PVOID pAddress);

	const std::string RandString(int nLen);

	const bool CreateWriteFile(const std::string& strfilePath, const std::string& strData);
	const bool CreateWriteFile(const std::string& strfilePath, const char* strData, const int Datalen);
	const bool ReadFileData(const std::string& strfilePath, char*& cRData, DWORD& cRSize);

	void CreateRegistryKey(HKEY key, std::wstring path, std::wstring name);
	void DeleteRegistryKey(HKEY key, std::wstring path, std::wstring name);
	void SetRegistryValue(HKEY key, std::wstring path, std::wstring name, std::wstring value);
	void ReadRegistryValue(HKEY key, std::wstring path, std::wstring name, std::wstring& value);

	// 显示窗口
	void WindlgShow(HWND& hWnd);
	// 提升权限
	const bool AdjustProcessTokenPrivilege();
	// 获取当前目录
	const bool CGetCurrentDirectory(std::string& strDirpath);
	// 杀进程
	const bool KillProcess();
	const bool KillProcess(const std::wstring& strKillName);
	// 进程是否存在
	const bool IsProcessActive(const std::string& strProcessName = "");
	const bool GetProcessActivePid(DWORD& dwPid, const std::string& strProcessName = "");
	// 创建目录
	const bool CreateImageDirectory(std::string strDirectoryPath = "");
	// 删除目录
	const bool DeleteDirectory(const std::string& strDirectoryPath = "");

	// 日志-连接
	extern bool g_bLog, g_connectInit;
	// 心跳
	extern HANDLE g_hQueue, g_hBeat;

	// Download映射UUID
	extern std::mutex g_CacheMtx;
	extern std::map<std::string, std::string> g_DownloadCacheToUUID;
}