#pragma once
#include "CPU.h"
#include "PPU.h"
#include "Cart.h"

typedef struct NES {
	CPU cpu;
	PPU ppu;
	Cart cart;
} NES;

NES CreateNES();
void SetCartNES(NES* nes, const char* path);
void ResetNES(NES *nes);
void TickNES(NES *nes);
void RemoveCartNES(NES* nes);
void DestroyNES(NES* nes);