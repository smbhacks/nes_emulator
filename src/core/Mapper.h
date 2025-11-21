#pragma once
#include <stdint.h>

enum {
	UNKNOWN_MAPPER = -1,
	NROM,
	MMC3
};

typedef int Mapper;
typedef struct CPU CPU;

// Mapper számok: https://www.nesdev.org/wiki/Mapper 
Mapper GetMapper(uint8_t mapperNumber, int* internalMapperNum);
uint8_t ReadCpuMemViaMapper(CPU* cpu, uint16_t address);