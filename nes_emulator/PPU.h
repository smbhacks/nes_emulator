#pragma once
#include <stdint.h>

typedef struct PPU {
	// 16 kb memória, amelyet a CPU tud manipulálni
	uint8_t* memory; 
	// (Object Attribute Memory) 256 byteos PPU belső memória, amely a "sprite"-okat rajzolja. 64 sprite tárolására képes. A CPU ezt   
	// 3 regiszterrel tudja módosítani. Általában csak 1-et használnak, az "OAMDMA" regisztert, amely kimásolja a CPU egy 256 bájtnyni részét a PPU OAMjába.
	uint8_t* oam; 
} PPU;

PPU CreatePPU();
void DestroyPPU(PPU* ppu);