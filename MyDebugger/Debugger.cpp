#include "Debugger.h"


bool Debugger::CreateDebugProcess(TCHAR * path)
{
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi = { 0 };
	BOOL bRet = CreateProcess(path, // exe路径
		NULL, NULL, NULL,             // 命令行参数 ，进程安全属性 ，线程安全属性
		FALSE,                      // 是否继承句柄
		// 只调试方式启动 | 创建新控制台
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
		NULL,                       // 运行环境变量
		NULL,                       // 工作目录
		&si,                        // 启动信息
		&pi                         // 进程信息
	);
	return bRet == TRUE ? true : false;
}

bool Debugger::CreateDebugActiveProcess(DWORD pid)
{
	return DebugActiveProcess(pid) == TRUE ? true : false;
}

bool Debugger::LoopDebugEvent()
{
	
	DEBUG_EVENT m_deEvent;
	DWORD dwDbgContinue = DBG_CONTINUE;
	printf("\n");
	int index = 0;
	// 循环等待调试事件
	while (!m_exit)
	{
		// 等待调试事件 -1 永久等待
		WaitForDebugEvent(&m_deEvent, -1);
		m_curPid = m_deEvent.dwProcessId;
		m_curTid = m_deEvent.dwThreadId;
		// 分发调试事件
		switch (m_deEvent.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:			    // 异常信息
			printf("%d:异常信息\n",++index);
			dwDbgContinue = OnExecptionDispatch(&m_deEvent.u.Exception);
			break;
		case CREATE_THREAD_DEBUG_EVENT:			// 线程创建
			printf("%d:线程创建:%p\n", ++index, m_deEvent.u.CreateThread.lpStartAddress);

			m_deEvent.u.CreateThread.lpStartAddress;       // 线程入口点
			break;
		case CREATE_PROCESS_DEBUG_EVENT:   		// 进程创建
			printf("%d:进程创建\n", ++index);
			m_OEP = m_deEvent.u.CreateProcessInfo.lpStartAddress;  // OEP 程序入口点		
			break;
		case EXIT_THREAD_DEBUG_EVENT:			// 线程结束		
			printf("%d:线程结束\n", ++index);
			break;
		case EXIT_PROCESS_DEBUG_EVENT:			// 进程结束	
			m_exit = TRUE;
			printf("%d:进程结束\n", ++index);
			break;
		case LOAD_DLL_DEBUG_EVENT:			   // DLL加载
			break;
		case UNLOAD_DLL_DEBUG_EVENT:		   // DLL卸载		
			break;
		case OUTPUT_DEBUG_STRING_EVENT:		   // 调试输出		
			//printf("%d:DLL卸载\n", ++index);
			break;
		case RIP_EVENT:break;                  // 系统错误
		}
		// 回复子系统 事件是否处理
		ContinueDebugEvent(
			m_deEvent.dwProcessId,  // 进程ID
			m_deEvent.dwThreadId,   // 线程ID
			DBG_CONTINUE			// 事件是否处理
		);
	}
	return true;
}

DWORD Debugger::OnExecptionDispatch(EXCEPTION_DEBUG_INFO *dgbExecption)
{
	DWORD dbg_continue = DBG_CONTINUE;

	// 异常代码
	DWORD code = dgbExecption->ExceptionRecord.ExceptionCode;

	// 异常地址
	LPVOID address = dgbExecption->ExceptionRecord.ExceptionAddress;

	// 检测异常类型
	switch (code)
	{
	case EXCEPTION_BREAKPOINT:   // 软件断点
		dbg_continue = OnExceptionBreakPoint(address);
		break;

	case EXCEPTION_ACCESS_VIOLATION: // 访问异常
		dbg_continue = OnExceptionAccess(address);
		break;

	case EXCEPTION_SINGLE_STEP:  // 硬件断点/单步断点
		dbg_continue = OnExceptionSingleStep(address);
		break;
	}
	return dbg_continue;
}

DWORD Debugger::OnExceptionBreakPoint(LPVOID address)
{
	// 检测系统断点
	if (m_firstSystemBreakPoint)
	{
		m_firstSystemBreakPoint = FALSE;
		ShowAsm(address, m_curPid);
		// 设置一个OEP断点
		SetBreakPoint(m_OEP, m_curPid, TRUE);
		return DBG_CONTINUE;
	}
	// 恢复原始数据
	if (RecoverBreakPoint(address, m_curPid))
	{
		EipSub1(m_curTid);
		SetTF(m_curTid);
		m_bpRecoverAddr = address;
		m_bpRecover = TRUE;
		ShowAsm(address, m_curPid);
		GetCommandStr();
		return DBG_CONTINUE;
	}



	return DBG_CONTINUE;
}

