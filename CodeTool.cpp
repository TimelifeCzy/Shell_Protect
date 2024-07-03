#include <Windows.h>
#include <io.h>
#include <vector>
#include <string>
#include <Tlhelp32.h>
#include <comutil.h>
#include <shellapi.h>
#include <comutil.h>
#include "Shlwapi.h"
#include "CodeTool.h"

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "comsuppw.lib ")
#pragma comment(lib, "comsuppwd.lib")

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static const std::vector<std::wstring> g_vecWxWorkArry = { L"WXWork.exe", L"WerFault.exe",L"TxBugReport.exe", L"WXWorkWeb.exe", L"WXDrive.exe" };

bool CodeTool::g_bLog = false, CodeTool::g_connectInit = false;
HANDLE CodeTool::g_hQueue = nullptr, CodeTool::g_hBeat = nullptr;

std::mutex CodeTool::g_CacheMtx;
std::map<std::string, std::string> CodeTool::g_DownloadCacheToUUID;

inline MD5::uint4 MD5::F(uint4 x, uint4 y, uint4 z) {
	return x & y | ~x & z;
}

inline MD5::uint4 MD5::G(uint4 x, uint4 y, uint4 z) {
	return x & z | y & ~z;
}

inline MD5::uint4 MD5::H(uint4 x, uint4 y, uint4 z) {
	return x ^ y ^ z;
}

inline MD5::uint4 MD5::I(uint4 x, uint4 y, uint4 z) {
	return y ^ (x | ~z);
}

inline MD5::uint4 MD5::rotate_left(uint4 x, int n) {
	return (x << n) | (x >> (32 - n));
}

inline void MD5::FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac) {
	a = rotate_left(a + F(b, c, d) + x + ac, s) + b;
}

inline void MD5::GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac) {
	a = rotate_left(a + G(b, c, d) + x + ac, s) + b;
}

inline void MD5::HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac) {
	a = rotate_left(a + H(b, c, d) + x + ac, s) + b;
}

inline void MD5::II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac) {
	a = rotate_left(a + I(b, c, d) + x + ac, s) + b;
}

MD5::MD5()
{
	init();
}

MD5::MD5(const std::string& text)
{
	init();
	update(text.c_str(), text.length());
	finalize();
}

void MD5::init()
{
	finalized = false;

	count[0] = 0;
	count[1] = 0;

	// load magic initialization constants.  
	state[0] = 0x67452301;
	state[1] = 0xefcdab89;
	state[2] = 0x98badcfe;
	state[3] = 0x10325476;
}

void MD5::decode(uint4 output[], const uint1 input[], size_type len)
{
	for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
		output[i] = ((uint4)input[j]) | (((uint4)input[j + 1]) << 8) |
		(((uint4)input[j + 2]) << 16) | (((uint4)input[j + 3]) << 24);
}

void MD5::encode(uint1 output[], const uint4 input[], size_type len)
{
	for (size_type i = 0, j = 0; j < len; i++, j += 4) {
		output[j] = input[i] & 0xff;
		output[j + 1] = (input[i] >> 8) & 0xff;
		output[j + 2] = (input[i] >> 16) & 0xff;
		output[j + 3] = (input[i] >> 24) & 0xff;
	}
}

