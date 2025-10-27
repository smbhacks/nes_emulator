#include "Cart.h"

Cart InsertCart(const char* path)
{
	Cart cart;

	FILE* cartFile;
	cartFile = fopen(path, "rb");

	// .nes: első 16 byte az a fejléc (iNES header)
	uint8_t iNES[16];
	if (fread(iNES, sizeof(iNES), 1, cartFile) != 1)
	{
		exit(-1);
	}
	cart.PRG_size = iNES[4] * 0x4000;
	cart.CHR_size = iNES[5] * 0x2000;
	cart.nametableArrangement = iNES[6] & 0b1;

	// .nes: a fejléc után következik a PRG adat
	cart.PRG = (uint8_t*)malloc(cart.PRG_size);
	if(fread(cart.PRG, cart.PRG_size, 1, cartFile) != 1)
	{
		exit(-1);
	}

	// .nes: a PRG után következik a CHR, ha van
	cart.CHR = (uint8_t*)malloc(cart.CHR_size);
	if(fread(cart.CHR, cart.CHR_size, 1, cartFile) != 1)
	{
		exit(-1);
	}

	fclose(cartFile);

	return cart;
}
