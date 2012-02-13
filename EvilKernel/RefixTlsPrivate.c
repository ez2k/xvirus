// ��ȡPEB
#define __NtCurrentPeb__()		(NtCurrentTeb()->Peb)

/*
 * �ṹ����
 */
typedef struct _TLS_DATA {
	__void *pStartAddressOfRawData;
	__integer iTlsDataSize;
	__integer iTlsZeroSize;
	PIMAGE_TLS_CALLBACK *pTlsAddressOfCallBacks;
	PLDR_DATA_TABLE_ENTRY Module;
} TLS_DATA, *PTLS_DATA;

/*
 * ȫ�ֱ���
 */
PTLS_DATA g_pTlsArray = NULL;
__word g_wTlsCount = 0;
__integer g_iTlsSize = 0;
#define __LDRP_PROCESS_CREATION_TIME__			0xFFFF

__INLINE__ __void __INTERNAL_FUNC__ SingleTlsCallback(PLDR_DATA_TABLE_ENTRY Module, __integer iReason) {
	PIMAGE_TLS_CALLBACK *pTlsCallback = NULL;
	if (Module->TlsIndex != 0xFFFF && Module->LoadCount == __LDRP_PROCESS_CREATION_TIME__) {
		Module->LoadCount = 1;//�������ü�����Ϊ1
		pTlsCallback = g_pTlsArray[Module->TlsIndex].pTlsAddressOfCallBacks;
		if (pTlsCallback) {
			while (*pTlsCallback) {
				(*pTlsCallback)(Module->DllBase, iReason, NULL);
				pTlsCallback++;
			}
		}/* end if */
	}/* end if */
}

__INLINE__ __void __INTERNAL_FUNC__ TlsCallback(__integer iReason) {
	PLIST_ENTRY ModuleListHead;
	PLIST_ENTRY Entry;
	PLDR_DATA_TABLE_ENTRY Module;

	// ���������Ѽ��ص�ģ��
	ModuleListHead = &__NtCurrentPeb__()->LdrData->InLoadOrderModuleList;
	Entry = ModuleListHead->Flink;
	while (Entry != ModuleListHead) {
		PIMAGE_DATA_DIRECTORY pTlsDataDirectory = NULL;
		Module = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		pTlsDataDirectory = (PIMAGE_DATA_DIRECTORY)ExistDataDirectory(Module->DllBase, IMAGE_DIRECTORY_ENTRY_TLS);
		// ����Ƿ����TLS
		if (pTlsDataDirectory)
			SingleTlsCallback(Module, iReason);
		Entry = Entry->Flink;
	}
}

__INLINE__ __void __INTERNAL_FUNC__ AcquireTlsSlot(PLDR_DATA_TABLE_ENTRY Module, __integer iSize) {
	Module->TlsIndex = (__word)g_wTlsCount;
	Module->LoadCount = __LDRP_PROCESS_CREATION_TIME__;//��������Ϊ����ʱ
	g_wTlsCount++;
	g_iTlsSize += iSize;
}

__INLINE__ __word __INTERNAL_FUNC__ ResetTlsTable() {
	PLIST_ENTRY ModuleListHead;
	PLIST_ENTRY Entry;
	PLDR_DATA_TABLE_ENTRY Module;
	__integer iTlsSize = 0;

	// ��������ȫ�ֱ���
	g_wTlsCount = 0;
	g_iTlsSize = 0;

	// ���������Ѽ��ص�ģ��
	ModuleListHead = &__NtCurrentPeb__()->LdrData->InLoadOrderModuleList;
	Entry = ModuleListHead->Flink;
	while (Entry != ModuleListHead) {
		PIMAGE_DATA_DIRECTORY pTlsDataDirectory = NULL;
		Module = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		pTlsDataDirectory = (PIMAGE_DATA_DIRECTORY)ExistDataDirectory(Module->DllBase, IMAGE_DIRECTORY_ENTRY_TLS);
		if (pTlsDataDirectory) {//����Ƿ����TLS
			PIMAGE_TLS_DIRECTORY TlsDirectory = (PIMAGE_TLS_DIRECTORY)__RvaToVa__(Module->DllBase, pTlsDataDirectory->VirtualAddress);
			iTlsSize = TlsDirectory->EndAddressOfRawData - TlsDirectory->StartAddressOfRawData + TlsDirectory->SizeOfZeroFill;//�����ģ��TLS���ݵĳ���
			AcquireTlsSlot(Module, iTlsSize);
		}
		Entry = Entry->Flink;
	}
	return g_wTlsCount;
}

