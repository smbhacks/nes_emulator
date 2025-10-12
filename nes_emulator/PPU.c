#include "PPU.h"

PPU CreatePPU()
{
	PPU ppu;

	ppu.memory = (uint8_t*)malloc(0x4000); // 16 kb heap memória 
	
	return ppu;
}

void DestroyPPU(PPU* ppu)
{
	free(ppu->memory);
}
