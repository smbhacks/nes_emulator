#pragma once
#include <stdint.h>
#include <stdio.h> // FILE
#include <stdlib.h>

typedef struct Cart {
	// fejléc
	unsigned int PRG_size;
	unsigned int CHR_size;
	int nametableArrangement; // 0: vertikális tükrözés, 1: horizontális tükrözés
	// ROM
	uint8_t *PRG;
	uint8_t *CHR;
} Cart;

Cart InsertCart(const char* path);