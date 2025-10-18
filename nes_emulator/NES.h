#pragma once
#include "CPU.h"
#include "PPU.h"
#include "Cart.h"
#include "Controller.h"

typedef struct NES {
	CPU cpu;
	PPU ppu;
	Cart cart;
	Controller controller;
} NES;

NES CreateNES();
void SetCartNES(NES* nes, const char* path);
void ResetNES(NES *nes);
void TickNES(NES *nes);
void RemoveCartNES(NES* nes);
void DestroyNES(NES* nes);