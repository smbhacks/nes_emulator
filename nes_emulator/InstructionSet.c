#include "InstructionSet.h"
#include "CPU.h"
#include "PPU.h"

// https://www.nesdev.org/wiki/Instruction_reference

void CheckPageCross(CPU* cpu, uint16_t examinedPC)
{
	if (examinedPC / 256 != cpu->PC)
		cpu->currentCycleTime++; // "page crossing" esetén +1 cycle
}

uint16_t GetPCOfAddressing(CPU* cpu, int addressingMode, bool checkPageCrossEnabled)
{
	switch (addressingMode)
	{
	case relative:
	case immediate: {
		return cpu->PC++;
	}
	case zeropage: {
		return cpu->memory[cpu->PC++];
	}
	case absolute: {
		uint8_t low = cpu->memory[cpu->PC++];
		uint8_t high = cpu->memory[cpu->PC++];
		return low + 256 * high;
	}
	case zeropage_x: {
		return (cpu->memory[cpu->PC++] + cpu->x) % 256;
	}
	case zeropage_y: {
		return (cpu->memory[cpu->PC++] + cpu->y) % 256;
	}
	case absolute_x: {
		uint8_t low = cpu->memory[cpu->PC++];
		uint8_t high = cpu->memory[cpu->PC++];
		uint16_t examinedPC = low + 256 * high + cpu->x;
		if(checkPageCrossEnabled) CheckPageCross(cpu, examinedPC);
		return examinedPC;
	}
	case absolute_y: {
		uint8_t low = cpu->memory[cpu->PC++];
		uint8_t high = cpu->memory[cpu->PC++];
		uint16_t examinedPC = low + 256 * high + cpu->y;
		if (checkPageCrossEnabled) CheckPageCross(cpu, examinedPC);
		return examinedPC;
	}
	case indexed_indirect: {
		uint16_t examinedPC = cpu->memory[(cpu->memory[cpu->PC] + cpu->x) % 256] + 256 * cpu->memory[(cpu->memory[cpu->PC] + cpu->x + 1) % 256];
		cpu->PC++;
		return examinedPC;
	}
	case indirect_indexed: {
		uint16_t examinedPC = cpu->memory[cpu->PC] + 256 * cpu->memory[(cpu->PC + 1) % 256] + cpu->y;
		if (checkPageCrossEnabled) CheckPageCross(cpu, examinedPC);
		cpu->PC++;
		return examinedPC;
	}
	default:
		break;
	}
}

// https://www.nesdev.org/wiki/CPU_addressing_modes
uint8_t GetValueWithAddressing(CPU* cpu, int addressingMode, bool checkPageCrossEnabled)
{
	uint16_t address = GetPCOfAddressing(cpu, addressingMode, checkPageCrossEnabled);
	return cpu->memory[address];
}

void SetZeroFlag(CPU* cpu, int val)
{
	if (val == 0)
		cpu->z = 1;
	else
		cpu->z = 0;
}

void SetOverflowFlag(CPU* cpu, int a, int m, int result)
{
	if ((result ^ a) & (result ^ m) & 0x80)
		cpu->v = 1;
	else
		cpu->v = 0;
}

void SetNegativeFlag(CPU* cpu, int val)
{
	if (val >= 128)
		cpu->n = 1;
	else
		cpu->n = 0;
}

void DoIllegal()
{
	// Illegális opkódok ezt futtatják. Ide talán debug print-et érdemes lenne tenni
}

void DoADC(CPU* cpu, Opcode* opcode)
{
	uint8_t operand = GetValueWithAddressing(cpu, opcode->addressingMode, true) + cpu->c;
	int result = operand + cpu->a;
	cpu->a = result % 256;
	if (result > 255)
		cpu->c = 1;
	else
		cpu->c = 0;
	SetZeroFlag(cpu, cpu->a);
	SetOverflowFlag(cpu, cpu->a, operand, result);
	SetNegativeFlag(cpu, cpu->a);
}

