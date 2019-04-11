#include "Expression.h"


Expression::Expression(DbgObject * pDbgObj)
	:m_pDbgObj(pDbgObj)
{
}

Expression::~Expression()
{
}

/*
   ����BYTE/WORD/DWORD/QWORD [ ʱ,�Ȼ�ȡ[]���ʽ�е�ֵ
	�����Ĵ���,��ȡ�Ĵ�����ֵ.
	���ȼ���: 0~9, ��ֵԽ�����ȼ�Խ��.
	ÿ�κ����ĵ��ö��ᴫ��һ�����ȼ���ֵ,�����ȼ����߼���֧
	���ܴ�������ȼ�����Ĳ���
*/

// �����ո���Ʊ��
const char *SkipSpace(const char*p)
{
	for (; *p == ' ' || *p == '\t'; p++);
	return p;
}


bool IsReg(const char* pStr, const char** pEnd)
{
	if (*pStr == 'e')
	{
		switch (*((WORD*)(pStr + 1)))
		{

		case 'xa': // eax
		case 'xc': // ecx
		case 'xd': // edx
		case 'xb': // ebx
		case 'is': // esi
		case 'id': // edi
		case 'ps': // esp
		case 'pb': // ebp
		case 'pi': // eip
			if (pEnd)
			{
				*pEnd = pStr + 3;
			}
			return true;
		}
	}
	else
	{
		switch (*((WORD*)(pStr + 1)))
		{

		case 'xa': // ax
		case 'xc': // cx
		case 'xd': // dx
		case 'xb': // bx
		case 'is': // si
		case 'id': // di
		case 'ps': // sp
		case 'pb': // bp
		case 'la': // al
		case 'ha': // ah
		case 'lc': // cl
		case 'hc': // ch
		case 'ld': // dl
		case 'hd': // dh
		case 'lb': // bl
		case 'hb': // bh
			if (pEnd)
			{
				*pEnd = pStr + 2;
			}
			return true;
		}
	}
	return false;
}


SSIZE_T Expression::ReadProcessMemory(LPVOID lpAddr, DWORD dwSize)
{
	SSIZE_T nValue = 0;
	if (dwSize == m_pDbgObj->ReadMemory(
		(uaddr)lpAddr, (pbyte)&nValue, dwSize))
		return nValue;
	return 0;
}

bool Expression::ReadRegValue(const char * pReg, const char ** pEnd, SSIZE_T & uRegValue)
{
	// ����Ϊ�վͷ���
	if (pReg == NULL)
		return false;
	CONTEXT ct = { 0 };
	ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
	// ��ȡ�Ĵ�����Ϣʧ�ܾͷ���
	if (!m_pDbgObj->GetRegInfo(ct))
		return false;
	pReg = SkipSpace(pReg);
	if (*pReg == 'e')
	{
		*pEnd = pReg + 3;
		switch (*(WORD*)(pReg + 1))
		{
		case 'xa': // eax
			uRegValue = ct.Eax;
			break;
		case 'xc'://ecx
			uRegValue = ct.Ecx;
			break;
		case 'xd'://edx
			uRegValue = ct.Edx;
			break;
		case 'xb'://ebx
			uRegValue = ct.Ebx;
			break;
		case 'is'://esi
			uRegValue = ct.Esi;
			break;
		case 'id'://edi
			uRegValue = ct.Edi;
			break;
		case 'ps'://esp
			uRegValue = ct.Esp;
			break;
		case 'pb'://ebp
			uRegValue = ct.Ebp;
			break;
		case 'pi':// eip
			uRegValue = ct.Eip;
			break;
		default:
			uRegValue = 0;
			return false;
		}

	}
	else
	{
		*pEnd = pReg + 2;
		switch (*((WORD*)(pReg + 1)))
		{
		case 'xa'://ax
			uRegValue = ct.Eax & 0xFFFF;
			break;
		case 'xc'://cx
			uRegValue = ct.Ecx & 0xFFFF;
			break;
		case 'xd'://dx
			uRegValue = ct.Edx & 0xFFFF;
			break;
		case 'xb'://bx
			uRegValue = ct.Ebx & 0xFFFF;
			break;
		case 'is'://si
			uRegValue = ct.Esi & 0xFFFF;
			break;
		case 'id'://di
			uRegValue = ct.Edi & 0xFFFF;
			break;
		case 'ps'://sp
			uRegValue = ct.Esp & 0xFFFF;
			break;
		case 'pb'://bp
			uRegValue = ct.Ebp & 0xFFFF;
			break;
		case 'la'://al
			uRegValue = ct.Eax & 0x0f;
			break;
		case 'ha'://ah
			uRegValue = ct.Eax & 0xf0;
			break;
		case 'lc'://cl
			uRegValue = ct.Ecx & 0x0f;
			break;
		case 'hc'://ch
			uRegValue = ct.Ecx & 0xf0;
			break;
		case 'ld'://dl
			uRegValue = ct.Edx & 0x0f;
			break;
		case 'hd'://dh
			uRegValue = ct.Edx & 0xf0;
			break;
		case 'lb'://bl
			uRegValue = ct.Ebx & 0x0f;
			break;
		case 'hb'://bh
			uRegValue = ct.Ebx & 0xf0;
			break;
		default:
			uRegValue = 0;
			return false;
		}
	}
	return true;
}

