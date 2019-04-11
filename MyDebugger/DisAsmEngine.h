#pragma once
#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine_4.1/Win32/headers/BeaEngine.h"


#ifndef _WIN64
#pragma comment(lib, "BeaEngine_4.1/Win32/Win32/Lib/BeaEngine.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"crt.lib\"")
#else
#pragma comment(lib, "../BeaEngine_4.1/Win64/Win64/Lib/BeaEngine64.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"crt64.lib\"")
#endif

#include <list>
#include <map>
#include <vector>
#include <strsafe.h>
#include "BreakpointEngine.h"
#include "BPSoft.h"
#include "Pectrl.h"
using std::list;
using std::map;
using std::pair;
using std::vector;
#define	MAX_OPCODESTRING	64

#ifndef DEASM_STRUCT
#define DEASM_STRUCT 
typedef	struct DISASMSTRUST
{
	SIZE_T	address;      // 地址
	DWORD	dwCodeLen;    // 指令长度
	CString	strOpCode;    // 机器指令
	CString	strAsm;       // 汇编代码
	CString strCom;       // 注释
}DISASMSTRUST;
#endif
// 为各个模块保存反汇编信息,地址和反汇编代码的映射关系
typedef struct MODULEDISASM
{
	LONG32	uImgBase;
	LONG32	uSize;
	map<LONG32, DISASMSTRUST> mapDisAsm;
}MODULEDISASM, *PMODULEDISASM;

class DisAsmEngine
{
	list<MODULEDISASM> m_moduleInfo;
	BreakpointEngine* m_BPEngine;
public:
	DisAsmEngine(BreakpointEngine* pBPEngine);
	~DisAsmEngine();
public:
	// 获取远程进程指定地址的指令的长度
	int GetCodeLen(const SIZE_T lpAddress);


	// 解析出远程进程的指定地址出的一条汇编指令
	int DiAsm(const SIZE_T lpAddress, CString& strOpCode, CString& strAsm, CString & strCom);
	int DiAsm(const SIZE_T lpAddress, vector<DISASMSTRUST>& vecAsm, DWORD dwLine);


	// 获取当前地址的前x行的反汇编指令地址
	void AnalysisDisAsm();
	bool GetDisAsm(const LPBYTE pData,
		LONG32 begAddr,
		LONG32 size,
		LONG32 addr,
		map<LONG32,
		DISASMSTRUST>& mapDisAsm);
};

