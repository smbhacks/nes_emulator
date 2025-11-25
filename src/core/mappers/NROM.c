#include <stdint.h>
#include "../CPU.h"

uint8_t NROM_Read(CPU* cpu, uint16_t address)
{
	address -= 0x8000;
	if (cpu->cart->PRG_size == 16384)
		address &= 0x3fff;

	return cpu->cart->PRG[address];
}
void NROM_Write(CPU* cpu, uint16_t address, uint8_t value)
{
	// nincs mappelve semmi!
}
uint8_t NROM_CHR(PPU* ppu, uint16_t address)
{
	return ppu->cart->CHR[address];
}