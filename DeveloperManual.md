# NES Emulátor programozói dokumentáció 

Az emulátor forráskódja a ``src/core`` mappában található.
A ``src/main.c`` fájl az, amely kezeli a grafikus megjelenést.

## CPU.h

### CPU struktúra

```c
typedef struct CPU {
    // 32 kb memória
    uint8_t* memory;

    // regiszterek
    uint8_t a, x, y, s;

    // flagek (1-bit)
    int n;
    int v;
    int d;
    int i;
    int z;
    int c;

    // programszámláló
    uint16_t PC;

    // utasítás futási idő
    int currentCycleTime; 
    int currentCycleTimeInFrame;

    // kapcsolódó egységek
    PPU* ppu;
    Controller* controller;
    Cart* cart;

    // debug
    FILE* logFile;
    char logBuff[128];
} CPU;
```
``memory``<br>
A CPU által elérhető 32 kb-os memória.

``a, x, y, s``<br>
8 bites regiszterek.
+ A: akkumulátor
+ X, Y: általában indexelésre használt regiszterek
+ S: Verem mutató

**Flagek:**
+ ``n``: Negatív flag
+ ``v``: Túlcsordulás flag (overflow)
+ ``d``: Decimális mód flag (bár ez a flag az NES-en nem csinál semmit)
+ ``i``: IRQ blokkoló flag
+ ``z``: Nulla flag
+ ``c``: Átvitel (carry) flag 

``PC``<br>
16 bites programszámláló.

``currentCycleTime``<br>
A jelenlegi utasítás ideje. A PPU ezzel szinkronizál.

``currentCycleTimeInFrame``<br>
A jelenlegi frame elkezdése óta eltelt utasítások idejének összege. Jelenlegi állapotban csak debuggolás során használt.

``ppu``, ``controller``, ``cart``<br>
A CPU kommunikál ezekkel a hardverkomponensekkel, ezért szükséges
"összekötni" a struktúrában.

``logFile``<br>
Megnyitott log.txt fájl mutatója.

``logBuff``<br>
Ideiglenes buffer egy sor kiírásához.

---

### Instruction enumeráció

```c
enum Instruction
{
    Illegal = 0,
    ADC,
    AND,
    ASL,
    BCC,
    ...
};
```

Egy egész szám értéket definiál minden instrukciónak **típusnak.**

---

### Opcode struktúra

```c
typedef struct Opcode {
    enum Instruction instruction;
    enum AddressingMode addressingMode;
    int cycles;
    void (*doInstructionFn)(CPU*, Opcode*);
} Opcode;
```

Az opkód egy instrukció típusból és egy címzési módból áll.
Emelett még hozzárendelek itt egy alap lefutási időt.

---

### Valid opkódok tömbbje

```c
static const struct {
    uint8_t opcodeValue;
    Opcode opcode;
} validOpcodes[] = {
    {0x69, {ADC, immediate,         2, &DoADC}},
    {0x65, {ADC, zeropage,          3, &DoADC}},
    {0x75, {ADC, zeropage_x,        4, &DoADC}},
    ...
};
```

Hozzárendeli a valid opkódokhoz az instrukciókat, címzési módokat és alap lefutási időket.

## CPU.c

```c
Opcode opcodes[256];
```
Az összes opkód amit a CPU tud futtatni. Ebből csak X% érvényes opkód, a többi az illegális, amelyeket a játékok általában nem használtak, ezért az emulátorban azok nem csinálnak semmit. 

--- 

```c
CPU* CreateCPU()
```

A kupacban allokál egy CPU struktúrát és annak visszaadja a mutatóját. A CPU stuktúrán belül a ``memory`` tömbnek is allokál 32 kb memóriát. Ez a függvény továbbá meghívja ``CreateOpcodes`` függvényt.

---

