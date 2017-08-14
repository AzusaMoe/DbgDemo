#include "stdafx.h"
#include "BhBp.h"
#include "MyType.h"

CBhBp::CBhBp()
{
}


CBhBp::~CBhBp()
{
}

//************************************
// Method:    SetBhExecBreakPoint
// FullName:  CBhBp::SetBhExecBreakPoint
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: DWORD dwThreadId
// Parameter: DWORD dwAddr
// Function:  ����Ӳ��ִ�жϵ�
//************************************
BOOL CBhBp::SetBhExecBreakPoint(DWORD dwThreadId, DWORD dwAddr)
{
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, NULL, dwThreadId);
	CONTEXT ct = { CONTEXT_DEBUG_REGISTERS };
	GetThreadContext(hThread, &ct);//��ȡ�̻߳�����
	DBG_REG7* pDr7 = (DBG_REG7*)&ct.Dr7;
	if (pDr7->L0==0)//L0û�б�ʹ��
	{
		ct.Dr0 = dwAddr;//��Dr0�Ĵ�����д���жϵ�ַ
		pDr7->RW0 = 0;//ִ�жϵ�
		pDr7->LEN0 = 0;//��1�ֽڳ��ȣ�
		pDr7->L0 = 1;//���øöϵ�
	}
	else if (pDr7->L1==0)
	{
		ct.Dr1 = dwAddr;
		pDr7->RW1 = 0;
		pDr7->LEN1 = 0;
		pDr7->L0 = 1;
	}
	else if (pDr7->L2 == 0)
	{
		ct.Dr2 = dwAddr;
		pDr7->RW2 = 0;
		pDr7->LEN2 = 0;
		pDr7->L0 = 1;
	}
	else if (pDr7->L3 == 0)
	{
		ct.Dr3 = dwAddr;
		pDr7->RW3 = 0;
		pDr7->LEN3 = 0;
		pDr7->L0 = 1;
	}
	else
	{
		return FALSE;
	}
	SetThreadContext(hThread, &ct);
	CloseHandle(hThread);
	return TRUE;
}

BOOL CBhBp::SetBhRwBreakPoint(DWORD dwThreadId, DWORD dwAddr, DWORD dwType, DWORD dwLen)
{
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, NULL, dwThreadId);
	CONTEXT ct = { CONTEXT_DEBUG_REGISTERS };
	GetThreadContext(hThread, &ct);//��ȡ�̻߳�����
	DBG_REG7* pDr7 = (DBG_REG7*)&ct.Dr7;

	//�Ե�ַ�ͳ��Ƚ��ж��봦������ȡ����
	if (dwLen==1)//���ֽڵĳ��ȶ���
	{
		dwAddr = dwAddr - dwAddr % 2;
	}
	else if (dwLen == 3)
	{
		dwAddr = dwAddr - dwAddr % 4;
	}
	else if (dwLen > 3)
	{
		return FALSE;
	}
	//�ж���Щ�Ĵ���û�б�ʹ��
	if (pDr7->L0 == 0)//L0û�б�ʹ��
	{
		ct.Dr0 = dwAddr;//��Dr0�Ĵ�����д���жϵ�ַ
		pDr7->RW0 = dwType;//ִ�жϵ�
		pDr7->LEN0 = dwLen;//��1�ֽڳ��ȣ�
	}
	else if (pDr7->L1 == 0)
	{
		ct.Dr1 = dwAddr;
		pDr7->RW1 = dwType;
		pDr7->LEN1 = dwLen;
	}
	else if (pDr7->L2 == 0)
	{
		ct.Dr2 = dwAddr;
		pDr7->RW2 = dwType;
		pDr7->LEN2 = dwLen;
	}
	else if (pDr7->L3 == 0)
	{
		ct.Dr3 = dwAddr;
		pDr7->RW3 = dwType;
		pDr7->LEN3 = dwLen;
	}
	else
	{
		return FALSE;
	}
	SetThreadContext(hThread, &ct);
	CloseHandle(hThread);
	return TRUE;
}

VOID CBhBp::ReSetAllBhRwBreakPoint(DWORD dwThreadId)
{
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, NULL, dwThreadId);
	CONTEXT ct = { CONTEXT_DEBUG_REGISTERS };
	GetThreadContext(hThread, &ct);//��ȡ�̻߳�����
	DBG_REG7* pDr7 = (DBG_REG7*)&ct.Dr7;
	//�ж�Dr0-Dr3���ĸ��Ĵ�������û��ֵ
	if (ct.Dr0)
	{
		pDr7->L0 = 1;//���øöϵ�
		SetThreadContext(hThread, &ct);
	}
	if (ct.Dr1)
	{
		pDr7->L1 = 1;//���øöϵ�
		SetThreadContext(hThread, &ct);
	}
	if (ct.Dr2)
	{
		pDr7->L2 = 1;//���øöϵ�
		SetThreadContext(hThread, &ct);
	}
	if (ct.Dr3)
	{
		pDr7->L3 = 1;//���øöϵ�
		SetThreadContext(hThread, &ct);
	}
}

BOOL CBhBp::CheckDr6ForBhRwBreakPoint(DWORD dwThreadId)
{
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, NULL, dwThreadId);
	CONTEXT ct = { CONTEXT_DEBUG_REGISTERS };
	GetThreadContext(hThread, &ct);//��ȡ�̻߳�����
	DBG_REG6* pDr6 = (DBG_REG6*)&ct.Dr6;
	DBG_REG7* pDr7 = (DBG_REG7*)&ct.Dr7;
	//���DR6�е�B0-B3λ
	if (pDr6->B0)
	{
		pDr7->L0 = 0;
		SetThreadContext(hThread, &ct);
		return TRUE;
	}
	if (pDr6->B1)
	{
		pDr7->L1 = 0;
		SetThreadContext(hThread, &ct);
		return TRUE;
	}
	if (pDr6->B2)
	{
		pDr7->L2 = 0;
		SetThreadContext(hThread, &ct);
		return TRUE;
	}
	if (pDr6->B3)
	{
		pDr7->L3 = 0;
		SetThreadContext(hThread, &ct);
		return TRUE;
	}
	return FALSE;
}
