#pragma once
#include "MemoryHelper.h"
#include <cstddef>



#define _DBG_

template <typename... Args>
void DBG(const char* format, Args... args)
{
#ifdef _DBG_
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, args...);
#endif 
}

struct PacketSM
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
			wchar_t str[20];
			uint64_t addr;
		}_GMH;
	}_packet;
	//bool responseStatus;//1-success,0-failure reserved
}_PacketSM;

PVOID address_to_structure = reinterpret_cast<PVOID>(&_PacketSM);



NTSTATUS Sleep(LONGLONG ms_duration)
{
	LARGE_INTEGER delay;
	delay.QuadPart = -1 * (10000 * ms_duration);
	return KeDelayExecutionThread(KernelMode, FALSE, &delay);
}
NTSTATUS copy_memory(PEPROCESS src_proc, PEPROCESS target_proc, PVOID src, PVOID dst, SIZE_T size);
NTSTATUS Test() {
	_MemoryHelper.attach(L"Client.exe");
	DWORD64 ClientBase = _MemoryHelper.get_module_handle(L"Client.exe") + 0x5200;
	while (!_MemoryHelper.GetProcessId(L"r5apex.exe"));
	uint32_t TargetProccess = _MemoryHelper.GetProcessId(L"r5apex.exe");
	uint64_t Base = get_modulehandle(L"r5apex.exe", TargetProccess); 
	DBG("PacketSM->Base-> %u", Base);
	write(ClientBase + 0x5660 - 0x5200, Base, uint64_t); // изменить на базу длл


	while (1) {
		_MemoryHelper.rm(ClientBase, (uint64_t)address_to_structure, sizeof(_PacketSM));
		//DBG("PacketSM->access-> %u", _PacketSM.access);
		if (_PacketSM.access) {
			//DBG("typePacket-> %u",_PacketSM._typePacket);
			if (_PacketSM._typePacket == PacketSM::typePacket::read) {
				
				PEPROCESS src_pid;
				PsLookupProcessByProcessId((HANDLE)_PacketSM._packet._RW.pid, &src_pid);
				//DBG("_PacketSM->_packet->_RW.pid-> %u", _PacketSM._packet._RW.pid);
				//DBG("src_pid %u", src_pid);
				//DBG("_PacketSM->_packet->_RW->src_addr-> %u", _PacketSM._packet._RW.src_addr);
				//DBG("_PacketSM->_packet->_RW->dst_addr-> %u", _PacketSM._packet._RW.dst_addr);
				//DBG("_PacketSM->_packet->_RW->size-> %u", _PacketSM._packet._RW.size);
				
				copy_memory(src_pid, _MemoryHelper.TargetPEprocess, (PVOID)_PacketSM._packet._RW.src_addr, (PVOID)_PacketSM._packet._RW.dst_addr, _PacketSM._packet._RW.size);
				write(ClientBase, 0, bool);
			}
			else if (_PacketSM._typePacket == PacketSM::typePacket::write) {
				PEPROCESS src_pid;

				PsLookupProcessByProcessId((HANDLE)_PacketSM._packet._RW.pid, &src_pid);
				copy_memory(_MemoryHelper.TargetPEprocess, src_pid, (PVOID)_PacketSM._packet._RW.dst_addr, (PVOID)_PacketSM._packet._RW.src_addr, _PacketSM._packet._RW.size);
				write(ClientBase, 0, bool);
				//copy_memory(ClientPE, src_pid, (PVOID)_PacketSM._packet._RW.src_addr, (PVOID)_PacketSM._packet._RW.dst_addr, _PacketSM._packet._RW.size);
			}
			else if (_PacketSM._typePacket == PacketSM::typePacket::get_module_handle) {

			}
			else {
				return STATUS_SUCCESS;
			}
		}
		if (!_MemoryHelper.GetProcessId(L"Client.exe")) { return STATUS_SUCCESS; }
		
		//_MemoryHelper.wm(ClientBase, sizeof(PacketS), address_to_structures);
		

	}
	return STATUS_SUCCESS;
}


NTSTATUS copy_memory(PEPROCESS src_proc, PEPROCESS target_proc, PVOID src, PVOID dst, SIZE_T size) {
	SIZE_T bytes;
	return MmCopyVirtualMemory(src_proc, src, target_proc, dst, size, UserMode, &bytes);
}
