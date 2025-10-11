#include "NES.h"

NES CreateNES()
{
	NES nes;

	nes.cpu = CreateCPU();

	return nes;
}

void SetCartNES(NES* nes, const char* path)
{
	nes->cart = InitCart(path);
}

void ResetNES(NES *nes)
{
	nes->cpu.s -= 3;
	nes->cpu.i = 1;
}

void TickNES(NES *nes)
{
}
