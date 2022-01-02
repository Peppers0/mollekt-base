#pragma once

typedef struct request_data_t {
	ULONG m_process_id;
	UINT_PTR m_address;
	ULONGLONG m_size;

	BOOLEAN is_request_write;
	BOOLEAN is_request_read;
	BOOLEAN is_request_base;

	void* m_buffer_address;

	void* m_output;

	const char* m_module_name;

	ULONG64 m_base_address;
} request_data;

struct handle_disponser_t {
	using pointer = HANDLE;
	void operator()(HANDLE handle) const {
		if (handle != NULL || handle != INVALID_HANDLE_VALUE) {
			CloseHandle(handle);
		}
	}
};

using unique_handle = std::unique_ptr<HANDLE, handle_disponser_t>;

template<typename ... Arg>
__forceinline uint64_t run(const Arg ... args) {
	void* hooked_func = GetProcAddress(LoadLibrary("win32u.dll"), "NtGdiDdDDINetDispStartMiracastDisplayDevice");

	auto ret = static_cast<uint64_t(_stdcall*)(Arg...)>(hooked_func);

	return ret(args ...);
}

namespace communication {
	uint32_t process_id;
	HWND	 hwnd;
	uintptr_t base_address;

	__forceinline uint32_t get_process_id(std::string process_name) {
		PROCESSENTRY32 process_entry;

		const unique_handle snapshot_handle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));

		if (snapshot_handle.get() == INVALID_HANDLE_VALUE)
			return 0;

		process_entry.dwSize = sizeof(MODULEENTRY32);

		while (Process32Next(snapshot_handle.get(), &process_entry) == TRUE) {
			if (process_name.compare(process_entry.szExeFile) == NULL)
				return process_entry.th32ProcessID;
		}

		return NULL;
	}

	__forceinline static ULONG64 get_base_address(const char* module_name) {
		request_data request;

		request.m_process_id = process_id;

		request.is_request_base = TRUE;
		request.is_request_read = FALSE;
		request.is_request_write = FALSE;

		request.m_module_name = module_name;

		run(&request);

		return request.m_base_address;
	}

	template <typename T>
	__forceinline T read(unsigned long long int address) {
		T response = {};

		request_data request;

		request.m_process_id = process_id;

		request.is_request_base = FALSE;
		request.is_request_read = TRUE;
		request.is_request_write = FALSE;

		request.m_address = address;
		request.m_size = sizeof(T);

		request.m_output = &response;

		run(&request);

		return response;
	}
	
		template<typename T>
	__forceinline T read_chain(unsigned long long int address, std::vector<uintptr_t> chain) {
		auto current = address;

		for (auto i = 0; i < chain.size() - 1; i++) {
			current = read<unsigned long long int>(current + chain[i]);
		}

		return read<T>(current + chain[chain.size() - 1]);
	}

	__forceinline bool write_value(unsigned long long int address, UINT_PTR value, SIZE_T write_size) {
		request_data request;

		request.m_process_id = process_id;

		request.is_request_base = FALSE;
		request.is_request_read = FALSE;
		request.is_request_write = TRUE;

		request.m_address = address;
		request.m_size = write_size;

		request.m_buffer_address = (void*)value;

		run(&request);

		return true;
	}

	template<typename T>
	__forceinline bool write(UINT_PTR write_address, const T& value) {
		return write_value(write_address, (UINT_PTR)&value, sizeof(T));
	}
}