void DoSBC(CPU* cpu, Opcode* opcode)
{
	// a carry itt azért működik fordítottan, mert így olcsóbb volt megvalósítani hardveren

	uint8_t operand = GetValueWithAddressing(cpu, opcode->addressingMode, true);
	int result = cpu->a - operand - (1 - cpu->c); 
	cpu->a = result % 256;
	if (result < 0)
		cpu->c = 0;
	else
		cpu->c = 1;
	SetZeroFlag(cpu, cpu->a);
	SetOverflowFlag(cpu, cpu->a, ~operand, result);
	SetNegativeFlag(cpu, cpu->a);
}

void DoAND(CPU* cpu, Opcode* opcode)
{
	uint8_t operand = GetValueWithAddressing(cpu, opcode->addressingMode, true);
	cpu->a &= operand;
	SetZeroFlag(cpu, cpu->a);
	SetNegativeFlag(cpu, cpu->a);
}

void DoBranchOpcode(CPU* cpu, int8_t offset)
{
	cpu->currentCycleTime++;
	uint8_t prevHiOfPC = cpu->PC / 256;
	cpu->PC += offset;
	uint8_t nowHiOfPC = cpu->PC / 256;
	if (nowHiOfPC != prevHiOfPC)
		cpu->currentCycleTime++; // page crossing történt
}

void DoBCC(CPU* cpu, Opcode* opcode)
{
	int8_t offset = (int8_t)GetValueWithAddressing(cpu, opcode->addressingMode, false);
	if (cpu->c == 0)
		DoBranchOpcode(cpu, offset);
}

void DoBCS(CPU* cpu, Opcode* opcode)
{
	int8_t offset = (int8_t)GetValueWithAddressing(cpu, opcode->addressingMode, false);
	if (cpu->c == 1)
		DoBranchOpcode(cpu, offset);
}

void DoBEQ(CPU* cpu, Opcode* opcode)
{
	int8_t offset = (int8_t)GetValueWithAddressing(cpu, opcode->addressingMode, false);
	if (cpu->z == 1)
		DoBranchOpcode(cpu, offset);
}

void DoBNE(CPU* cpu, Opcode* opcode)
{
	int8_t offset = (int8_t)GetValueWithAddressing(cpu, opcode->addressingMode, false);
	if (cpu->z == 0)
		DoBranchOpcode(cpu, offset);
}

void DoBPL(CPU* cpu, Opcode* opcode)
{
	int8_t offset = (int8_t)GetValueWithAddressing(cpu, opcode->addressingMode, false);
	if (cpu->n == 0)
		DoBranchOpcode(cpu, offset);
}

void DoBMI(CPU* cpu, Opcode* opcode)
{
	int8_t offset = (int8_t)GetValueWithAddressing(cpu, opcode->addressingMode, false);
	if (cpu->n == 1)
		DoBranchOpcode(cpu, offset);
}

void DoBVC(CPU* cpu, Opcode* opcode)
{
	int8_t offset = (int8_t)GetValueWithAddressing(cpu, opcode->addressingMode, false);
	if (cpu->v == 0)
		DoBranchOpcode(cpu, offset);
}

void DoBVS(CPU* cpu, Opcode* opcode)
{
	int8_t offset = (int8_t)GetValueWithAddressing(cpu, opcode->addressingMode, false);
	if (cpu->v == 1)
		DoBranchOpcode(cpu, offset);
}

void DoBRK(CPU* cpu, Opcode* opcode)
{
	// todo, de nem annyira fontos, mivel a programok nem használják
}

void DoBIT(CPU* cpu, Opcode* opcode)
{
	uint8_t value_to_test = GetValueWithAddressing(cpu, opcode->addressingMode, false);
	SetZeroFlag(cpu, value_to_test);
	SetNegativeFlag(cpu, value_to_test);
	// overflow flag = 6. bit
	if (value_to_test & 0b01000000)
		cpu->v = 1;
	else
		cpu->v = 0;
}

void DoCLC(CPU* cpu, Opcode* opcode)
{
	cpu->c = 0;
}

void DoCLD(CPU* cpu, Opcode* opcode)
{
	cpu->d = 0;
}

void DoCLI(CPU* cpu, Opcode* opcode)
{
	cpu->i = 0;
}

void DoCLV(CPU* cpu, Opcode* opcode)
{
	cpu->v = 0;
}

