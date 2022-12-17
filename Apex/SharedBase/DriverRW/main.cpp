#include "Controle.h"
void NTAPI main_thread(PVOID);

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{	
	HANDLE main_thread_handle = nullptr;
	auto status = PsCreateSystemThread(
		&main_thread_handle,
		GENERIC_ALL,
		nullptr,
		nullptr,
		nullptr,
		main_thread,
		nullptr
	);
	NtClose(main_thread_handle);
	

	return STATUS_SUCCESS;
}




void NTAPI main_thread(PVOID)
{
	Test();

}

NTSTATUS UnloadDriver(PUNICODE_STRING DriverServiceName) {
	DbgPrintEx(0, 0, "Goodbye from ring0");

	//BSOD fix
	ZwUnloadDriver(DriverServiceName);
	return STATUS_SUCCESS;
}