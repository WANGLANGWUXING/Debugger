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
	SIZE_T	address;      // ��ַ
	DWORD	dwCodeLen;    // ָ���
	CString	strOpCode;    // ����ָ��
	CString	strAsm;       // ������
	CString strCom;       // ע��
}DISASMSTRUST;
#endif
// Ϊ����ģ�鱣�淴�����Ϣ,��ַ�ͷ��������ӳ���ϵ
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
	// ��ȡԶ�̽���ָ����ַ��ָ��ĳ���
	int GetCodeLen(const SIZE_T lpAddress);


	// ������Զ�̽��̵�ָ����ַ����һ�����ָ��
	int DiAsm(const SIZE_T lpAddress, CString& strOpCode, CString& strAsm, CString & strCom);
	int DiAsm(const SIZE_T lpAddress, vector<DISASMSTRUST>& vecAsm, DWORD dwLine);


	// ��ȡ��ǰ��ַ��ǰx�еķ����ָ���ַ
	void AnalysisDisAsm();
	bool GetDisAsm(const LPBYTE pData,
		LONG32 begAddr,
		LONG32 size,
		LONG32 addr,
		map<LONG32,
		DISASMSTRUST>& mapDisAsm);
};

