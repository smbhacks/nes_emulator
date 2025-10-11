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

int TickCPU(CPU *cpu)
{
    // handle a opcode
    Opcode opcode = opcodes[cpu->memory[cpu->PC]];
    cpu->PC++;
    opcode.doInstructionFn(&cpu, &opcode);
    cpu->currentCycleTime += opcode.cycles;
    return 0;
}

void CreateOpcodes()
{
    // opcodes tábla init
    Opcode invalidOpcode = { Illegal, none, 2, &DoIllegal};
    for (int i = 0; i < 256; i++)
        opcodes[i] = invalidOpcode;

    // csináljuk meg az opcodes táblát amelyet lehet indexelni opkód érték alapján (pl 0xEA -> NOP)
    int numValidOpcodes = sizeof(validOpcodes) / sizeof(validOpcodes[0]);
    for (int i = 0; i < numValidOpcodes; i++)
    {
        uint8_t opcodeValue = validOpcodes[i].opcodeValue;
        opcodes[opcodeValue] = validOpcodes[i].opcode;
    }
}

