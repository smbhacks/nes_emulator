#include "PPU.h"

uint8_t palettes[];

PPU CreatePPU()
{
	PPU ppu;

	memset(&ppu, 0, sizeof(ppu));
	ppu.memory = (uint8_t*)malloc(0x4000); // 16 kb 
	ppu.oam = (uint8_t*)malloc(0x100); // 256 bájt
	ppu.display = (uint8_t*)malloc(256 * 240 * 3); // RGB24 kijelző

	return ppu;
}

void DestroyPPU(PPU* ppu)
{
	free(ppu->memory);
	free(ppu->oam);
}

// used for debugging purposes
void PutRGBAtPPUDot(PPU* ppu, uint8_t r, uint8_t g, uint8_t b)
{
	int x = ppu->ppuDotX - 1;
	int y = ppu->ppuDotY;
	if (x < 256 && y < 240 && y > -1)
	{
		uint8_t* pixel = ppu->display + y * 256 * 3 + x * 3;
		pixel[0] = r;
		pixel[1] = g;
		pixel[2] = b;
	}
}

void WritingToPPUReg(PPU* ppu, uint16_t reg, uint8_t value)
{
	switch (reg)
	{
	case PPU_REG_CTRL: {
		int prevNMIenabled = ppu->nmiEnabled;

		ppu->t.nametableSelect = value & 0b11;
		ppu->vram32Increment			  = value & (1 << 2);
		ppu->spritesSecondPatternSelected = value & (1 << 3);
		ppu->bgSecondPatternSelected	  = value & (1 << 4);
		ppu->spritesAre8x16				  = value & (1 << 5);
		ppu->nmiEnabled					  = value & (1 << 7);

		// https://www.nesdev.org/wiki/NMI#Old_emulators
		// az NES akkor is generál egy NMI-t, ha a PPU_REG_CTRL-on 0-ról 1-re írjuk az NMI enable bitet és a PPU vblank flagje magas 
		if (ppu->vblankFlag && !prevNMIenabled && ppu->nmiEnabled)
			ppu->generateNMI = true;

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
			ppu->fineX = value & 0b111;
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
			ppu->t.value = (ppu->t.value & 0b11111111) + ((value & 0b00111111) << 8);
		}
		else
		{
			ppu->t.value = (ppu->t.value & 0b111111100000000) + value;

			ppu->v = ppu->t;
		}
		ppu->secondWrite = !ppu->secondWrite;
		break;
	}
	case PPU_REG_DATA: {
		if(ppu->v.value == 0x3f10)
			ppu->memory[0x3f00] = value; // a 0x3f00 és 0x3f10 változtatja meg a háttérszínt
		else
			ppu->memory[ppu->v.value] = value; 

		if (ppu->vram32Increment)
			ppu->v.value += 32;
		else
			ppu->v.value += 1;
		break;
	}
	default:
		break;
	}
}

uint8_t ReadingFromPPUReg(PPU* ppu, uint16_t reg)
{
	switch (reg)
	{
	case PPU_REG_CTRL: {
		return 0;
	}
	case PPU_REG_MASK: {
		return 0;
	}
	case PPU_REG_STATUS: {
		uint8_t value = (ppu->vblankFlag << 7) + (ppu->sprite0Flag << 6);
		ppu->vblankFlag = 0; //olvasáson törtlése kerül
		return value;
	}
	case PPU_REG_OAMADDR: {
		return 0;
	}
	case PPU_REG_OAMDATA: {
		// most nem implementálom, mert már maga a konzolon bugos és szinte semmilyen játék nem használja
		return 0;
	}
	case PPU_REG_SCROLL: {
		return 0;
	}
	case PPU_REG_ADDR: {
		return 0;
	}
	case PPU_REG_DATA: {
		// több részlet PPU.h-ban a PPU struct-ban
		uint8_t value = ppu->PPUReadBuff;
		ppu->PPUReadBuff = ppu->memory[ppu->v.value];
		if (ppu->vram32Increment)
			ppu->v.value += 32;
		else
			ppu->v.value += 1;
		return value;
	}
	default:
		return 0;
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

//	PutRGBAtPPUDot(ppu, 255, 0, 0); // debug
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

		// 257-nél a t X-et átmásoljuk v X-be és nametable alsó bitjét beállítjuk
		if (ppu->ppuDotX == 257) {
			ppu->v.coarseX = ppu->t.coarseX;
			ppu->v.nametableSelect = (ppu->v.nametableSelect & 2) | (ppu->t.nametableSelect & 1); // alsó bit
		}

		// pre-render scanline-on vert(v) = vert(t) 280 és 304 között
		if (ppu->ppuDotY == -1 && (280 <= ppu->ppuDotX && ppu->ppuDotX <= 304)) {
			ppu->v.coarseY = ppu->t.coarseY;
			ppu->v.nametableSelect = (ppu->v.nametableSelect & 1) | (ppu->t.nametableSelect & 2); // felső bit
			ppu->v.fineY = ppu->t.fineY;
		}
	}
}

