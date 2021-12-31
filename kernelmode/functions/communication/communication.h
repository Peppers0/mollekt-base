#pragma once

#include "../functions.h"

namespace communication {
	bool initialize(void* kernel_function_address);

	NTSTATUS handler(PVOID called_parameter);
}