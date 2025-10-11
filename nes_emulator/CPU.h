// 6502 CPU
#include <SDL.h> // uint8_t miatt

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

typedef struct CPU {
	// 32 kb memória 
	uint8_t *memory; 
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