void UpdateVblankFlag(PPU* ppu)
{
	// flag beállítása (1, 241)-nél
	if (ppu->ppuDotX == 1 && ppu->ppuDotY == 241)
		ppu->vblankFlag = 1;

	// flag törlése (1, -1)-nél (pre-render)
	if (ppu->ppuDotX == 1 && ppu->ppuDotY == -1)
		ppu->vblankFlag = 0;
}

void UpdateSprite0Flag(PPU* ppu)
{
	// flag törlése (1, -1)-nél (pre-render)
	if (ppu->ppuDotX == 1 && ppu->ppuDotY == -1)
		ppu->sprite0Flag = 0;
}

int GetPixelColor(PPU* ppu, uint8_t tileValue, bool useSecondPattern, int x, int y)
{
	// address megszerzése
	uint16_t tileAddress = tileValue * 0x10;
	if (useSecondPattern)
		tileAddress += 0x1000;

	// tile-en belüli szín érték megszerzése
	int pixelRowByteIndex = y % 8; // bájt érték
	int pixelColBitIndex = x % 8; // bit érték
	uint16_t pixelRowAddress = tileAddress + pixelRowByteIndex;
	uint8_t pixelRowPlane1 = ppu->memory[pixelRowAddress + 0];
	uint8_t pixelRowPlane2 = ppu->memory[pixelRowAddress + 8];
	int pixelColorPlane1 = (pixelRowPlane1 & (0x80 >> pixelColBitIndex)) >> (7 - pixelColBitIndex);
	int pixelColorPlane2 = (pixelRowPlane2 & (0x80 >> pixelColBitIndex)) >> (7 - pixelColBitIndex);

	return pixelColorPlane1 + 2 * pixelColorPlane2;
}

void DrawPixel(PPU* ppu, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	// rajzolás
	uint8_t* pixel = ppu->display + y * 256 * 3 + x * 3;
	pixel[0] = r;
	pixel[1] = g;
	pixel[2] = b;
}

void DrawPixelWithPal(PPU* ppu, uint8_t paletteValue, int x, int y)
{
	// pixel szín értékhez paletta hozzárendelés
	uint8_t r = palettes[3 * paletteValue + 0];
	uint8_t g = palettes[3 * paletteValue + 1];
	uint8_t b = palettes[3 * paletteValue + 2];

	DrawPixel(ppu, x, y, r, g, b);
}

void SubFromNamX(uint16_t* addr, int x)
{
	int result = (*addr % 32) - x;
	while (result < 0)
	{
		// és mozgassuk el az előző nametablebe
		if ((*addr & 0xc00) != 0x000 && (*addr & 0xc00) != 0x800)
			*addr -= 0x400;
		else
			*addr += 0x400; // a nametablet 3-ra állítjuk, ha 0.-on vagyunk most (0-tól számozva)

		*addr += 0x20; // javítsuk meg az coarseY-et

		result += 32;
	}
	*addr -= x;
}

void AddToNamX(uint16_t* addr, int x)
{
	int result = (*addr % 32) + x;
	while (result >= 32)
	{
		// és mozgassuk el az következő nametablebe
		if ((*addr & 0xc00) != 0x400 && (*addr & 0xc00) != 0xc00)
			*addr += 0x400;
		else
			*addr -= 0x400;

		*addr -= 0x20; // javítsuk meg az coarseY-et

		result -= 32;
	}
	*addr += x;
}

