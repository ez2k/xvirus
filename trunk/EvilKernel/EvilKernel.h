#if !defined(__EVILKERNEL_H__)
#define __EVILKERNEL_H__

#include "Common.h"


#if defined(__cplusplus)
extern "C"
{
#endif

// �����Ҫ���Բ�������ô˺�
#define __DEBUG_EVIL_PLUGIN__				1

/*
 * ���ýṹ
 */
#define __MAX_EVIL_PLUGIN_COUNT__			128	//а����������
#define __EVIL_PLUGIN_NAME_LENGTH__			256 //а���������ĳ���
typedef struct _EVILKERNEL_CONFIGURE {
	__bool bIsDll;//�Ƿ���DLL
	__integer iTargetFileSize;//Ŀ���ļ�����
	__integer iPluginTotalCount;//�������
	__integer PluginSizeArray[__MAX_EVIL_PLUGIN_COUNT__];//����Ĵ�С����
#if defined(__DEBUG_EVIL_PLUGIN__)
	/*
	 * �ڵ���а����ʱ�ſ���
	 */
	__char PluginNameArray[__MAX_EVIL_PLUGIN_COUNT__][__EVIL_PLUGIN_NAME_LENGTH__];
#endif
} EVILKERNEL_CONFIGURE, *PEVILKERNEL_CONFIGURE;

/*
 * �����DLL,����Ĳ����ṹ
 * �������ֻ��xVirusDllʹ��
 * û��eax,Ѿ���ñ���
 */
typedef struct _XVIRUSDLL_ARG {
	__dword dwOrigEsp;
	__dword dwOrigEbp;
	__dword dwOrigEsi;
	__dword dwOrigEdi;
	__dword dwOrigEbx;
	__dword dwOrigEdx;
	__dword dwOrigEcx;
	__address addrOrigImageBase;//ԭʼ��ӳ���ַ,�ṩ��EvilKernel�л�ȡԭʼӳ��
} XVIRUSDLL_ARG, *PXVIRUSDLL_ARG;

#if defined(__cplusplus)
}
#endif

#endif
