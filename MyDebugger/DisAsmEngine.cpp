#include "DisAsmEngine.h"


DisAsmEngine::DisAsmEngine(BreakpointEngine * pBPEngine)
	:m_BPEngine(pBPEngine)
{
	//m_BPEngine->InitSymbol(pBPEngine->m_hCurrProcess);
}

DisAsmEngine::~DisAsmEngine()
{
}

// ��ȡָ���	
int DisAsmEngine::GetCodeLen(const SIZE_T lpAddress)
{
	// 1. �����Գ�����ڴ�(OPCode)���Ƶ�����
	SIZE_T  dwRetSize = 0;
	LPBYTE  lpRemote_Buf[32];
	ZeroMemory(lpRemote_Buf, 32);
	ReadProcessMemory(m_BPEngine->m_hCurrProcess,
		(LPVOID)lpAddress,
		lpRemote_Buf,
		32,
		&dwRetSize);

	// 2. ��ʼ�����������
	DISASM objDiasm;
	objDiasm.EIP = (UIntPtr)lpRemote_Buf;      // ��ʼ��ַ
	objDiasm.VirtualAddr = (UINT64)lpAddress;  // �����ڴ��ַ��������������ڼ����ַ��
	objDiasm.Archi = 0;                        // AI-X86
	objDiasm.Options = 0x000;                  // MASM

	// 3. ��������
	int nLen = Disasm(&objDiasm);
	return nLen;
}


void byteArr2HexStr(const LPBYTE& lpbArr, DWORD dwBSize, char* pszHexStr, const char szSpace)
{
	// һ���ֽ�ת����һ��TCHAR
	DWORD	i = 0;
	TCHAR	ucNum[3] = { 0 };
	BYTE	byteNum = 0;
	DWORD	dwIndex = szSpace == 0 ? 2 : 3;
	DWORD	j = 0;
	while (j < dwBSize)
	{
		byteNum = *((PBYTE)(lpbArr + j));

		// ת���ַ���
		sprintf_s(pszHexStr + i, 3 + 1, "%02X%c", byteNum, szSpace);
		i += dwIndex;
		++j;
	}
}
// ��ȡopcode�ͻ��ָ��
int DisAsmEngine::DiAsm(const SIZE_T lpAddress, CString & strOpCode, CString & strAsm, CString & strCom)
{
	strOpCode.Empty();// ���
	strAsm.Empty();// ���
	strCom.Empty();// ���

				   // 1. �����Գ�����ڴ�(OPCode)���Ƶ�����
	BYTE lpRemote_Buf[64] = { 0 };

	if (m_BPEngine->ReadMemory(lpAddress, lpRemote_Buf, 64) != 64)
		return -1;

	// ����ϵ�����0xcc,������ȷ��opcode,
	// �ж������ַ����û������ϵ�,����ԭ�������ݱ��浽�����������ֽ�
	// �����޷����������
	BPObject* pBP = m_BPEngine->FindBreakpoint((uaddr)lpAddress, breakpointType_soft);
	if (pBP != NULL)
	{
		*lpRemote_Buf = *(char*)&((BPSoft*)pBP)->m_uData;
	}

	// 2. ��ʼ�����������
	DISASM objDiasm;
	objDiasm.EIP = (UIntPtr)lpRemote_Buf; // ��ʼ��ַ
	objDiasm.VirtualAddr = (UINT64)lpAddress;// �����ڴ��ַ��������������ڼ����ַ��
	objDiasm.Archi = 0;                   // AI-X86
	objDiasm.Options = 0x000;             // MASM

										  // 3. ��������
	UINT unLen = Disasm(&objDiasm);
	if (-1 == unLen)
		return unLen;

	// 4. ��������ת��Ϊ�ַ���
	strOpCode.GetBufferSetLength(unLen*2 + 1);/*һ���ֽڵõ������ַ��������ַ���������*/
	char * szTemp = new char[strOpCode.GetLength() + 1];
	strcpy_s(szTemp, strOpCode.GetLength() + 1, strOpCode);

	byteArr2HexStr((LPBYTE)lpRemote_Buf, unLen, szTemp, 0);
	strOpCode = szTemp;
	delete szTemp;
	// 6. ���淴������ָ��
	strAsm = objDiasm.CompleteInstr;
	
	// �鿴ָ���Ƿ�����˺���, ��������˺���, ��ʹ�õ��Է��ŷ�����������������
	// call �Ļ������� :
	//  0xe8	: 5byte ,
	//  0x15ff	: 6byte
	// jmp �Ļ�������:
	//	0xe9	: 5byte,
	//  0x25ff	: 6byte
	SIZE_T uDesAddress = 0;
	if (*lpRemote_Buf == 0xe8 || *lpRemote_Buf == 0xe9)
	{
		// ��ȡ��תƫ��,��ȡ��ǰָ���ַ, �����Ŀ���ַ
		// ���㹫ʽ: ƫ�� = Ŀ���ַ - ��ǰ��ַ - 5
		DWORD dwDispAddr = *(DWORD*)(lpRemote_Buf + 1);
		// Ŀ���ַ = ƫ�� + ��ǰ��ַ + 5
		uDesAddress = dwDispAddr + 5 + lpAddress;
	}
	else if (*(WORD*)lpRemote_Buf == 0x15ff || *(WORD*)lpRemote_Buf == 0x25ff)
	{
		uDesAddress = *(DWORD*)(lpRemote_Buf + 2);
	}
	if (uDesAddress)
	{
		m_BPEngine->GetFunctionName(m_BPEngine->m_hCurrProcess,
			uDesAddress,
			strCom);
	}

	return unLen;
}
// ��ָ����ַ��opcode�����,���浽������vector��
int DisAsmEngine::DiAsm(const SIZE_T lpAddress, vector<DISASMSTRUST>& vecAsm, DWORD dwLine)
{
	TCHAR szOpCode[MAX_OPCODESTRING] = { 0 };
	TCHAR szAsm[MAX_OPCODESTRING] = { 0 };
	DISASMSTRUST	stcDeAsm = { 0 };
	vecAsm.clear();
	SIZE_T	address = (SIZE_T)lpAddress;

	vecAsm.resize(dwLine);
	for (DWORD i = 0; i < dwLine; ++i)
	{
		vecAsm[i].dwCodeLen = DiAsm(address,
			vecAsm[i].strOpCode,
			vecAsm[i].strAsm,
			vecAsm[i].strCom
		);
		vecAsm[i].address = address;
		if (vecAsm[i].dwCodeLen == -1)
		{
			return vecAsm[i].dwCodeLen;
		}
		address += vecAsm[i].dwCodeLen;
	}
	return TRUE;
}
// ���������
void DisAsmEngine::AnalysisDisAsm()
{
	// ��ȡģ�����
	list<MODULEFULLINFO> moduleInfo;
	m_BPEngine->GetModuleList(moduleInfo);

	m_moduleInfo.resize(moduleInfo.size());

	// ��ȡÿһ��ģ��ķ����
	// ��������ַ�ͷ��������ӳ���ϵ
	// 
	// ���õݹ��½�����opcode
	list<MODULEDISASM>::iterator itr = m_moduleInfo.begin();
	BYTE*	pHdrData = new BYTE[0x200];
	PIMAGE_SECTION_HEADER pScnHdr = nullptr;
	BYTE*   pCode = nullptr;
	for (auto& i : moduleInfo)
	{
		itr->uImgBase = (LONG32)i.uStart;
		itr->uSize = i.uSize;

		if (0x200 != m_BPEngine->ReadMemory(itr->uImgBase, pHdrData, 0x200))
			goto _ERROR;

		// ��ȡ���������
		pScnHdr = GetPEFirScnHdr(pHdrData);
		for (; !(pScnHdr->Characteristics&IMAGE_SCN_MEM_EXECUTE); ++pScnHdr);
		if (pScnHdr->Characteristics&IMAGE_SCN_MEM_EXECUTE)
		{
			pCode = new BYTE[pScnHdr->SizeOfRawData];
			if (pScnHdr->SizeOfRawData
				!= m_BPEngine->ReadMemory(pScnHdr->VirtualAddress, pCode, pScnHdr->SizeOfRawData))
				goto _ERROR;

			// ��ȡ������ڵ�ַ
			LONG32 oep = GetPEOptHdr32(pHdrData)->AddressOfEntryPoint + itr->uImgBase;

			// ��ʼ�ݹ��½�����

			delete[] pCode;
		}
	}


	goto _SUCESS;
_ERROR:
	for (auto& i : m_moduleInfo)
		i.mapDisAsm.clear();
	m_moduleInfo.clear();

_SUCESS:
	if (pHdrData != nullptr)
		delete[] pHdrData;
	if (pCode != nullptr)
		delete[] pCode;
}

