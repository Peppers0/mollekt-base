#include "../functions/functions.h"

DWORD WINAPI initialize_functions(LPVOID p) { // here u can find local player ptr and etc. // example: auto m_local_pointer = communication::read<uintptr_t>(communication::base_address + local_player_offset);
	while (true) {
		Sleep(25);
	}
}

int main() {
	std::cout << _("[+] welcome back, ") << _("user") << std::endl;

	system(_("cls"));

	std::cout << _("[+] start the game") << std::endl;

	while (!communication::hwnd) {
		communication::hwnd = FindWindowA(NULL, _("AssaultCube"));

		Sleep(500);
	}

	Sleep(2000);

	system(_("cls"));

	std::cout << _("[+] game was found: ") << _("SUCCESS") << std::endl;

	system(_("cls"));

	while (communication::process_id == 0)
		communication::process_id = communication::get_process_id(_("ac_client.exe").decrypt());

	while (communication::base_address == 0)
		communication::base_address = communication::get_base_address(_("ac_client.exe").decrypt());

	CloseHandle(CreateThread(0, 0, initialize_functions, 0, 0, 0));
	
	Sleep(3000);

	std::cout << _("[+] cheat initialized: ") << _("SUCCESS") << std::endl;

	Sleep(3000);

	system(_("cls"));

	std::cout << _("[-] base address: 0x") << communication::base_address << std::endl;

	while (true) {

	}

	return 1;
}