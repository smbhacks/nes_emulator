#include <stdint.h>
#include "../CPU.h"
#include "Generic.h"

// https://www.nesdev.org/wiki/MMC1

static int prgBank;
static int chrBank0;
static int chrBank1;

static int shiftReg;
static int shiftRegCounter;
static int nametableArrangement;
static int prgRomBankMode = 3; // kell egy mapper init függvény
static int chrRomBankMode;

uint8_t MMC1_Read(CPU* cpu, uint16_t address)
{
    int bankToUse;
    int bankSize;
    switch (prgRomBankMode)
    {
        case 0: case 1: {
            bankSize = 0x8000;
            bankToUse = prgBank >> 1;
            break;
        }
        case 2:
        {
            bankSize = 0x4000;
            if(address < 0xc000)
                bankToUse = 0;
            else
                bankToUse = prgBank;
            break;
        }
        case 3:
        {
            bankSize = 0x4000;
            if(address < 0xc000)
                bankToUse = prgBank;
            else
                bankToUse = -1;
            break;
        }
    }
    return ReadPrgInBank(cpu, address, bankSize, bankToUse);
}
void MMC1_Write(CPU* cpu, uint16_t address, uint8_t value)
{
    if(value & 0x80)
    {
        // regiszterek resetelése
        shiftRegCounter = 5;
        shiftReg = 0;
    }
    else
    {
        shiftReg >>= 1;
        shiftReg |= ((value & 0b1) << 4); // az érték legalsó bitjét shifteljük be a regiszter 5. bitjébe
        if(--shiftRegCounter == 0)
        {
            // kész az 5-bites értékünk a shiftregiszterben
            // kezeljük az address szerint
            switch (address & (0x8000 | 0x4000 | 0x2000))
            {
                case 0x8000: {
                    nametableArrangement = shiftReg & 0b00011;
                    prgRomBankMode = (shiftReg & 0b01100) >> 2;
                    chrRomBankMode = shiftReg & 0b10000 ? 1 : 0; 
                    break;
                }
                case 0xa000: {
                    chrBank0 = shiftReg;
                    break;
                }
                case 0xc000: {
                    chrBank1 = shiftReg;
                    break;
                }
                case 0xe000: {
                    prgBank = shiftReg;
                    break;
                }
            }
            shiftReg = 0;
            shiftRegCounter = 5;
        }
    }
}
uint8_t MMC1_CHR(PPU* ppu, uint16_t address)
{
    int bankToUse;
    int bankSize;
    if(chrRomBankMode)
    {
        // =1
        bankSize = 0x1000;
        if(address < 0x1000)
            bankToUse = chrBank0;
        else
            bankToUse = chrBank1;
    }
    else
    {
        // =0
        bankSize = 0x2000; // 8 kb
        bankToUse = chrBank0 >> 1;
    }
    return ReadChrInBank(ppu, address, bankSize, bankToUse);
}