void DoSEC(CPU* cpu, Opcode* opcode)
{
	cpu->c = 1;
}

void DoSED(CPU* cpu, Opcode* opcode)
{
	cpu->d = 1;
}

void DoSEI(CPU* cpu, Opcode* opcode)
{
	cpu->i = 1;
}

void DoComparisonOpcode(CPU* cpu, Opcode* opcode, uint8_t* reg)
{
	uint8_t operand = GetValueWithAddressing(cpu, opcode->addressingMode, true);
	uint8_t result = *reg - operand;
	if (*reg >= operand)
		cpu->c = 1;
	else
		cpu->c = 0;
	if (*reg == operand)
		cpu->z = 1;
	else
		cpu->z = 0;
	SetNegativeFlag(cpu, result);
}

void DoCMP(CPU* cpu, Opcode* opcode)
{
	DoComparisonOpcode(cpu, opcode, &cpu->a);
}

void DoCPX(CPU* cpu, Opcode* opcode)
{
	DoComparisonOpcode(cpu, opcode, &cpu->x);
}

void DoCPY(CPU* cpu, Opcode* opcode)
{
	DoComparisonOpcode(cpu, opcode, &cpu->y);
}

void DoDecrementOpcode(CPU* cpu, Opcode* opcode, uint8_t* operand)
{
	*operand -= 1;
	SetZeroFlag(cpu, *operand);
	SetNegativeFlag(cpu, *operand);
}

void DoDEC(CPU* cpu, Opcode* opcode)
{
	DoDecrementOpcode(cpu, opcode, &cpu->memory[GetPCOfAddressing(cpu, opcode->addressingMode, false)]);
}

void DoDEX(CPU* cpu, Opcode* opcode)
{
	DoDecrementOpcode(cpu, opcode, &cpu->x);
}

void DoDEY(CPU* cpu, Opcode* opcode)
{
	DoDecrementOpcode(cpu, opcode, &cpu->y);
}

void DoIncreaseOpcode(CPU* cpu, Opcode* opcode, uint8_t* operand)
{
	*operand += 1;
	SetZeroFlag(cpu, *operand);
	SetNegativeFlag(cpu, *operand);
}

void DoINC(CPU* cpu, Opcode* opcode)
{
	DoIncreaseOpcode(cpu, opcode, &cpu->memory[GetPCOfAddressing(cpu, opcode->addressingMode, false)]);
}

void DoINX(CPU* cpu, Opcode* opcode)
{
	DoIncreaseOpcode(cpu, opcode, &cpu->x);
}

void DoINY(CPU* cpu, Opcode* opcode)
{
	DoIncreaseOpcode(cpu, opcode, &cpu->y);
}

void DoJMP(CPU* cpu, Opcode* opcode)
{
	uint8_t hiOfNewPC, loOfNewPC;
	if (opcode->addressingMode == absolute)
	{
		loOfNewPC = cpu->memory[cpu->PC++];
		hiOfNewPC = cpu->memory[cpu->PC];
	}
	else
	{
		// addressingMode az indirekt JMP ($xxxx)
		// itt van egy 6502 bug, ha $xxxx = $xxFF, mert abban az esetben 
		// a PC $xxFF-t és $xx00-t olvassa. Pl JMP ($03ff) az új PC értékét $03ff-ból és $0300-ból kapja.
		loOfNewPC = cpu->memory[cpu->memory[cpu->PC]];
		if (++cpu->PC % 256 == 0) // a bug emulálása
			cpu->PC -= 256;
		hiOfNewPC = cpu->memory[cpu->memory[cpu->PC]];
	}
	cpu->PC = loOfNewPC + 256 * hiOfNewPC;
}

// a stack 0x100 és 0x1FF között van
void PushToStack(CPU* cpu, uint8_t val)
{
	
	cpu->memory[0x100 + cpu->s] = val;
	cpu->s -= 1;
}
uint8_t PopFromStack(CPU* cpu)
{
	cpu->s += 1;
	return cpu->memory[0x100 + cpu->s];
}

void DoPHA(CPU* cpu, Opcode* opcode)
{
	PushToStack(cpu, cpu->a);
}