void DrawPPUDot(PPU* ppu)
{
	int x = ppu->ppuDotX - 1;
	int y = ppu->ppuDotY;

	// tile cím megszerzése
	uint16_t addrOnNam = PPU_MEM_NAMETABLES_START + ppu->v.coarseX + (ppu->v.coarseY << 5) + PPU_MEM_NAMETABLE_SIZE * ppu->v.nametableSelect;
	// itt azért vonok ki 2-őt a végéből, mert az NES az első 2 oszlop grafikáját az előző scanline láthatatlan részén fetcheli
	SubFromNamX(&addrOnNam, 2);
	AddToNamX(&addrOnNam, ((x % 8) + ppu->fineX) / 8);

	uint8_t tileValue = ppu->memory[addrOnNam];
	int pixelColor = GetPixelColor(ppu, tileValue, ppu->bgSecondPatternSelected, (x % 8) + ppu->fineX, y);

	if (pixelColor != 0)
	{
		// attribute chunk (2x2 metatilera van, minden metatile-ra jut 2 bit)
		// 1 metatile = 2x2 tile
		uint8_t attributeByte = ppu->memory[0x3c0 + (addrOnNam & 0xfc00) + (addrOnNam % 32) / 4 + ((addrOnNam & 0x380) >> 4)];
		//(jobb also << 6) | (bal also << 4) | (jobb felso << 2) | (bal felso << 0)
		int aX = addrOnNam & 0b10 ? 2 : 0;
		int aY = addrOnNam & 0x40 ? 4 : 0;
		int attrBitLoc = aX + aY;
		int paletteIndex = (attributeByte & (0b11 << attrBitLoc)) >> attrBitLoc;
		int paletteValue = ppu->memory[PPU_MEM_PALETTES_START + 4 * paletteIndex + pixelColor];

		DrawPixelWithPal(ppu, paletteValue, x, y);
	}
	else
	{
		// ha átlátszó pixel, rajzoljunk magentát egyelőre 
		// (hogy majd a spriteok tudják, hogy lehet-e az adott pixelen rajzolódni-e
		DrawPixel(ppu, x, y, 255, 0, 255);
	}
}

