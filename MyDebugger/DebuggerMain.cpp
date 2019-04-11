#include "DebuggerMain.h"
/**
* ����DbgEngine���Open()��һ�����̽��е���.
* ����DbgEngine���Exec()���ձ����Խ��̵ĵ����¼�
* ����DbgEngine���AddBreakpoint()���öϵ�
* ����WaitForBreakpointEvent()�����ȴ��ϵ��¼�
* ����DbgEngine���GetRegInfo()��ȡ�Ĵ�����Ϣ
* ����DbgEngine���SetRegInfo()���üĴ�����Ϣ
* ����DbgEngine���ReadMomory()��ȡָ����ַ���ڴ�
* ����DbgEngine���WriteMomory()����ָ����ַ���ڴ�
* ����DisAssambly��DiAsm()����һ��opcodeת���ɻ�����
* ����XEDParse��XEDParseAssemble()����һ�λ�����ת����opcode
*/
#include <Windows.h>
#include "DbgEngine.h"         // ��������
#include "DbgUi.h"             // �û�����
#include "Expression.h"        // ����ʽģ��
#include "DisAsmEngine.h"      // ���������
#include "XEDParse.h"          // �������
#include "Pectrl.h"

#pragma comment(lib,"XEDParse.lib")

#include "AddPlugin.h"

#include <iostream>
#include <conio.h>
using namespace std;


HANDLE g_hProcess;
HANDLE g_hThread;
unsigned int threadId;
CONTEXT context1;
//���̼��ػ�ַ
uaddr imgBase;
PIMAGE_DOS_HEADER pDosH;	//DOSͷ
char* buff;
CString file_Name;

// ��ʾ�����������а�����Ϣ
void ShowHelp();
// ���ַ����ָ�������ַ���(����һ�������Ŀո��滻���ַ���������)
char* GetSecondArg(char* pBuff);
inline char* SkipSpace(char* pBuff);
// ��ȡ�������еĲ���
void GetCmdLineArg(char* pszCmdLine, int nArgCount, ...);
// ���öϵ�
void SetBreakpoint(DbgEngine* pDbg, DbgUi* pUi, char* szCmdLine, DisAsmEngine* disAsm);
//ͨ������Զ���̸߳��������̼���Dll
BOOL LoadRometeDll(DWORD dwProcessId, LPTSTR lpszLibName);

DWORD	g_dwProcessStatus = 0;



//�����ϵ�Ļص�����
unsigned int __stdcall DbgBreakpointEvent(void* uParam);
// ����Ϊ����Ȩ��
BOOL WINAPI EnablePrivileges()
{
	HANDLE hToken;
	// ���ƽṹ��
	TOKEN_PRIVILEGES tkp;
	// ���޸�Ȩ�޵ķ�ʽ�򿪽��̵�����
	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return(FALSE);
	// ���LUID
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME,
		&tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	// �޸�Ȩ��
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
		(PTOKEN_PRIVILEGES)NULL, 0);

	if (GetLastError() != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}


int main()
{
	//����Ȩ��
	EnablePrivileges();
	printf("--������--\n");

	DbgEngine dbgEng; // �������������

	char szPath[MAX_PATH];
	bool bCreateThread = false;
	unsigned int taddr = 0;
	uintptr_t	 tid = 0;
	uint processId = 0;

	int mFlag = 0;
	while (true)
	{
		printf("->��ѡ����Է�ʽ\n");
		printf("->1.�������̣�2.���ӽ���\n");
		printf("->");
		scanf_s("%d", &mFlag);

		bool bFlag = false;
		// ����ѡ��ʹ�õ��Է�ʽ
		if (mFlag == 1)
		{
			printf("�������ļ�·�����ļ���ק���˽��е��ԣ�");
			scanf_s("%s", szPath, MAX_PATH);
			AnalysisPE(szPath);
			bFlag = dbgEng.Open(szPath);
		}
		else if (mFlag == 2)
		{
			int mPid = 0;
			printf("�����븽�ӽ���PID��");
			scanf_s("%d", &mPid);
			bFlag = dbgEng.Open(mPid);
		}
		else
		{
			continue;
		}
		/*AnalysisPE("CRACKME.EXE");
		bFlag = dbgEng.Open("CRACKME.EXE");
*/
		if (bFlag)
		{
			printf("���Խ��̴����ɹ������Կ�ʼ����");

			g_dwProcessStatus = 0;
			// ���������û�������߳�
			tid = _beginthreadex(0, 0, DbgBreakpointEvent, &dbgEng, 0, &taddr);
			while (1)
			{
				// ���е�����,Exec����������״̬,�����Ҫ����whileѭ����.
				if (e_s_processQuit == dbgEng.Exec()) {
					dbgEng.Close();
					system("cls");
					cout << "�����Խ������˳�\n";
					g_dwProcessStatus = 1;
					break;
				}
			}

		}

	}
}



