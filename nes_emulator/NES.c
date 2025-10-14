#include "NES.h"

NES CreateNES()
{
	NES nes;

	nes.cpu = CreateCPU();
	nes.ppu = CreatePPU();

	nes.cpu.ppu = &nes.ppu;

	return nes;
}

void RemoveCartNES(NES* nes)
{
	if (nes->cart.PRG != NULL)
		free(nes->cart.PRG);
	if (nes->cart.CHR != NULL)
		free(nes->cart.CHR);
}

void DestroyNES(NES* nes)
{
	free(nes->cpu.memory);
}


void SetCartNES(NES* nes, const char* path)
{
	nes->cart = InsertCart(path);
}

void ResetNES(NES *nes)
{
	nes->cpu.s -= 3;
	nes->cpu.i = 1;

	// mapper specifikus inicializálás itt. az enyém csak NROM-ot fogad
	// NROM esetén ha 16kb PRG van, akkor azt CPU:8000~BFFF és CPU:C000~FFFF-re rakja. 32kb esetén CPU:8000~FFFF
	int j = 0;
	for (int i = 0x8000; i <= 0xFFFF; i++)
	{
		nes->cpu.memory[i] = nes->cart.PRG[j];

		j++;
		if (nes->cart.PRG_size == 16384 && j == 16384)
		{
			// 16kb esetén tükrözés a CPU memóriájában
			j = 0;
		}
	}

	// ugrás a reset rutinra, amelynek címe CPU:FFFC-nél van
	nes->cpu.PC = nes->cpu.memory[0xFFFC] + 256 * nes->cpu.memory[0xFFFD];
	nes->cpu.currentCycleTimeInFrame = 7;
}

void TickNES(NES *nes)
{
	while (1)
	{
		TickCPU(&nes->cpu);
		nes->cpu.currentCycleTimeInFrame += nes->cpu.currentCycleTime;
		int ppuCyclesToDo = 3 * nes->cpu.currentCycleTime; // a PPU egy CPU órajel alatt 3 órajelet fut
		while (ppuCyclesToDo)
		{
			TickPPU(&nes->ppu);
			ppuCyclesToDo--;
		}
	}
	nes->cpu.currentCycleTimeInFrame = 0;
}
