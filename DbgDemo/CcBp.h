#pragma once
#include "windows.h"

class CCcBp
{
public:
	CCcBp();
	~CCcBp();
public:
	BOOL SetBsBreakPoint(DWORD dwAddr, HANDLE hProcess);
	BOOL RemoveBsBreakPoint(DWORD dwAddr, HANDLE hProcess);
private:
	BYTE  m_oldByte;//�¶ϵ�λ��ԭ�������ݣ��ָ���
};