// ��ʾ�����
void ShowAsm(DbgEngine& dbgEngine,
	DbgUi& ui,
	DisAsmEngine& disAsmEng,
	int nLine = 30,
	SIZE_T Addr = 0)
{
	static CONTEXT ct = { CONTEXT_CONTROL };
	if (Addr == 0)
	{
		dbgEngine.GetRegInfo(ct);
		Addr = ct.Eip;
	}

	vector<DISASMSTRUST> vecDisAsm;
	disAsmEng.DiAsm(Addr, vecDisAsm, nLine);

	for (vector<DISASMSTRUST>::iterator i = vecDisAsm.begin();
		i != vecDisAsm.end();
		++i)
	{
		ui.ShowAsm(Addr,
			(*i).strOpCode,
			(*i).strAsm,
			(*i).strCom
		);
		Addr += (*i).dwCodeLen;
	}
}



//�޸��ڴ� e
DWORD CmdModifyData(CString& str1)
{
	DWORD	dwModifyAddr;
	DWORD	dwOldProtect;
	DWORD	dwReadCount;
	DWORD	dwInputValue;
	byte	bBuffer;
	char	szBuffer[10];

	if (str1.IsEmpty())
	{
		printf("û������Ҫ�޸ĵ��ڴ��ַ\r\n");
	}
	//ת������
	char	*pRet = NULL;
	USES_CONVERSION;

	dwModifyAddr = strtoul(T2CA(str1), &pRet, 16);
	//ת��ʧ��
	if (*pRet != NULL)
	{
		printf("Ҫ�޸ĵ��ڴ��ַ�������\r\n");
		return 0;
	}
	//
	while (1)
	{
		//�޸��ڴ汣������
		VirtualProtectEx(g_hProcess, (LPVOID)dwModifyAddr, 1, PAGE_READWRITE, &dwOldProtect);
		if (!ReadProcessMemory(g_hProcess, (LPVOID)dwModifyAddr, &bBuffer, 1, &dwReadCount)) {

			printf("Ҫ�޸ĵ��ڴ��ַ��Ч\r\n");
			VirtualProtectEx(g_hProcess, (LPVOID)dwModifyAddr, 1, dwOldProtect, &dwReadCount);
			return 0;
		}


		//��ʾԭֵ
		printf("%p  %02X\r\n", dwModifyAddr, bBuffer);
		//��ȡ�û������޸ĺ��ֵ
		fflush(stdin);
		memset(szBuffer, 0, sizeof(szBuffer));
		////scanf_s("%2[^\n]", szBuffer);
		//cin >> szBuffer;
		scanf_s("%s", szBuffer,10);
		fflush(stdin);
		//����Ϊ�����˳�e����
		if (!_stricmp(szBuffer,"quit")) {
			VirtualProtectEx(g_hProcess, (LPVOID)dwModifyAddr, 1, dwOldProtect, &dwReadCount);
			return TRUE;
		}
		//����ת��Ϊ��ֵ����
		dwInputValue = strtoul(szBuffer, &pRet, 16);
		if (*pRet != NULL)
		{
			printf("�����ֵ����\r\n");
			VirtualProtectEx(g_hProcess, (LPVOID)dwModifyAddr, 1, dwOldProtect, &dwReadCount);
			return 0;
		}
		bBuffer = dwInputValue;

		//д���޸ĺ��ֵ
		if (!WriteProcessMemory(g_hProcess, (LPVOID)dwModifyAddr, &bBuffer, 1, &dwReadCount))
		{
			printf("�޸ĺ��ֵд��ʧ��\r\n");
			VirtualProtectEx(g_hProcess, (LPVOID)dwModifyAddr, 1, dwOldProtect, &dwReadCount);
			return 0;
		}

		dwModifyAddr++;
		//��ԭ�ڴ�����
		VirtualProtectEx(g_hProcess, (LPVOID)dwModifyAddr, 1, dwOldProtect, &dwReadCount);
	}

}



