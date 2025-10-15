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
    cpu->currentCycleTime = 0;

    // egy opkódot végrehajtása
    Opcode opcode = opcodes[cpu->memory[cpu->PC]];
    cpu->PC++;

    if (LOG_CPU) {
        printf("%04X    ", cpu->PC);
        printf("%s ", InstructionString[opcode.instruction]);
    }

    opcode.doInstructionFn(cpu, &opcode);
    cpu->currentCycleTime += opcode.cycles;

    if (LOG_CPU) {
        // P = nv11dizc
        uint8_t processorFlags = 0;
        processorFlags += (cpu->c << 0);
        processorFlags += (cpu->z << 1);
        processorFlags += (cpu->i << 2);
        processorFlags += (cpu->d << 3);
        processorFlags += (cpu->v << 6);
        processorFlags += (cpu->n << 7);
        printf("\t\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X Cyc:%d CycInFrame:%d\n", cpu->a, cpu->x, cpu->y, processorFlags, cpu->s, cpu->currentCycleTime, cpu->currentCycleTimeInFrame);
    }

    if (cpu->ppu->generateNMI) {
        PushToStack(cpu, cpu->PC / 256); //magas PC
        PushToStack(cpu, cpu->PC % 256); //alacsony PC
        DoPHP(cpu, NULL); // processor flagek
        uint16_t nmiVector = cpu->memory[0xFFFA] + 256*cpu->memory[0xFFFB]; // A CPU-ban a 0xFFFA-nál található az NMI címe
        cpu->PC = nmiVector;
        cpu->ppu->generateNMI = false;
    }

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