void DoPHP(CPU* cpu, Opcode* opcode)
{
	// ezt rakja a stackre: nv11dizc
	uint8_t processorFlags = 0;
	processorFlags += (cpu->c << 0);
	processorFlags += (cpu->z << 1);
	processorFlags += (cpu->i << 2);
	processorFlags += (cpu->d << 3);
	processorFlags += (0b11   << 5);
	processorFlags += (cpu->v << 6);
	processorFlags += (cpu->n << 7);
	PushToStack(cpu, processorFlags);
}

void DoPLA(CPU* cpu, Opcode* opcode)
{
	cpu->a = PopFromStack(cpu);
}

int IsBitOn(int value, int bit)
{
	if (value & (1 << bit))
		return 1;
	else
		return 0;
}

// beállítja a cpu flageket és visszatérési érték az i flag (mert RTI és PLP máshogy kezeli). 
int PopProcessorFlagsFromStack(CPU* cpu)
{
	uint8_t processorFlags = PopFromStack(cpu);
	cpu->c = IsBitOn(processorFlags, 0);
	cpu->z = IsBitOn(processorFlags, 1);
	uint8_t i = IsBitOn(processorFlags, 2); 
	cpu->d = IsBitOn(processorFlags, 3);
	cpu->v = IsBitOn(processorFlags, 6);
	cpu->n = IsBitOn(processorFlags, 7);

	return i;
}

void DoPLP(CPU* cpu, Opcode* opcode)
{
	cpu->i = PopProcessorFlagsFromStack(cpu); // elvileg i flaget késleltetni kell 1 instrukcióval, de ezt most egyelőre így hagyom
}

uint16_t Pop16BitFromStack(CPU* cpu)
{
	uint8_t lo, hi;
	lo = PopFromStack(cpu);
	hi = PopFromStack(cpu);
	return lo + 256 * hi;
}

void DoRTI(CPU* cpu, Opcode* opcode)
{
	cpu->i = PopProcessorFlagsFromStack(cpu); // itt nem kell késleltetni az i flaget, szóval egyből beállítjuk
	cpu->PC = Pop16BitFromStack(cpu);
}

void DoRTS(CPU* cpu, Opcode* opcode)
{
	cpu->PC = Pop16BitFromStack(cpu) + 1;
}

void DoJSR(CPU* cpu, Opcode* opcode)
{
	uint8_t hiOfNewPC, loOfNewPC;
	loOfNewPC = cpu->memory[cpu->PC++];
	hiOfNewPC = cpu->memory[cpu->PC];
	PushToStack(cpu, cpu->PC / 256); //magas először!
	PushToStack(cpu, cpu->PC % 256); //alacsony

	cpu->PC = loOfNewPC + 256 * hiOfNewPC;
}

void DoLoadOpcode(CPU* cpu, Opcode* opcode, uint8_t* reg)
{
	uint8_t value = GetValueWithAddressing(cpu, opcode->addressingMode, true);
	*reg = value;

	SetZeroFlag(cpu, *reg);
	SetNegativeFlag(cpu, *reg);
}

void DoLDA(CPU* cpu, Opcode* opcode)
{
	DoLoadOpcode(cpu, opcode, &cpu->a);
}

void DoLDX(CPU* cpu, Opcode* opcode)
{
	DoLoadOpcode(cpu, opcode, &cpu->x);
}

void DoLDY(CPU* cpu, Opcode* opcode)
{
	DoLoadOpcode(cpu, opcode, &cpu->y);
}

void DoStoreOpcode(CPU* cpu, Opcode* opcode, uint8_t value)
{
	uint16_t addr = GetPCOfAddressing(cpu, opcode->addressingMode, false);

	if (addr / 256 == 0x20)
		WritingToPPUReg(cpu->ppu, addr, value);
	else
		cpu->memory[addr] = value;
}

void DoSTA(CPU* cpu, Opcode* opcode)
{
	DoStoreOpcode(cpu, opcode, cpu->a);
}

void DoSTX(CPU* cpu, Opcode* opcode)
{
	DoStoreOpcode(cpu, opcode, cpu->x);
}

