#pragma once
#include <stdint.h>

enum {
	UNKNOWN_MAPPER = -1,
	NROM,
	MMC3
};

typedef int Mapper;
typedef struct CPU CPU;
typedef struct PPU PPU;

// Mapper számok: https://www.nesdev.org/wiki/Mapper 
Mapper GetMapper(uint8_t mapperNumber, int* internalMapperNum);
uint8_t ReadCpuMemViaMapper(CPU* cpu, uint16_t address);
uint8_t ReadChrMemViaMapper(PPU* ppu, uint16_t address);