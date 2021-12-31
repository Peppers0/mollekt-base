#include "communication.h"

bool communication::initialize(void* kernel_function_address) {
	if (!kernel_function_address)
		return false;

	// u can change function, ill leave the working options ->
	// NtDCompositionCurrentBatchId
	// NtDCompositionTelemetryAnimationScenarioBegin
	// NtQueryCompositionSurfaceStatistics
	// and etc..

	// !!!! IMPORTANT !!!! if u change function in driver u also need to change in usermode

	auto function = reinterpret_cast<PVOID*>(functions::get_system_module_export("\\SystemRoot\\System32\\drivers\\dxgkrnl.sys", "NtGdiDdDDINetDispStartMiracastDisplayDevice"));

	if (!function)
		return false;

	BYTE original[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	BYTE shell_code[] = { 0x48, 0xB8 }; // mov rax | this shellcode is detected, u need to change him
	BYTE shell_code_end[] = { 0xFF, 0xE0 }; // jmp rax | this shellcode is detected, u need to change him

	RtlSecureZeroMemory(&original, sizeof(original));

	memcpy(reinterpret_cast<PVOID>(reinterpret_cast<ULONG_PTR>(original)), &shell_code, sizeof(shell_code));

	auto hook_address = reinterpret_cast<uintptr_t>(kernel_function_address);

	memcpy(reinterpret_cast<PVOID>(reinterpret_cast<ULONG_PTR>(original) + sizeof(shell_code)), &hook_address, sizeof(void*));
	memcpy(reinterpret_cast<PVOID>(reinterpret_cast<ULONG_PTR>(original + sizeof(shell_code) + sizeof(void*))), &shell_code_end, sizeof(shell_code_end));

	functions::write_to_read_only_memory(function, &original, sizeof(original));

	return true;
}

NTSTATUS communication::handler(PVOID called_parameter) {
	auto request = static_cast<request_data*>(called_parameter);

	if (request->is_request_base) {
		ANSI_STRING ansi_string;
		UNICODE_STRING module_name;

		PEPROCESS process;

		RtlInitAnsiString(&ansi_string, request->m_module_name);
		RtlAnsiStringToUnicodeString(&module_name, &ansi_string, TRUE);

		PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(request->m_process_id), &process);

		auto base_address64 = functions::get_module_base_x64(process, module_name);

		request->m_base_address = base_address64;

		RtlFreeUnicodeString(&module_name);
	}

	else if (request->is_request_read) {
		if (request->m_address < 0x7FFFFFFFFFFF && request->m_address > 0) {
			functions::read_kernel_memory(reinterpret_cast<HANDLE>(request->m_process_id), request->m_address, request->m_output, request->m_size);
		}
	}

	else if (request->is_request_write) {
		if (request->m_address < 0x7FFFFFFFFFFF && request->m_address > 0) {
			auto kernel_buffer = ExAllocatePool(NonPagedPool, request->m_size);

			if (!kernel_buffer)
				return STATUS_UNSUCCESSFUL;

			if (!memcpy(kernel_buffer, request->m_buffer_address, request->m_size))
				return STATUS_UNSUCCESSFUL;

			PEPROCESS process = {};

			PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(request->m_process_id), &process);

			functions::write_kernel_memory(reinterpret_cast<HANDLE>(request->m_process_id), request->m_base_address, kernel_buffer, request->m_size);

			ExFreePool(kernel_buffer);
		}
	}

	return STATUS_SUCCESS;
}