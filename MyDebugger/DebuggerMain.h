#pragma once
#include "Windows.h"
#include "TlHelp32.h"
#include <atlstr.h>

extern HANDLE g_hProcess;  //�����Խ��̵ľ��
extern HANDLE g_hThread;   //�������߳̾��
extern PIMAGE_DOS_HEADER pDosH;   //pe�ļ�dosͷ
extern char* buff;   //�ļ�����
extern CString file_Name;   //�ļ���
extern unsigned int threadId;//�������߳�ID
extern CONTEXT context1;//�������߳�ģ��������
extern unsigned int imgBase;//���̼��ػ�ַ
