#pragma once
#include <Windows.h>
#include <list>
#include <atlstr.h>
#include <Psapi.h>
using std::list;
/**
* ���Զ���
* 1. ���ػ�ַ
* 2. PID
* 3. TID
* 4. hProcess
* 5. hThread
* 6. OEP
*/
//�Զ����������� 
typedef unsigned int uint;
typedef unsigned int uaddr;
typedef unsigned char byte, *pbyte;

// ģ����Ϣ �����ƣ�ģ����ʼλ�ã�ģ���С��
typedef struct _MODULE_FULL_INFO
{
	CStringA name;
	LONG64 uStart;
	LONG32 uSize;
}MODULEFULLINFO,*PMODULEFULLINFO;

class DbgObject
{
public:
	DbgObject();
	~DbgObject();
public:
	uaddr         m_imgBase;       // ���ػ�ַ
	uint          m_oep;		   // ������ڵ�ַ
	uint          m_pid;		   // ����id
	uint          m_tid;		   // �߳�id
	HANDLE		  m_hCurrProcess;  // ��ǰ�����쳣�Ľ��̾��
	HANDLE		  m_hCurrThread;   // ��ǰ�����쳣���߳̾��
	list<HANDLE>  m_hProcessList;  // ���̾����
	list<HANDLE>  m_hThreadList;   // �߳̾����
public:
	// �򿪽��̵���
	bool Open(const char* pszFile);
	// ���ӽ��̵���
	bool Open(const uint uPid);
	// ���Խ����Ƿ�򿪣�����Ƿ񲻵���0��
	bool IsOpen();
	// ���Խ��̾���Ƿ�رգ�����Ƿ����0��
	bool IsClose();
	// �������Խ���
	void Close();


	// ��ȡ�ڴ�
	uint ReadMemory(uaddr uAddress, pbyte pBuff, uint uSize);
	// д���ڴ�
	uint WriteMemory(uaddr uAddress, const pbyte pBuff, uint uSize);

	// ��ȡ�Ĵ�����Ϣ
	bool GetRegInfo(CONTEXT& ct);
	// ���üĴ�����Ϣ
	bool SetRegInfo(CONTEXT& ct);

	// ����µ��̵߳��߳��б�
	void AddThread(HANDLE hThread);
	// ����µĽ��̵������б�
	void AddProcess(HANDLE hProcess, HANDLE hThread);
	// ���߳��б����Ƴ�һ���߳�
	bool RemoveThread(HANDLE hThread);
	// �ӽ����б����Ƴ�һ������
	bool RemoveProcess(HANDLE hProcess);

	//��ȡģ���б�
	void GetModuleList(list<MODULEFULLINFO>& moduleList);
}; 

