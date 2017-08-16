#include "stdafx.h"
#include "Compilation.h"

#include "DbgEngine.h"
//1. ����ͷ�ļ�
//2. ������̬��
#include "keystone/keystone.h"
#pragma comment (lib,"keystone/x86/keystone_x86.lib")

CCompilation::CCompilation()
{
}


CCompilation::~CCompilation()
{
}

bool CCompilation::GetOpcode(HANDLE hProcess)
{
	ks_engine *pengine = NULL;
	if (KS_ERR_OK != ks_open(KS_ARCH_X86, KS_MODE_32, &pengine)) {
		printf("����������ʼ��ʧ��\n");
		return false;
	}


	// ���ָ��
	// ����ʹ�÷ֺţ����߻��з���ָ��ָ���
	char asmCode[MAX_INPUT] = { 0 };
	printf("��������Ҫ����Ļ��ָ�");
	gets_s(asmCode, MAX_INPUT);
	int nAddr;//Ҫ������ָ����׵�ַ
	printf("��������Ҫ����ָ����׵�ַ��");
	scanf("%x", &nAddr);

	unsigned char* opcode = NULL; // ���õ���opcode�Ļ������׵�ַ
	int nRet = 0; // ���溯���ķ���ֵ�������жϺ����Ƿ�ִ�гɹ�
	size_t stat_count = 0; // ����ɹ�����ָ�������
	size_t nOpcodeSize = 0;//��������opcode�ֽ���

	nRet = ks_asm(pengine, /* �����������ͨ��ks_open�����õ�*/
		asmCode, /*Ҫת���Ļ��ָ��*/
		nAddr, /*���ָ�����ڵĵ�ַ*/
		&opcode,/*�����opcode*/
		&nOpcodeSize,/*�����opcode���ֽ���*/
		&stat_count /*����ɹ�����ָ�������*/
	);

	// ����ֵ����-1ʱ��������
	if (nRet == -1) {
		// ���������Ϣ
		// ks_errno ��ô�����
		// ks_strerror ��������ת�����ַ���������������ַ���
		printf("������Ϣ��%s\n", ks_strerror(ks_errno(pengine)));
		return false;
	}
	//�޸��ڴ��ҳ���ԣ���Ϊ�ɶ���д
	DWORD dwOldProtect;
	VirtualProtectEx(hProcess, (LPVOID)nAddr, nOpcodeSize, PAGE_READWRITE, &dwOldProtect);
	DWORD dwSize = 0;//ʵ��д����ֽ���
	if (!WriteProcessMemory(hProcess, (LPVOID)nAddr, opcode, nOpcodeSize, &dwSize))
	{
		//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
		VirtualProtectEx(hProcess, (LPVOID)nAddr, nOpcodeSize, dwOldProtect, &dwOldProtect);
		return false;
	}
	//���޸Ĺ����ڴ��ҳ���ԸĻ�ȥ
	VirtualProtectEx(hProcess, (LPVOID)nAddr, nOpcodeSize, dwOldProtect, &dwOldProtect);
	// �ͷſռ�
	ks_free(opcode);
	// �رվ��
	ks_close(pengine);
	return true;
}

