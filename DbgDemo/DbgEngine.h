#pragma once
#include <windows.h>
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
	// �����ϵ��쳣
	DWORD OnExceptionCc(DEBUG_EVENT& de);
	// �����쳣
	DWORD OnExceptionSingleStep(DEBUG_EVENT& de);
	// �ڴ�����쳣
	DWORD OnExceptionAccess(DEBUG_EVENT& de);
	// �쳣�����¼�---------��----------
	// ��ӡ�Ĵ�����Ϣ
	VOID ShowRegisterInfo(CONTEXT& ct);
	// �ȴ��û������������
	DWORD WaitforUserCommand();
	// �û�����
	// t����
	void UserCommandStepInto();
	// b����
	void UserCommandB(CHAR* pCommand);

	// u����
	void UserCommandDisasm(CHAR* pCommand);
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
};
