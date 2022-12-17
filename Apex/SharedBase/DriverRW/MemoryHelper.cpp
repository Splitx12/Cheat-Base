#include "MemoryHelper.h"

MemoryHelper _MemoryHelper;

NTSTATUS MemoryHelper::attach(LPCWSTR process_name)
{
	if (!GetProcessId(process_name))return STATUS_FAIL_CHECK;

	pid = (HANDLE)GetProcessId(process_name);

	return PsLookupProcessByProcessId(pid, &TargetPEprocess);//получить EPROCESS через pid 
}


ULONGLONG get_modulehandle(LPCWSTR module_name, uint32_t pid) {


	PEPROCESS TargetPEprocess2;

	PsLookupProcessByProcessId((HANDLE)pid, &TargetPEprocess2);//получить EPROCESS через pid 
	ULONGLONG base = 0;

	//делаем адрессное пространоство игры доступным в нашем потоке
	KeAttachProcess((PKPROCESS)TargetPEprocess2);

	//берем peb игры
	PPEB peb = PsGetProcessPeb(TargetPEprocess2);

	if (!peb)goto end;

	if (!peb->Ldr || !peb->Ldr->Initialized)goto end;

	UNICODE_STRING module_name_unicode;
	RtlInitUnicodeString(&module_name_unicode, module_name);
	//пробегаемся по цепочке модулей и находим искомый 
	for (PLIST_ENTRY list = peb->Ldr->InLoadOrderModuleList.Flink;
		list != &peb->Ldr->InLoadOrderModuleList;
		list = list->Flink) {
		PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		if (RtlCompareUnicodeString(&entry->BaseDllName, &module_name_unicode, TRUE) == 0) {
			base = (ULONGLONG)entry->DllBase;
			goto end;
		}
	}

end:
	KeDetachProcess();
	return base;
}

ULONGLONG MemoryHelper::get_module_handle(LPCWSTR module_name) {

	ULONGLONG base = 0;

	//делаем адрессное пространоство игры доступным в нашем потоке
	KeAttachProcess((PKPROCESS)TargetPEprocess);

	//берем peb игры
	PPEB peb = PsGetProcessPeb(TargetPEprocess);

	if (!peb)goto end;

	if (!peb->Ldr || !peb->Ldr->Initialized)goto end;

	UNICODE_STRING module_name_unicode;
	RtlInitUnicodeString(&module_name_unicode, module_name);
	//пробегаемся по цепочке модулей и находим искомый 
	for (PLIST_ENTRY list = peb->Ldr->InLoadOrderModuleList.Flink;
		list != &peb->Ldr->InLoadOrderModuleList;
		list = list->Flink) {
		PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		if (RtlCompareUnicodeString(&entry->BaseDllName, &module_name_unicode, TRUE) == 0) {
			base = (ULONGLONG)entry->DllBase;
			goto end;
		}
	}

end:
	KeDetachProcess();
	return base;
}

INT MemoryHelper::virtual_protect(PVOID address, size_t size, INT protect)
{
	NTSTATUS status;
	KAPC_STATE apc;
	ULONG old_protection;

	KeStackAttachProcess(TargetPEprocess, &apc);
	status = ZwProtectVirtualMemory(ZwCurrentProcess(), &address, &size, protect, &old_protection);
	if (status != STATUS_SUCCESS)return NULL;

	KeUnstackDetachProcess(&apc);
	ObfDereferenceObject(TargetPEprocess);
	return old_protection;

}


// PRIVATE______________________________________________________________________________________________________________________

UINT MemoryHelper::GetProcessId(PCWSTR ImageName)
{
	UNICODE_STRING unicode_ImageName;
	RtlInitUnicodeString(&unicode_ImageName, ImageName);

	ULONG cb = 0x20000;

	PVOID buf;
	NTSTATUS status;
	do
	{
		status = STATUS_INSUFFICIENT_RESOURCES;

		if (buf = ExAllocatePool(PagedPool, cb))
		{
			if (0 <= (status = ZwQuerySystemInformation(SystemProcessInformation, buf, cb, &cb)))
			{
				union {
					PVOID pv;
					PBYTE pb;
					PSYSTEM_PROCESS_INFORMATION pspi;
				};

				pv = buf;
				ULONG NextEntryOffset;

				goto __0;

				do
				{
					pb += NextEntryOffset;
				__0:
					if (RtlEqualUnicodeString(&pspi->ImageName, &unicode_ImageName, TRUE))
					{
						return (UINT)pspi->UniqueProcessId;
						break;
					}

				} while (NextEntryOffset = pspi->NextEntryOffset);
			}
			ExFreePool(buf);
		}

	} while (status == STATUS_INFO_LENGTH_MISMATCH);

	return NULL;
}