bool DisAsmEngine::GetDisAsm(const LPBYTE pData, LONG32 begAddr, LONG32 size, LONG32 addr, map<LONG32, DISASMSTRUST>& mapDisAsm)
{
	// �ж�opcode�Ƿ���callָ��,jmpָ��,jccָ��
	// call ָ����ֽ�����:
	//  0xe8 : 5byte ,
	// 	*0xff :
	// 
	LONG32	dwDisp = 0;
	DISASMSTRUST	disAsmInfo;
	DWORD	dwNextCodeAddress = addr;
	if (*pData == 0xe8 || *pData == 0xe9) // �������úͶ���ת
	{
		// ������ָ��������,������ת���ߵ��õ�Ŀ���ַ�����������
		// �����Ŀ���ַ�ĵ�ַ
		// ��ʽ: 
		//  Ŀ���ַ  = ��תƫ�� + ��ǰָ���ַ + 5
		dwDisp = *(LONG32*)(pData + 1);
		dwNextCodeAddress += dwDisp;
	}
	else if (*(WORD*)pData == 0x15ff || *(WORD*)pData == 0x25ff)
	{
		dwDisp = *(LONG32*)(pData + 2);
	}

	disAsmInfo.address = addr;
	disAsmInfo.dwCodeLen = DiAsm(addr,
		disAsmInfo.strOpCode,
		disAsmInfo.strAsm,
		disAsmInfo.strCom
	);
	pair<map<LONG32, DISASMSTRUST>::iterator, bool> ret =
		mapDisAsm.insert(pair<LONG32, DISASMSTRUST>(addr, disAsmInfo));
	if (ret.second == false)
		return false;

	GetDisAsm(pData + disAsmInfo.dwCodeLen,
		begAddr,
		size,
		dwNextCodeAddress + disAsmInfo.dwCodeLen,
		mapDisAsm);

	return 0;
}
