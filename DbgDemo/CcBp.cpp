#include "stdafx.h"
#include "CcBp.h"




CCcBp::CCcBp()
{
}


CCcBp::~CCcBp()
{
}


BOOL CCcBp::SetBsBreakPoint(DWORD dwAddr, HANDLE hProcess)
{
	//��ȡ�����ڴ棬����һ���ֽڵ�����
	DWORD dwSize = 0;
	if (!ReadProcessMemory(hProcess,&dwAddr,&m_oldByte,1,&dwSize))
		return FALSE;
	//д��һ���ֽڣ�\xcc����int3ָ��Ļ�����
	BYTE cc = '\xcc';
	if (!WriteProcessMemory(hProcess, &dwAddr, &cc, 1, &dwSize))
		return FALSE;
	return TRUE;
}

BOOL CCcBp::RemoveBsBreakPoint(DWORD dwAddr, HANDLE hProcess)
{
	DWORD dwSize = 0;
	return WriteProcessMemory(hProcess, &dwAddr, &m_oldByte, 1, &dwSize);
}
