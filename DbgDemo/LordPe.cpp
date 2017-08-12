#include "stdafx.h"
#include "LordPe.h"
#include <winternl.h>
#pragma comment(lib,"ntdll.lib")

CLordPe::CLordPe()
{
}


CLordPe::~CLordPe()
{
	delete m_pBuf;
	m_pBuf = NULL;
}


//************************************
// Method:    GetDosHead
// FullName:  CLordPe::GetDosHead
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: LPCTSTR filePath
// Function:  �������Խ���PE�ļ����ݶ�ȡ�������������ڴ�ռ�
//************************************
BOOL CLordPe::GetDosHead(LPCTSTR filePath)
{
	// 1. ���ļ�,���ļ���ȡ���ڴ�.
	// CreateFile,ReadFile.
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	DWORD dwFileSize = 0;
	dwFileSize = GetFileSize(hFile, NULL);

	// 2. �����ڴ�ռ�
	BYTE* pBuf = new BYTE[dwFileSize];
	m_pBuf = pBuf;//�������������������ͷſռ�

	// 3. ���ļ����ݶ�ȡ���ڴ���
	DWORD dwRead = 0;
	ReadFile(hFile, pBuf, dwFileSize, &dwRead, NULL);

	// ������������DOSͷ�ṹ��������
	m_pDosHdr = (IMAGE_DOS_HEADER*)pBuf;//��DOSͷָ�뱣������

										// ntͷ,�������ļ�ͷ����չͷ
	IMAGE_NT_HEADERS* pNtHdr;
	pNtHdr = (IMAGE_NT_HEADERS*)(m_pDosHdr->e_lfanew + (DWORD)m_pDosHdr);

	// �ж��Ƿ���һ����Ч��pe�ļ�
	if (m_pDosHdr->e_magic != IMAGE_DOS_SIGNATURE || pNtHdr->Signature != IMAGE_NT_SIGNATURE)
	{
		return FALSE;
	}
	return TRUE;
}

//************************************
// Method:    GetOep
// FullName:  CLordPe::GetOep
// Access:    public 
// Returns:   DWORD
// Qualifier:
// Function:  ��ȡ�����Խ���OEP
//************************************
DWORD CLordPe::GetOep()
{
	// ntͷ,�������ļ�ͷ����չͷ
	IMAGE_NT_HEADERS* pNtHdr = (IMAGE_NT_HEADERS*)(m_pDosHdr->e_lfanew + (DWORD)m_pDosHdr);

	IMAGE_OPTIONAL_HEADER* pOptHdr = &(pNtHdr->OptionalHeader);//��չͷ

	return pOptHdr->AddressOfEntryPoint;

}


//************************************
// Method:    getLoadAddress
// FullName:  CLordPe::getLoadAddress
// Access:    public 
// Returns:   LPCVOID
// Qualifier:
// Parameter: DWORD dwProcessId
// Function:  ��ȡ�����Խ��̼��ػ�ַ
//************************************
LPCVOID CLordPe::getLoadAddress(DWORD dwProcessId)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, 0, dwProcessId);

	HMODULE hModule = LoadLibrary(TEXT("Ntdll.dll "));
	if (NULL == hModule)
		return NULL;

	typedef NTSTATUS(WINAPI *NtQueryInformationProcess)(
		_In_      HANDLE           ProcessHandle,
		_In_      PROCESSINFOCLASS ProcessInformationClass,
		_Out_     PVOID            ProcessInformation,
		_In_      ULONG            ProcessInformationLength,
		_Out_opt_ PULONG           ReturnLength
		);
	NtQueryInformationProcess Func;

	Func = (NtQueryInformationProcess)GetProcAddress(hModule, "NtQueryInformationProcess");
	PROCESS_BASIC_INFORMATION pbi = { 0 };

	LONG status = Func(hProcess, ProcessBasicInformation, (PVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (NULL != hModule)
		FreeLibrary(hModule);

	if (NULL != hProcess)
		CloseHandle(hProcess);

	return pbi.PebBaseAddress->Reserved3[1];
}