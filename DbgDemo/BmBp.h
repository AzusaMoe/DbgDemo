#pragma once
#include "windows.h"
class CBmBp
{
public:
	CBmBp();
	~CBmBp();
	BOOL SetBmBreakPoint(DWORD dwAddr, HANDLE hProcess);
	BOOL RemoveBmBreakPoint(HANDLE hProcess);
	BOOL ReSetBmBreakPoint(HANDLE hProcess);
private:
	DWORD m_dwOldProtect;//��¼ԭ�ȵ��ڴ��ҳ��������
	DWORD m_dwAddr;//��¼Ҫ���õ��ڴ���ʶϵ��ַ
};

