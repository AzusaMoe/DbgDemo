#include "stdafx.h"
#include "CcBp.h"




CCcBp::CCcBp()
{
}


CCcBp::~CCcBp()
{
}



//************************************
// Method:    SetBsBreakPoint
// FullName:  CCcBp::SetBsBreakPoint
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: DWORD dwAddr �ϵ��ַ
// Parameter: HANDLE hProcess Ŀ�����
// Function:  ��������ϵ�
//************************************
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

//************************************
// Method:    RemoveBsBreakPoint
// FullName:  CCcBp::RemoveBsBreakPoint
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: DWORD dwAddr �ϵ��ַ
// Parameter: HANDLE hProcess Ŀ�����
// Function:  ȡ������ϵ�
//************************************
BOOL CCcBp::RemoveBsBreakPoint(DWORD dwAddr, HANDLE hProcess)
{
	DWORD dwSize = 0;
	return WriteProcessMemory(hProcess, &dwAddr, &m_oldByte, 1, &dwSize);
}


//************************************
// Method:    EipSubOne
// FullName:  CCcBp::EipSubOne
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: DWORD dwThreadId
// Function:  ���������߳�EIP��һ
//************************************
BOOL CCcBp::EipSubOne(DWORD dwThreadId)
{
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, NULL, dwThreadId);
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// ָ��Ҫ��ȡ��д�Ĵ�������Ϣ������Ҫ
	GetThreadContext(hThread, &ct);
	ct.Eip--;
	SetThreadContext(hThread, &ct);
	CloseHandle(hThread);
	return TRUE;
}