void DoSTY(CPU* cpu, Opcode* opcode)
{
	DoStoreOpcode(cpu, opcode, cpu->y);
}

void DoNOP(CPU* cpu, Opcode* opcode)
{
	// NO oPeration
	// Nem kell semmit csinálni
}

void DoEOR(CPU* cpu, Opcode* opcode)
{
	uint8_t operand = GetValueWithAddressing(cpu, opcode->addressingMode, true);
	cpu->a ^= operand;
	SetZeroFlag(cpu, cpu->a);
	SetNegativeFlag(cpu, cpu->a);
}

void DoORA(CPU* cpu, Opcode* opcode)
{
	uint8_t operand = GetValueWithAddressing(cpu, opcode->addressingMode, true);
	cpu->a |= operand;
	SetZeroFlag(cpu, cpu->a);
	SetNegativeFlag(cpu, cpu->a);
}

uint8_t* GetRegOrAddrOperand(CPU* cpu, Opcode* opcode, bool checkPageCrossEnabled)
{
	if (opcode->addressingMode == none)
	{
		return &cpu->a;
	}
	else
	{
		return &cpu->memory[GetPCOfAddressing(cpu, opcode->addressingMode, checkPageCrossEnabled)];
	}
}

void DoASL(CPU* cpu, Opcode* opcode)
{
	uint8_t* operand = GetRegOrAddrOperand(cpu, opcode, false);
	// Carry flag beállítása bit 7-re
	if (*operand >= 128)
		cpu->c = 1;
	else
		cpu->c = 0;
	*operand *= 2;
	SetZeroFlag(cpu, *operand);
	SetNegativeFlag(cpu, *operand);
}

void DoLSR(CPU* cpu, Opcode* opcode)
{
	uint8_t* operand = GetRegOrAddrOperand(cpu, opcode, false);
	// Carry flag beállítása bit 0-ra
	if (*operand & 0b1)
		cpu->c = 1;
	else
		cpu->c = 0;
	*operand /= 2;
	SetZeroFlag(cpu, *operand);
	cpu->n = 0;
}

void DoROL(CPU* cpu, Opcode* opcode)
{
	uint8_t* operand = GetRegOrAddrOperand(cpu, opcode, false);
	// Carry flag beállítása bit 7-re
	if (*operand >= 128)
		cpu->c = 1;
	else
		cpu->c = 0;
	*operand *= 2;
	*operand += cpu->c; // c -> 0. bit
	SetZeroFlag(cpu, *operand);
	SetNegativeFlag(cpu, *operand);
}

void DoROR(CPU* cpu, Opcode* opcode)
{
	uint8_t* operand = GetRegOrAddrOperand(cpu, opcode, false);
	// Carry flag beállítása bit 0-ra
	if (*operand & 0b1)
		cpu->c = 1;
	else
		cpu->c = 0;
	*operand /= 2;
	*operand += (cpu->c << 7); // c -> 7. bit
	SetZeroFlag(cpu, *operand);
	cpu->n = 0;
}

void DoTransferOpcode(CPU* cpu, uint8_t *from, uint8_t *to)
{
	*to = *from;

	SetZeroFlag(cpu, *to);
	SetNegativeFlag(cpu, *to);
}

void DoTAX(CPU* cpu, Opcode* opcode)
{
	DoTransferOpcode(cpu, &cpu->a, &cpu->x);
}

void DoTAY(CPU* cpu, Opcode* opcode)
{
	DoTransferOpcode(cpu, &cpu->a, &cpu->y);
}

void DoTSX(CPU* cpu, Opcode* opcode)
{
	DoTransferOpcode(cpu, &cpu->s, &cpu->x);
}

void DoTXA(CPU* cpu, Opcode* opcode)
{
	DoTransferOpcode(cpu, &cpu->x, &cpu->a);
}

void DoTXS(CPU* cpu, Opcode* opcode)
{
	DoTransferOpcode(cpu, &cpu->x, &cpu->s);
}

void DoTYA(CPU* cpu, Opcode* opcode)
{
	DoTransferOpcode(cpu, &cpu->y, &cpu->a);
}