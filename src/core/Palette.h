#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct PPU PPU;

uint8_t* GetDefPalette();
uint8_t* MallocPalette(const char* path);
void FreeCustomPalette(PPU* ppu);