bool Expression::WriteRegValue(const char * pReg, const char ** pEnd, SSIZE_T & uRegValue)
{
	if (pReg == NULL)
		return false;
	CONTEXT ct = { 0 };
	ct.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
	if (!m_pDbgObj->GetRegInfo(ct))
		return false;
	pReg = SkipSpace(pReg);
	if (*pReg == 'e')
	{
		*pEnd = pReg + 3;
		switch (*((WORD*)(pReg + 1)))
		{
		case 'xa'://eax
			ct.Eax = uRegValue;
			break;
		case 'xc'://ecx
			ct.Ecx = uRegValue;
			break;
		case 'xd'://edx
			ct.Edx = uRegValue;
			break;
		case 'xb'://ebx
			ct.Ebx = uRegValue;
			break;
		case 'is'://esi
			ct.Esi = uRegValue;
			break;
		case 'id'://edi
			ct.Edi = uRegValue;
			break;
		case 'ps'://esp
			ct.Esp = uRegValue;
			break;
		case 'pb'://ebp
			ct.Ebp = uRegValue;
			break;
		case 'pi':// eip
			ct.Eip = uRegValue;
			break;
		default:
			0;
			return false;
		}
	}
	else
	{
		*pEnd = pReg + 2;
		switch (*((WORD*)(pReg + 1)))
		{
		case 'xa'://ax
			ct.Eax = uRegValue & 0xFFFF;
			break;
		case 'xc'://cx
			ct.Ecx = uRegValue & 0xFFFF;
			break;
		case 'xd'://dx
			ct.Edx = uRegValue & 0xFFFF;
			break;
		case 'xb'://bx
			ct.Ebx = uRegValue & 0xFFFF;
			break;
		case 'is'://si
			ct.Esi = uRegValue & 0xFFFF;
			break;
		case 'id'://di
			ct.Edi = uRegValue & 0xFFFF;
			break;
		case 'ps'://sp
			ct.Esp = uRegValue & 0xFFFF;
			break;
		case 'pb'://bp
			ct.Ebp = uRegValue & 0xFFFF;
			break;
		case 'la'://al
			ct.Eax = uRegValue & 0x0f;
			break;
		case 'ha'://ah
			ct.Eax = uRegValue & 0xf0;
			break;
		case 'lc'://cl
			ct.Ecx = uRegValue & 0x0f;
			break;
		case 'hc'://ch
			ct.Ecx = uRegValue & 0xf0;
			break;
		case 'ld'://dl
			ct.Edx = uRegValue & 0x0f;
			break;
		case 'hd'://dh
			ct.Edx = uRegValue & 0xf0;
			break;
		case 'lb'://bl
			ct.Ebx = uRegValue & 0x0f;
			break;
		case 'hb'://bh
			ct.Ebx = uRegValue & 0xf0;
			break;
		default:
			0;
			return false;
		}
	}

	if (!m_pDbgObj->SetRegInfo(ct))
		return false;
	return true;
}

