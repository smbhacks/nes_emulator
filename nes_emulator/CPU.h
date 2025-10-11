// 6502 CPU
#pragma once
#include <SDL.h> // uint8_t miatt

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
} CPU;

void CreateOpcodes();
CPU CreateCPU();

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

// addressing módok
enum AddressingMode
{
    none = 0, // implicit, 'a' regiszter, stb. (pl RTS)
    immediate, // (pl LDA #0)
    relative, // (pl BCS #1)
    zeropage, // (pl LDA $00)
    absolute, // (pl LDA $1000)
    indirect, // (pl JMP ($00))
    zeropage_x, // (pl LDA $00,x)
    zeropage_y, // (pl LDA $00,y)
    absolute_x, // (pl LDA $1000,x)
    absolute_y, // (pl LDA $1000,y)
    indexed_indirect, // (pl LDA ($00,x))
    indirect_indexed // (pl LDA ($00),y)
};

typedef struct Opcode {
    enum Instruction instruction;
    enum AddressingMode addressingMode;
    int cycles; // egy opkód időtartama amíg "lefut"
    int cycle_if_page_crossed;
} Opcode;

Opcode opcodes[256];

static const struct {
    uint8_t opcodeValue;
    Opcode opcode;
} validOpcodes[] = {
    {0x69, {ADC, immediate,         2, 0}},
    {0x65, {ADC, zeropage,          3, 0}},
    {0x75, {ADC, zeropage_x,        4, 0}},
    {0x6d, {ADC, absolute,          4, 0}},
    {0x7d, {ADC, absolute_x,        4, 1}},
    {0x79, {ADC, absolute_y,        4, 1}},
    {0x61, {ADC, indexed_indirect,  6, 0}},
    {0x71, {ADC, indirect_indexed,  5, 1}},

    {0x29, {AND, immediate,         2, 0}},
    {0x25, {AND, zeropage,          2, 0}},
    {0x35, {AND, zeropage_x,        2, 0}},
    {0x2d, {AND, absolute,          3, 0}},
    {0x3d, {AND, absolute_x,        3, 1}},
    {0x39, {AND, absolute_y,        3, 1}},
    {0x21, {AND, indexed_indirect,  2, 0}},
    {0x31, {AND, indirect_indexed,  2, 1}},

    {0x0A, {ASL, none,              2, 0}},
    {0x06, {ASL, zeropage,          5, 0}},
    {0x16, {ASL, zeropage_x,        6, 0}},
    {0x0e, {ASL, absolute,          6, 0}},
    {0x1e, {ASL, absolute_x,        7, 0}},

    {0x90, {BCC, relative,          2, 1}},
    {0xB0, {BCS, relative,          2, 1}},
    {0xF0, {BEQ, relative,          2, 1}},
    {0x30, {BMI, relative,          2, 1}},
    {0xD0, {BNE, relative,          2, 1}},
    {0x10, {BPL, relative,          2, 1}},
    {0x50, {BVC, relative,          2, 1}},
    {0x70, {BVS, relative,          2, 1}},

    {0x00, {BRK, none,              7, 0}},

    {0x24, {BIT, zeropage,          3, 0}},
    {0x2c, {BIT, absolute,          4, 0}},

    {0x18, {CLC, none,              2, 0}},
    {0xd8, {CLD, none,              2, 0}},
    {0x58, {CLI, none,              2, 0}},
    {0xb8, {CLV, none,              2, 0}},

    {0xc9, {CMP, immediate,         2, 0}},
    {0xc5, {CMP, zeropage,          3, 0}},
    {0xd5, {CMP, zeropage_x,        4, 0}},
    {0xcd, {CMP, absolute,          4, 0}},
    {0xdd, {CMP, absolute_x,        4, 1}},
    {0xd9, {CMP, absolute_y,        4, 1}},
    {0xc1, {CMP, indexed_indirect,  6, 0}},
    {0xd1, {CMP, indirect_indexed,  5, 1}},
    {0xe0, {CPX, immediate,         2, 0}},
    {0xe4, {CPX, zeropage,          3, 0}},
    {0xec, {CPX, absolute,          4, 0}},
    {0xc0, {CPY, immediate,         2, 0}},
    {0xc4, {CPY, zeropage,          3, 0}},
    {0xcc, {CPY, absolute,          4, 0}},

    {0xc6, {DEC, absolute,          5, 0}},
    {0xd6, {DEC, zeropage_x,        6, 0}},
    {0xce, {DEC, absolute,          6, 0}},
    {0xde, {DEC, absolute_x,        7, 0}},
    {0xca, {DEX, none,              2, 0}},
    {0x88, {DEY, none,              2, 0}},

    {0x49, {EOR, immediate,         2, 0}},
    {0x45, {EOR, zeropage,          3, 0}},
    {0x55, {EOR, zeropage_x,        4, 0}},
    {0x4d, {EOR, absolute,          4, 0}},
    {0x5d, {EOR, absolute_x,        4, 1}},
    {0x59, {EOR, absolute_y,        4, 1}},
    {0x41, {EOR, indexed_indirect,  6, 0}},
    {0x51, {EOR, indirect_indexed,  5, 1}},

    {0xe6, {INC, zeropage,          5, 0}},
    {0xf6, {INC, zeropage_x,        6, 0}},
    {0xee, {INC, absolute,          6, 0}},
    {0xfe, {INC, absolute_x,        7, 0}},
    {0xe8, {INX, none,              2, 0}},
    {0xc8, {INY, none,              2, 0}},

    {0x4c, {JMP, absolute,          3, 0}},
    {0x6c, {JMP, indirect,          5, 0}},

    {0x20, {JSR, absolute,          6, 0}},

    {0xa9, {LDA, immediate,         2, 0}},
    {0xa5, {LDA, zeropage,          3, 0}},
    {0xb5, {LDA, zeropage_x,        4, 0}},
    {0xad, {LDA, absolute,          4, 0}},
    {0xbd, {LDA, absolute_x,        4, 1}},
    {0xb9, {LDA, absolute_y,        4, 1}},
    {0xa1, {LDA, indexed_indirect,  6, 0}},
    {0xb1, {LDA, indirect_indexed,  5, 1}},

    {0xa2, {LDX, immediate,         2, 0}},
    {0xa6, {LDX, zeropage,          3, 0}},
    {0xb6, {LDX, zeropage_y,        4, 0}},
    {0xae, {LDX, absolute,          4, 0}},
    {0xbe, {LDX, absolute_y,        4, 1}},

    {0xa0, {LDY, immediate,         2, 0}},
    {0xa4, {LDY, zeropage,          3, 0}},
    {0xb4, {LDY, zeropage_x,        4, 0}},
    {0xac, {LDY, absolute,          4, 0}},
    {0xbc, {LDY, absolute_x,        4, 1}},

    {0x4a, {LSR, none,              2, 0}},
    {0x46, {LSR, zeropage,          5, 0}},
    {0x56, {LSR, zeropage_x,        6, 0}},
    {0x4e, {LSR, absolute,          6, 0}},
    {0x5e, {LSR, absolute_x,        7, 0}},

    {0xea, {NOP, none,              2, 0}},

    {0x09, {ORA, immediate,         2, 0}},
    {0x05, {ORA, zeropage,          3, 0}},
    {0x15, {ORA, zeropage_x,        4, 0}},
    {0x0d, {ORA, absolute,          4, 0}},
    {0x1d, {ORA, absolute_x,        4, 1}},
    {0x19, {ORA, absolute_y,        4, 1}},
    {0x01, {ORA, indexed_indirect,  6, 0}},
    {0x11, {ORA, indirect_indexed,  5, 1}},

    {0x48, {PHA, none,              3, 0}},
    {0x08, {PHP, none,              3, 0}},
    {0x68, {PLA, none,              4, 0}},
    {0x28, {PLP, none,              4, 0}},

    {0x2a, {ROL, none,              2, 0}},
    {0x26, {ROL, zeropage,          5, 0}},
    {0x36, {ROL, zeropage_x,        6, 0}},
    {0x2e, {ROL, absolute,          6, 0}},
    {0x3e, {ROL, absolute_x,        7, 0}},

    {0x6a, {ROR, none,              2, 0}},
    {0x66, {ROR, zeropage,          5, 0}},
    {0x76, {ROR, zeropage_x,        6, 0}},
    {0x6e, {ROR, absolute,          6, 0}},
    {0x7e, {ROR, absolute_x,        7, 0}},

    {0x40, {RTI, none,              6, 0}},
    {0x60, {RTS, none,              6, 0}},

    {0xe9, {SBC, immediate,         2, 0}},
    {0xe5, {SBC, zeropage,          3, 0}},
    {0xf5, {SBC, zeropage_x,        4, 0}},
    {0xed, {SBC, absolute,          4, 0}},
    {0xfd, {SBC, absolute_x,        4, 1}},
    {0xf9, {SBC, absolute_y,        4, 1}},
    {0xe1, {SBC, indexed_indirect,  6, 0}},
    {0xf1, {SBC, indirect_indexed,  5, 1}},

    {0x38, {SEC, none,              2, 0}},
    {0xf8, {SED, none,              2, 0}},
    {0x78, {SEI, none,              2, 0}},

    {0x85, {STA, zeropage,          3, 0}},
    {0x95, {STA, zeropage_x,        4, 0}},
    {0x8d, {STA, absolute,          4, 0}},
    {0x9d, {STA, absolute_x,        5, 0}},
    {0x99, {STA, absolute_y,        5, 0}},
    {0x81, {STA, indexed_indirect,  6, 0}},
    {0x91, {STA, indirect_indexed,  6, 0}},

    {0x86, {STX, zeropage,          3, 0}},
    {0x96, {STX, zeropage_y,        4, 0}},
    {0x8e, {STX, absolute,          4, 0}},

    {0x84, {STY, zeropage,          3, 0}},
    {0x94, {STY, zeropage_y,        4, 0}},
    {0x8c, {STY, absolute,          4, 0}},

    {0xaa, {TAX, none,              2, 0}},
    {0xa8, {TAY, none,              2, 0}},
    {0xba, {TSX, none,              2, 0}},
    {0x8a, {TXA, none,              2, 0}},
    {0x9a, {TXS, none,              2, 0}},
    {0x98, {TYA, none,              2, 0}},
};