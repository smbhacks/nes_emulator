// 6502 CPU
#pragma once
#include <stdint.h>
#include <stdio.h>
#include "InstructionSet.h"
#include "AddressingEnum.h"
#include <stdbool.h>
#include <stdlib.h>
#include "PPU.h"
#include "Controller.h"

#define LOG_CPU false

typedef struct CPU {
    // 32 kb memória 
    uint8_t* memory;
    // a, x, y: program által beállított
    // s: verem mutató (stack)
    uint8_t a, x, y, s; // 8-bites regiszterek
    // 1-bites regiszterek
    int n; // negatív
    int v; // túlcsordulás
    int d; // decimális
    int i; // interrupt disable (csak IRQ)
    int z; // zero
    int c; // carry
    // program counter (programszámláló)
    uint16_t PC;

    // jelenlegi opkód cycle ideje
    int currentCycleTime; // azért van itt, hogy az instructionSet is tudja módosítani
    int currentCycleTimeInFrame;

    // cpu tud kommunikálni ezekkel:
    PPU* ppu;
    Controller* controller;

    FILE* logFile;
    char logBuff[128];
} CPU;

void CreateOpcodes();
CPU CreateCPU();
int TickCPU(CPU *cpu); // returns number of cycles

enum Instruction
{
    Illegal = 0, // vannak olyan instrukciók amelyek "illegálisak", de attól lehet használni őket. én ezeket most nem fogom implementálni
    ADC,
    AND,
    ASL,
    BCC,
    BCS,
    BEQ,
    BIT,
    BMI,
    BNE,
    BPL,
    BRK,
    BVC,
    BVS,
    CLC,
    CLD,
    CLI,
    CLV,
    CMP,
    CPX,
    CPY,
    DEC,
    DEX,
    DEY,
    EOR,
    INC,
    INX,
    INY,
    JMP,
    JSR,
    LDA,
    LDX,
    LDY,
    LSR,
    NOP,
    ORA,
    PHA,
    PHP,
    PLA,
    PLP,
    ROL,
    ROR,
    RTI,
    RTS,
    SBC,
    SEC,
    SED,
    SEI,
    STA,
    STX,
    STY,
    TAX,
    TAY,
    TSX,
    TXA,
    TXS,
    TYA
};

