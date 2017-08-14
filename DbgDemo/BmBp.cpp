#include "stdafx.h"
#include "BmBp.h"


CBmBp::CBmBp()
{
}


CBmBp::~CBmBp()
{
}

BOOL CBmBp::SetBmBreakPoint(DWORD dwAddr, HANDLE hProcess)
{
	m_dwAddr = dwAddr;//�����ڴ�ϵ��ַ
	//�޸��ڴ��ҳ���ԣ���Ϊû���κη���Ȩ��
	if (VirtualProtectEx(hProcess, (LPVOID)dwAddr, 1, PAGE_NOACCESS, &m_dwOldProtect))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CBmBp::RemoveBmBreakPoint(HANDLE hProcess)
{
	//�޸��ڴ��ҳ���ԣ���Ϊ��ǰȨ��
	DWORD OldProtect;//����Ҫ��ֵӦ��ΪPAGE_NOACCESS
	if (VirtualProtectEx(hProcess, (LPVOID)m_dwAddr, 1, m_dwOldProtect, &OldProtect))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CBmBp::ReSetBmBreakPoint(HANDLE hProcess)
{
	//�޸��ڴ��ҳ���ԣ���Ϊû���κη���Ȩ��
	if (VirtualProtectEx(hProcess, (LPVOID)m_dwAddr, 1, PAGE_NOACCESS, &m_dwOldProtect))
	{
		return TRUE;
	}
	return FALSE;
}