bool Expression::GetValue(SSIZE_T & uValue, const char * pExpression, const char ** pEnd, int nPriorty)
{
	// �жϱ��ʽ�����ڴ��ַ�ı��ʽ������ͨ��ֵ���ʽ
	bool bFlag = true;
	pExpression = SkipSpace(pExpression);
	if (*pExpression == 0)
		return 0;

	// �жϱ��ʽ�Ƿ��ڴ�Ѱַ
	// ������ڴ�Ѱַ������ڴ�Ѱַ�ı��ʽ��Ϊ�ӱ��ʽ���ӱ��ʽ��Ҫ������ֵ
	if (*pExpression == '[')
	{
		if (!GetValue(uValue, pExpression + 1, &pExpression, 9))
		{
			bFlag = false;
		}
	}
	// �����ִ�Сд�ıȽ�_strnicmp
	else if (_strnicmp(pExpression, "BYTE", 4) == 0)
	{
		// ��ȡ�ӱ��ʽ��ֵ
		pExpression = SkipSpace(pExpression + 4);
		GetValue(uValue, pExpression, &pExpression, 9);
		if (*pExpression == ']')
		{
			++pExpression;
			// ��ʱ uValue ���ڴ��ַ����ȡ���ڴ��ַ����ֵ
			uValue = ReadProcessMemory((LPVOID)uValue, sizeof(BYTE));
		}
		else
		{
			bFlag = false;
		}
	}
	else if (_strnicmp(pExpression, "WORD", 4) == 0)
	{
		pExpression = SkipSpace(pExpression + 4);
		// ��ȡ�ӱ��ʽ��ֵ
		GetValue(uValue, pExpression, &pExpression, 9);
		if (*pExpression == ']')
		{
			++pExpression;
			// ��ʱ uValue ���ڴ��ַ����ȡ���ڴ��ַ����ֵ
			uValue = ReadProcessMemory((LPVOID)uValue, sizeof(WORD));
		}
		else
		{
			bFlag = false;
		}
	}
	else if (_strnicmp(pExpression, "DWORD", 5) == 5)
	{
		pExpression = SkipSpace(pExpression + 5);
		// ��ȡ�ӱ��ʽ��ֵ
		GetValue(uValue, pExpression, &pExpression, 9);
		if (*pExpression == ']')
		{
			++pExpression;
			// ��ʱ uValue ���ڴ��ַ����ȡ���ڴ��ַ����ֵ
			uValue = ReadProcessMemory((LPVOID)uValue, sizeof(DWORD));
		}
		else 
		{
			bFlag = false;
		}
	}
	else if (_strnicmp(pExpression, "QWORD", 5) == 0)
	{
		pExpression = SkipSpace(pExpression + 5);
		// ��ȡ�ӱ��ʽ��ֵ
		GetValue(uValue, pExpression, &pExpression, 9);
		
		if (*pExpression == ']')
		{
			++pExpression;
			// ��ʱ uValue ���ڴ��ַ����ȡ���ڴ��ַ����ֵ
			uValue = ReadProcessMemory((LPVOID)uValue, sizeof(__int64));
		}
		else
		{
			bFlag = false;
		}
	}
	else if (*pExpression == '0'&&pExpression[1] == 'x')
	{
		// ���ַ���ת��Ϊ������ֵ 
		uValue = strtol(pExpression, (char**)&pExpression, 16);
	}
	else if (IsReg(pExpression, NULL))// �����Ǹ��Ĵ�����ֵ
	{
		const char * pReg = pExpression;
		IsReg(pExpression, &pReg);
		pReg = SkipSpace(pReg);
		if (*pReg == '='&&*(pReg + 1) != '=') // ���Ĵ�����ֵ
		{
			// ��ȡ������
			SSIZE_T nlValue = 0;
			if (GetValue(nlValue, pReg + 1, &pReg, 9))
			{
				uValue = nlValue;
				WriteRegValue(pExpression, &pExpression, nlValue);
				pExpression = pReg;

			}
			else
			{
				bFlag = false;
			}
		}
		else 
		{
			if (!ReadRegValue(pExpression, &pExpression, uValue))
			{
				bFlag = false;
			}
		}
	}

	else if ('0' <= *pExpression && *pExpression <= '9')
	{
		uValue = strtol(pExpression, (char**)&pExpression, 10);
	}
	else if (*pExpression == '+')
	{
		pExpression = SkipSpace(pExpression + 1);
		uValue = GetValue(uValue, pExpression, &pExpression, 0);
	}
	else if (*pExpression == '-')
	{
		pExpression = SkipSpace(pExpression + 1);
		if (GetValue(uValue, pExpression, &pExpression, 0))
		{
			uValue = -uValue;
		}
	}
	else if (*pExpression == '(')//�������ȼ����
	{
		pExpression = SkipSpace(pExpression + 1);
		GetValue(uValue, pExpression, &pExpression, 9);
		pExpression = SkipSpace(pExpression);
		if (*pExpression == ')')
			pExpression = SkipSpace(pExpression + 1);
		else
			bFlag = false;
	}
	else
	{
		bFlag = false;
	}
	if (bFlag == false)
		return false;
	// ��ȡ�����
	pExpression = SkipSpace(pExpression);
	
	if (*pExpression == '+' && nPriorty > 3)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 1, &pExpression, 3))
			return false;
		uValue += right;
	}
	else if (*pExpression == '-'  && nPriorty > 3)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 1, &pExpression, 3))
			return false;
		uValue -= right;
	}
	else if (*pExpression == '*'  && nPriorty > 2)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 1, &pExpression, 2))
			return false;
		uValue *= right;
	}
	else if (*pExpression == '/'  && nPriorty > 2)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 1, &pExpression, 2))
			return false;
		uValue /= right;
	}
	else if (*pExpression == '%'  && nPriorty > 2)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 1, &pExpression, 2))
			return false;
		uValue %= right;
	}
	else if (*pExpression == '<' && nPriorty>9)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 1, &pExpression, 9))
			return false;
		uValue = uValue < right;
	}
	else if (*pExpression == '>'&& nPriorty>9)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 1, &pExpression, 9))
			return false;
		uValue = uValue > right;
	}
	else if (*pExpression == '='&&pExpression[1] == '='&& nPriorty >= 9)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 2, &pExpression, 9))
			return false;
		uValue = uValue == right;
	}
	else if (*pExpression == '&'&&pExpression[1] == '&' && nPriorty >= 9)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 2, &pExpression, 9))
			return false;
		uValue = uValue && right;
	}
	else if (*pExpression == '|'&&pExpression[1] == '|'&& nPriorty >= 9)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 2, &pExpression, 9))
			return false;
		uValue = uValue || right;
	}
	else if (*pExpression == '<'&&pExpression[1] == '='&& nPriorty >= 9)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 2, &pExpression, 9))
			return false;
		uValue = uValue <= right;
	}
	else if (*pExpression == '>'&&pExpression[1] == '='&& nPriorty >= 9)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 2, &pExpression, 9))
			return false;
		uValue = uValue >= right;
	}
	else if (*pExpression == '!'&&pExpression[1] == '='&& nPriorty >= 9)
	{
		// ��ȡ�ڶ���������
		SSIZE_T right = 0;
		if (!GetValue(right, pExpression + 2, &pExpression, 9))
			return false;
		uValue = uValue != right;
	}
	*pEnd = pExpression;
	return bFlag;
}


SSIZE_T Expression::GetValue(const char * pExpression)
{
	if (pExpression == nullptr)
		return 0;
	const char* pEnd = pExpression;
	SSIZE_T uValue = 0;
	if (GetValue(uValue, pExpression, &pEnd, 9))
		return uValue;
	return 0;
}
