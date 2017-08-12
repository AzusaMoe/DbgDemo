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
	//��������ϵ��ַ
	m_vecOldByteAddr.push_back(dwAddr);
	//��ȡ�����ڴ棬����һ���ֽڵ�����
	DWORD dwSize = 0;
	BYTE oldByte;//�¶ϵ�λ��ԭ�������ݣ��ָ���

	//�޸��ڴ��ҳ���ԣ���Ϊ�ɶ���д
	DWORD dwOldProtect;
	VirtualProtectEx(hProcess, (LPVOID)dwAddr, 1, PAGE_READWRITE, &dwOldProtect);

	if (!ReadProcessMemory(hProcess, (LPVOID)dwAddr, &oldByte, 1, &dwSize))
	{
		//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
		VirtualProtectEx(hProcess, (LPVOID)dwAddr, 1, dwOldProtect, &dwOldProtect);
		return FALSE;
	}
	//��������ϵ��ַ��ԭ����
	m_vecOldByte.push_back(oldByte);
	//д��һ���ֽڣ�\xcc����int3ָ��Ļ�����
	BYTE cc = '\xcc';
	if (!WriteProcessMemory(hProcess, (LPVOID)dwAddr, &cc, 1, &dwSize))
	{
		//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
		VirtualProtectEx(hProcess, (LPVOID)dwAddr, 1, dwOldProtect, &dwOldProtect);
		return FALSE;
	}

	//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
	VirtualProtectEx(hProcess, (LPVOID)dwAddr, 1, dwOldProtect, &dwOldProtect);
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
// Function:  ȡ��ָ������ϵ㣨����ϵ㴥��ʱ���ã�
//************************************
BOOL CCcBp::RemoveBsBreakPoint(DWORD dwAddr, HANDLE hProcess)
{
	BYTE oldByte;
	//�ҵ���ַ��Ӧ��ԭ����
	for (size_t i = 0; i < m_vecOldByteAddr.size(); i++)
	{
		if (m_vecOldByteAddr[i]==dwAddr)
		{
			oldByte = m_vecOldByte[i];
		}
	}
	//�޸��ڴ��ҳ���ԣ���Ϊ�ɶ���д
	DWORD dwOldProtect;
	VirtualProtectEx(hProcess, (LPVOID)dwAddr, 1, PAGE_READWRITE, &dwOldProtect);

	DWORD dwSize = 0;
	if (!WriteProcessMemory(hProcess, (LPVOID)dwAddr, &oldByte, 1, &dwSize))
	{
		//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
		VirtualProtectEx(hProcess, (LPVOID)dwAddr, 1, dwOldProtect, &dwOldProtect);
		return FALSE;
	}
	//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
	VirtualProtectEx(hProcess, (LPVOID)dwAddr, 1, dwOldProtect, &dwOldProtect);
	return TRUE;

}


//************************************
// Method:    RemoveAllBsBreakPoint
// FullName:  CCcBp::RemoveAllBsBreakPoint
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: HANDLE hProcess
// Function:  ȡ�����е�����ϵ㣨�����ʱ���ã�
//************************************
BOOL CCcBp::RemoveAllBsBreakPoint(HANDLE hProcess)
{
	for (size_t i = 0; i < m_vecOldByteAddr.size(); i++)
	{
		//�޸��ڴ��ҳ���ԣ���Ϊ�ɶ���д
		DWORD dwOldProtect;
		VirtualProtectEx(hProcess, (LPVOID)m_vecOldByteAddr[i], 1, PAGE_READWRITE, &dwOldProtect);

		DWORD dwSize = 0;
		if (!WriteProcessMemory(hProcess, (LPVOID)m_vecOldByteAddr[i], &m_vecOldByte[i], 1, &dwSize))
		{
			//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
			VirtualProtectEx(hProcess, (LPVOID)m_vecOldByteAddr[i], 1, dwOldProtect, &dwOldProtect);
			return FALSE;
		}
		
		//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
		VirtualProtectEx(hProcess, (LPVOID)m_vecOldByteAddr[i], 1, dwOldProtect, &dwOldProtect);

	}
	return TRUE;
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
	if (NULL==hThread)
	{
		return FALSE;
	}
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// ָ��Ҫ��ȡ��д�Ĵ�������Ϣ������Ҫ
	if (!GetThreadContext(hThread, &ct))
	{
		return FALSE;
	}
	ct.Eip--;
	if (!SetThreadContext(hThread, &ct))
	{
		return FALSE;
	}	
	CloseHandle(hThread);
	return TRUE;
}

//************************************
// Method:    ReSetAllBsBreakPoint
// FullName:  CCcBp::ReSetAllBsBreakPoint
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: HANDLE hProcess
// Function:  ��ȫ��������ϵ�ָ�
//************************************
BOOL CCcBp::ReSetAllBsBreakPoint(HANDLE hProcess)
{
	//д��һ���ֽڣ�\xcc����int3ָ��Ļ�����
	BYTE cc = '\xcc';
	DWORD dwSize;
	for (auto each : m_vecOldByteAddr)
	{
		//�޸��ڴ��ҳ���ԣ���Ϊ�ɶ���д
		DWORD dwOldProtect;
		VirtualProtectEx(hProcess, (LPVOID)each, 1, PAGE_READWRITE, &dwOldProtect);

		if (!WriteProcessMemory(hProcess, (LPVOID)each, &cc, 1, &dwSize))
		{
			//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
			VirtualProtectEx(hProcess, (LPVOID)each, 1, dwOldProtect, &dwOldProtect);
			return FALSE;
		}
		//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
		VirtualProtectEx(hProcess, (LPVOID)each, 1, dwOldProtect, &dwOldProtect);
	}
	return TRUE;
}