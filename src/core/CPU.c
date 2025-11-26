#include "CPU.h"

Opcode opcodes[256];

// ez csak debuggolásra kell!!!!!!!!!
const char* InstructionString[] = {
    "???",
    "ADC",
    "AND",
    "ASL",
    "BCC",
    "BCS",
    "BEQ",
    "BIT",
    "BMI",
    "BNE",
    "BPL",
    "BRK",
    "BVC",
    "BVS",
    "CLC",
    "CLD",
    "CLI",
    "CLV",
    "CMP",
    "CPX",
    "CPY",
    "DEC",
    "DEX",
    "DEY",
    "EOR",
    "INC",
    "INX",
    "INY",
    "JMP",
    "JSR",
    "LDA",
    "LDX",
    "LDY",
    "LSR",
    "NOP",
    "ORA",
    "PHA",
    "PHP",
    "PLA",
    "PLP",
    "ROL",
    "ROR",
    "RTI",
    "RTS",
    "SBC",
    "SEC",
    "SED",
    "SEI",
    "STA",
    "STX",
    "STY",
    "TAX",
    "TAY",
    "TSX",
    "TXA",
    "TXS",
    "TYA"
};

void LogCPU(CPU* cpu)
{
    //printf("%s", cpu->logBuff);
    fputs(cpu->logBuff, cpu->logFile);
}

CPU* CreateCPU()
{
    printf("Creating CPU...\n");

    CPU* cpu = malloc(sizeof(CPU));

    memset(cpu, 0, sizeof(CPU)); // minden lenullázása
    cpu->memory = malloc(0x10000); // 32kb memória (0000~FFFF)

    // opkódok "betöltése"
    CreateOpcodes();

    if (LOG_CPU) {
        cpu->logFile = fopen("log.txt", "w");
    }

    cpu->cart = NULL;

    return cpu;
}

void FreeCPU(CPU* cpu)
{
    printf("Freeing CPU...\n");

    free(cpu->memory);
    if (LOG_CPU) {
        fclose(cpu->logFile);
    }
    free(cpu);
}

uint8_t ReadCpuMem(CPU* cpu, uint16_t address)
{
    if (address < 0x2000)
        return cpu->memory[address & 0x7ff];
	else if (0x2000 <= address && address <= 0x2fff)
		return ReadingFromPPUReg(cpu->ppu, address);
	else if (address == CONTROLLER_REG_4016 || address == CONTROLLER_REG_4017)
		return ReadingFromControllerReg(cpu->controller);
    else if (0x6000 <= address && address <= 0x7fff)
        return cpu->memory[address];

    return ReadCpuMemViaMapper(cpu, address);
}

void WriteCpuMem(CPU* cpu, uint16_t address, uint8_t value)
{
    if (address < 0x2000)
        cpu->memory[address & 0x7ff] = value;
    else if (0x6000 <= address && address <= 0x7fff)
        cpu->memory[address] = value;
    else
        WriteCpuMemViaMapper(cpu, address, value);
}

int TickCPU(CPU *cpu)
{
    cpu->currentCycleTime = 0;

    // egy opkódot végrehajtása
    Opcode opcode = opcodes[ReadCpuMem(cpu, cpu->PC)];
    cpu->PC++;

    if (LOG_CPU) {
        sprintf(cpu->logBuff, "%04X    ", cpu->PC-1);
        LogCPU(cpu);
        sprintf(cpu->logBuff, "%s ", InstructionString[opcode.instruction]);
        LogCPU(cpu);
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
        sprintf(cpu->logBuff, "\t\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X Cyc:%d CycInFrame:%d\n", cpu->a, cpu->x, cpu->y, processorFlags, cpu->s, cpu->currentCycleTime, cpu->currentCycleTimeInFrame);
        LogCPU(cpu);
    }

    if (cpu->ppu->generateNMI) {
        PushToStack(cpu, cpu->PC / 256); //magas PC
        PushToStack(cpu, cpu->PC % 256); //alacsony PC
        DoPHP(cpu, NULL); // processor flagek
        uint16_t nmiVector = ReadCpuMem(cpu, 0xFFFA) + 256 * ReadCpuMem(cpu, 0xFFFB); // A CPU-ban a 0xFFFA-nál található az NMI címe
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

