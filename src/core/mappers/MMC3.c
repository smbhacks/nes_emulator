#include <stdint.h>
#include "../CPU.h"
#include "Generic.h"

// https://www.nesdev.org/wiki/MMC3

// regiszterek által elérhető
static int bankSelect;
static int prgRomBankMode;
static int chrA12inversion;

// belső
static int R[8];

uint8_t MMC3_Read(CPU* cpu, uint16_t address)
{
    int bankToUse;
    switch (address & (0x8000 | 0x4000 | 0x2000))
    {
        case 0x8000: {
            bankToUse = prgRomBankMode ? -2 : R[6];
            break;
        }
        case 0xa000: {
            bankToUse = R[7];
            break;
        }
        case 0xc000: {
            bankToUse = prgRomBankMode ? R[6] : -2;
            break;
        }
        case 0xe000: {
            bankToUse = -1;
            break;
        }
    }
    return ReadPrgInBank(cpu, address, 0x2000, bankToUse);
}
void MMC3_Write(CPU* cpu, uint16_t address, uint8_t value)
{
    bool even = !(address % 2);
    if(0x8000 <= address && address <= 0x9FFF)
    {
        if(even)
        {
            // bank select regiszter írás
            bankSelect =      value & 0b111;
            prgRomBankMode =  value & 0x40 ? 1 : 0;
            chrA12inversion = value & 0x80 ? 1 : 0;
        }
        else
        {
            // bank data regiszter írás
            R[bankSelect] = value;
        }
    }
}
uint8_t MMC3_CHR(PPU* ppu, uint16_t address)
{
    int bankToUse;
    switch (address & (0x0400 | 0x0800 | 0x1000))
    {
        case 0x0000: {
            bankToUse = chrA12inversion ? R[2] : R[0]+0;
            break;
        }
        case 0x0400: {
            bankToUse = chrA12inversion ? R[3] : R[0]+1;
            break;
        }        
        case 0x0800: {
            bankToUse = chrA12inversion ? R[4] : R[1]+0;
            break;
        }        
        case 0x0c00: {
            bankToUse = chrA12inversion ? R[5] : R[1]+1;
            break;
        }        
        case 0x1000: {
            bankToUse = chrA12inversion ? R[0]+0 : R[2];
            break;
        }        
        case 0x1400: {
            bankToUse = chrA12inversion ? R[0]+1 : R[3];
            break;
        }        
        case 0x1800: {
            bankToUse = chrA12inversion ? R[1]+0 : R[4];
            break;
        }        
        case 0x1c00: {
            bankToUse = chrA12inversion ? R[1]+1 : R[5];
            break;
        }        
    }
    return ReadChrInBank(ppu, address, 0x0400, bankToUse);
}