#pragma once
#include <Windows.h>
#include <iostream>
#include <tlhelp32.h>



typedef struct PacketSM
{
	bool access; // 1 = Ожидает операции | 0 = Выполнено 
	enum  typePacket
	{
		read = 0,
		write = 1,
		get_module_handle = 2
	} _typePacket;
	union packet {
		struct RW {
			ULONG pid;
			uint64_t src_addr;
			uint64_t dst_addr;
			size_t size;
		}_RW;
		struct GMH {
			ULONG pid;
			wchar_t str[30];
			uint64_t addr;
		}_GMH;
	}_packet;
	//bool responseStatus;//1-success,0-failure reserved
}_PacketSM;

uint32_t get_process_pid(const char* process_name) {
	PROCESSENTRY32 process_entry{};
	process_entry.dwSize = sizeof(PROCESSENTRY32);
	uint32_t pid = 0;
	auto snapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL) };
	if (snapshot == INVALID_HANDLE_VALUE)
		return false;
	if (Process32First(snapshot, &process_entry)) {
		do {
			if (!strcmp(process_name, process_entry.szExeFile)) {
				pid = process_entry.th32ProcessID;
				CloseHandle(snapshot);
				return pid;
			}
		} while (Process32Next(snapshot, &process_entry));
	}
	CloseHandle(snapshot);
	return 0;
}

class sharedController
{
	bool Debug = 1;//0 false - 1 true;
	DWORD64 shared_base = 0;
	uint32_t pid = 0;
public:


	bool attach(const char* proc_name)
	{
		if (!get_process_pid(proc_name))
		{
			MessageBox(0, "[ kernel attach ] Not find Game", "ERROR", MB_OK | MB_ICONERROR);
			return false;
		}
		pid = get_process_pid(proc_name);

		shared_base = (DWORD64)GetModuleHandleA(NULL) + 0x5200;
		

		if (Debug) {
			std::cout<<std::hex << "Shared Base [=]" << shared_base << std::endl;
		}
		
		ZeroMemory((void*)shared_base, sizeof(PacketSM));
		return shared_base;
	}
	bool CopyMem(PacketSM::typePacket type, uint64_t dst_addr, uint64_t src_addr, size_t size)
	{
		bool Buffer= *(LPBYTE*)shared_base;
		if (Buffer)
			return false;
		PacketSM pck{};
		pck.access = 1;
		pck._typePacket = type;

		pck._packet._RW.pid = pid;
		pck._packet._RW.src_addr = src_addr;
		pck._packet._RW.dst_addr = dst_addr;
		pck._packet._RW.size = size;

		memcpy((void*)shared_base, &pck, sizeof(_PacketSM));
	


		return  true;

	}
	uint64_t GetModuleBase(const std::string& module_name) {
		bool Buffer = *(LPBYTE*)shared_base;
		if (Buffer)
			return false;
		PacketSM pck{};
		pck._packet._GMH.pid = pid;
		pck._packet._GMH.addr = 0;
		std::wstring wstr{ std::wstring(module_name.begin(), module_name.end()) };
		//memset(pck._packet._GMH.str, 0, sizeof(WCHAR) * 30);
		//wcscpy(pck._packet._GMH.str, wstr.c_str());
		DWORD bytes_read;
		while (!pck._packet._GMH.addr);
		return pck._packet._GMH.addr;
	}

	template <typename type>
	type readmem(uint64_t src) {
		type ret = {};
		while (CopyMem(PacketSM::typePacket::read, (uint64_t)&ret, src, sizeof(type)) == 0) {
			Sleep(0.00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001);
		};
		return ret;
	}

	template <typename type>
	void writemem(uint64_t dst_addr, type var) {
		type inp = var;
		while (CopyMem(PacketSM::typePacket::write, (uint64_t)&inp, dst_addr, sizeof(type)) == 0) {
			Sleep(0.00000000000000000000000001);
		};
	}

};
 sharedController _sharedController;

#define read(src_addr,type)  _sharedController.readmem<type>(src_addr)
#define write(dst_addr,var,type)  _sharedController.writemem<type>(dst_addr,var)