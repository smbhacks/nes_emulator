#include <stdint.h>
#include "../CPU.h"

uint8_t NROM_Read(CPU* cpu, uint16_t address)
{
	if (address < 0x8000)
		return 0xcd;

	address -= 0x8000;
	if (cpu->cart->PRG_size == 1)
		address &= 0x3fff;

	return cpu->cart->PRG[address];
}
void NROM_Write(CPU* cpu, uint16_t address)
{
	// nincs mappelve semmi!
}