#pragma once
#include "CPU.h"
#include "PPU.h"
#include "Cart.h"
#include "Controller.h"
#include "Palette.h"
#include <debugmalloc.h>

typedef struct NES {
	CPU* cpu;
	PPU* ppu;
	Cart* cart;
	Controller* controller;

	bool cartInserted;
} NES;

NES* CreateNES();
void SetCartNES(NES* nes, const char* path);
void ResetNES(NES *nes);
void TickNES(NES *nes);
void RemoveCartNES(NES* nes);
void DestroyNES(NES* nes);
void UseCustomPalette(NES* nes, uint8_t* pal_ptr);