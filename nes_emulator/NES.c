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
}

void TickNES(NES *nes)
{
	while (1)
	{
		nes->cpu.currentCycleTime = 0;
		TickCPU(&nes->cpu);
	}
}
