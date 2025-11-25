#include <stdint.h>
#include "../CPU.h"
#include "../PPU.h"

uint8_t ReadPrgInBank(CPU* cpu, uint16_t address, uint16_t bankSize, int bankNumber);
uint8_t ReadChrInBank(PPU* ppu, uint16_t address, uint16_t bankSize, int bankNumber);