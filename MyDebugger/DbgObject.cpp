#include "DbgObject.h"
#include "HidePEB.h"


DbgObject::DbgObject()
	:m_imgBase(), m_oep(), m_pid(), m_tid(), m_hCurrProcess(), m_hCurrThread()
{
}


DbgObject::~DbgObject()
{
}
// 打开进程调试
bool DbgObject::Open(const char * pszFile)
{
	if (pszFile == nullptr)
		return false;
	// 启动信息
	STARTUPINFOA        si = { sizeof(STARTUPINFOA) };
	// 进程信息
	PROCESS_INFORMATION pi = { 0 };
	// 创建调试进程
	BOOL bRet = FALSE;
	bRet = CreateProcessA(
		pszFile,				// 可执行模块路径
		NULL,					// 命令行
		NULL,                   // 安全描述符
		NULL,               	// 线程属性是否可继承
		FALSE,					// 是否从调用进程处继承了句柄
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,// 只调试方式启动 | 创建新控制台
		NULL,					// 新进程的环境块
		NULL,					// 新进程的当前工作路径（当前工作目录）
		&si,					// 指定进程的主窗口特性
		&pi                     // 接收新进程的进程及主线程句柄及ID
	);
	if (bRet == FALSE)
		return false;
	// 将创建出的进程消息保存到成员函数中
	//HidePEBDebug(pi.hProcess);
	m_hCurrProcess = pi.hProcess;
	m_hCurrThread = pi.hThread;
	m_pid = pi.dwProcessId;
	m_tid = pi.dwThreadId;
	//HidePEB(m_pid);

	return true;
}

bool DbgObject::Open(const uint uPid)
{
	// 调试进程结束后被调试进程不结束
	//DebugSetProcessKillOnExit(FALSE);
	BOOL bRet = DebugActiveProcess(uPid);
	return bRet ==TRUE;
}

bool DbgObject::IsOpen()
{
	return m_hCurrProcess != 0;
}

bool DbgObject::IsClose()
{
	return m_hCurrProcess == 0;
}

void DbgObject::Close()
{
	TerminateProcess(m_hCurrProcess, 0);
	m_hCurrProcess = 0;
}

uint DbgObject::ReadMemory(uaddr uAddress, pbyte pBuff, uint uSize)
{
	DWORD dwRead = 0;
	ReadProcessMemory(m_hCurrProcess, (LPVOID)uAddress, pBuff, uSize, &dwRead);
	return dwRead;
}

uint DbgObject::WriteMemory(uaddr uAddress, const pbyte pBuff, uint uSize)
{
	DWORD dwWrite = 0;
	WriteProcessMemory(m_hCurrProcess, (LPVOID)uAddress, pBuff, uSize, &dwWrite);
	return dwWrite;
}

bool DbgObject::GetRegInfo(CONTEXT & ct)
{
	return GetThreadContext(m_hCurrThread, &ct) == TRUE;
}

bool DbgObject::SetRegInfo(CONTEXT & ct)
{
	return SetThreadContext(m_hCurrThread, &ct) == TRUE;
}

void DbgObject::AddThread(HANDLE hThread)
{
	list<HANDLE>::iterator i = m_hThreadList.begin();
	for (; i != m_hThreadList.end(); ++i)
	{
		if (*i == hThread)
			return;
	}
	m_hThreadList.push_back(hThread);
}

void DbgObject::AddProcess(HANDLE hProcess, HANDLE hThread)
{
	AddThread(hThread);
	list<HANDLE>::iterator i = m_hProcessList.begin();
	for (; i != m_hProcessList.end(); ++i)
	{
		if (*i == hProcess)
			return;
	}
	m_hProcessList.push_back(hProcess);
}

bool DbgObject::RemoveThread(HANDLE hThread)
{
	list<HANDLE>::iterator i = m_hThreadList.begin();
	for (; i != m_hThreadList.end(); ++i)
	{
		if (*i == hThread)
		{
			m_hThreadList.erase(i);
			return true;
		}
	}
	return false;
}

bool DbgObject::RemoveProcess(HANDLE hProcess)
{
	list<HANDLE>::iterator i = m_hProcessList.begin();
	for (; i != m_hProcessList.end(); ++i)
	{
		if (*i == hProcess)
		{
			m_hProcessList.erase(i);
			return true;
		}
	}
	return false;
}

void DbgObject::GetModuleList(list<MODULEFULLINFO>& moduleList)
{
	// 枚举进程模块
	DWORD dwNeed = 0;
	EnumProcessModulesEx(
		m_hCurrProcess,     // 查询进程的句柄
		nullptr,			// 模块句柄列表的数组
		0,					// 数组的大小，以字节为单位。
		&dwNeed,			// 数组中存储所有模块句柄所需的字节数。
		LIST_MODULES_ALL);  // 过滤条件 LIST_MODULES_ALL列出所有模块（32位和64位）
	DWORD dwModuleCount = dwNeed / sizeof(HMODULE);
	HMODULE *phModule = new HMODULE[dwModuleCount];
	EnumProcessModulesEx(m_hCurrProcess,
		phModule,
		dwNeed,
		&dwNeed,
		LIST_MODULES_ALL
	);
	MODULEINFO mi = { 0 };
	char szPath[MAX_PATH];
	moduleList.resize(dwModuleCount);
	list<MODULEFULLINFO>::iterator itr = moduleList.begin();
	// 循环获取模块信息
	for (DWORD i = 0; i < dwModuleCount; i++)
	{
		// 获取模块路径
		GetModuleFileNameExA(
			m_hCurrProcess,  // 包含模块的进程句柄
			phModule[i],     // 模块的句柄
			szPath,          // 模块的路径
			MAX_PATH         // 模块路径长度
		);
		// 获取模块其他信息
		GetModuleInformation(
			m_hCurrProcess,   // 包含模块的进程句柄
			phModule[i],      // 模块的句柄
			&mi,			  // 模块信息结构体
			sizeof(MODULEINFO)// 模块结构体大小
		);

		itr->name = PathFindFileNameA(szPath);
		itr->uStart = (LONG64)mi.lpBaseOfDll;
		itr->uSize = mi.SizeOfImage;
		++itr;
	}

	delete[] phModule;
}