DWORD Debugger::OnExceptionAccess(LPVOID address)
{
	return DBG_CONTINUE;
}

DWORD Debugger::OnExceptionSingleStep(LPVOID address)
{
	// 恢复软件断点
	if (m_bpRecover)
	{
		m_bpRecover = FALSE;
		for (auto i : m_vecBpList)
		{
			if (m_bpRecoverAddr == i.address)
			{
				SetBreakPoint(i.address, m_curPid, FALSE);
				break;
			}
		}
	}
	return DBG_CONTINUE;
}

DWORD Debugger::ShowAsm(LPVOID address, DWORD pid)
{
	char buff[200] = { 0 };
	DWORD dwSize;
	DWORD len;
	// 1. 打开进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	// 2. 读取进程地址数据
	ReadProcessMemory(hProcess, address, buff, 200, &dwSize);
	// 3. 反汇编机器码
	DISASM dis = { 0 };
	dis.Archi = 0;                       // x86汇编
	dis.EIP = (UIntPtr)buff;             // 反汇编地址
	dis.VirtualAddr = (UINT64)address;   //显示地址
	len = Disasm(&dis);

	// 4. 显示汇编代码   格式 ：地址   指令
	printf("0x%08X  |  %s\n", (DWORD)address, dis.CompleteInstr);
	// 5. 关闭进程句柄
	CloseHandle(hProcess);
	// 6. 返回长度
	return len;
}

bool Debugger::SetBreakPoint(LPVOID address, DWORD pid, BOOL save)
{

	BREAKPOINT bp;
	bp.address = address;
	bp.enbale = TRUE;

	// 1. 打开进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	// 2. 修改页面属性
	VirtualProtectEx(hProcess,
		address, 1,
		PAGE_EXECUTE_READWRITE,
		&bp.oldProtect);
	// 3. 读取原始数据
	DWORD dwSize;
	ReadProcessMemory(hProcess, address, &bp.oldByte, 1, &dwSize);
	// 4. 写入cc断点
	WriteProcessMemory(hProcess, address, "\xcc", 1, &dwSize);
	// 5. 保存断点信息
	if (save)
	{
		m_vecBpList.push_back(bp);
	}
	// 6. 恢复页面属性
	VirtualProtectEx(hProcess,
		address, 1,
		bp.oldProtect,
		&dwSize);
	// 7. 关闭进程句柄
	CloseHandle(hProcess);
	return true;
}

bool Debugger::RecoverBreakPoint(LPVOID address, DWORD pid)
{
	for (size_t i = 0; i < m_vecBpList.size(); i++)
	{
		// 如果是断点列表中的异常
		if (m_vecBpList[i].address == address)
		{
			// 1. 打开进程
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,
				FALSE, pid);
			// 2. 修改页面属性
			DWORD dwProtect;
			VirtualProtectEx(hProcess,
				address, 1,
				PAGE_EXECUTE_READWRITE,
				&dwProtect);

			DWORD dwSize;

			// 4. 写入原始数据
			WriteProcessMemory(hProcess, address,
				&m_vecBpList[i].oldByte, 1, &dwSize);

			// 6. 恢复页面属性
			VirtualProtectEx(hProcess,
				address, 1,
				m_vecBpList[i].oldProtect,
				&dwSize);
			// 7. 关闭进程句柄
			CloseHandle(hProcess);
			return true;
		}
	}
	return false;
}

void Debugger::EipSub1(DWORD tid)
{
	// 打开线程

	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);

	CONTEXT con = { CONTEXT_ALL };
	// 获取线程上下文
	GetThreadContext(hThread, &con);
	// 修改
	con.Eip -= 1;
	// 设置线程上下文
	SetThreadContext(hThread, &con);
	// 关闭句柄
	CloseHandle(hThread);

}

void Debugger::SetTF(DWORD tid)
{
	// 打开线程

	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);

	CONTEXT con = { CONTEXT_ALL };
	// 获取线程上下文
	GetThreadContext(hThread, &con);
	// 修改 TF 位置为1
	con.EFlags |= 0x100;
	// 设置线程上下文
	SetThreadContext(hThread, &con);
	// 关闭句柄
	CloseHandle(hThread);
}

void Debugger::GetCommandStr()
{

	char buf[250] = { 0 };
	while (true)
	{
		printf("->");
		scanf_s("%s", buf, 250);
		if (strcmp(buf, "g") == 0)
		{
			break;
		}
	}


}