// ez csak debuggolásra kell!!!!!!!!!
static const char* InstructionString[] = {
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

typedef struct Opcode {
    enum Instruction instruction;
    enum AddressingMode addressingMode;
    int cycles; // egy opkód időtartama amíg "lefut" (ez egy minimális érték, változni tud)
    void (*doInstructionFn)(CPU*, Opcode*);
} Opcode;

Opcode opcodes[256];

// https://www.nesdev.org/obelisk-6502-guide/reference.html
static const struct {
    uint8_t opcodeValue;
    Opcode opcode;
} validOpcodes[] = {
    {0x69, {ADC, immediate,         2, &DoADC}},
    {0x65, {ADC, zeropage,          3, &DoADC}},
    {0x75, {ADC, zeropage_x,        4, &DoADC}},
    {0x6d, {ADC, absolute,          4, &DoADC}},
    {0x7d, {ADC, absolute_x,        4, &DoADC}},
    {0x79, {ADC, absolute_y,        4, &DoADC}},
    {0x61, {ADC, indexed_indirect,  6, &DoADC}},
    {0x71, {ADC, indirect_indexed,  5, &DoADC}},
    {0x29, {AND, immediate,         2, &DoAND}},
    {0x25, {AND, zeropage,          2, &DoAND}},
    {0x35, {AND, zeropage_x,        2, &DoAND}},
    {0x2d, {AND, absolute,          3, &DoAND}},
    {0x3d, {AND, absolute_x,        3, &DoAND}},
    {0x39, {AND, absolute_y,        3, &DoAND}},
    {0x21, {AND, indexed_indirect,  2, &DoAND}},
    {0x31, {AND, indirect_indexed,  2, &DoAND}},
    {0x0A, {ASL, none,              2, &DoASL}},
    {0x06, {ASL, zeropage,          5, &DoASL}},
    {0x16, {ASL, zeropage_x,        6, &DoASL}},
    {0x0e, {ASL, absolute,          6, &DoASL}},
    {0x1e, {ASL, absolute_x,        7, &DoASL}},
    {0x90, {BCC, relative,          2, &DoBCC}},
    {0xB0, {BCS, relative,          2, &DoBCS}},
    {0xF0, {BEQ, relative,          2, &DoBEQ}},
    {0x30, {BMI, relative,          2, &DoBMI}},
    {0xD0, {BNE, relative,          2, &DoBNE}},
    {0x10, {BPL, relative,          2, &DoBPL}},
    {0x50, {BVC, relative,          2, &DoBVC}},
    {0x70, {BVS, relative,          2, &DoBVS}},
    {0x00, {BRK, none,              7, &DoBRK}},
    {0x24, {BIT, zeropage,          3, &DoBIT}},
    {0x2c, {BIT, absolute,          4, &DoBIT}},
    {0x18, {CLC, none,              2, &DoCLC}},
    {0xd8, {CLD, none,              2, &DoCLD}},
    {0x58, {CLI, none,              2, &DoCLI}},
    {0xb8, {CLV, none,              2, &DoCLV}},
    {0xc9, {CMP, immediate,         2, &DoCMP}},
    {0xc5, {CMP, zeropage,          3, &DoCMP}},
    {0xd5, {CMP, zeropage_x,        4, &DoCMP}},
    {0xcd, {CMP, absolute,          4, &DoCMP}},
    {0xdd, {CMP, absolute_x,        4, &DoCMP}},
    {0xd9, {CMP, absolute_y,        4, &DoCMP}},
    {0xc1, {CMP, indexed_indirect,  6, &DoCMP}},
    {0xd1, {CMP, indirect_indexed,  5, &DoCMP}},
    {0xe0, {CPX, immediate,         2, &DoCPX}},
    {0xe4, {CPX, zeropage,          3, &DoCPX}},
    {0xec, {CPX, absolute,          4, &DoCPX}},
    {0xc0, {CPY, immediate,         2, &DoCPY}},
    {0xc4, {CPY, zeropage,          3, &DoCPY}},
    {0xcc, {CPY, absolute,          4, &DoCPY}},
    {0xc6, {DEC, zeropage,          5, &DoDEC}},
    {0xd6, {DEC, zeropage_x,        6, &DoDEC}},
    {0xce, {DEC, absolute,          6, &DoDEC}},
    {0xde, {DEC, absolute_x,        7, &DoDEC}},
    {0xca, {DEX, none,              2, &DoDEX}},
    {0x88, {DEY, none,              2, &DoDEY}},
    {0x49, {EOR, immediate,         2, &DoEOR}},
    {0x45, {EOR, zeropage,          3, &DoEOR}},
    {0x55, {EOR, zeropage_x,        4, &DoEOR}},
    {0x4d, {EOR, absolute,          4, &DoEOR}},
    {0x5d, {EOR, absolute_x,        4, &DoEOR}},
    {0x59, {EOR, absolute_y,        4, &DoEOR}},
    {0x41, {EOR, indexed_indirect,  6, &DoEOR}},
    {0x51, {EOR, indirect_indexed,  5, &DoEOR}},
    {0xe6, {INC, zeropage,          5, &DoINC}},
    {0xf6, {INC, zeropage_x,        6, &DoINC}},
    {0xee, {INC, absolute,          6, &DoINC}},
    {0xfe, {INC, absolute_x,        7, &DoINC}},
    {0xe8, {INX, none,              2, &DoINX}},
    {0xc8, {INY, none,              2, &DoINY}},
    {0x4c, {JMP, absolute,          3, &DoJMP}},
    {0x6c, {JMP, indirect,          5, &DoJMP}},
    {0x20, {JSR, absolute,          6, &DoJSR}},
    {0xa9, {LDA, immediate,         2, &DoLDA}},
    {0xa5, {LDA, zeropage,          3, &DoLDA}},
    {0xb5, {LDA, zeropage_x,        4, &DoLDA}},
    {0xad, {LDA, absolute,          4, &DoLDA}},
    {0xbd, {LDA, absolute_x,        4, &DoLDA}},
    {0xb9, {LDA, absolute_y,        4, &DoLDA}},
    {0xa1, {LDA, indexed_indirect,  6, &DoLDA}},
    {0xb1, {LDA, indirect_indexed,  5, &DoLDA}},
    {0xa2, {LDX, immediate,         2, &DoLDX}},
    {0xa6, {LDX, zeropage,          3, &DoLDX}},
    {0xb6, {LDX, zeropage_y,        4, &DoLDX}},
    {0xae, {LDX, absolute,          4, &DoLDX}},
    {0xbe, {LDX, absolute_y,        4, &DoLDX}},
    {0xa0, {LDY, immediate,         2, &DoLDY}},
    {0xa4, {LDY, zeropage,          3, &DoLDY}},
    {0xb4, {LDY, zeropage_x,        4, &DoLDY}},
    {0xac, {LDY, absolute,          4, &DoLDY}},
    {0xbc, {LDY, absolute_x,        4, &DoLDY}},
    {0x4a, {LSR, none,              2, &DoLSR}},
    {0x46, {LSR, zeropage,          5, &DoLSR}},
    {0x56, {LSR, zeropage_x,        6, &DoLSR}},
    {0x4e, {LSR, absolute,          6, &DoLSR}},
    {0x5e, {LSR, absolute_x,        7, &DoLSR}},
    {0xea, {NOP, none,              2, &DoNOP}},
    {0x09, {ORA, immediate,         2, &DoORA}},
    {0x05, {ORA, zeropage,          3, &DoORA}},
    {0x15, {ORA, zeropage_x,        4, &DoORA}},
    {0x0d, {ORA, absolute,          4, &DoORA}},
    {0x1d, {ORA, absolute_x,        4, &DoORA}},
    {0x19, {ORA, absolute_y,        4, &DoORA}},
    {0x01, {ORA, indexed_indirect,  6, &DoORA}},
    {0x11, {ORA, indirect_indexed,  5, &DoORA}},
    {0x48, {PHA, none,              3, &DoPHA}},
    {0x08, {PHP, none,              3, &DoPHP}},
    {0x68, {PLA, none,              4, &DoPLA}},
    {0x28, {PLP, none,              4, &DoPLP}},
    {0x2a, {ROL, none,              2, &DoROL}},
    {0x26, {ROL, zeropage,          5, &DoROL}},
    {0x36, {ROL, zeropage_x,        6, &DoROL}},
    {0x2e, {ROL, absolute,          6, &DoROL}},
    {0x3e, {ROL, absolute_x,        7, &DoROL}},
    {0x6a, {ROR, none,              2, &DoROR}},
    {0x66, {ROR, zeropage,          5, &DoROR}},
    {0x76, {ROR, zeropage_x,        6, &DoROR}},
    {0x6e, {ROR, absolute,          6, &DoROR}},
    {0x7e, {ROR, absolute_x,        7, &DoROR}},
    {0x40, {RTI, none,              6, &DoRTI}},
    {0x60, {RTS, none,              6, &DoRTS}},
    {0xe9, {SBC, immediate,         2, &DoSBC}},
    {0xe5, {SBC, zeropage,          3, &DoSBC}},
    {0xf5, {SBC, zeropage_x,        4, &DoSBC}},
    {0xed, {SBC, absolute,          4, &DoSBC}},
    {0xfd, {SBC, absolute_x,        4, &DoSBC}},
    {0xf9, {SBC, absolute_y,        4, &DoSBC}},
    {0xe1, {SBC, indexed_indirect,  6, &DoSBC}},
    {0xf1, {SBC, indirect_indexed,  5, &DoSBC}},
    {0x38, {SEC, none,              2, &DoSEC}},
    {0xf8, {SED, none,              2, &DoSED}},
    {0x78, {SEI, none,              2, &DoSEI}},
    {0x85, {STA, zeropage,          3, &DoSTA}},
    {0x95, {STA, zeropage_x,        4, &DoSTA}},
    {0x8d, {STA, absolute,          4, &DoSTA}},
    {0x9d, {STA, absolute_x,        5, &DoSTA}},
    {0x99, {STA, absolute_y,        5, &DoSTA}},
    {0x81, {STA, indexed_indirect,  6, &DoSTA}},
    {0x91, {STA, indirect_indexed,  6, &DoSTA}},
    {0x86, {STX, zeropage,          3, &DoSTX}},
    {0x96, {STX, zeropage_y,        4, &DoSTX}},
    {0x8e, {STX, absolute,          4, &DoSTX}},
    {0x84, {STY, zeropage,          3, &DoSTY}},
    {0x94, {STY, zeropage_y,        4, &DoSTY}},
    {0x8c, {STY, absolute,          4, &DoSTY}},
    {0xaa, {TAX, none,              2, &DoTAX}},
    {0xa8, {TAY, none,              2, &DoTAY}},
    {0xba, {TSX, none,              2, &DoTSX}},
    {0x8a, {TXA, none,              2, &DoTXA}},
    {0x9a, {TXS, none,              2, &DoTXS}},
    {0x98, {TYA, none,              2, &DoTYA}},
};