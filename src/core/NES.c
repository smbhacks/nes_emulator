#include "NES.h"
#include "CPU.h"

NES* CreateNES()
{
	printf("Creating NES...\n");

	NES* nes = malloc(sizeof(NES));

	nes->cpu = CreateCPU();
	nes->ppu = CreatePPU();
	nes->controller = CreateController();

    nes->cpu->ppu = nes->ppu;
    nes->cpu->controller = nes->controller;

	nes->cartInserted = false;

	return nes;
}

void UseCustomPalette(NES* nes, uint8_t* pal_ptr)
{
	FreeCustomPalette(nes->ppu);

	nes->ppu->palette = pal_ptr;
	nes->ppu->using_default_palette = false;
}

void RemoveCartNES(NES* nes)
{
	if(nes->cartInserted) 
	{
		FreeCart(nes->cart);
		nes->cartInserted = false;
	}
}

void DestroyNES(NES* nes)
{
	FreeCPU(nes->cpu);
	FreePPU(nes->ppu);
	FreeController(nes->controller);

	printf("Freeing NES...\n");
	free(nes);
}

void SetCartNES(NES* nes, const char* path)
{
	nes->cart = InsertCart(path);
	if (nes->cart != NULL)
	{
		nes->cartInserted = true;
		nes->cpu->cart = nes->cart;
		nes->ppu->cart = nes->cart;
	}
	else
		nes->cartInserted = false;
}

void ResetNES(NES *nes)
{
	if (!nes->cartInserted)
		return;

	nes->cpu->s -= 3;
	nes->cpu->i = 1;

	/*
	// mapper specifikus inicializálás itt. az enyém csak NROM-ot fogad
	// NROM esetén ha 16kb PRG van, akkor azt CPU:8000~BFFF és CPU:C000~FFFF-re rakja. 32kb esetén CPU:8000~FFFF
	// PRG:
	int j = 0;
	for (int i = 0x8000; i <= 0xFFFF; i++)
	{
		nes->cpu->memory[i] = nes->cart->PRG[j];

		j++;
		if (nes->cart->PRG_size == 16384 && j == 16384)
		{
			// 16kb esetén tükrözés a CPU memóriájában
			j = 0;
		}
	}
	// CHR
	// Ha 0, akkor CHR-RAMot használ a játék, vagyis a CPU generálja a grafikát a PPU-ra 
	// Ha nem 0, akkor mi fogjuk rárakni (és ezt egyébként a CPU nem tudja felülírni), ez CHR-ROM
	// Ha nem 0, akkor van legalább 8kb CHR-ROM
	if (nes->cart->CHR_size != 0)
	{
		for (int i = 0x0000; i <= 0x1FFF; i++)
		{
			nes->ppu->memory[i] = nes->cart->CHR[i];
		}
	}
	*/

	// ugrás a reset rutinra, amelynek címe CPU:FFFC-nél van
	nes->cpu->PC = ReadCpuMem(nes->cpu, 0xFFFC) + 256 * ReadCpuMem(nes->cpu, 0xFFFD);
	//nes->cpu.PC = 0xC5F5;
	nes->cpu->currentCycleTimeInFrame = 7;
}

void TickNES(NES *nes)
{
	if(!nes->cartInserted)
		return;

	nes->ppu->endOfFrame = false;
	while (!nes->ppu->endOfFrame)
	{
		TickCPU(nes->cpu);
		nes->cpu->currentCycleTimeInFrame += nes->cpu->currentCycleTime;
		int ppuCyclesToDo = 3 * nes->cpu->currentCycleTime; // a PPU egy CPU órajel alatt 3 órajelet fut
		while (ppuCyclesToDo)
		{
			TickPPU(nes->ppu);
			ppuCyclesToDo--;
		}
	}
	DrawSprites(nes->ppu); // mivel a játékok általában Vblankben hívják meg az OAM DMA-t, ezért én most így egyszerűen kezelem
	DrawBackgroundColor(nes->ppu);
	nes->cpu->currentCycleTimeInFrame = 0;
}
