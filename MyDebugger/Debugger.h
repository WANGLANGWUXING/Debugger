#pragma once
#include <Windows.h>
#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine_4.1/Win32/headers/BeaEngine.h"


#ifndef _WIN64
#pragma comment(lib, "BeaEngine_4.1\\Win32\\Win32\\Lib\\BeaEngine.lib")
#pragma comment(lib,"legacy_stdio_definitions.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"crt.lib\"")
#else
#pragma comment(lib, "BeaEngine_4.1\\Win64\\Win64\\Lib\\BeaEngine64.lib")
#pragma comment(lib,"legacy_stdio_definitions.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"crt64.lib\"")
#endif

#include <vector>
using std::vector;

typedef struct _breakpoint{
	LPVOID address;     // 地址
	BYTE oldByte;       // 原始数据
	DWORD oldProtect;   // 页面属性
	DWORD enbale;       // 断点是否启用
}BREAKPOINT, *PBREAKPOINT;


class Debugger
{
public:
	// 调试方式
	// 1. 创建进程
	bool CreateDebugProcess(TCHAR *path);
	// 2. 附加进程
	bool CreateDebugActiveProcess(DWORD pid);
	// 3. 循环等待事件
	bool LoopDebugEvent();
	// 4. 异常分发
	DWORD OnExecptionDispatch(EXCEPTION_DEBUG_INFO *dgbExecption);

	// 4.1 软件异常
	DWORD OnExceptionBreakPoint(LPVOID address);


	// 4.2 访问异常
	DWORD OnExceptionAccess(LPVOID address);

	// 4.3 单步 / 硬件
	DWORD OnExceptionSingleStep(LPVOID address);


	// 显示反汇编
	DWORD ShowAsm(LPVOID address, DWORD pid);

	// 设置软件断点
	bool SetBreakPoint(LPVOID address, DWORD pid, BOOL save);

	// 恢复软件断点
	bool RecoverBreakPoint(LPVOID address, DWORD pid);

	// eip-1
	void EipSub1(DWORD tid);

	// 设置单步
	void SetTF(DWORD tid);

	// 接收命令行
	void GetCommandStr();
private:
	BOOL m_firstSystemBreakPoint = TRUE;  // 系统断点
	BOOL m_exit = FALSE;				  // 进程是否结束
	LPVOID m_OEP;                         // 程序OEP
	DWORD m_curPid;                       // 当前进程id  
	DWORD m_curTid;						  // 当前线程id
	BOOL m_bpRecover = FALSE;             // 软件断点恢复使用
	LPVOID m_bpRecoverAddr;               // 要写入cc的地址
	vector<BREAKPOINT> m_vecBpList;
};

