#pragma once
#include "CPU.h"
#include "Cart.h"

typedef struct NES {
	CPU cpu;
	Cart cart;
} NES;

NES CreateNES();
void SetCartNES(NES* nes, const char* path);
void ResetNES(NES *nes);
void TickNES(NES *nes);