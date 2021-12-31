#include "functions.h"

PVOID functions::get_system_module_base(const char* module_name) {
	ULONG bytes = {};

	auto status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);

	if (!bytes)
		return nullptr;

	auto modules = static_cast<PRTL_PROCESS_MODULES>(ExAllocatePoolWithTag(NonPagedPool, bytes, 0x4e554c4c));

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

	if (!NT_SUCCESS(status))
		return nullptr;

	auto module = modules->Modules;

	PVOID module_base = {};

	for (ULONG i = 0; i < modules->NumberOfModules; i++) {
		if (strcmp(reinterpret_cast<char*>(module[i].FullPathName), module_name) == 0) {
			module_base = module[i].ImageBase;

			break;
		}
	}

	if (modules)
		ExFreePoolWithTag(modules, 0);

	if (module_base <= 0)
		return nullptr;

	return module_base;
}

PVOID functions::get_system_module_export(const char* module_name, LPCSTR routine_name) {
	auto p_module = get_system_module_base(module_name);

	if (!p_module)
		return 0;

	return RtlFindExportedRoutineByName(p_module, routine_name);
}

bool functions::write_memory(void* address, void* buffer, size_t size) {
	if (!RtlCopyMemory(address, buffer, size))
		return false;

	return true;
}

bool functions::write_to_read_only_memory(void* address, void* buffer, size_t size) {
	auto model = IoAllocateMdl(address, size, FALSE, FALSE, 0);

	if (!model)
		return false;

	MmProbeAndLockPages(model, KernelMode, IoReadAccess);

	auto mapping = MmMapLockedPagesSpecifyCache(model, KernelMode, MmNonCached, 0, FALSE, NormalPagePriority);

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
		return 0;

	KAPC_STATE state = {};

	KeStackAttachProcess(proc, &state);

	auto ldr = peb->Ldr;

	if (!ldr) {
		KeUnstackDetachProcess(&state);

		return 0;
	}

	for (PLIST_ENTRY list = static_cast<PLIST_ENTRY>(ldr->ModuleListLoadOrder.Flink); list != &ldr->ModuleListLoadOrder; list = static_cast<PLIST_ENTRY>(list->Flink)) {
		PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);

		if (RtlCompareUnicodeString(&entry->BaseDllName, &module_name, TRUE) == 0) {
			auto base_address = reinterpret_cast<ULONG64>(entry->DllBase);

			KeUnstackDetachProcess(&state);

			return base_address;
		}
	}

	KeUnstackDetachProcess(&state);

	return 0;
}

bool functions::read_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size) {
	if (!address || !buffer || !size)
		return false;

	SIZE_T bytes = {};

	PEPROCESS process = {};

	PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(pid), &process);

	auto status = MmCopyVirtualMemory(process, reinterpret_cast<void*>(address), static_cast<PEPROCESS>(PsGetCurrentProcess()), reinterpret_cast<void*>(buffer), size, KernelMode, &bytes);

	if (!NT_SUCCESS(status))
		return false;

	return true;
}

bool functions::write_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size) {
	if (!address || !buffer || !size)
		return false;

	PEPROCESS process = {};

	PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(pid), &process);

	KAPC_STATE state = {};

	KeStackAttachProcess(reinterpret_cast<PEPROCESS>(process), &state);

	MEMORY_BASIC_INFORMATION info = {};

	auto status = ZwQueryVirtualMemory(ZwCurrentProcess(), reinterpret_cast<PVOID>(address), MemoryBasicInformation, &info, sizeof(info), 0);

	if (!NT_SUCCESS(status)) {
		KeUnstackDetachProcess(&state);

		return false;
	}

	if ((reinterpret_cast<uintptr_t>(info.BaseAddress) + info.RegionSize) < (address + size)) {
		KeUnstackDetachProcess(&state);

		return false;
	}

	if (!(info.State & MEM_COMMIT) || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS))) {
		KeUnstackDetachProcess(&state);

		return false;
	}

	if ((info.Protect & PAGE_EXECUTE_READWRITE) || (info.Protect & PAGE_EXECUTE_WRITECOPY) || (info.Protect & PAGE_READWRITE) || (info.Protect & PAGE_WRITECOPY))
		RtlCopyMemory(reinterpret_cast<void*>(address), buffer, size);

	KeUnstackDetachProcess(&state);

	return true;
}

