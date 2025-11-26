#include "Cart.h"

Cart* InsertCart(const char* path)
{
	printf("Creating Cart...\n");

	Cart* cart = malloc(sizeof(Cart));

	FILE* cartFile;
	cartFile = fopen(path, "rb");

	// .nes: első 16 byte az a fejléc (iNES header)
	uint8_t iNES[16];
	if (fread(iNES, sizeof(iNES), 1, cartFile) != 1)
	{
		exit(-1);
	}
	cart->PRG_size = iNES[4] * 0x4000;
	cart->CHR_size = iNES[5] * 0x2000;
	cart->nametableArrangement = iNES[6] & 0b1;

	uint8_t mapperNumber = (iNES[7] & 0xF0) | (iNES[6] >> 4);
	Mapper mapper = GetMapper(mapperNumber, &cart->internalMapperNum);
	if (mapper == UNKNOWN_MAPPER)
	{
		printf("Nem tamogatott mappert hasznal ez a ROM\n");
		return NULL;
	}

	// .nes: a fejléc után következik a PRG adat
	cart->PRG = (uint8_t*)malloc(cart->PRG_size);
	if(fread(cart->PRG, cart->PRG_size, 1, cartFile) != 1)
	{
		exit(-1);
	}

	// .nes: a PRG után következik a CHR, ha van
	// ha a CHR_size 0, az azt jelenti, hogy a játék CHR-RAMot használ
	if(cart->CHR_size != 0)
	{
		cart->CHR = (uint8_t*)malloc(cart->CHR_size);
		if(fread(cart->CHR, cart->CHR_size, 1, cartFile) != 1)
		{
			exit(-1);
		}
	}
	else
		cart->CHR = NULL;

	fclose(cartFile);

	return cart;
}

void FreeCart(Cart* cart)
{
	printf("Freeing Cart...\n");

	free(cart->PRG);
	free(cart->CHR);
	free(cart);
}