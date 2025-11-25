#include "Generic.h"

uint8_t ReadDataInBank(uint8_t* memory, int memorySize, uint16_t address, uint16_t bankSize, int bankNumber)
{
    int numOfBanksInPRG = memorySize / bankSize;
    if(bankNumber < 0)
        bankNumber = numOfBanksInPRG - bankNumber;
    else
        bankNumber %= numOfBanksInPRG;
    int index = (address % bankSize) + bankNumber * bankSize;
    return memory[index];
};
uint8_t ReadPrgInBank(CPU* cpu, uint16_t address, uint16_t bankSize, int bankNumber)
{
    return ReadDataInBank(cpu->cart->PRG, cpu->cart->PRG_size, address, bankSize, bankNumber);
}
uint8_t ReadChrInBank(PPU* ppu, uint16_t address, uint16_t bankSize, int bankNumber)
{
    return ReadDataInBank(ppu->cart->CHR, ppu->cart->CHR_size, address, bankSize, bankNumber);
}