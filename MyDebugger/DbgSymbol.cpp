#include "DbgSymbol.h"



DbgSymbol::DbgSymbol()
{
}


DbgSymbol::~DbgSymbol()
{
}

void DbgSymbol::InitSymbol(HANDLE hProcess)
{
	// ��õ�ǰDbgHelp��ѡ������
	DWORD dwOptions = SymGetOptions();
	// SYMOPT_DEBUG: ͨ��OutputDebugString�� SymRegisterCallbackProc64�ص��������ݵ������
	dwOptions |= SYMOPT_DEBUG;
	// ����DbgHelp��ѡ��
	::SymSetOptions(dwOptions);
	// ��ʼ�����̵ķ��Ŵ������
	::SymInitialize(
		hProcess, // ���ڱ�ʶ�����ߵľ�� 
		0,        // �������������ļ�������˲���ΪNULL����Ⳣ�Դ�����Դ�γɷ���·����
		          //     Ӧ�ó���ĵ�ǰ����Ŀ¼
		          //     _NT_SYMBOL_PATH��������
		          //     _NT_ALTERNATE_SYMBOL_PATH��������
		TRUE      // �����ֵΪTRUE����ö�ٽ��̵��Ѽ���ģ�飬��Ϊÿ��ģ����Ч�ص��� SymLoadModule64������
	);
}

SIZE_T DbgSymbol::FindApiAddress(HANDLE hProcess, const char * pszName)
{
	DWORD64 dwDisplacement = 0;
	// SYMBOL_INFO ������Ϣ�ṹ��  MAX_SYM_NAME �������ļ�������
	// �洢��������
	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO); // �ṹ�Ĵ�С�����ֽ�Ϊ��λ��
	pSymbol->MaxNameLen = MAX_SYM_NAME;          // Name�������Ĵ�С�����ַ�Ϊ��λ��
	// ����ָ�����Ƶķ�����Ϣ��
	if (!SymFromName(hProcess, pszName, pSymbol))
		return 0;
	return (SIZE_T)pSymbol->Address;

}

BOOL DbgSymbol::GetFunctionName(HANDLE hProcess, SIZE_T nAddress, CString & strName)
{
	// ���ſ�ʼ��ƫ��
	DWORD64 dwDisplacement = 0;
	// �洢��������
	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO); // �ṹ�Ĵ�С�����ֽ�Ϊ��λ��
	pSymbol->MaxNameLen = MAX_SYM_NAME;          // Name�������Ĵ�С�����ַ�Ϊ��λ��
	// ����ָ����ַ�ķ�����Ϣ��
	if (!SymFromAddr(hProcess, nAddress, &dwDisplacement, pSymbol))
		return FALSE;
	strName = pSymbol->Name;
	return TRUE;
}