// �ϵ��¼��ص�����
unsigned int __stdcall DbgBreakpointEvent(void * uParam)
{
	DbgEngine* pDbg = (DbgEngine*)uParam;
	DbgUi ui(pDbg);
	Expression exp(pDbg);
	DisAsmEngine disAsm(pDbg);

	char szCmdLine[64];
	CONTEXT ct = { CONTEXT_ALL };
	vector<DISASMSTRUST> vecDisAsm;
	char* pCmdLine = 0;


	DWORD	dwStatus = 0;
	while (1)
	{
		if (pDbg->WaitForBreakpointEvent(30))
		{
			// ����
			system("cls");
			// ��ȡ�Ĵ�����Ϣ
			pDbg->GetRegInfo(ct);
			// ʹ��uiģ�齫�Ĵ�����Ϣ���
			ui.ShowReg(ct);
			// ��������
			ShowAsm(*pDbg, ui, disAsm, 20, ct.Eip);
			dwStatus = 1;
		}

		if (dwStatus)
		{
			//printf("%s>", dwStatus == 1 ? "��ͣ��" : "������");
			printf(">");
			dwStatus = 0;
		}
		// �����û����������
		if (_kbhit())
		{
			do
			{
				gets_s(szCmdLine, 64);
			} while (*szCmdLine == '\0');
			dwStatus = 2;
		}
		else
			continue;
		// ������ͷ�ո�
		pCmdLine = SkipSpace(szCmdLine);
		// �ж��Ƿ���Ҫ�˳�������
		if (*(DWORD*)pCmdLine == 'tixe' || g_dwProcessStatus == 1) {
			pDbg->Close();
			pDbg->FinishBreakpointEvent();
			return 0;
		}
		// �����û����������
		switch (*pCmdLine)
		{
		/*�鿴������*/
		case 'u':
		{
			dwStatus = 1;
			char *pAddr = 0;
			char* pLineCount = 0;
			GetCmdLineArg(pCmdLine + 1, 2, &pAddr, &pLineCount);
			if (pAddr == nullptr)
				pAddr = "eip";
			if (pLineCount == nullptr)
				pLineCount = "20";
			// ��ʾ�����
			ShowAsm(*pDbg, ui, disAsm, exp.GetValue(pLineCount), exp.GetValue(pAddr));
			break;
		}
		/*�޸Ļ�����*/
		case 'a':
		{
			dwStatus = 1;
			// ��ȡ��ʼ��ַ
			XEDPARSE xpre = { 0 };
			xpre.x64 = false; // �Ƿ�ת����64λ��opCode
			memset(xpre.dest, 0x90, XEDPARSE_MAXASMSIZE);
			pCmdLine = SkipSpace(pCmdLine + 1);
			if (*pCmdLine == 0)
			{
				printf("ָ���ʽ����, ��ʽΪ: a ��ַ\n");
				continue;// ��������whileѭ��
			}
			printf("����quit�˳����ģʽ\n");
			uaddr address = exp.GetValue(pCmdLine);
			if (address == 0)
				continue;// ��������whileѭ��

			while (true)
			{
				printf("%08X: ", address);
				cin.getline(xpre.instr, XEDPARSE_MAXBUFSIZE);
				if (strcmp(xpre.instr, "quit") == 0)
					break;
				DWORD uLen = disAsm.GetCodeLen(address);
				xpre.cip = address;// ָ�����ڵĵ�ַ
				if (false == XEDParseAssemble(&xpre))
				{
					printf("%s\n", xpre.error);
					continue;// ��������whileѭ��
				}
				// ������д�뵽Ŀ�����
				if (!pDbg->WriteMemory(address, xpre.dest, uLen))
					continue;// ��������whileѭ��
							 // ��ַ++
				address += xpre.dest_size;
			}
			break;
		}
		/*�鿴�ڴ�*/
		case 'd':
		{
			dwStatus = 1;
			char *p = &szCmdLine[1];
			// ɸѡ���ݸ�ʽ
			switch (*p)
			{
			case 'u':/*unicode�ַ���*/
			{
				SSIZE_T uAddr = exp.GetValue(szCmdLine + 2);
				BYTE lpBuff[16 * 6];
				pDbg->ReadMemory(uAddr, lpBuff, 16 * 6);
				ui.ShowMem(uAddr, lpBuff, 16 * 6, 1);
			}
			break;

			case 'a':/*ansi�ַ���*/
				p = &szCmdLine[2];

			default:
			{
				SSIZE_T uAddr = exp.GetValue(p);
				BYTE lpBuff[16 * 6];
				pDbg->ReadMemory(uAddr, lpBuff, 16 * 6);
				ui.ShowMem(uAddr, lpBuff, 16 * 6, 0);
			}
			break;

			}
			break;
		}
		//�޸��ڴ�
		case 'x':
		{
			dwStatus = 1;
			pCmdLine = SkipSpace(pCmdLine + 1);

			//printf("%s",pCmdLine);
			CString str(pCmdLine);
			CmdModifyData(str);
			break;
		}
		/*�鿴ջ*/
		case 'k':
		{
			dwStatus = 1;
			pDbg->GetRegInfo(ct);
			BYTE	buff[sizeof(SIZE_T) * 20];
			pDbg->ReadMemory(ct.Esp, buff, sizeof(SIZE_T) * 20);
			ui.ShowStack(ct.Esp, buff, sizeof(SIZE_T) * 20);
			break;
		}
		/*�鿴���޸ļĴ���*/
		case 'r':
		{
			dwStatus = 1;
			// ��ȡ�Ĵ�����ֵ:
			// r �Ĵ����� 
			// ���üĴ�����ֵ
			// r �Ĵ����� = ����ʽ
			char* p = szCmdLine + 1;
			p = SkipSpace(p);
			if (*p == 0)
			{
				ct.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
				pDbg->GetRegInfo(ct);
				ui.ShowReg(ct);
				break;
			}
			SSIZE_T nValue = exp.GetValue(szCmdLine + 1);

			char* pSecArg = SkipSpace(szCmdLine + 1);
			pSecArg = GetSecondArg(pSecArg);
			if (*pSecArg == 0)
				printf("%s = 0x%X\n", szCmdLine + 1, nValue);
			break;
		}
		/*�鿴���Գ����ģ����Ϣ*/
		case 'm':
		{
			dwStatus = 1;
			if (*SkipSpace(pCmdLine + 1) == 'l')
			{
				list<MODULEFULLINFO> modList;
				pDbg->GetModuleList(modList);
				printf("+------------------+----------+----------------------------------------------------+\n");
				printf("|     ���ػ�ַ     + ģ���С |                    ģ����                          |\n");
				printf("+------------------+----------+----------------------------------------------------+\n");
				for (auto &i : modList)
				{
					printf("| %016I64X | %08X | %-50s |\n", i.uStart, i.uSize, (LPCSTR)i.name);
				}
				printf("+------------------+----------+----------------------------------------------------+\n");
			}
			continue;// ��������whileѭ��
		}
		/*��������*/
		case 't':
		{
			// ʹ�õ���������ĺ���������һ��TF�ϵ�
			BPObject *pBp = pDbg->AddBreakpoint(0, breakpointType_tf);
			if (pBp == nullptr)
				return 0;
			char* pCondition = 0;
			GetCmdLineArg(pCmdLine + 1, 1, &pCondition);
			// ���öϵ���ж�����
			if (pCondition != 0)
			{
				pBp->SetCondition(pCondition);
			}
			else
				pBp->SetCondition(true);
			pDbg->FinishBreakpointEvent();
			break;
		}
		/*��������*/
		case 'p':
		{
			BPObject *pBp = nullptr;

			// �жϵ�ǰ�Ƿ���callָ��
			pDbg->GetRegInfo(ct);
			SIZE_T uEip = ct.Eip;
			BYTE c[2] = { 0 };
			pDbg->ReadMemory(uEip, c, 2);
			DWORD dwCodeLen = 5;
			/**
			��* call �Ļ�������:
			 ��* 0xe8 : 5byte,
			  ��* 0x9a : 7byte,
			   ��* 0xff :
				��*	 0x10ff ~ 0x1dff
				 ��* rep ǰ׺��ָ��Ҳ���Բ���
				  ��*/
			if (c[0] == 0xe8/*call*/
				|| c[0] == 0xf3/*rep*/
				|| c[0] == 0x9a/*call*/
				|| (c[0] == 0xff && 0x10 <= c[1] && c[1] <= 0x1d)/*call*/
				)
			{
				dwCodeLen = disAsm.GetCodeLen(uEip);

				pBp = pDbg->AddBreakpoint(uEip + dwCodeLen, breakpointType_soft);

			}
			else
				pBp = pDbg->AddBreakpoint(0, breakpointType_tf);
			// ����ϵ�û�����ӳɹ�1
			if (pBp == nullptr)
			{
				pDbg->FinishBreakpointEvent();
				break;
			}
				
			// ��ȡ����
			char* pCondition = SkipSpace(pCmdLine + 1);
			if (*pCondition != 0)
			{
				pBp->SetCondition(pCondition);
			}
			else
				pBp->SetCondition(true);

			pDbg->FinishBreakpointEvent();
			break;
		}
		/*���öϵ�*/
		case 'b':
		{
			dwStatus = 1;
			SetBreakpoint(pDbg, &ui, pCmdLine, &disAsm);
			break;
		}
		/*���г���*/
		case 'g':
		{
			pDbg->FinishBreakpointEvent();
			break;
		}

		/*���*/
		case 'c':
		{
			PluginInit();
			break;
		}

		/*�鿴����*/
		case 'h':
		{
			dwStatus = 1;
			ShowHelp();
			break;
		}
		/*�鿴�����*/
		case 'i':
		{
			GetPEImpTab(pDosH);
			cout << "������Ҫ�鿴�ڼ���DLL��" << endl;
			int numIndex;
			cin >> numIndex;
			ShowImpFunc(numIndex);
			break;
		}
		/*�鿴������*/
		case 'o':
		{
			GetPEExpTab(pDosH);
			ShowExpFunc();
			break;
		}
		}
	}


	return 0;
}


