#include "PPU.h"

PPU CreatePPU()
{
	PPU ppu;

	memset(&ppu, 0, sizeof(ppu));
	ppu.memory = (uint8_t*)malloc(0x4000); // 16 kb 
	ppu.oam = (uint8_t*)malloc(0x100); // 256 bájt

	return ppu;
}

void DestroyPPU(PPU* ppu)
{
	free(ppu->memory);
	free(ppu->oam);
}

void WritingToPPUReg(PPU* ppu, uint16_t reg, uint8_t value)
{
	switch (reg)
	{
	case PPU_REG_CTRL: {
		ppu->t.nametableSelect = value & 0b11;
		ppu->vram32Increment			  = value & (1 << 2);
		ppu->spritesSecondPatternSelected = value & (1 << 3);
		ppu->bgSecondPatternSelected	  = value & (1 << 4);
		ppu->spritesAre8x16				  = value & (1 << 5);
		ppu->nmiEnabled					  = value & (1 << 7);
		break;
	}
	case PPU_REG_MASK: {
		ppu->greyscale			  = value & (1 << 0);
		ppu->bgShowLeftmost8	  = value & (1 << 1);
		ppu->spritesShowLeftmost8 = value & (1 << 2);
		ppu->renderBg			  = value & (1 << 3);
		ppu->renderSprites		  = value & (1 << 4);
		ppu->emphasizeRed		  = value & (1 << 5);
		ppu->emphasizeGreen		  = value & (1 << 6);
		ppu->emphasizeBlue		  = value & (1 << 7);
		break;
	}
	case PPU_REG_STATUS: {
		break;
	}
	case PPU_REG_OAMADDR: {
		// ezt most én nem implementálom, mivel a játékok általában nem használják
		break;
	}
	case PPU_REG_OAMDATA: {
		// ezt most én nem implementálom, mivel a játékok általában nem használják
		break;
	}
	case PPU_REG_SCROLL: {
		if (!ppu->secondWrite)
		{
			ppu->t.coarseX = (value >> 3);
			ppu->x = value & 0b111;
		}
		else
		{
			ppu->t.coarseY = (value >> 3);
			ppu->t.fineY = value & 0b111;
		}
		ppu->secondWrite = !ppu->secondWrite;
		break;
	}
	case PPU_REG_ADDR: {
		if (!ppu->secondWrite)
		{
			ppu->t.value = (ppu->t.value & 0b11111111) + (value & 0b00111111) << 8;
		}
		else
		{
			ppu->t.value = (ppu->t.value & 0b11111100000000) + value;

			ppu->v = ppu->t;
		}
		ppu->secondWrite = !ppu->secondWrite;
		break;
	}
	case PPU_REG_DATA: {
		ppu->memory[ppu->v.value] = value;
		if (ppu->vram32Increment)
			ppu->v.value += 32;
		else
			ppu->v.value += 1;
		break;
	}
	case PPU_REG_OAMDMA: {
		ppu->oamDmaPage = value;
		break;
	}
	default:
		break;
	}
}
