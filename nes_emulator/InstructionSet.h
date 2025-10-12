#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AddressingEnum.h"

typedef struct CPU CPU;
typedef struct Opcode Opcode;

uint8_t GetValueWithAddressing(CPU* cpu, int addressingMode, bool checkPageCrossEnabled);
void DoIllegal();
void DoADC(CPU *cpu, Opcode *opcode);
void DoAND(CPU *cpu, Opcode *opcode);
void DoASL(CPU *cpu, Opcode *opcode);
void DoBCC(CPU *cpu, Opcode *opcode);
void DoBCS(CPU *cpu, Opcode *opcode);
void DoBEQ(CPU *cpu, Opcode *opcode);
void DoBMI(CPU *cpu, Opcode *opcode);
void DoBNE(CPU *cpu, Opcode *opcode);
void DoBPL(CPU *cpu, Opcode *opcode);
void DoBVC(CPU *cpu, Opcode *opcode);
void DoBVS(CPU *cpu, Opcode *opcode);
void DoBRK(CPU *cpu, Opcode *opcode);
void DoBIT(CPU *cpu, Opcode *opcode);
void DoCLC(CPU *cpu, Opcode *opcode);
void DoCLD(CPU *cpu, Opcode *opcode);
void DoCLI(CPU *cpu, Opcode *opcode);
void DoCLV(CPU *cpu, Opcode *opcode);
void DoCMP(CPU *cpu, Opcode *opcode);
void DoCPX(CPU *cpu, Opcode *opcode);
void DoCPY(CPU *cpu, Opcode *opcode);
void DoDEC(CPU *cpu, Opcode *opcode);
void DoDEX(CPU *cpu, Opcode *opcode);
void DoDEY(CPU *cpu, Opcode *opcode);
void DoEOR(CPU *cpu, Opcode *opcode);
void DoINC(CPU *cpu, Opcode *opcode);
void DoINX(CPU *cpu, Opcode *opcode);
void DoINY(CPU *cpu, Opcode *opcode);
void DoJMP(CPU *cpu, Opcode *opcode);
void DoJSR(CPU *cpu, Opcode *opcode);
void DoLDA(CPU *cpu, Opcode *opcode);
void DoLDX(CPU *cpu, Opcode *opcode);
void DoLDY(CPU *cpu, Opcode *opcode);
void DoLSR(CPU *cpu, Opcode *opcode);
void DoNOP(CPU *cpu, Opcode *opcode);
void DoORA(CPU *cpu, Opcode *opcode);
void DoPHA(CPU *cpu, Opcode *opcode);
void DoPHP(CPU *cpu, Opcode *opcode);
void DoPLA(CPU *cpu, Opcode *opcode);
void DoPLP(CPU *cpu, Opcode *opcode);
void DoROL(CPU *cpu, Opcode *opcode);
void DoROR(CPU *cpu, Opcode *opcode);
void DoRTI(CPU *cpu, Opcode *opcode);
void DoRTS(CPU *cpu, Opcode *opcode);
void DoSBC(CPU *cpu, Opcode *opcode);
void DoSEC(CPU *cpu, Opcode *opcode);
void DoSED(CPU *cpu, Opcode *opcode);
void DoSEI(CPU *cpu, Opcode *opcode);
void DoSTA(CPU *cpu, Opcode *opcode);
void DoSTX(CPU *cpu, Opcode *opcode);
void DoSTY(CPU *cpu, Opcode *opcode);
void DoTAX(CPU *cpu, Opcode *opcode);
void DoTAY(CPU *cpu, Opcode *opcode);
void DoTSX(CPU *cpu, Opcode *opcode);
void DoTXA(CPU *cpu, Opcode *opcode);
void DoTXS(CPU *cpu, Opcode *opcode);
void DoTYA(CPU *cpu, Opcode *opcode);