// ��ʾ������Ϣ
void ShowHelp()
{
	printf("----------------------------------------------------\n");
	printf("h : �鿴����\n");
	// ��������
	// ��ʾ/�޸Ļ�����
	printf("u : �鿴�����\n");
	printf("    ��ʽΪ : u ��ʼ��ַ ָ������\n");
	//printf("    ����   : u eip\n");
	//printf("    ����   : u eax 100\n");
	//printf("    ����   : u 0x401000 100\n");
	printf("a : �޸Ļ�����\n");
	printf("    ��ʽΪ : a ��ʼ��ַ����ַ��ʽ��0x���֣�\n");
	printf("    ����quit�˳�������༭\n");
	// �鿴/�޸��ڴ����ݣ��鿴ջ
	printf("d : �鿴�ڴ�����\n");
	printf("    ��ʽΪ : d ��ʼ��ַ\n");
	//printf("    ��ʽΪ : da ��ʼ��ַ(��ʾ�ַ���ʱʹ��ANSIII�ַ�)\n");
	//printf("    ��ʽΪ : du ��ʼ��ַ(��ʾ�ַ���ʱʹ��Unicode�ַ�)\n");
	//printf("    ��ʽΪ : dp DUMP����\n");
	printf("x : �޸��ڴ�����\n");
	printf("    ��ʽΪ : x ��ʼ��ַ(��������ֱ�������޸�ֵ)\n");
	printf("k : �鿴ջ\n");
	// �鿴/�޸ļĴ���
	printf("r : �鿴/�޸ļĴ���\n");
	printf("    �鿴��ʽΪ : r �Ĵ�����\n");
	printf("    �޸ļĴ��� r eax = 0x1000\n");
	// �鿴���ó���ģ����Ϣ
	printf("ml: �鿴���Գ���ģ����Ϣ\n");
	// �ϵ㹦��
	printf("b : ���öϵ�\n");
	printf("    ��ʽ:\n");
	printf("    bp ��ַ =>�����ϵ�\t");
	//printf("    ����: bp 0x401000 eax==0 && byte[0x403000]==97\n");
	printf("    bh ��ַ =>Ӳ���ϵ�\t");
	//printf("    ����: bh 0x401000 e \n");
	printf("    bm ��ַ =>�ڴ�ϵ�\n");
	//printf("    ����: bm 0x401000 e \n");
	printf("    bl �г����жϵ�\n");
	//printf("    bc ��� ɾ��ָ����ŵĶϵ�\n");
	printf("t : ��������\n");
	printf("p : ��������\n");
	printf("g : ���г���\n");
	// ���ӹ���
	printf("i : �鿴�����Գ���ĵ����\n");
	printf("o : �鿴�����Գ���ĵ�����\n");
	printf("c : ���ز��\n");
	//printf("exit: �˳����ԻỰ\n");
	printf("----------------------------------------------------\n");

}

