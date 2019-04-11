#include "Debugger.h"


bool Debugger::CreateDebugProcess(TCHAR * path)
{
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi = { 0 };
	BOOL bRet = CreateProcess(path, // exe·��
		NULL, NULL, NULL,             // �����в��� �����̰�ȫ���� ���̰߳�ȫ����
		FALSE,                      // �Ƿ�̳о��
		// ֻ���Է�ʽ���� | �����¿���̨
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
		NULL,                       // ���л�������
		NULL,                       // ����Ŀ¼
		&si,                        // ������Ϣ
		&pi                         // ������Ϣ
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
	// ѭ���ȴ������¼�
	while (!m_exit)
	{
		// �ȴ������¼� -1 ���õȴ�
		WaitForDebugEvent(&m_deEvent, -1);
		m_curPid = m_deEvent.dwProcessId;
		m_curTid = m_deEvent.dwThreadId;
		// �ַ������¼�
		switch (m_deEvent.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:			    // �쳣��Ϣ
			printf("%d:�쳣��Ϣ\n",++index);
			dwDbgContinue = OnExecptionDispatch(&m_deEvent.u.Exception);
			break;
		case CREATE_THREAD_DEBUG_EVENT:			// �̴߳���
			printf("%d:�̴߳���:%p\n", ++index, m_deEvent.u.CreateThread.lpStartAddress);

			m_deEvent.u.CreateThread.lpStartAddress;       // �߳���ڵ�
			break;
		case CREATE_PROCESS_DEBUG_EVENT:   		// ���̴���
			printf("%d:���̴���\n", ++index);
			m_OEP = m_deEvent.u.CreateProcessInfo.lpStartAddress;  // OEP ������ڵ�		
			break;
		case EXIT_THREAD_DEBUG_EVENT:			// �߳̽���		
			printf("%d:�߳̽���\n", ++index);
			break;
		case EXIT_PROCESS_DEBUG_EVENT:			// ���̽���	
			m_exit = TRUE;
			printf("%d:���̽���\n", ++index);
			break;
		case LOAD_DLL_DEBUG_EVENT:			   // DLL����
			break;
		case UNLOAD_DLL_DEBUG_EVENT:		   // DLLж��		
			break;
		case OUTPUT_DEBUG_STRING_EVENT:		   // �������		
			//printf("%d:DLLж��\n", ++index);
			break;
		case RIP_EVENT:break;                  // ϵͳ����
		}
		// �ظ���ϵͳ �¼��Ƿ���
		ContinueDebugEvent(
			m_deEvent.dwProcessId,  // ����ID
			m_deEvent.dwThreadId,   // �߳�ID
			DBG_CONTINUE			// �¼��Ƿ���
		);
	}
	return true;
}

DWORD Debugger::OnExecptionDispatch(EXCEPTION_DEBUG_INFO *dgbExecption)
{
	DWORD dbg_continue = DBG_CONTINUE;

	// �쳣����
	DWORD code = dgbExecption->ExceptionRecord.ExceptionCode;

	// �쳣��ַ
	LPVOID address = dgbExecption->ExceptionRecord.ExceptionAddress;

	// ����쳣����
	switch (code)
	{
	case EXCEPTION_BREAKPOINT:   // ����ϵ�
		dbg_continue = OnExceptionBreakPoint(address);
		break;

	case EXCEPTION_ACCESS_VIOLATION: // �����쳣
		dbg_continue = OnExceptionAccess(address);
		break;

	case EXCEPTION_SINGLE_STEP:  // Ӳ���ϵ�/�����ϵ�
		dbg_continue = OnExceptionSingleStep(address);
		break;
	}
	return dbg_continue;
}

DWORD Debugger::OnExceptionBreakPoint(LPVOID address)
{
	// ���ϵͳ�ϵ�
	if (m_firstSystemBreakPoint)
	{
		m_firstSystemBreakPoint = FALSE;
		ShowAsm(address, m_curPid);
		// ����һ��OEP�ϵ�
		SetBreakPoint(m_OEP, m_curPid, TRUE);
		return DBG_CONTINUE;
	}
	// �ָ�ԭʼ����
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
	// �ָ�����ϵ�
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
	// 1. �򿪽���
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	// 2. ��ȡ���̵�ַ����
	ReadProcessMemory(hProcess, address, buff, 200, &dwSize);
	// 3. ����������
	DISASM dis = { 0 };
	dis.Archi = 0;                       // x86���
	dis.EIP = (UIntPtr)buff;             // ������ַ
	dis.VirtualAddr = (UINT64)address;   //��ʾ��ַ
	len = Disasm(&dis);

	// 4. ��ʾ������   ��ʽ ����ַ   ָ��
	printf("0x%08X  |  %s\n", (DWORD)address, dis.CompleteInstr);
	// 5. �رս��̾��
	CloseHandle(hProcess);
	// 6. ���س���
	return len;
}

bool Debugger::SetBreakPoint(LPVOID address, DWORD pid, BOOL save)
{

	BREAKPOINT bp;
	bp.address = address;
	bp.enbale = TRUE;

	// 1. �򿪽���
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	// 2. �޸�ҳ������
	VirtualProtectEx(hProcess,
		address, 1,
		PAGE_EXECUTE_READWRITE,
		&bp.oldProtect);
	// 3. ��ȡԭʼ����
	DWORD dwSize;
	ReadProcessMemory(hProcess, address, &bp.oldByte, 1, &dwSize);
	// 4. д��cc�ϵ�
	WriteProcessMemory(hProcess, address, "\xcc", 1, &dwSize);
	// 5. ����ϵ���Ϣ
	if (save)
	{
		m_vecBpList.push_back(bp);
	}
	// 6. �ָ�ҳ������
	VirtualProtectEx(hProcess,
		address, 1,
		bp.oldProtect,
		&dwSize);
	// 7. �رս��̾��
	CloseHandle(hProcess);
	return true;
}

bool Debugger::RecoverBreakPoint(LPVOID address, DWORD pid)
{
	for (size_t i = 0; i < m_vecBpList.size(); i++)
	{
		// ����Ƕϵ��б��е��쳣
		if (m_vecBpList[i].address == address)
		{
			// 1. �򿪽���
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,
				FALSE, pid);
			// 2. �޸�ҳ������
			DWORD dwProtect;
			VirtualProtectEx(hProcess,
				address, 1,
				PAGE_EXECUTE_READWRITE,
				&dwProtect);

			DWORD dwSize;

			// 4. д��ԭʼ����
			WriteProcessMemory(hProcess, address,
				&m_vecBpList[i].oldByte, 1, &dwSize);

			// 6. �ָ�ҳ������
			VirtualProtectEx(hProcess,
				address, 1,
				m_vecBpList[i].oldProtect,
				&dwSize);
			// 7. �رս��̾��
			CloseHandle(hProcess);
			return true;
		}
	}
	return false;
}

void Debugger::EipSub1(DWORD tid)
{
	// ���߳�

	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);

	CONTEXT con = { CONTEXT_ALL };
	// ��ȡ�߳�������
	GetThreadContext(hThread, &con);
	// �޸�
	con.Eip -= 1;
	// �����߳�������
	SetThreadContext(hThread, &con);
	// �رվ��
	CloseHandle(hThread);

}

void Debugger::SetTF(DWORD tid)
{
	// ���߳�

	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);

	CONTEXT con = { CONTEXT_ALL };
	// ��ȡ�߳�������
	GetThreadContext(hThread, &con);
	// �޸� TF λ��Ϊ1
	con.EFlags |= 0x100;
	// �����߳�������
	SetThreadContext(hThread, &con);
	// �رվ��
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
