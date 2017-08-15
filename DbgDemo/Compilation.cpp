#include "stdafx.h"
#include "Compilation.h"

//1. ����ͷ�ļ�
#include "keystone/keystone.h"
#include "DbgEngine.h"

//2. ������̬��
#pragma comment (lib,"keystone/x86/keystone_x86.lib")

CCompilation::CCompilation()
{
}


CCompilation::~CCompilation()
{
}

bool CCompilation::GetOpcode(unsigned char* opcode, size_t& nOpcodeSize, int& nAddr)
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
	printf("��������Ҫ����ָ����׵�ַ��");
	scanf("%X", nAddr);

	opcode = NULL; // ���õ���opcode�Ļ������׵�ַ
	int nRet = 0; // ���溯���ķ���ֵ�������жϺ����Ƿ�ִ�гɹ�
	size_t stat_count = 0; // ����ɹ�����ָ�������

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

	printf("һ��ת����%d��ָ��\n", stat_count);
	// �ͷſռ�
	ks_free(opcode);
	// �رվ��
	ks_close(pengine);
	return true;
}

