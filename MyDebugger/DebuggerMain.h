#pragma once
#include "Windows.h"
#include "TlHelp32.h"
#include <atlstr.h>

extern HANDLE g_hProcess;  //被调试进程的句柄
extern HANDLE g_hThread;   //被调试线程句柄
extern PIMAGE_DOS_HEADER pDosH;   //pe文件dos头
extern char* buff;   //文件缓冲
extern CString file_Name;   //文件名
extern unsigned int threadId;//被调试线程ID
extern CONTEXT context1;//被调试线程模块上下文
extern unsigned int imgBase;//进程加载基址
