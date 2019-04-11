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
	LPVOID address;     // ��ַ
	BYTE oldByte;       // ԭʼ����
	DWORD oldProtect;   // ҳ������
	DWORD enbale;       // �ϵ��Ƿ�����
}BREAKPOINT, *PBREAKPOINT;


class Debugger
{
public:
	// ���Է�ʽ
	// 1. ��������
	bool CreateDebugProcess(TCHAR *path);
	// 2. ���ӽ���
	bool CreateDebugActiveProcess(DWORD pid);
	// 3. ѭ���ȴ��¼�
	bool LoopDebugEvent();
	// 4. �쳣�ַ�
	DWORD OnExecptionDispatch(EXCEPTION_DEBUG_INFO *dgbExecption);

	// 4.1 ����쳣
	DWORD OnExceptionBreakPoint(LPVOID address);


	// 4.2 �����쳣
	DWORD OnExceptionAccess(LPVOID address);

	// 4.3 ���� / Ӳ��
	DWORD OnExceptionSingleStep(LPVOID address);


	// ��ʾ�����
	DWORD ShowAsm(LPVOID address, DWORD pid);

	// ��������ϵ�
	bool SetBreakPoint(LPVOID address, DWORD pid, BOOL save);

	// �ָ�����ϵ�
	bool RecoverBreakPoint(LPVOID address, DWORD pid);

	// eip-1
	void EipSub1(DWORD tid);

	// ���õ���
	void SetTF(DWORD tid);

	// ����������
	void GetCommandStr();
private:
	BOOL m_firstSystemBreakPoint = TRUE;  // ϵͳ�ϵ�
	BOOL m_exit = FALSE;				  // �����Ƿ����
	LPVOID m_OEP;                         // ����OEP
	DWORD m_curPid;                       // ��ǰ����id  
	DWORD m_curTid;						  // ��ǰ�߳�id
	BOOL m_bpRecover = FALSE;             // ����ϵ�ָ�ʹ��
	LPVOID m_bpRecoverAddr;               // Ҫд��cc�ĵ�ַ
	vector<BREAKPOINT> m_vecBpList;
};

