#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define PPU_MEM_PATTERN_TABLE_SIZE     0x1000
#define PPU_MEM_PATTERN_TABLES_START   0x0000
#define PPU_MEM_NAMETABLE_SIZE		   0x3c0
#define PPU_MEM_ATTRIBUTE_TABLE_SIZE   0x40
#define PPU_MEM_NAMETABLES_START	   0x2000
#define PPU_MEM_PALETTES_SIZE          0x20
#define PPU_MEM_PALETTES_START         0x3f00

#define PPU_REG_CTRL 	  0x2000
#define PPU_REG_MASK 	  0x2001
#define PPU_REG_STATUS    0x2002
#define PPU_REG_OAMADDR   0x2003
#define PPU_REG_OAMDATA   0x2004
#define PPU_REG_SCROLL    0x2005
#define PPU_REG_ADDR      0x2006
#define PPU_REG_DATA      0x2007
#define PPU_REG_OAMDMA    0x4014

typedef struct PPU {
	// 16 kb memória, amelyet a CPU tud manipulálni
	uint8_t* memory; 
	// (Object Attribute Memory) 256 byteos PPU belső memória, amely a "sprite"-okat rajzolja. 64 sprite tárolására képes. A CPU ezt   
	// 3 regiszterrel tudja módosítani. Általában csak 1-et használnak, az "OAMDMA" regisztert, amely kimásolja a CPU egy 256 bájtnyni részét a PPU OAMjába.
	uint8_t* oam;

	// 0 idle
	// 1~256 látható pontok
	// 257~340 nem látható pontok (most az egyszerűség kedvéért ezt elhanyagolom)
	int ppuDotX;
	// -1 renderelés előtti scanline (pre-render scanline)
	// 0~239 láthathó scanline (kijelzőn látszik!)
	// 240 renderelés utáni scanline
	// 241~260 vblank 
	int ppuDotY;
	
	// PPU_REG_CTRL
	bool vram32Increment; //false: +1, előre megy, true: +32, lefelé megy (PPU_REG_DATA írás után)
	bool spritesSecondPatternSelected; //false: 0x0000, true: 0x1000
	bool bgSecondPatternSelected; //false: 0x0000, true: 0x1000
	bool spritesAre8x16; //false: 8x8, true:8x16 (pixelben)
	bool nmiEnabled;

	// PPU_REG_MASK
	bool greyscale;
	bool bgShowLeftmost8; //a kijelző bal oldalán lévő 8 pixeles hosszú sávban rajzoljunk-e bg-t?
	bool spritesShowLeftmost8;
	bool renderBg;
	bool renderSprites;
	bool emphasizeRed;
	bool emphasizeGreen;
	bool emphasizeBlue;

	// PPU_REG_STATUS
	int vblankFlag; //ez egy belső flag, nem mindig tükrözi a valóságot (https://www.nesdev.org/wiki/PPU_registers#Vblank_flag)
	int sprite0Flag; //true ha a 0. sprite rajzolását érzékelte a PPU

	// A PPU túl lassú, hogy időben odaadja az $2007 által olvasott értéket, ezért
	// a PPU egy buffert használ, amely minden olvasásnál frissül.
	// Ettől a PPU $2007 olvasása mindig 1 olvasással késik, ezért a játékok egy dummy olvasást csinálnak az address beállítása után
	uint8_t PPUReadBuff; 

	bool generateNMI;
	bool endOfFrame;

	uint8_t oamDmaPage; // OAM DMA esetén a CPU-ban ezen a "page"-en lévő adatot másoljuk a PPU OAMjába

	// belső regiszterek (https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers)
	union {
		struct {
			unsigned int coarseX : 5;
			unsigned int coarseY : 5;
			unsigned int nametableSelect : 2;
			unsigned int fineY : 3;
		};
		uint16_t value;
	} v, t; //15-bit, jelenlegi VRAM address (ebből csak 14-bit használható), és egy temp VRAM address
	uint8_t x; //3-bit, finom X scroll
	bool secondWrite; //1-bit, ezzel különbözteti meg ugyanazon a regiszteren érkező A és B értéket (w)

	uint8_t* display; // 256x240 kijelző memóriája (RGB24). Az SDL ezt fogja majd megjeleníteni!
} PPU;

PPU CreatePPU();
void DestroyPPU(PPU* ppu);
void WritingToPPUReg(PPU* ppu, uint16_t reg, uint8_t value);
uint8_t ReadingFromPPUReg(PPU* ppu, uint16_t reg);
void TickPPU(PPU* ppu);