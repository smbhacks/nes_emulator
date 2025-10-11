#include "CPU.h"

CPU CreateCPU()
{
    CPU cpu;

    memset(&cpu, 0, sizeof(cpu)); // minden lenullázása
    cpu.memory = malloc(0x10000); // 32kb memória (0000~FFFF)

    // opkódok "betöltése"
    CreateOpcodes();

    return cpu;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html
void CreateOpcodes()
{
    memset(&opcodes, 0, sizeof(opcodes));

    // csináljuk meg az opcodes táblát amelyet lehet indexelni opkód érték alapján (pl 0xEA -> NOP)
    int numValidOpcodes = sizeof(validOpcodes) / sizeof(validOpcodes[0]);
    for (int i = 0; i < numValidOpcodes; i++)
    {
        uint8_t opcodeValue = validOpcodes[i].opcodeValue;
        opcodes[opcodeValue] = validOpcodes[i].opcode;
    }
}