void MD5::transform(const uint1 block[blocksize])
{
	uint4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];
	decode(x, block, blocksize);

	/* Round 1 */
	FF(a, b, c, d, x[0], S11, 0xd76aa478); /* 1 */
	FF(d, a, b, c, x[1], S12, 0xe8c7b756); /* 2 */
	FF(c, d, a, b, x[2], S13, 0x242070db); /* 3 */
	FF(b, c, d, a, x[3], S14, 0xc1bdceee); /* 4 */
	FF(a, b, c, d, x[4], S11, 0xf57c0faf); /* 5 */
	FF(d, a, b, c, x[5], S12, 0x4787c62a); /* 6 */
	FF(c, d, a, b, x[6], S13, 0xa8304613); /* 7 */
	FF(b, c, d, a, x[7], S14, 0xfd469501); /* 8 */
	FF(a, b, c, d, x[8], S11, 0x698098d8); /* 9 */
	FF(d, a, b, c, x[9], S12, 0x8b44f7af); /* 10 */
	FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG(a, b, c, d, x[1], S21, 0xf61e2562); /* 17 */
	GG(d, a, b, c, x[6], S22, 0xc040b340); /* 18 */
	GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG(b, c, d, a, x[0], S24, 0xe9b6c7aa); /* 20 */
	GG(a, b, c, d, x[5], S21, 0xd62f105d); /* 21 */
	GG(d, a, b, c, x[10], S22, 0x2441453); /* 22 */
	GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG(b, c, d, a, x[4], S24, 0xe7d3fbc8); /* 24 */
	GG(a, b, c, d, x[9], S21, 0x21e1cde6); /* 25 */
	GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG(c, d, a, b, x[3], S23, 0xf4d50d87); /* 27 */
	GG(b, c, d, a, x[8], S24, 0x455a14ed); /* 28 */
	GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG(d, a, b, c, x[2], S22, 0xfcefa3f8); /* 30 */
	GG(c, d, a, b, x[7], S23, 0x676f02d9); /* 31 */
	GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH(a, b, c, d, x[5], S31, 0xfffa3942); /* 33 */
	HH(d, a, b, c, x[8], S32, 0x8771f681); /* 34 */
	HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH(a, b, c, d, x[1], S31, 0xa4beea44); /* 37 */
	HH(d, a, b, c, x[4], S32, 0x4bdecfa9); /* 38 */
	HH(c, d, a, b, x[7], S33, 0xf6bb4b60); /* 39 */
	HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH(d, a, b, c, x[0], S32, 0xeaa127fa); /* 42 */
	HH(c, d, a, b, x[3], S33, 0xd4ef3085); /* 43 */
	HH(b, c, d, a, x[6], S34, 0x4881d05); /* 44 */
	HH(a, b, c, d, x[9], S31, 0xd9d4d039); /* 45 */
	HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH(b, c, d, a, x[2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II(a, b, c, d, x[0], S41, 0xf4292244); /* 49 */
	II(d, a, b, c, x[7], S42, 0x432aff97); /* 50 */
	II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II(b, c, d, a, x[5], S44, 0xfc93a039); /* 52 */
	II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II(d, a, b, c, x[3], S42, 0x8f0ccc92); /* 54 */
	II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II(b, c, d, a, x[1], S44, 0x85845dd1); /* 56 */
	II(a, b, c, d, x[8], S41, 0x6fa87e4f); /* 57 */
	II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II(c, d, a, b, x[6], S43, 0xa3014314); /* 59 */
	II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II(a, b, c, d, x[4], S41, 0xf7537e82); /* 61 */
	II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II(c, d, a, b, x[2], S43, 0x2ad7d2bb); /* 63 */
	II(b, c, d, a, x[9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	// Zeroize sensitive information.  
	memset(x, 0, sizeof x);
}

void MD5::update(const unsigned char input[], size_type length)
{
	// compute number of bytes mod 64  
	size_type index = count[0] / 8 % blocksize;

	// Update number of bits  
	if ((count[0] += (length << 3)) < (length << 3))
		count[1]++;
	count[1] += (length >> 29);

	// number of bytes we need to fill in buffer  
	size_type firstpart = 64 - index;

	size_type i;

	// transform as many times as possible.  
	if (length >= firstpart)
	{
		// fill buffer first, transform  
		memcpy(&buffer[index], input, firstpart);
		transform(buffer);

		// transform chunks of blocksize (64 bytes)  
		for (i = firstpart; i + blocksize <= length; i += blocksize)
			transform(&input[i]);

		index = 0;
	}
	else
		i = 0;

	// buffer remaining input  
	memcpy(&buffer[index], &input[i], length - i);
}

void MD5::update(const char input[], size_type length)
{
	update((const unsigned char*)input, length);
}

MD5& MD5::finalize()
{
	static unsigned char padding[64] = {
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	if (!finalized) {
		// Save number of bits  
		unsigned char bits[8];
		encode(bits, count, 8);

		// pad out to 56 mod 64.  
		size_type index = count[0] / 8 % 64;
		size_type padLen = (index < 56) ? (56 - index) : (120 - index);
		update(padding, padLen);

		// Append length (before padding)  
		update(bits, 8);

		// Store state in digest  
		encode(digest, state, 16);

		// Zeroize sensitive information.  
		memset(buffer, 0, sizeof buffer);
		memset(count, 0, sizeof count);

		finalized = true;
	}

	return *this;
}

std::string MD5::hexdigest() const
{
	if (!finalized)
		return "";

	char buf[33];
	for (int i = 0; i < 16; i++)
		sprintf(buf + i * 2, "%02x", digest[i]);
	buf[32] = 0;

	return std::string(buf);
}

const std::string CodeTool::md5(const std::string& str)
{
	try
	{
		MD5 md5 = MD5(str);
		return md5.hexdigest();
	}
	catch (const std::exception&)
	{
		return "";
	}
}

const std::string CodeTool::RandString(int nLen)
{
	try
	{
		srand((int)time(NULL));
		char buf[256] = { 0 };
		for (int i = 0; i < nLen; i++)
		{
			buf[i] = rand() % 26 + 97;
		}
		return buf;
	}
	catch (const std::exception&)
	{
		return "bbfabf";
	}
}

const std::string CodeTool::wstring2string(const std::wstring& ws)
{
	try
	{
		_bstr_t t = ws.c_str();
		char* pchar = (char*)t;
		std::string result = pchar;
		return result;
	}
	catch (const std::exception&)
	{
		return "";
	}
}
const std::wstring CodeTool::string2wstring(const std::string& s)
{
	try
	{
		_bstr_t t = s.c_str();
		wchar_t* pwchar = (wchar_t*)t;
		std::wstring result = pwchar;
		return result;
	}
	catch (const std::exception&)
	{
		return L"";
	}
}

const std::string CodeTool::UTF8ToGBK(const char* src_str) {
	if (src_str == nullptr) {
		return "";
	}
	try
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
		wchar_t* wszGBK = new wchar_t[len + 1];
		memset(wszGBK, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
		len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
		char* szGBK = new char[len + 1];
		memset(szGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
		std::string strTemp(szGBK);
		if (wszGBK) delete[] wszGBK;
		if (szGBK) delete[] szGBK;
		return strTemp;
	}
	catch (const std::exception&)
	{
		return "";
	}
}
const std::string CodeTool::GBKToUTF8(const char* src_str) {
	if (src_str == nullptr) {
		return "";
	}
	try
	{
		int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[len + 1];
		memset(wstr, 0, len + 1);
		MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		char* str = new char[len + 1];
		memset(str, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
		std::string strTemp = str;
		if (wstr) delete[] wstr;
		if (str) delete[] str;
		return strTemp;
	}
	catch (const std::exception&)
	{
		return "";
	}
}

void CodeTool::WindlgShow(HWND& hWnd)
{
	//typedef void    (WINAPI* PROCSWITCHTOTHISWINDOW)    (HWND, BOOL);
	//PROCSWITCHTOTHISWINDOW    SwitchToThisWindow;
	//HMODULE    hUser32 = GetModuleHandle(L"user32");
	//SwitchToThisWindow = (PROCSWITCHTOTHISWINDOW)GetProcAddress(hUser32, "SwitchToThisWindow");
	//SwitchToThisWindow(hWnd, TRUE);
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
}

const bool CmdKillProcess(const int Pid)
{
	try
	{
		const std::wstring cmdLline = L"/c taskkill /F /PID " + std::to_wstring(Pid);
		if (ShellExecute(NULL, L"open", L"cmd.exe", cmdLline.c_str(), NULL, SW_HIDE))
			return true;
		return false;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
const bool CodeTool::KillProcess()
{// 遍历相关进程
	try
	{
		const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (INVALID_HANDLE_VALUE == hSnapshot)
			return false;
		PROCESSENTRY32 pi;
		pi.dwSize = sizeof(PROCESSENTRY32);
		BOOL bRet = Process32First(hSnapshot, &pi);
		while (bRet)
		{
			for (const auto& pRocNameiter : g_vecWxWorkArry)
			{
				if (0 == lstrcmpW(pRocNameiter.c_str(), pi.szExeFile))
				{
					// 多账户下只有当前账户的PID才会有权限Open
					const HANDLE hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.th32ProcessID);
					if (hprocess)
					{
						const BOOL ret = TerminateProcess(hprocess, -1);
						if (!ret)
							CmdKillProcess(pi.th32ProcessID);
						CloseHandle(hprocess);
					}
				}
			}
			bRet = Process32Next(hSnapshot, &pi);
		}
		CloseHandle(hSnapshot);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

const bool CodeTool::KillProcess(const std::wstring& strKillName)
{// 指定进程名
	const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (INVALID_HANDLE_VALUE == hSnapshot)
		return false;
	PROCESSENTRY32 pi;
	pi.dwSize = sizeof(PROCESSENTRY32);
	BOOL bRet = Process32First(hSnapshot, &pi);
	while (bRet)
	{
		if (0 == lstrcmpW(strKillName.c_str(), pi.szExeFile))
		{
			const HANDLE hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.th32ProcessID);
			if (hprocess)
			{
				TerminateProcess(hprocess, -1);
				CloseHandle(hprocess);
			}
		}
		bRet = Process32Next(hSnapshot, &pi);
	}
	CloseHandle(hSnapshot);
	return true;
}

const bool CodeTool::AdjustProcessTokenPrivilege()
{
	LUID luidTmp;
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return false;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidTmp))
	{
		CloseHandle(hToken);
		return FALSE;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luidTmp;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		CloseHandle(hToken);
		return FALSE;
	}
	return true;
}

const bool CodeTool::CreateImageDirectory(std::string strDirectoryPath)
{
	if (strDirectoryPath.empty()) {
		CGetCurrentDirectory(strDirectoryPath);
		if (strDirectoryPath.empty()) {
			OutputDebugString(L"创建图片缓存目录失败");
			return false;
		}
		strDirectoryPath.append("ImageCache");
	}
	if (0 == _access(strDirectoryPath.c_str(), 0))
		return true;
	if (CreateDirectoryA(strDirectoryPath.c_str(), NULL))
		return true;
	return false;
}

const bool CodeTool::DeleteDirectory(const std::string& strDirectoryPath)
{
	try
	{
		if (strDirectoryPath.empty())
			return false;

		const std::wstring wstrDirectory = string2wstring(strDirectoryPath).c_str();
		const int buflens = (wstrDirectory.size() + 2) * 2;
		wchar_t* buffer = new wchar_t[buflens];
		RtlSecureZeroMemory(buffer, buflens);
		wcscpy_s(buffer, wstrDirectory.length() + 2, wstrDirectory.c_str());
		buffer[wstrDirectory.length() + 1] = L'\0';

		SHFILEOPSTRUCT FileOp;
		FileOp.fFlags = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION;
		FileOp.hNameMappings = NULL;
		FileOp.hwnd = NULL;
		FileOp.lpszProgressTitle = NULL;
		FileOp.pFrom = buffer;
		FileOp.pTo = NULL;
		FileOp.wFunc = FO_DELETE;
		const int icode = SHFileOperation(&FileOp);
		if (buffer)
			delete[] buffer;
		return (icode == 0);
	}
	catch (const std::exception&)
	{
		return false;
	}
}

const bool CodeTool::GetProcessActivePid(DWORD& dwPid, const std::string& strProcessName)
{
	try
	{
		if (strProcessName.empty())
			return false;
		const std::wstring wstrProcName = string2wstring(strProcessName);
		if (wstrProcName.empty())
			return false;

		const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (INVALID_HANDLE_VALUE == hSnapshot)
			return false;
		PROCESSENTRY32 pi;
		pi.dwSize = sizeof(PROCESSENTRY32);
		BOOL bRet = Process32First(hSnapshot, &pi);
		while (bRet)
		{
			if (0 == lstrcmpW(wstrProcName.c_str(), pi.szExeFile))
			{
				// 多账户下只有当前账户的PID才会有权限Open
				const HANDLE hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.th32ProcessID);
				if (hprocess)
				{
					dwPid = pi.th32ProcessID;
					CloseHandle(hprocess);
					CloseHandle(hSnapshot);
					return true;
				}
			}
			bRet = Process32Next(hSnapshot, &pi);
		}
		CloseHandle(hSnapshot);
		return false;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

const bool CodeTool::IsProcessActive(const std::string& strProcessName)
{
	try
	{
		if (strProcessName.empty())
			return false;
		const std::wstring wstrProcName = string2wstring(strProcessName);
		if (wstrProcName.empty())
			return false;

		const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (INVALID_HANDLE_VALUE == hSnapshot)
			return false;
		PROCESSENTRY32 pi;
		pi.dwSize = sizeof(PROCESSENTRY32);
		BOOL bRet = Process32First(hSnapshot, &pi);
		while (bRet)
		{
			if (0 == lstrcmpW(wstrProcName.c_str(), pi.szExeFile))
			{
				CloseHandle(hSnapshot);
				return true;
			}
			bRet = Process32Next(hSnapshot, &pi);
		}
		CloseHandle(hSnapshot);
		return false;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

const bool CodeTool::CGetCurrentDirectory(std::string & strDirpath)
{
	// 获取当前目录路径
	char szModule[1024] = { 0, };
	GetModuleFileNameA(NULL, szModule, sizeof(szModule) / sizeof(char));
	strDirpath = szModule;
	if (0 >= strDirpath.size())
	{
		OutputDebugString(L"[WXwinDlgMsgSimu] GetModuleFileNameA Error");
		return 0;
	}
	int offset = strDirpath.rfind("\\");
	if (0 >= offset)
	{
		OutputDebugString(L"[WXwinDlgMsgSimu] GetModuleFileNameA Size < 0");
		return 0;
	}
	strDirpath = strDirpath.substr(0, offset + 1);
	return true;
}

const size_t ConvertHexStrToInt(const char* hex_str, const size_t length)
/*
* 	//const int bufLen = buffer.size() / 2;
	//const auto strChar = buffer.c_str();
	//if (!strChar)
	//	return false;
	//uint8_t* uFilebuf = (uint8_t*)malloc(bufLen);
	//if (!uFilebuf)
	//	return false;
	//memset(uFilebuf, 0, bufLen);
	//for (size_t i = 0, j = 0; i < bufLen; i += 2, ++j)
	//{
	//	const uint8_t value = (uint8_t)ConvertHexStrToInt(strChar + i, 2);
	//	uFilebuf[j] = value;
	//}
*/
{
	try
	{
		size_t sum = 0;
		for (size_t i = 0; i < length; ++i)
		{
			int asc = (int)hex_str[i];
			const size_t r1 = (asc & 0x40) ? (asc & 0x0F) + 0x9 : (asc & 0x0F);
			sum += (r1 * pow(16, length - i - 1));
		}
		return sum;
	}
	catch (const std::exception&)
	{
		return 0;
	}
}

void convert_ASCII(const std::string& hex,std::string& strBuf) {
	try
	{
		strBuf = "";
		for (size_t i = 0; i < hex.length(); i += 2) {
			//taking two characters from hex string
			std::string part = hex.substr(i, 2);
			//changing it into base 16
			char ch = std::stoul(part, nullptr, 16);
			//putting it into the ASCII string
			strBuf += ch;
		}
	}
	catch (const std::exception&)
	{
		strBuf = "";
	}
}

const bool CodeTool::CreateWriteFile(const std::string& strfilePath, const std::string &strData)
{
	const HANDLE hFile = CreateFileA(strfilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile || !hFile) {
		//LOG4CPLUS_INFO(g_logger, "Create Failuer Code: " << GetLastError() << " Path: " << strfilePath.c_str());
		return false;
	}
	bool bRet = false;
	DWORD dwSize = 0;
	if (WriteFile(hFile, strData.c_str(), strData.size(), &dwSize, NULL)) {
		bRet = true;
	}
	if (hFile)
		CloseHandle(hFile);
	return bRet;
}

const bool CodeTool::CreateWriteFile(const std::string& strfilePath, const char* strData, const int Datalen)
{
	const HANDLE hFile = CreateFileA(strfilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile || !hFile) {
		//LOG4CPLUS_INFO(g_logger, "Create Failuer Code: " << GetLastError() << " Path: " << strfilePath.c_str());
		return false;
	}
	bool bRet = false;
	DWORD dwSize = 0;
	if (strData && Datalen) {
		if (WriteFile(hFile, strData, Datalen, &dwSize, NULL)) {
			bRet = true;
		}
	}
	if (hFile)
		CloseHandle(hFile);
	return bRet;
}

const bool CodeTool::ReadFileData(const std::string& strfilePath, char*& cRData, DWORD& cRSize)
{
	const HANDLE hFile = CreateFileA(strfilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile || !hFile) {
		//LOG4CPLUS_INFO(g_logger, "OpenFile Failuer Code: " << GetLastError() << " Path: " << strfilePath.c_str());
		return false;
	}
	bool bRet = false;
	const DWORD file_size = GetFileSize(hFile, NULL);
	do
	{
		if (!file_size)
			break;
		cRSize = file_size;
		cRData = new char[file_size];
		if (!cRData)
			break;
		RtlSecureZeroMemory(cRData, file_size);
		DWORD dwRead = 0;
		bRet = ReadFile(hFile, cRData, file_size, &dwRead, NULL);
	} while (false);
	//LOG4CPLUS_INFO(g_logger, "ReadFile Code: " << GetLastError() << " FileSize: " << file_size << " bRet: " << (bRet == true ? 1 : 0));
	if (hFile)
		CloseHandle(hFile);
	return bRet;
}

void CodeTool::CreateRegistryKey(HKEY key, std::wstring path, std::wstring name)
{
	HKEY hKey;
	if (RegOpenKeyExW(key, path.c_str(), 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey) == ERROR_SUCCESS && hKey != NULL)
	{
		HKEY hKeyResult;
		RegCreateKeyW(hKey, name.c_str(), &hKeyResult);
		RegCloseKey(hKey);
	}
}

void CodeTool::DeleteRegistryKey(HKEY key, std::wstring path, std::wstring name)
{
	HKEY hKey;
	if (RegOpenKeyExW(key, path.c_str(), 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey) == ERROR_SUCCESS && hKey != NULL)
	{
		RegDeleteKeyW(hKey, name.c_str());
		RegCloseKey(hKey);
	}
}

void CodeTool::SetRegistryValue(HKEY key, std::wstring path, std::wstring name, std::wstring value)
{
	HKEY hKey;
	if (RegOpenKeyExW(key, path.c_str(), 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey) == ERROR_SUCCESS && hKey != NULL)
	{
		RegSetValueExW(hKey, name.c_str(), 0, REG_SZ, (BYTE*)value.c_str(), ((DWORD)wcslen(value.c_str()) + 1) * sizeof(wchar_t));
		RegCloseKey(hKey);
	}
}

void CodeTool::ReadRegistryValue(HKEY key, std::wstring path, std::wstring name, std::wstring& value)
{
	HKEY hKey;
	if (RegOpenKeyExW(key, path.c_str(), 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey) == ERROR_SUCCESS && hKey != NULL)
	{
		DWORD dwSize = MAX_PATH; DWORD dwRegType = REG_SZ;
		wchar_t wcharUserid[MAX_PATH] = { 0, };
		RegQueryValueExW(hKey, name.c_str(), NULL, &dwRegType, (BYTE*)wcharUserid, &dwSize);
		RegCloseKey(hKey);
		value = wcharUserid;
	}
}

const DWORD CodeTool::stringToAddr(PVOID pAddress)
{
	DWORD dwRet = 0;
	try
	{
		if (NULL == pAddress)
		{
			return dwRet;
		}
#ifdef _DEBUG
		dwRet = ((DWORD)pAddress);
#else
		dwRet = ((DWORD)pAddress + 4);
#endif
	}
	catch (...)
	{
		dwRet = 0;
	}
	return dwRet;
}