__INLINE__ __bool __INTERNAL_FUNC__ InitializeTlsForProccess() {
	PLIST_ENTRY ModuleListHead;
	PLIST_ENTRY Entry;
	PLDR_DATA_TABLE_ENTRY Module;
	PIMAGE_TLS_DIRECTORY TlsDirectory;
	__integer iSize;

	// �������TLS
	if (g_wTlsCount > 0) {
		g_pTlsArray = (PTLS_DATA)HeapAlloc(GetProcessHeap(), 0, g_wTlsCount * sizeof(TLS_DATA));
		if (g_pTlsArray == NULL)
			return FALSE;

		// ���������Ѽ��ص�ģ��
		ModuleListHead = &__NtCurrentPeb__()->LdrData->InLoadOrderModuleList;
		Entry = ModuleListHead->Flink;
		while (Entry != ModuleListHead) {
		   Module = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		   // ����Ǿ�̬���������TLS�ع�
		   if (Module->LoadCount == __LDRP_PROCESS_CREATION_TIME__ && Module->TlsIndex != 0xFFFF) {
			   PIMAGE_DATA_DIRECTORY pTlsDataDirectory = 
				   (PIMAGE_DATA_DIRECTORY)ExistDataDirectory(Module->DllBase, IMAGE_DIRECTORY_ENTRY_TLS);
				if (pTlsDataDirectory) {
					__dword dwOldProtect = 0;
					__integer iTlsTableSize = 0;
					__memory pTlsTable = NULL;
					PTLS_DATA TlsData = NULL;
					TlsDirectory = (PIMAGE_TLS_DIRECTORY)__RvaToVa__(Module->DllBase, pTlsDataDirectory->VirtualAddress);
					TlsData = &g_pTlsArray[Module->TlsIndex];
					TlsData->pStartAddressOfRawData = (__void *)TlsDirectory->StartAddressOfRawData;
					TlsData->iTlsDataSize = TlsDirectory->EndAddressOfRawData - TlsDirectory->StartAddressOfRawData;
					TlsData->iTlsZeroSize = TlsDirectory->SizeOfZeroFill;
					if (TlsDirectory->AddressOfCallBacks)
						TlsData->pTlsAddressOfCallBacks = (PIMAGE_TLS_CALLBACK *)TlsDirectory->AddressOfCallBacks;
					else
						TlsData->pTlsAddressOfCallBacks = NULL;
					TlsData->Module = Module;

					// �޸��ڴ�����Ϊ��д
					pTlsTable = (__memory)TlsDirectory;
					iTlsTableSize = pTlsDataDirectory->Size;
					VirtualProtect((__void *)pTlsTable, iTlsTableSize, PAGE_EXECUTE_WRITECOPY, &dwOldProtect);
					*(__dword *)(TlsDirectory->AddressOfIndex) = Module->TlsIndex;
					VirtualProtect((__void *)pTlsTable, iTlsTableSize, dwOldProtect, NULL);
				}/* end if */
			}/* end if */
			// ��һ��ģ��
			Entry = Entry->Flink;
		}/* end while */
	}
	return TRUE;
}

__INLINE__ __bool InitializeTlsForThread() {
	__void **pTlsPointers = NULL;
	PTLS_DATA pTlsInfo = NULL;
	__void *pTlsData = NULL;
	__integer i = 0;
	PTEB Teb = NtCurrentTeb();

	Teb->StaticUnicodeString.Length = 0;
	Teb->StaticUnicodeString.MaximumLength = sizeof(Teb->StaticUnicodeBuffer);
	Teb->StaticUnicodeString.Buffer = Teb->StaticUnicodeBuffer;

	// �������TLS
	if (g_wTlsCount > 0) {
		pTlsPointers = (__void **)HeapAlloc(GetProcessHeap(), 0, g_wTlsCount * sizeof(__void *) + g_iTlsSize);
		if (!pTlsPointers)
			return FALSE;

		pTlsData = (__void *)((__memory)pTlsPointers + g_wTlsCount * sizeof(__void *));//����TLS��������ָ��
		Teb->ThreadLocalStoragePointer = pTlsPointers;

		pTlsInfo = g_pTlsArray;//ȡ����ǰ�ı�
		for (i = 0; i < g_wTlsCount; i++, pTlsInfo++) {
			pTlsPointers[i] = pTlsData;//����ָ��ǰ�������ݵ�ָ��
			if (pTlsInfo->iTlsDataSize) {
				__logic_memcpy__(pTlsData, pTlsInfo->pStartAddressOfRawData, pTlsInfo->iTlsDataSize);
				pTlsData = (__void *)((__memory)pTlsData + pTlsInfo->iTlsDataSize);
			}
			if (pTlsInfo->iTlsZeroSize) {
				__logic_memset__(pTlsData, 0, pTlsInfo->iTlsZeroSize);
				pTlsData = (__void *)((__memory)pTlsData + pTlsInfo->iTlsZeroSize);
			}
		}/* end for */
	}

	return TRUE;
}
