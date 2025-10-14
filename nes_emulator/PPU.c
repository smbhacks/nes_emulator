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

void IncHoriV(PPU* ppu)
{
	// coarseX ezért 5-bites, mivel ezzel választjuk ki, hogy melyik 8 pixel hosszú tile-on vagyunk
	// ebből 32 fér a képernyőre 
	ppu->v.coarseX++;
	if (ppu->v.coarseX == 0)
		// ha coarseX túlcsordulás történt (31->0), akkor válasszuk ki a következő horizontális nametable-t
		// 0: 0x2000 bal 1
		// 1: 0x2400 jobb 1
		// 2: 0x2800 bal 2
		// 3: 0x2c00 jobb 2
		ppu->v.nametableSelect ^= 1;
}

void IncVertV(PPU* ppu)
{
	ppu->v.fineY++;
	if (ppu->v.fineY == 0) {
		// ha fineY túlcsordulás történt (7->0), akkor növeljük coarseY-t
		ppu->v.coarseY++;
		if (ppu->v.coarseY == 30) {
			// ha egy nametable végére értünk
			ppu->v.coarseY = 0;
			ppu->v.nametableSelect ^= 2; // válasszuk ki a következő vertikális nametable-t (0, 1: "fent"; 2, 3: "lent")
		}
	}
}

void UpdateV(PPU* ppu)
{
	if (ppu->ppuDotY < 240) // pre-rendertől utolsó látható scanline-ig
	{
		// hori(v) növelés 0~256 között és 328 után
		if (((0 < ppu->ppuDotX && ppu->ppuDotX <= 256) || ppu->ppuDotX >= 328) && (ppu->ppuDotX % 8 == 0))
			IncHoriV(ppu);

		// vert(v) növelés
		if (ppu->ppuDotX == 256)
			IncVertV(ppu);

		// 257-nél a t X-et átmásoljuk v X-be
		if (ppu->ppuDotX == 257) 
			ppu->v.coarseX = ppu->t.coarseX;

		// pre-render scanline-on vert(v) = vert(t) 280 és 304 között
		if (ppu->ppuDotY == -1 && (280 <= ppu->ppuDotX && ppu->ppuDotX <= 304)) {
			ppu->v.coarseY = ppu->t.coarseY;
			ppu->v.fineY = ppu->t.fineY;
		}
	}
}

void UpdateVblankFlag(PPU* ppu)
{
	// flag beállítása (1, 241)-nél
	if (ppu->ppuDotX == 1 && ppu->ppuDotY == 241)
		ppu->vblankFlag = true;

	// flag törlése (1, -1)-nél (pre-render)
	if (ppu->ppuDotX == 1 && ppu->ppuDotY == -1)
		ppu->vblankFlag = false;
}

void UpdateSprite0Flag(PPU* ppu)
{
	// flag törlése (1, -1)-nél (pre-render)
	if (ppu->ppuDotX == 1 && ppu->ppuDotY == -1)
		ppu->sprite0Flag = false;
}

// https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
void TickPPU(PPU* ppu)
{
	// flagek updatelése 
	UpdateSprite0Flag(ppu);
	UpdateVblankFlag(ppu);
	if(ppu->renderBg)
		UpdateV(ppu);

	// következő ppu pont beállítása
	ppu->ppuDotX++;
	if (ppu->ppuDotX >= 340)
	{
		ppu->ppuDotX = 0;
		ppu->ppuDotY++;
		if (ppu->ppuDotY >= 260)
		{
			ppu->ppuDotY = -1;
		}
	}
}
