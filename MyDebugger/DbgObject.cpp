#include "DbgObject.h"
#include "HidePEB.h"


DbgObject::DbgObject()
	:m_imgBase(), m_oep(), m_pid(), m_tid(), m_hCurrProcess(), m_hCurrThread()
{
}


DbgObject::~DbgObject()
{
}
// �򿪽��̵���
bool DbgObject::Open(const char * pszFile)
{
	if (pszFile == nullptr)
		return false;
	// ������Ϣ
	STARTUPINFOA        si = { sizeof(STARTUPINFOA) };
	// ������Ϣ
	PROCESS_INFORMATION pi = { 0 };
	// �������Խ���
	BOOL bRet = FALSE;
	bRet = CreateProcessA(
		pszFile,				// ��ִ��ģ��·��
		NULL,					// ������
		NULL,                   // ��ȫ������
		NULL,               	// �߳������Ƿ�ɼ̳�
		FALSE,					// �Ƿ�ӵ��ý��̴��̳��˾��
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,// ֻ���Է�ʽ���� | �����¿���̨
		NULL,					// �½��̵Ļ�����
		NULL,					// �½��̵ĵ�ǰ����·������ǰ����Ŀ¼��
		&si,					// ָ�����̵�����������
		&pi                     // �����½��̵Ľ��̼����߳̾����ID
	);
	if (bRet == FALSE)
		return false;
	// ���������Ľ�����Ϣ���浽��Ա������
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
	// ���Խ��̽����󱻵��Խ��̲�����
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
	// ö�ٽ���ģ��
	DWORD dwNeed = 0;
	EnumProcessModulesEx(
		m_hCurrProcess,     // ��ѯ���̵ľ��
		nullptr,			// ģ�����б������
		0,					// ����Ĵ�С�����ֽ�Ϊ��λ��
		&dwNeed,			// �����д洢����ģ����������ֽ�����
		LIST_MODULES_ALL);  // �������� LIST_MODULES_ALL�г�����ģ�飨32λ��64λ��
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
	// ѭ����ȡģ����Ϣ
	for (DWORD i = 0; i < dwModuleCount; i++)
	{
		// ��ȡģ��·��
		GetModuleFileNameExA(
			m_hCurrProcess,  // ����ģ��Ľ��̾��
			phModule[i],     // ģ��ľ��
			szPath,          // ģ���·��
			MAX_PATH         // ģ��·������
		);
		// ��ȡģ��������Ϣ
		GetModuleInformation(
			m_hCurrProcess,   // ����ģ��Ľ��̾��
			phModule[i],      // ģ��ľ��
			&mi,			  // ģ����Ϣ�ṹ��
			sizeof(MODULEINFO)// ģ��ṹ���С
		);

		itr->name = PathFindFileNameA(szPath);
		itr->uStart = (LONG64)mi.lpBaseOfDll;
		itr->uSize = mi.SizeOfImage;
		++itr;
	}

	delete[] phModule;
}