// ��ȡ�ڶ�������(����֮���Կո�����
char* GetSecondArg(char* pBuff)
{
	for (; *pBuff != 0; ++pBuff)
	{
		if (*pBuff == ' ')//�ҵ���һ���ո�
		{
			*pBuff = 0; // �ѿո����ַ���������,�ָ���������
			return pBuff + 1;//���صڶ��������Ŀ�ʼ��ַ
		}
	}
	return pBuff;
}

// �����ո�(�������з�,tab��)
inline char* SkipSpace(char* pBuff)
{
	for (; *pBuff == ' ' || *pBuff == '\t' || *pBuff == '\r' || *pBuff == '\n'; ++pBuff);
	return pBuff;
}


void GetCmdLineArg(char* pszCmdLine, int nArgCount, ...)
{
	va_list argptr;
	va_start(argptr, nArgCount);

	while (nArgCount-- > 0 && *pszCmdLine != 0)
	{
		if (*pszCmdLine == ' ' || *pszCmdLine == '\t')
			*pszCmdLine++ = 0;

		pszCmdLine = SkipSpace(pszCmdLine);
		if (*pszCmdLine == 0)
			break;
		DWORD*& dwArg = va_arg(argptr, DWORD*);
		*dwArg = (DWORD)pszCmdLine;

		for (; *pszCmdLine != 0 && *pszCmdLine != ' ' && *pszCmdLine != '\t'; ++pszCmdLine);
	}
	va_end(argptr);
}

