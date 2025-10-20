#include "NES.h"

NES CreateNES()
{
	NES nes;

	nes.cpu = CreateCPU();
	nes.ppu = CreatePPU();
	nes.controller = CreateController();

	//nes.cpu.ppu = &nes.ppu; // EZ ITT NEM JÓ, MERT A VEREMRE KERÜLT PPU TÖRLÉSRE KERÜL A FÜGGVÉNY UTÁN, ÉS EZ A POINTER ROSSZ CÍMRE MUTAT

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
	// PRG:
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
	// CHR
	// Ha 0, akkor CHR-RAMot használ a játék, vagyis a CPU generálja a grafikát a PPU-ra 
	// Ha nem 0, akkor mi fogjuk rárakni (és ezt egyébként a CPU nem tudja felülírni), ez CHR-ROM
	// Ha nem 0, akkor van legalább 8kb CHR-ROM
	if (nes->cart.CHR_size != 0)
	{
		for (int i = 0x0000; i <= 0x1FFF; i++)
		{
			nes->ppu.memory[i] = nes->cart.CHR[i];
		}
	}

	// ugrás a reset rutinra, amelynek címe CPU:FFFC-nél van
	nes->cpu.PC = nes->cpu.memory[0xFFFC] + 256 * nes->cpu.memory[0xFFFD];
	//nes->cpu.PC = 0xC5F5;
	nes->cpu.currentCycleTimeInFrame = 7;
}

void TickNES(NES *nes)
{
	nes->ppu.endOfFrame = false;
	while (!nes->ppu.endOfFrame)
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
	DrawSprites(&nes->ppu); // mivel a játékok általában Vblankben hívják meg az OAM DMA-t, ezért én most így egyszerűen kezelem
	DrawBackgroundColor(&nes->ppu);
	nes->cpu.currentCycleTimeInFrame = 0;
}
