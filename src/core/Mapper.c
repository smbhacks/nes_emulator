#include "Mapper.h"
#include "CPU.h"

typedef struct MapperAndNumber
{
	Mapper mapper;
	uint8_t mapperNumber;
} MapperAndNumber;
const MapperAndNumber mappers[] = {
	{NROM, 0},
	{MMC3, 4}
};

// ezt automatizálni kell az enumeráció sorrendje szerint
uint8_t NROM_Read(CPU* cpu, uint16_t address);
void	NROM_Write(CPU* cpu, uint16_t address);
uint8_t MMC3_Read(CPU* cpu, uint16_t address);
void	MMC3_Write(CPU* cpu, uint16_t address);
typedef struct MapperFunctions {
	Mapper mapper;
	uint8_t(*ReadCpuByte)(CPU* cpu, uint16_t);
	uint8_t(*WriteCpuByte)(CPU* cpu, uint16_t);
} MapperFunctions;
const MapperFunctions mapperFns[] = {
	{NROM, NROM_Read, NROM_Write},
	{MMC3, MMC3_Read, MMC3_Write}
};

Mapper GetMapper(uint8_t mapperNumber, int* internalMapperNum)
{
	for (int i = 0; i < sizeof(mappers) / sizeof(MapperAndNumber); i++)
		if (mappers[i].mapperNumber == mapperNumber)
		{
			*internalMapperNum = i;
			return mappers[i].mapper;
		}

	*internalMapperNum = 0;
	return UNKNOWN_MAPPER;
}

uint8_t ReadCpuMemViaMapper(CPU* cpu, uint16_t address)
{
	mapperFns[cpu->cart->internalMapperNum].ReadCpuByte(cpu, address);
}