// ���öϵ�
void SetBreakpoint(DbgEngine* pDbg, DbgUi* pUi, char* szCmdLine, DisAsmEngine* disAsm)
{
	char* pAddr = 0;//�ϵ��ַ
	char* pType = 0;//�ϵ�����
	char* pLen = 0;//�ϵ㳤��
	char* pRule = 0; // �ϵ����й���
	Expression exp(pDbg);

	char  cType = *(SkipSpace(szCmdLine + 1));// �ϵ�����
	E_BPType   bpType = e_bt_none;
	SIZE_T   uAddr = 0; // �¶ϵ�ַ
	uint		uBPLen = 1;
	switch (cType)
	{
	case 'p':/*��ͨ�ϵ�*/
	{
		char* pAddrr = 0;
		bpType = breakpointType_soft;
		GetCmdLineArg(szCmdLine + 2, 2, &pAddrr, &pRule);
		if (pAddrr == nullptr)
			pAddrr = "eip";

		// �õ���ַ
		uAddr = exp.GetValue(pAddrr);
		// �õ�����
		uBPLen = disAsm->GetCodeLen(uAddr);
		break;
	}
	case 'l':/*�ϵ��б�*/
	{
		pUi->ShowBreakPointList(pDbg->GetBPListBegin(), pDbg->GetBPListEnd());
		return;
	}
	case 'c':/*ɾ���ϵ�*/
	{
		DWORD	dwIndex = 0;
		sscanf_s(szCmdLine + 2, "%d", &dwIndex);

		pDbg->DeleteBreakpoint(dwIndex);

		return;
	}

	//Ӳ���ϵ��������m��֧����
	case 'h':/*Ӳ���ϵ�*/
	case 'm': /*�ڴ���ʶϵ�*/
	{
		GetCmdLineArg(szCmdLine + 2, 4, &pAddr, &pType, &pLen, &pRule);
		if (pAddr == 0 || pType == 0)
		{
			printf("bm/bh ��ַ ����(r/w/e) ����(1/2/4)(��ѡ) ����(��ѡ)\n");
			return;
		}

		uAddr = exp.GetValue(pAddr);
		switch (*pType) // ɸѡ�ϵ������
		{
		case 'r':bpType = cType == 'm' ? breakpointType_acc_r : breakpointType_hard_r; break;
		case 'w':bpType = cType == 'm' ? breakpointType_acc_w : breakpointType_hard_w; break;
		case 'e':bpType = cType == 'm' ? breakpointType_acc_e : breakpointType_hard_e; break;
		default:
			printf("�ϵ��������ô���,���ʶϵ��������: r(��),w(д),e(ִ��)\n");
			return;
		}

		uBPLen = exp.GetValue(pLen);
		if (uBPLen == 0)
			uBPLen = 1;

		if (pLen == 0 && cType == 'h') //�����Ӳ���ϵ�,�򽫳�����Ϊ0
			pLen = "0";
		else if (pLen > 0 && cType == 'h') // ���Ӳ���ϵ��ַ�ͳ��ȵĶ�Ӧ��ϵ
		{
			if (*pType == 'e') // �����ִ�жϵ�,����ֻ��Ϊ0
				uBPLen = 0;
			else // ����Ƕ�д�ϵ�,�ϵ��ַ�ͳ��ȱ��������Ӧ��ϵ
			{
				// Ĭ�ϳ���Ϊ4���ֽ�
				uBPLen = 3;
				// ���������4 , ����ַ������4�ı���,��ϵ㳤�������2���ֽ�
				if (*pLen == '4' && uAddr % 4 != 0)
					uBPLen = 1;
				// ���������2 , ����ַ������2�ı���,��ϵ㳤�������1���ֽ�
				if (*pLen == '2' && uAddr % 2 != 0)
					uBPLen = 0;
			}
		}
		break;
	}
	default:
		cout << "û�и����͵Ķϵ�\n";
		return;
	}

	// ��ȡ��ϵ�ĵ�ַ,����,������, �����¶�.
	BPObject* pBp = pDbg->AddBreakpoint(uAddr, bpType, uBPLen);
	if (pBp == nullptr)
	{
		printf("���öϵ�ʧ��\n");
		return;
	}

	// ����ϵ�Я������ʽ, ��ѱ���ʽ���õ��ϵ���
	if (pRule != nullptr)
		BreakpointEngine::SetExp(pBp, pRule);
	return;
}
