#pragma once
#include <windows.h>
#include "MyType.h"
#include <list>
#include "TfBp.h"
#include "CcBp.h"
#include <string>
#include "LordPe.h"
#include "BhBp.h"
#include "BmBp.h"
#include <TlHelp32.h>
#include "FindTaskmgr.h"
using std::string;
using std::list;


typedef struct _MyLOAD_DLL_DEBUG_INFO {
	HANDLE hFile;
	LPVOID lpBaseOfDll;
	DWORD dwDebugInfoFileOffset;
	DWORD nDebugInfoSize;
	LPVOID lpImageName;
	WORD fUnicode;
} MyLOAD_DLL_DEBUG_INFO, *LPMyLOAD_DLL_DEBUG_INFO;

#define NUMOFBPTYPE 3

class CDbgEngine {
public:
#define MAX_INPUT 1024   // ����̨������󳤶�
	CDbgEngine();
	~CDbgEngine();
	// ������ѭ��
	void DebugMain();
	// �����¼��ַ�
	DWORD DispatchDbgEvent(DEBUG_EVENT& de);
	// �����¼�---------��----------
	// ���̴����¼�
	DWORD OnCreateProcess(DEBUG_EVENT& de);
	// ģ������¼�
	DWORD OnLoadDll(DEBUG_EVENT& de);
	// ģ��ж���¼�
	DWORD OnUnLoadDll(DEBUG_EVENT& de);
	// �쳣�����¼�����Ŀʱ�䶼����
	DWORD OnException(DEBUG_EVENT& de);
	// �����¼�---------��----------

	// �쳣�����¼�---------��----------
	// ����ϵ��쳣
	DWORD OnExceptionCc(DEBUG_EVENT& de);

	// �쳣�����¼�---------��----------
	// ��ӡ�Ĵ�����Ϣ
	VOID ShowRegisterInfo(CONTEXT& ct);
	// �ȴ��û������������
	VOID WaitforUserCommand();
	// �û�����
	// b����
	void UserCommandB(CHAR* pCommand);

	// u����
	void UserCommandDisasm(CHAR* pCommand);
	//��ȡ��ջ��Ϣ
	BOOL GetStackInfo(DWORD dwThreadId, HANDLE hProcess);
	//���������Խ���ģ��
	BOOL EnumModules(DWORD dwPid);
	//��ӡģ����Ϣ
	VOID PrintfModulesInfo();

	//�ж������ϵ��Ƿ�����
	BOOL IsConditionBreakPoint(DWORD dwThreadId);
private:
	// ����ຯ��
	void DisasmAtAddr(DWORD addr, DWORD dwCount = 10);
	UINT DBG_Disasm(HANDLE hProcess, LPVOID lpAddress, PWCHAR pOPCode, PWCHAR pASM, PWCHAR pComment);
private:
	// ���Խ��̵���Ϣ�����̴����¼���ʱ��ֵOnCreateProcess
	PROCESS_INFORMATION m_pi;
	// ������Ϣ***!!ָ��!!****
	// ��һ�εĵ���ѭ����ʼ��ʱ�����¸�ֵ
	LPDEBUG_EVENT m_pDbgEvt;
private:
	list<DWORD> m_bpAddrList[NUMOFBPTYPE];//�����������õĶϵ��ַ�Ͷϵ����͵�list
	vector<MyLOAD_DLL_DEBUG_INFO> m_vecLoadDllInfo;
	vector<MODULEENTRY32> m_vecModule;
	DWORD m_bmAddr;//��¼�ڴ���ʶϵ��ַ��ֻ��������һ���ڴ���ʶϵ�
	CTfBp* m_pTfBp;//���õ����ϵ�������ָ��
	CCcBp* m_pCcBp;//��������ϵ������ָ��
	CBhBp m_BhBp;//����Ӳ���ϵ������
	CBmBp m_bmBp;//�����ڴ�ϵ������
	CFindTaskmgr m_findTaskmgr;//�������������Ƿ�򿪵���

	CLordPe* m_pLordPe;//����pe��ָ��

	BOOL isSystemBp;//�Ƿ��ǵ�һ��ϵͳ�ϵ�
	BOOL m_isUserTf;//�Ƿ����û�����ִ�в������õ�TF�ϵ�
	BOOL m_isCcTf;//�Ƿ���Ϊ����������ϵ����õ�TF�ϵ�
	BOOL m_isBhTf;//�Ƿ���Ϊ������Ӳ���ϵ����õ�TF�ϵ�
	BOOL m_isBmTf;//�Ƿ���Ϊ�������ڴ�ϵ����õ�TF�ϵ�
	BOOL m_notWaitUser;//�Ƿ�Ҫ�����û�����
	DWORD m_dwOep;//�����Խ���OEP
	DWORD m_dwBaseAddr;//�����Խ��̻���ַ

	DWORD m_dwLeft;//������¼���ʽ������Ĵ�������
	DWORD m_dwSymbol;//������¼���ʽ��������
	DWORD m_dwRight;//������¼���ʽ�Ҳ�����
};