// https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
void TickPPU(PPU* ppu)
{
	// jelenlegi ppu pont rajzolása
	if (1 <= ppu->ppuDotX && ppu->ppuDotX <= 256 && 0 <= ppu->ppuDotY && ppu->ppuDotY <= 239)
		DrawPPUDot(ppu);

	// flagek updatelése 
	UpdateSprite0Flag(ppu);
	UpdateVblankFlag(ppu);
	if(ppu->renderBg)
		UpdateV(ppu);

	// Vblank kezdetén:
	if (ppu->ppuDotX == 0 && ppu->ppuDotY == 240) {
		ppu->endOfFrame = true; // ez az SDL/emuláció loop miatt fontos, hogy tudjunk frissíteni kijelzőt 60hz-ben
		
		if(ppu->nmiEnabled)
			ppu->generateNMI = true; // a konzol generáljon egy NMI interruptot, ha engedélyezve van
	}

	// állítjuk be a sprite0 flaget, ha sikerült rajzolni
	if (ppu->ppuDotX > ppu->sprite0_X && ppu->ppuDotY >= (ppu->sprite0_Y-1))
	{
		ppu->sprite0Flag = true;
	}

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

bool isDisplayMagenta(PPU* ppu, int x, int y)
{
	return ppu->display[3 * (256 * y + x) + 0] == 255 && ppu->display[3 * (256 * y + x) + 1] == 0 && ppu->display[3 * (256 * y + x) + 2] == 255;
}

void DrawOneSprite(PPU* ppu, uint8_t spriteX, uint8_t spriteY, uint8_t spriteTile, uint8_t spriteAttributes, bool isSprite0)
{
	for (int y = 0; y < 8; y++)
	{
		if (spriteY + y >= 240)
			break; // ha túlmentünk y irányban a képernyőn, akkor már végeztünk ezzel a sprite-tal

		for (int x = 0; x < 8; x++)
		{
			if (spriteX + x >= 256)
				break; // ha túlmentünk x irányban a képernyőn, akkor végeztünk ezzel a sorral

			int xToDraw = spriteAttributes & 0x40 ? 7 - x : x; // horizontális flip, ha az attribute 6.-ik bitje 1
			int yToDraw = spriteAttributes & 0x80 ? 7 - y : y; // vertikális flip, ha az attribute 7.-ik bitje 1
			int pixelColor = GetPixelColor(ppu, spriteTile, ppu->spritesSecondPatternSelected, xToDraw, yToDraw);
			if (pixelColor != 0)
			{
				// rajzoljuk, ha nem egy átlátszó pixel
				int paletteIndex = spriteAttributes & 0b11;
				int paletteValue = ppu->memory[PPU_MEM_PALETTES_START + 4 * paletteIndex + pixelColor + 0x10];
				int displayX = spriteX + x;
				int displayY = spriteY + y;
				bool behindBg = spriteAttributes & 0b00100000;
				if(!behindBg || isDisplayMagenta(ppu, displayX, displayY))
					DrawPixelWithPal(ppu, paletteValue, displayX, displayY);
				if (isSprite0)
				{
					ppu->sprite0_X = displayX;
					ppu->sprite0_Y = displayY;
				}
			}
		}
	}
}

// https://www.nesdev.org/wiki/PPU_OAM
void DrawSprites(PPU* ppu)
{
	// oam-t rajzolja
	for (int i = 0x100-4; i >= 0; i -= 4)
	{
		uint8_t spriteY			 = ppu->oam[i + 0] + 1; // +1 mert a valóságban a PPU 1 scanline-nal később rajzolná
 		uint8_t spriteTile		 = ppu->oam[i + 1];
		uint8_t spriteAttributes = ppu->oam[i + 2];
		uint8_t spriteX			 = ppu->oam[i + 3];
		DrawOneSprite(ppu, spriteX, spriteY, spriteTile, spriteAttributes, i == 0);
	}
}

void DrawBackgroundColor(PPU* ppu)
{
	uint8_t bgPal = ppu->memory[0x3f00];
	for (int y = 0; y < 240; y++)
		for (int x = 0; x < 256; x++)
			if (isDisplayMagenta(ppu, x, y))
				DrawPixelWithPal(ppu, bgPal, x, y);
}

// https://www.nesdev.org/wiki/File:2C02G_wiki.pal
// 00~3F-ig alap NES színek RGB24
uint8_t palettes[] = {
	0x62, 0x62, 0x62, 0x00, 0x1C, 0x95, 0x19, 0x04, 0xAC, 0x42, 0x00, 0x9D, 0x61, 0x00, 0x6B, 0x6E,
	0x00, 0x25, 0x65, 0x05, 0x00, 0x49, 0x1E, 0x00, 0x22, 0x37, 0x00, 0x00, 0x49, 0x00, 0x00, 0x4F,
	0x00, 0x00, 0x48, 0x16, 0x00, 0x35, 0x5E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xAB, 0xAB, 0xAB, 0x0C, 0x4E, 0xDB, 0x3D, 0x2E, 0xFF, 0x71, 0x15, 0xF3, 0x9B, 0x0B, 0xB9, 0xB0,
	0x12, 0x62, 0xA9, 0x27, 0x04, 0x89, 0x46, 0x00, 0x57, 0x66, 0x00, 0x23, 0x7F, 0x00, 0x00, 0x89,
	0x00, 0x00, 0x83, 0x32, 0x00, 0x6D, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0x57, 0xA5, 0xFF, 0x82, 0x87, 0xFF, 0xB4, 0x6D, 0xFF, 0xDF, 0x60, 0xFF, 0xF8,
	0x63, 0xC6, 0xF8, 0x74, 0x6D, 0xDE, 0x90, 0x20, 0xB3, 0xAE, 0x00, 0x81, 0xC8, 0x00, 0x56, 0xD5,
	0x22, 0x3D, 0xD3, 0x6F, 0x3E, 0xC1, 0xC8, 0x4E, 0x4E, 0x4E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xBE, 0xE0, 0xFF, 0xCD, 0xD4, 0xFF, 0xE0, 0xCA, 0xFF, 0xF1, 0xC4, 0xFF, 0xFC,
	0xC4, 0xEF, 0xFD, 0xCA, 0xCE, 0xF5, 0xD4, 0xAF, 0xE6, 0xDF, 0x9C, 0xD3, 0xE9, 0x9A, 0xC2, 0xEF,
	0xA8, 0xB7, 0xEF, 0xC4, 0xB6, 0xEA, 0xE5, 0xB8, 0xB8, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};