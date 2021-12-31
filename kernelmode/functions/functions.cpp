#include "functions.h"

PVOID functions::get_system_module_base(const char* module_name) {
	ULONG bytes = {};

	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, bytes, &bytes);

	if (!bytes)
		return nullptr;

	auto modules = static_cast<PRTL_PROCESS_MODULES>(ExAllocatePoolWithTag(NonPagedPool, bytes, 0x4e554c4c));

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

	if (!NT_SUCCESS(status))
		return nullptr;

	auto module = modules->Modules;

	PVOID module_base = {};

	for (ULONG i = 0; i < modules->NumberOfModules; i++) {
		if (strcmp(reinterpret_cast<char*>(module[i].FullPathName), module_name) == NULL) {
			module_base = module[i].ImageBase;

			break;
		}
	}

	if (modules)
		ExFreePoolWithTag(modules, NULL);

	if (module_base <= NULL)
		return nullptr;

	return module_base;
}

PVOID functions::get_system_module_export(const char* module_name, LPCSTR routine_name) {
	PVOID p_module = get_system_module_base(module_name);

	if (!p_module)
		return NULL;

	return RtlFindExportedRoutineByName(p_module, routine_name);
}

bool functions::write_memory(void* address, void* buffer, size_t size) {
	if (!RtlCopyMemory(address, buffer, size))
		return false;

	return true;
}

bool functions::write_to_read_only_memory(void* address, void* buffer, size_t size) {
	PMDL model = IoAllocateMdl(address, size, FALSE, FALSE, NULL);

	if (!model)
		return false;

	MmProbeAndLockPages(model, KernelMode, IoReadAccess);

	PVOID mapping = MmMapLockedPagesSpecifyCache(model, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);

	MmProtectMdlSystemAddress(model, PAGE_READWRITE);

	write_memory(mapping, buffer, size);

	MmUnmapLockedPages(mapping, model);
	MmUnlockPages(model);
	IoFreeMdl(model);

	return true;
}

ULONG64 functions::get_module_base_x64(PEPROCESS proc, UNICODE_STRING module_name) {
	auto peb = PsGetProcessPeb(proc);

	if (!peb)
		return NULL;

	KAPC_STATE state = {};

	KeStackAttachProcess(proc, &state);

	auto ldr = peb->Ldr;

	if (!ldr) {
		KeUnstackDetachProcess(&state);

		return NULL;
	}

	for (PLIST_ENTRY list = static_cast<PLIST_ENTRY>(ldr->ModuleListLoadOrder.Flink); list != &ldr->ModuleListLoadOrder; list = static_cast<PLIST_ENTRY>(list->Flink)) {
		PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);

		if (RtlCompareUnicodeString(&entry->BaseDllName, &module_name, TRUE) == NULL) {
			auto base_address = reinterpret_cast<ULONG64>(entry->DllBase);

			KeUnstackDetachProcess(&state);

			return base_address;
		}
	}

	KeUnstackDetachProcess(&state);

	return NULL;
}

bool functions::read_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size) {
	if (!address || !buffer || !size)
		return false;

	SIZE_T bytes = 0;
	PEPROCESS process;

	PsLookupProcessByProcessId((HANDLE)pid, &process);

	auto status = MmCopyVirtualMemory(process, (void*)address, (PEPROCESS)PsGetCurrentProcess(), (void*)buffer, size, KernelMode, &bytes);

	if (!NT_SUCCESS(status))
		return false;

	return true;
}

bool functions::write_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size) {
	if (!address || !buffer || !size)
		return false;

	PEPROCESS process;
	PsLookupProcessByProcessId((HANDLE)pid, &process);

	KAPC_STATE state;
	KeStackAttachProcess((PEPROCESS)process, &state);

	MEMORY_BASIC_INFORMATION info;

	auto status = ZwQueryVirtualMemory(ZwCurrentProcess(), (PVOID)address, MemoryBasicInformation, &info, sizeof(info), NULL);
	
	if (!NT_SUCCESS(status)) {
		KeUnstackDetachProcess(&state);

		return false;
	}

	if (((uintptr_t)info.BaseAddress + info.RegionSize) < (address + size)) {
		KeUnstackDetachProcess(&state);

		return false;
	}

	if (!(info.State & MEM_COMMIT) || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS))) {
		KeUnstackDetachProcess(&state);

		return false;
	}

	if ((info.Protect & PAGE_EXECUTE_READWRITE) || (info.Protect & PAGE_EXECUTE_WRITECOPY) || (info.Protect & PAGE_READWRITE) || (info.Protect & PAGE_WRITECOPY))
		RtlCopyMemory((void*)address, buffer, size);

	KeUnstackDetachProcess(&state);

	return true;
}