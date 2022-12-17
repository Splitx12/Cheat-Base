#include <iostream>
#include <Windows.h>

#include "RWDRV.h"

namespace GameVars {
	uint64_t BaseAddress;
	uint64_t LocalPlayer;
	uint64_t EntityList;
}


struct vec3
{
	float x, y, z;

	vec3 operator-(vec3 ape)
	{
		return { x - ape.x, y - ape.y, z - ape.z };
	}

	vec3 operator+(vec3 ape)
	{
		return { x + ape.x, y + ape.y, z + ape.z };
	}

	vec3 operator*(float ape)
	{
		return { x * ape, y * ape, z * ape };
	}

	vec3 operator/(float ape)
	{
		return { x / ape, y / ape, z / ape };
	}

	vec3 operator/=(float ape)
	{
		x /= ape;
		y /= ape;
		z /= ape;

		return *this;
	}

	vec3 operator+=(vec3 ape)
	{
		return { x += ape.x, y += ape.y, z += ape.z };
	}

	vec3 operator-=(vec3 ape)
	{
		return { x -= ape.x, y -= ape.y, z -= ape.z };
	}



	void Normalize()
	{
		while (x > 89.0f)
			x -= 180.f;

		while (x < -89.0f)
			x += 180.f;

		while (y > 180.f)
			y -= 360.f;

		while (y < -180.f)
			y += 360.f;
	}

	float Length()
	{
		return sqrt((x * x) + (y * y) + (z * z));
	}

	float Length2D()
	{
		return sqrt((x * x) + (y * y));
	}

	float DistTo(vec3 ape)
	{
		return (*this - ape).Length();
	}

	float Dist2D(vec3 ape)
	{
		return (*this - ape).Length2D();
	}
};

using namespace std;

/*
void HideModule(HINSTANCE hModule)
{
	DWORD dwPEB_LDR_DATA = 0;
	_asm
	{
		pushad;
		pushfd;
		mov eax, fs: [30h] ;
		mov eax, [eax + 0Ch];
		mov dwPEB_LDR_DATA, eax;
		mov esi, [eax + 0Ch];
		mov edx, [eax + 10h];
	LoopInLoadOrderModuleList:
		lodsd;
		mov esi, eax;
		mov ecx, [eax + 18h];
		cmp ecx, hModule;
		jne SkipA
			mov ebx, [eax]
			mov ecx, [eax + 4]
			mov[ecx], ebx
			mov[ebx + 4], ecx
			jmp InMemoryOrderModuleList
			SkipA :
		cmp edx, esi
			jne LoopInLoadOrderModuleList
			InMemoryOrderModuleList :
		mov eax, dwPEB_LDR_DATA
			mov esi, [eax + 14h]
			mov edx, [eax + 18h]
			LoopInMemoryOrderModuleList :
			lodsd
			mov esi, eax
			mov ecx, [eax + 10h]
			cmp ecx, hModule
			jne SkipB
			mov ebx, [eax]
			mov ecx, [eax + 4]
			mov[ecx], ebx
			mov[ebx + 4], ecx
			jmp InInitializationOrderModuleList
			SkipB :
		cmp edx, esi
			jne LoopInMemoryOrderModuleList
			InInitializationOrderModuleList :
		mov eax, dwPEB_LDR_DATA
			mov esi, [eax + 1Ch]
			mov edx, [eax + 20h]
			LoopInInitializationOrderModuleList :
			lodsd
			mov esi, eax
			mov ecx, [eax + 08h]
			cmp ecx, hModule
			jne SkipC
			mov ebx, [eax]
			mov ecx, [eax + 4]
			mov[ecx], ebx
			mov[ebx + 4], ecx
			jmp Finished
			SkipC :
		cmp edx, esi
			jne LoopInInitializationOrderModuleList
			Finished :
		popfd;
		popad;
	}
}
*/
void EraseHeader(HINSTANCE hModule)
{
	if (!hModule)
		return;

	DWORD size, protect;


	PIMAGE_DOS_HEADER pDoH = (PIMAGE_DOS_HEADER)(hModule);
	PIMAGE_NT_HEADERS pNtH = (PIMAGE_NT_HEADERS)((LONG)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);

	size = sizeof(IMAGE_DOS_HEADER);
	if (VirtualProtect(pDoH, size, PAGE_READWRITE, &protect))
		for (DWORD i = 0; i < size; i++)
			*(BYTE*)((BYTE*)pDoH + i) = 1;

	size = sizeof(IMAGE_NT_HEADERS);
	if (pNtH && VirtualProtect(pNtH, size, PAGE_READWRITE, &protect))
		for (DWORD i = 0; i < size; i++)
			*(BYTE*)((BYTE*)pNtH + i) = 1;

	return;
}
vec3 oldPunch = { 0.f, 0.f, 0.f };
struct GlowMode
{
	int8_t GeneralGlowMode, BorderGlowMode, BorderSize, TransparentLevel;
};
struct vec2
{
	float x, y;

