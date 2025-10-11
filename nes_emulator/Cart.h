#include <SDL.h> // uint8_t miatt
#include <stdio.h> // FILE

typedef struct Cart {
	// fejléc
	unsigned int PRG_size;
	unsigned int CHR_size;
	int nametableArrangement; // 0: vertikális tükrözés, 1: horizontális tükrözés
	// ROM
	uint8_t *PRG;
	uint8_t *CHR;
} Cart;

Cart InitCart(const char* path);