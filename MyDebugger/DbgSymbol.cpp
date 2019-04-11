#include "DbgSymbol.h"



DbgSymbol::DbgSymbol()
{
}


DbgSymbol::~DbgSymbol()
{
}

void DbgSymbol::InitSymbol(HANDLE hProcess)
{
	// 获得当前DbgHelp的选项设置
	DWORD dwOptions = SymGetOptions();
	// SYMOPT_DEBUG: 通过OutputDebugString或 SymRegisterCallbackProc64回调函数传递调试输出
	dwOptions |= SYMOPT_DEBUG;
	// 设置DbgHelp的选项
	::SymSetOptions(dwOptions);
	// 初始化进程的符号处理程序。
	::SymInitialize(
		hProcess, // 用于标识调用者的句柄 
		0,        // 用于搜索符号文件。如果此参数为NULL，则库尝试从以下源形成符号路径：
		          //     应用程序的当前工作目录
		          //     _NT_SYMBOL_PATH环境变量
		          //     _NT_ALTERNATE_SYMBOL_PATH环境变量
		TRUE      // 如果此值为TRUE，则枚举进程的已加载模块，并为每个模块有效地调用 SymLoadModule64函数。
	);
}

SIZE_T DbgSymbol::FindApiAddress(HANDLE hProcess, const char * pszName)
{
	DWORD64 dwDisplacement = 0;
	// SYMBOL_INFO 符号信息结构体  MAX_SYM_NAME 最大符号文件名长度
	// 存储符号名称
	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO); // 结构的大小，以字节为单位。
	pSymbol->MaxNameLen = MAX_SYM_NAME;          // Name缓冲区的大小，以字符为单位。
	// 检索指定名称的符号信息。
	if (!SymFromName(hProcess, pszName, pSymbol))
		return 0;
	return (SIZE_T)pSymbol->Address;

}

BOOL DbgSymbol::GetFunctionName(HANDLE hProcess, SIZE_T nAddress, CString & strName)
{
	// 符号开始的偏移
	DWORD64 dwDisplacement = 0;
	// 存储符号名称
	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO); // 结构的大小，以字节为单位。
	pSymbol->MaxNameLen = MAX_SYM_NAME;          // Name缓冲区的大小，以字符为单位。
	// 检索指定地址的符号信息。
	if (!SymFromAddr(hProcess, nAddress, &dwDisplacement, pSymbol))
		return FALSE;
	strName = pSymbol->Name;
	return TRUE;
}