	vec2 operator-(vec2 ape)
	{
		return { x - ape.x, y - ape.y };
	}

	vec2 operator+(vec2 ape)
	{
		return { x + ape.x, y + ape.y };
	}

	vec2 operator*(float ape)
	{
		return { x * ape, y * ape };
	}

	vec2 operator/(float ape)
	{
		return { x / ape, y / ape };
	}

	vec2 operator/=(float ape)
	{
		x /= ape;
		y /= ape;

		return *this;
	}

	vec2 operator+=(vec2 ape)
	{
		return { x += ape.x, y += ape.y };
	}

	vec2 operator-=(vec2 ape)
	{
		return { x -= ape.x, y -= ape.y };
	}

	void Normalize()
	{
		if (x > 89.0f)
			x -= 180.f;

		if (x < -89.0f)
			x += 180.f;

		if (y > 180.f)
			y -= 360.f;

		if (y < -180.f)
			y += 360.f;
	}

	float Length2D()
	{
		return sqrt((x * x) + (y * y));
	}

	float Dist2D(vec2 ape)
	{
		return (*this - ape).Length2D();
	}
};

int main()
{

	SetConsoleTitle("Fuck EasyAntiCheats <3");

	std::cout << std::hex << "[Debug Info] -> [Module] -> " << (DWORD64)&GameVars::BaseAddress - (DWORD64)GetModuleHandle(0) << std::endl;
	while (!get_process_pid("r5apex.exe")) { std::cout << "[Wait] -> [Start the game(Apex)]"; Sleep(6000); system("cls");};
	_sharedController.attach("r5apex.exe");
	std::cout << "[STATUS] -> [Module Status]" << std::endl;
	std::cout << "[SUCCES] -> Attach the game" << std::endl;
	while (GameVars::BaseAddress == NULL) {
		std::cout << "[STATUS] -> [Module] -> " <<  GameVars::BaseAddress << " -> " << (DWORD64)&GameVars::BaseAddress - (DWORD64)GetModuleHandle(0) << std::endl;
		Sleep(3000);
	}
	GameVars::LocalPlayer = read(GameVars::BaseAddress + 0x1ebf528, uint64_t);
	GameVars::EntityList = read(GameVars::BaseAddress + 0x1b0e718, uint64_t);
	while (true) {
		std::cout << "[SUCCES] -> Start while" << std::endl;
		if (GetAsyncKeyState(VK_RBUTTON)) {
			// get current angles and recoil (punch angle)
			vec3 viewAngles = read(GameVars::LocalPlayer + 0x2584 - 0x14, vec3);
			vec3 punchAngle = read(GameVars::LocalPlayer + 0x2488, vec3);

			// calculate the new angles by adding the punchangle to the viewangles, it is important to subtract the old punch angle
			vec3 newAngle = viewAngles + (oldPunch - punchAngle) * (100 / 100.f);

			newAngle.Normalize(); // clamp angles
			vec2 Buffer;
			Buffer.x = newAngle.x;
			Buffer.y = newAngle.y;

			write(GameVars::LocalPlayer + 0x2584 - 0x14, Buffer, vec2); // overwrite old angles

			oldPunch = punchAngle;
		};

	}





	return TRUE;
}

