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
Visszatérési értéke a CPU struktúra címe.

---

```c
void FreeCPU(CPU* cpu)
```

Felszabadítja a paraméterben megadott CPU struktúra ``memory`` tömbjét, utána meg magát a struktúrát.

---

```c
uint8_t ReadCpuMem(CPU* cpu, uint16_t address)
```

Egy adott CPU címen olvas egy értéket. Az address értéke
bármi lehet. (0x0000-0xFFFF).
Visszatérési értéke a kiolvasott érték.

---

```c
void WriteCpuMem(CPU* cpu, uint16_t address, uint8_t value)
```

Egy adott CPU címre ír egy értéket. Az addressre ennek kell teljesülnie: 0x0000 <= address < 0x2000, vagy 0x6000 <= address. A többi memóriaterületekre való írás ebben a függvényben már nem csinál semmit.

---

```c
int TickCPU(CPU *cpu)
```

Futtat 1 instrukciót a CPU-n. A cpu->currentCycleTime-ban megjeleníti az eltelt futásidőt. Ha a PPU generateNMI flagje 1-re állította magát, akkor az instrukció után az NMI címre ugrik a programszámláló. 

---

```c
void CreateOpcodes()
```

Létrehozza az ``opcodes`` tömbben az összes opkód struktúráját.
Ehhez felhasználja a ``validOpcodes`` tömböt, az architektúra által nem támogatott opkódokhoz pedig "illegális instrukció" függvényt rendel (amelyek nem csinálnak semmit).

## AddressingEnum.h

```c
enum AddressingMode
{
    none = 0,
    immediate,
    relative,
    zeropage,
    absolute,
    indirect,
    zeropage_x,
    zeropage_y,
    absolute_x,
    absolute_y,
    indexed_indirect,
    indirect_indexed
};
```
A címzési módok jelentése:
+ ``none`` (Implicit/Accumulator): Az utasításnak nincs operandusa (pl. RTS), vagy az akkumulátoron végez műveletet (pl. LSR A).
+ ``immediate``: Az utasítás utáni bájt maga az adat, nem egy memóriacím (pl. LDA **#10**).
+ ``relative``: Feltételes ugrásoknál (branch) használt relatív eltolás (offset).
+ ``zeropage``: A memória első 256 bájtjának (0. lap, $00xx) címzése. Gyorsabb és rövidebb, mint az abszolút címzés.
+ ``absolute``: Teljes 16 bites memóriacím megadása ($xxxx).
+ ``indirect``: Indirekt címzés (pointer), ahol a megadott cím a tényleges címet tartalmazza. A 6502-n ezt csak a JMP utasítás használja.
+ ``zeropage_x``, ``zeropage_y``: A 0. lapon belüli címzés, az X vagy Y regiszter értékével eltolva. Ha az összeg túlcsordul, a cím a 0. lapon belül marad.
+ ``absolute_x``, ``absolute_y``: 16 bites abszolút címzés, az X vagy Y regiszter értékével eltolva.
+ ``indexed_indirect`` (Indirect, X): "Pre-indexed" mód. A 0. lap címéhez hozzáadjuk az X regisztert, és az így kapott címről olvassuk ki a tényleges 16 bites címet.
+ ``indirect_indexed`` (Indirect), Y: "Post-indexed" mód. A 0. lapon megadott címről kiolvasunk egy 16 bites báziscímet, és ahhoz adjuk hozzá az Y regiszter értékét.

## PPU.h

### PPU struktúra

```c
typedef struct PPU {
	uint8_t* memory; 
	uint8_t* oam;

	int ppuDotX;
	int ppuDotY;
	
	bool vram32Increment;
    bool spritesSecondPatternSelected;
    
    bool bgSecondPatternSelected;
	bool spritesAre8x16;
	bool nmiEnabled;

	bool greyscale;
	bool bgShowLeftmost8;
    bool spritesShowLeftmost8;
	bool renderBg;
	bool renderSprites;
	bool emphasizeRed;
	bool emphasizeGreen;
	bool emphasizeBlue;

	int vblankFlag;
    int sprite0Flag;

	uint8_t PPUReadBuff; 

	bool generateNMI;
	bool endOfFrame;

	union {
		struct {
			unsigned int coarseX : 5;
			unsigned int coarseY : 5;
			unsigned int nametableSelect : 2;
			unsigned int fineY : 3;
		};
		uint16_t value;
	} v, t;
    uint8_t fineX;
	bool secondWrite;

	int sprite0_X; 
	int sprite0_Y;
	bool drewSprite0;

	uint8_t* display;
    uint8_t* palette;
	bool using_default_palette;

	Cart* cart;
} PPU;
```

``memory``<br>
16 kb memória, amelyet a CPU tud manipulálni
 
``oam``<br>
(Object Attribute Memory) 256 byteos PPU belső memória, amely a "sprite"-okat rajzolja. 64 sprite tárolására képes. A CPU ezt 
3 regiszterrel tudja módosítani. Általában csak 1-et használnak, az "OAMDMA" regisztert, amely kimásolja a CPU egy 256 bájtnyni részét a PPU OAMjába.

``ppuDotX``<br>
+ 0: "idle"<br>
+ 1~256: látható pontok<br>
+ 257~340: nem látható pontok 

``ppuDotY``<br>
+ -1: renderelés előtti scanline (pre-render scanline)<br>
+ 0~239: láthathó scanline (kijelzőn látszik!)<br>
+ 240: renderelés utáni scanline<br>
+ 241~260: "vblank" rész

``vram32Increment``<br>
Egy PPU adatírás/adatolvasás után a PPU internális regisztere (v):
+ hamis: +1, "előre megy"<br>
+ igaz: +32, "lefelé megy"

``spritesSecondPatternSelected``<br>
A spriteok CHR (grafika) adata a VRAM-ban innentől kezdődik:
+ hamis: 0x0000<br>
+ igaz: 0x1000

``bgSecondPatternSelected``<br>
A háttér CHR (grafika) adata a VRAM-ban innentől kezdődik:
+ hamis: 0x0000<br>
+ igaz: 0x1000

``spritesAre8x16``<br>
Egy sprite mérete pixelben (hossz x magasság):
+ hamis: 8x8<br>
+ igaz: 8x16

``nmiEnabled``<br>
NMI interruptot engedélyező flag 

``greyscale``<br>
Szürkeárnyalatos mód.<br>
Az emulátor ezt nem támogatja, de elmenti az értékét.

``bgShowLeftmost8``<br>
Meghatározza, hogy a PPU rajzoljon-e a kijelző bal oldalán lévő 8 pixeles hosszú sávban hátteret.

``spritesShowLeftmost8``<br>
Meghatározza, hogy a PPU rajzoljon-e a kijelző bal oldalán lévő 8 pixeles hosszú sávban spriteokat.

``renderBg``<br>
Háttér rajzolást engedélyező flag

``renderSprites``<br>
Sprite rajzolást engedélyező flag

``emphasizeRed``<br>
Piros színt kiemelő flag.
Az emulátor ezt nem támogatja, de elmenti az értékét.

``emphasizeGreen``<br>
Zöld színt kiemelő flag.
Az emulátor ezt nem támogatja, de elmenti az értékét.

``emphasizeBlue``<br>
Kék színt kiemelő flag.
Az emulátor ezt nem támogatja, de elmenti az értékét.

``vblankFlag``
Ez egy PPU internális flag, amely visszaadja, hogy a katódcső az a VBlank részt elkezdte-e már, de a valóságban nem mindig tükrözi a valóságot. https://www.nesdev.org/wiki/PPU_registers#Vblank_flag<br>

``sprite0Flag``<br>
Igaz, ha a PPU érzékelte már a 0. sprite rajzolását.
A játékok ezt CPU-PPU szinkronizálásra használják.

``PPUReadBuff``<br>
A PPU túl lassú, hogy időben odaadja az PPUDATA által olvasott értéket, ezért a PPU egy buffert használ, amely minden olvasásnál frissül. Ettől a PPUDATA olvasása mindig 1 olvasással késik, ezért a játékok egy "dummy" olvasást csinálnak az address beállítása után.

``generateNMI``<br>
Ha a PPU elér a VBlank részbe, és a ``nmiEnabled`` belső flag igaz, akkor beállítja ezt az értéket igazra, hogy azt majd a ``TickCPU`` függvény érzékelje, és ugorjon az NMI címre.

``endOfFrame``<br>
Ha a PPU elér a VBlank részbe, akkor ez beállítódik igazra, hogy a ``TickNES`` függvény érzékelje, hogy letelt ~1/60 másodperc, azaz egy frame idő.

https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers

### Belső regiszterek

```c
union {
	struct {
        // "Kamera" durva X értéke
		unsigned int coarseX : 5;

        // "Kamera" durva Y értéke
		unsigned int coarseY : 5;

        // Bázis nametable választó a VRAM-ban
		unsigned int nametableSelect : 2;

        // "Kamera" finom Y értéke
		unsigned int fineY : 3;
	};
	uint16_t value;
} v, t;
```

``fineX``<br>
3-bit, finom X scroll

``secondWrite``<br>
1-bit, ezzel különbözteti meg ugyanazon a regiszteren érkező A és B értéket (w)

``sprite0_X``<br> 
0. sprite utolsó rajzolt X elhelyezése. 

``sprite0_Y``<br>
0. sprite utolsó rajzolt Y elhelyezése. 

``drewSprite0``<br>
Azt határozza meg, hogy a jelenlegi frameben rajzoltunk-e 0. spriteot.

``display``<br>
Mutató egy 256x240 RGB24 bufferre (uint8_t*), **amelyet a core futtatónak kell legfoglalnia és beállítania**. Ide fogja a PPU rajzolni az adott képkockát.

``palette``
64x RGB24 tömbbre (palettára) mutató (uint8_t*)

``using_default_palette``
Ha ez igaz, akkor a core egy konstans palette tömbre mutat, a futtatónak meg nem kell foglalkoznia a paletta biztosításával

``Cart* cart``<br>
Összeköttetés a kazettával, hogy a PPU el tudja érni a CHR ROM-ot.

## PPU.c

```c
PPU* CreatePPU()
```
Létrehoz egy új PPU struktúrát a kupacban. Inicializálja a belső változókat nullára. Lefoglalja a PPU ``memory`` (16 KB) és az ``oam`` (256 bájt) mezőit a kupacban.
Beállítja az alapértelmezett palettát a ``ResetPalette`` függvénnyel és visszaadja a struktúra mutatóját.

---

```c
void FreePPU(PPU* ppu)
```

Felszabadítja a PPU-hoz tartozó memóriaterületeket (paletta, memória, oam), majd magát a PPU struktúrát is.

---

```c
void ResetPalette(PPU* ppu)
```

Visszaállítja a PPU-t az alapértelmezett, emulátorba épített paletta használatára, és szükség esetén felszabadítja az egyéni palettát.

---

```c
void WritingToPPUReg(PPU* ppu, uint16_t reg, uint8_t value)
```

Kezeli a CPU írásait a PPU regisztereibe (0x2000-0x2007)
+ ``PPU_REG_CTRL (0x2000)``: Beállítja a nametable kiválasztást, cím inkrementálás mértékét, sprite/háttér VRAMban kezdeti címét, és az NMI engedélyezést. Ha VBlank alatt engedélyezi a játék az NMI-t ezen a regiszeren keresztül, akkor az azonnal lefut.
+ ``PPU_REG_MASK (0x2001)``: Renderelési beállítások (szürkeárnyalat, bal oldali 8 pixel elrejtése, háttér/sprite renderelés engedélyezése, színkiemelés).
+ ``PPU_REG_SCROLL (0x2005)``: A "kamera" pozíciójának beállítása. A ``secondWrite`` belső flag segítségével különbözteti meg az első írást és a második írást. Frissíti a belső ``t`` (temporary) regiszter részeit.
+ ``PPU_REG_ADDR (0x2006)``: A VRAM cím beállítása. Szintén két írást igényel (felső bájt, majd alsó bájt). Frissíti a ``t`` regisztert, majd a második írásnál átmásolja ``t``-t a ``v`` regiszterbe.
+ ``PPU_REG_DATA (0x2007)``: Adat írása a VRAM-ba a beállított ``v`` címre. Automatikusan növeli a ``v`` címet (+1 vagy +32).

---

```c
uint8_t ReadingFromPPUReg(PPU* ppu, uint16_t reg)
```

Kezeli a CPU olvasásait a PPU regiszterekből. Visszatérési értéke a kiolvasott érték.
+ ``PPU_REG_STATUS (0x2002)``: Visszaadja a státusz biteket (VBlank, Sprite 0 hit). Olvasáskor törli a VBlank flaget.
+ ``PPU_REG_DATA (0x2007)``: Adat olvasása a VRAM-ból. Az olvasás bufferelt, azaz mindig az előző olvasás eredményét kapjuk vissza (PPUReadBuff), miközben a belső buffer frissül az aktuális értékkel.

---

```c
void TickPPU(PPU* ppu)
```

Lefuttatja a PPU-t 1 órajel erejéig.
Feladatai:
+ Rajzolás: Ha a látható tartományban vagyunk (X: 1-256, Y: 0-239), meghívja a ``DrawPPUDot`` függvényt.
+ Flag frissítés: Kezeli a Sprite 0 és VBlank flagek beállítását és törlését az időzítésnek megfelelően.
+ Görgetés: Frissíti a ``v`` regisztert.
+ Frame vége: Ha a ciklus eléri a VBlank kezdetét, beállítja az ``endOfFrame`` és ``generateNMI`` változókat (utóbbit akkor, ha engedélyezett az ``nmiEnabled`` által).
+ Koordináta léptetés: Növeli a ``ppuDotX`` és ``ppuDotY`` értékeket, azaz mozgatja a katódsugárcsövet.

---

```c
void DrawPPUDot(PPU* ppu)
```

Kirajzol egyetlen pixelt a jelenlegi (``ppuDotX``, ``ppuDotY``) pontban a ``display`` bufferbe.
Folyamata:
+ Kiszámolja a tile címét a nametable-ben a görgetés (``v`` regiszter és ``fineX``) figyelembevételével.
+ Beolvassa a tile értéket az adott tile címen.
+ Meghívja a ``GetPixelColor``-t, hogy lekérje a tile adott pontban lévő pixel szín értéket.
+ Kiválasztja a megfelelő színpalettát.
+ Összeállítja a végső színt és kiírja a képernyő bufferbe a ``DrawPixelWithPal`` függvénnyel. Ha a pixel átlátszó, magenta színt ír, hogy később a sprite-ok tudják, hova rajzolhatnak "mögé".

---

```c
void DrawSprites(PPU* ppu)
```

Végigiterál az OAM memórián (fordított sorrendben a prioritás miatt), és kirajzolja a sprite-okat az aktuális ``display``-re. Minden sprite-ra meghívja a ``DrawOneSprite`` függvényt.

```c
void DrawOneSprite(
    PPU* ppu, 
    uint8_t spriteX, uint8_t spriteY, 
    uint8_t spriteTile, 
    uint8_t spriteAttributes, 
    bool isSprite0
)
```

Ellenőrzi, hogy a sprite a képernyőn van-e.<br>
Kezeli a horizontális és vertikális tükrözést (flip).<br>
Lekéri a sprite tile pixel színét a ``GetPixelColor`` segítségével.<br>
Kezeli a prioritást (háttér előtt/mögött).<br>
"Sprite 0 Hit": Ha a 0. sprite épp nem átlátszó pixelt rajzol egy nem átlátszó háttér pixelre, beállítja a ``drewSprite0`` flaget és a koordinátákat (``sprite0_X``, ``sprite0_Y``).

---

```c
void DrawBackgroundColor(PPU* ppu)
```

A frame végén fut le, és minden olyan pixelt, ami magenta maradt, azaz sem a háttér, sem a sprite-ok nem fedték le, oda az 
univerzális háttérszín értékét írja (PPU memória 0x3F00-as memóriacím tartalma).

---

```c
void IncHoriV(PPU* ppu)
```

Növeli a ``coarseX``-et. Ha túlcsordul, vált a horizontális nametable-n.

---

```c
void IncVertV(PPU* ppu)
```

Növeli a ``fineY``-t. Ha túlcsordul, növeli a ``coarseY``-t. Ha az is túlcsordul (0~29-ig, utána 0-ra állítódik), vált a vertikális nametable-n.

---

```c
void UpdateV(PPU* ppu)
```

Meghívja az ``IncHoriV``, valamint ``IncVertV`` függvényeket továbbá kezeli a belső regiszterek tartalmát, amikor szükséges (időzett ``ppuDotX`` és ``ppuDotY`` alapján).

---

```c
void UpdateVblankFlag(PPU* ppu)
```

Frissíti szükség esetén a ``vblankFlag``-et a ``ppuDotX`` és ``ppuDotY`` alapján.

---

```c
void UpdateSprite0Flag(PPU* ppu)
```

Frissíti szükség esetén a ``sprite0Flag``-et a ``ppuDotX`` és ``ppuDotY`` alapján.

---

```c
void DrawPixelWithPal(PPU* ppu, uint8_t paletteValue, int x, int y)
```

Rajzol egy adott NES színértéket egy adott x és y pontban a ``display``-re. Ehhez meghívja a ``DrawPixel`` függvényt.

---

```c
void DrawPixel(PPU* ppu, int x, int y, uint8_t r, uint8_t g, uint8_t b)
```

Rajzol egy adott RGB értéket egy adott x és y pontban a ``display``-re.

---

```c
void SubFromNamX(uint16_t* addr, int x)
```

Kivon egy nametable addressből (``uint16_t* address``) úgy, hogy kezelje a túlcsordulást nametable váltással.

---

```c
void AddToNamX(uint16_t* addr, int x)
```

Hozzáad egy nametable addresshez (``uint16_t* address``) úgy, hogy kezelje a túlcsordulást nametable váltással.

---

```c
bool isDisplayMagenta(PPU* ppu, int x, int y)
```

Visszaadja, hogy az adott x, y pontban magenta szín van-e a ``display``-en.

## InstructionSet

Ez a fájl tartalmazza a 6502 alapú CPU utasításkészletének implementációját, valamint a címzési módok kezelését.

--- 

```c
uint16_t GetPCOfAddressing(
    CPU* cpu, 
    int addressingMode, 
    bool checkPageCrossEnabled
)
```

Az aktuális utasítás címzési módja alapján kiszámolja a memóriacímet, ahol az operandus található. Kezeli a programszámláló léptetését.<br>
Meghívja a ``CheckPageCross`` függvényt, ha annak ellenőrzését figyelembe kell venni az adott instrukcióban.<br>

---

```c
uint8_t GetValueWithAddressing(
    CPU* cpu, 
    int addressingMode, 
    bool checkPageCrossEnabled
)
```

Meghívja a ``GetPCOfAddressing`` függvényt, majd kiolvassa az azon a címen lévő értéket.

---

```c
void SetZeroFlag(CPU* cpu, uint8_t val)
```

Ha a ``val`` = 0, akkor beállítja a ``z`` flaget 1-re, egyébként 0-ra.

---

```c
void SetOverflowFlag(CPU* cpu, uint8_t a, uint8_t m, uint8_t result)
```

Beállítja a ``v`` flaget 1-re, az ``a``, ``m`` és a ``result`` MSB-k alapján. Előjeles összeadás után a rossz előjeles végeredményt jelzi.

---

```c
void SetNegativeFlag(CPU* cpu, uint8_t val)
```

Beállítja a ``n`` flaget 1-re, ha a ``val`` MSB-je 1-es.

---

```c
void DoIllegal(CPU* cpu, Opcode* opcode)
void DoBRK(CPU* cpu, Opcode* opcode)
void DoNOP(CPU* cpu, Opcode* opcode)
```

Nem csinál semmit.

---

```c
void DoADC(CPU* cpu, Opcode* opcode)
void DoSBC(CPU* cpu, Opcode* opcode)
```

Összeadás és kivonás carry-vel az A regiszterbe. Beállítja a ``c``, ``v``, ``z``, ``n`` flageket.

---

```c
void DoAND(CPU* cpu, Opcode* opcode)
```

Az operandussal ÉS-eli az A regisztert. Beállítja a ``z``, ``n`` flageket.

---

```c
void DoBranchOpcode(CPU* cpu, int8_t offset)
```

A program "branchel" a CPU programszámláló beállításával.
*Page crossing*-t figyelmbe veszi.<br>
Ezt a függvényt a következő függvények hívják meg:  
+ ``DoBCC``: branch, ha ``c``=0
+ ``DoBCS``: branch, ha ``c``=1
+ ``DoBEQ``: branch, ha ``z``=1
+ ``DoBNE``: branch, ha ``z``=0
+ ``DoBPL``: branch, ha ``n``=0
+ ``DoBMI``: branch, ha ``n``=1
+ ``DoBVC``: branch, ha ``v``=0
+ ``DoBVS``: branch, ha ``v``=1

---

```c
void DoBIT(CPU* cpu, Opcode* opcode)
```

Teszteli a memóriában lévő biteket az akkumulátorral (A regiszter), anélkül, hogy megváltoztatná azt (``n``, ``v``, ``z`` flagek beállítása).

---

```c
void DoCLC(CPU* cpu, Opcode* opcode)
void DoCLD(CPU* cpu, Opcode* opcode)
void DoCLI(CPU* cpu, Opcode* opcode)
void DoCLV(CPU* cpu, Opcode* opcode)
```

Beállítja az egyes flageket 0-ra.

---

```c
void DoSEC(CPU* cpu, Opcode* opcode)
void DoSED(CPU* cpu, Opcode* opcode)
void DoSEI(CPU* cpu, Opcode* opcode)
```

Beállítja az egyes flageket 1-re.

---

```c
void DoComparisonOpcode(CPU* cpu, Opcode* opcode, uint8_t* reg)
```

Végrehajt egy komparátor instrukciót, amelynek eredményét a ``c``, ``z``, ``n`` flagekben jeleníti meg. 
Ezt a függvényt a következő függvények hívják meg: ``DoCMP``, ``DoCPX``, ``DoCPY``
A függvény ``*reg`` paramétere a regiszter címe kerül (A, X, Y), amellyel összehasonlítunk. 

---

```c
void DoDecrementOpcode(CPU* cpu, Opcode* opcode, uint8_t* operand)
```
Végrehajt egy dekrementálás instrukciót. Beállítja a ``z``, ``n`` flageket.
Ezt a függvényt a következő függvények hívják meg: ``DoDEC``, ``DoDEX``, ``DoDEY``
A függvény ``*reg`` paramétere az operandus címe (RAM érték, X, Y).

---

```c
void DoIncreaseOpcode(CPU* cpu, Opcode* opcode, uint8_t* operand)
```
Végrehajt egy inkrementálás instrukciót. Beállítja a ``z``, ``n`` flageket.
Ezt a függvényt a következő függvények hívják meg: ``DoINC``, ``DoINX``, ``DoINY``
A függvény ``*reg`` paramétere az operandus címe (RAM érték, X, Y).

---

```c
void DoJMP(CPU* cpu, Opcode* opcode)
```

Feltétel nélkül ugrás.<br>
Hardware Bug Emuláció: Az indirekt ugrásnál (JMP ($xxxx)), ha a vektor címe oldalhatárra esik (pl. $xxFF), a 6502 processzor hibásan olvassa a cím felső bájtját (nem lép át a következő page-re), hanem $xx00-t olvassa. 

---

```c
void PushToStack(CPU* cpu, uint8_t val)
uint8_t PopFromStack(CPU* cpu)
```

A CPU memória 0x0100~0x1FF része a verem. Ezek a függvények oda pusholnak és onnan poppolnak értéket.

---

```c
void DoPHA(CPU* cpu, Opcode* opcode)
void DoPLA(CPU* cpu, Opcode* opcode)
void DoPHP(CPU* cpu, Opcode* opcode)
```

PHA: Akkumulátor pusholása a veremre.
PLA: Akkumulátorba poppolás a veremről.
PHP: Processzor flagek pusholása a veremre.
Ezek a függvények a ``PushToStack`` és ``PopFromStack``-et hívják meg.

---

```c
int IsBitOn(int value, int bit)
```

Bit tesztelésre használt segédfüggvény.<br>
``value``: vizsgálandó érték<br>
``bit``: vizsgálandó bit sorszáma 0.-tól sorszámozva<br>
Visszatérési érték az adott sorszámú bit értéke az értékben (0 vagy 1).

---

```c
int PopProcessorFlagsFromStack(CPU* cpu)
```

Függvény, amely a veremről poppol egy érték a processzor flagekbe. A ``DoPLP`` opkód mellett a ``DoRTI`` is ezt hívja meg.

---

```c
uint16_t Pop16BitFromStack(CPU* cpu)
```

Kétszer hívja meg a ``PopFromStack`` függvényt, és egy 16-bites értékkel tér vissza 8-bit helyett. A ``DoRTS`` és a ``DoRTI`` függvények hívják meg.

---

```c
void DoRTS(CPU* cpu, Opcode* opcode)
void DoRTI(CPU* cpu, Opcode* opcode)
```

Meghívják a ``Pop16BitFromStack`` függvényt, és annak visszatérési értékére állítják a programszámlálót. A ``DoRTI`` abban különleges, hogy előtte még a processzor flagjeit beállítja a ``PopProcessorFlagsFromStack`` visszatérési értékére.

---

```c
void DoJSR(CPU* cpu, Opcode* opcode)
```

Szubrutin hívás. Veremre felrakja a visszatérendő programszámláló értékét a ``PushToStack`` függvénnyel, és utána az adott címre ugrik.

---

```c
void DoLoadOpcode(CPU* cpu, Opcode* opcode, uint8_t* reg)
```

Végrehajt egy betöltés instrukciót. Beállítja a ``z``, ``n`` flageket.
Ezt a függvényt a következő függvények hívják meg: ``DoLDA``, ``DoLDX``, ``DoLDY``
A függvény ``*reg`` paramétere az operandus címe kerül (A, X, Y).

---

```c
void DoOamDma(CPU* cpu, PPU* ppu, uint8_t page)
```

Végrehajt egy egész OAM DMA-t. 
A PPU ``oam``-jába másolja a paraméterként megadott ``page`` 256 bájtnyi CPU memória területet. 
Ezalatt a CPU "megáll", amit az emulátor úgy kezel, hogy az ``currentCycleTime``-hoz hozzáad 513-at (kb. eddig tart az OAM DMA).

---

```c
void DoStoreOpcode(CPU* cpu, Opcode* opcode, uint8_t value)
```

A ``STA``, ``STX``, ``STY`` utasítások közös kezelője. Ez nem csak a RAM-ba írhat, hanem ezzel keresztül kommunikálhat különböző NES egységekkel:
+ PPU Regiszterek (0x2000-0x3FFF): Ha a cím ebbe a tartományba esik, a ``WritingToPPUReg`` függvényt hívja.
+ OAM DMA (0x4014): Ha erre a címre írunk, meghívja a ``DoOamDma`` függvényt.
+ Kontroller (0x4016): A kontroller input polling indítása.
Egyéb esetben sima írás a CPU memóriába, vagy mapperrel való kommunikálás: ``WriteCpuMem``

---

```c
void DoEOR(CPU* cpu, Opcode* opcode)
```

Az operandussal KIZÁRÓ VAGY-olja az A regisztert. Beállítja a ``z``, ``n`` flageket.

---

```c
void DoORA(CPU* cpu, Opcode* opcode)
```

Az operandussal VAGY-olja az A regisztert. Beállítja a ``z``, ``n`` flageket.

---

```c
uint8_t* GetRegOrAddrOperand(CPU* cpu, Opcode* opcode, bool checkPageCrossEnabled)
```

Ha az opkód címzési módja implicit, semmi (akkumulátor), akkor az akkumulátor ``cpu->a`` címét adja vissza.
Egyébként a CPU memóriájában egy érték címét adja vissza az adott címzési módnak megfelelően.

---

```c
void DoASL(CPU* cpu, Opcode* opcode)
void DoLSR(CPU* cpu, Opcode* opcode)
void DoROL(CPU* cpu, Opcode* opcode)
void DoROR(CPU* cpu, Opcode* opcode)
```

Biteltoló és forgató utasítások. Az ``ASL``, ``LSR`` belépő bitjei 0-ák, míg a ``ROL`` és ``ROR`` opkódoké a ``c`` flag.
Az utóbbi kettő továbbá beállítja a ``c`` flaget a kilépő bitre. 
Ezen kívül még más flageket is beállítanak.

---

```c
void DoTransferOpcode(CPU* cpu, uint8_t *from, uint8_t *to, bool setFlags)
```

Átmásolja a ``*from`` értékét a ``*to`` változóba. Regiszterek közötti adatmozgató opkódok használják: ``DoTAX``, ``DoTAY``, ``DoTSX``, ``DoTXA``, ``DoTXS``, ``DoTYA``.<br>
Ha az adott instrukció követeli, akkor beállítja a ``z`` és ``n`` flageket a ``setFlags`` paraméteren keresztül.

## NES.h

### NES struktúra

```c
typedef struct NES {
	CPU* cpu;
	PPU* ppu;
	Cart* cart;
	Controller* controller;

	bool cartInserted;
} NES;
```

``cpu``<br>
Mutató a dinamikusan foglalt ``CPU`` struktúrára.

``ppu``<br>
Mutató a dinamikusan foglalt ``PPU`` struktúrára.

``cart``<br>
Mutató a behelyezett kazettára (``Cart`` struktúra).

``controller``<br>
Mutató a kontroller kezelőjére (``Controller`` struktúra).

``cartInserted``<br>
Logikai érték, amely jelzi, ha van érvényes kazetta betöltve.

## NES.c

```c
NES* CreateNES()
```

Létrehoz egy új NES példányt.
Lefoglalja a memóriát a NES struktúrának.
Meghívja a komponensek létrehozó függvényeit (``CreateCPU``, ``CreatePPU``, ``CreateController``).
Összekötés: Beállítja a ``cpu``-ban a ``ppu`` és ``controller`` mutatókat, hogy a processzor elérje azokat.
Beállítja a ``cartInserted`` változót hamisra.

---

```c
void UseCustomPalette(NES* nes, uint8_t* pal_ptr)
```

Lehetővé teszi egy egyéni színpaletta betöltését a PPU számára. Felszabadítja az előző egyéni palettát (ha volt), és beállítja az új mutatót.

---

```c
void RemoveCartNES(NES* nes)
```

Ha van behelyezett kazetta, felszabadítja annak memóriáját (``FreeCart``) és a ``cartInserted`` változót hamisra állítja.

---

```c
void DestroyNES(NES* nes)
```

Az emulátor leállításakor hívandó. Felszabadítja az összes alrendszert (CPU, PPU, Controller), végül felszabadítja magát a NES struktúrát.

---

```c
bool SetCartNES(NES* nes, const char* path)
```

Megpróbál betölteni egy iNES ROM fájlt a megadott elérési útról.
Ha sikeres a betöltés (``InsertCart`` visszatérési értéke), a kazetta pointerét átadja a CPU-nak és a PPU-nak is, hogy azok elérjék a ROM-ot. Beállítja a ``cartInserted``-et igazra.
Igazzal tér vissza, ha a betöltés sikerességét. 

---

```c
void ResetNES(NES *nes)
```

A konzol RESET gombjának megnyomását emulálja.<br>
Csak behelyezett kazettával fut le.<br>
Reset vektorra ugrás: A CPU program számlálóját a 0xFFFC és 0xFFFD memóriacímeken található 16 bites értékre állítja.

---

```c
void TickNES(NES *nes)
```

Ez a függvény felelős egy teljes képkocka (frame) lefuttatásáért. Addig futtatja a ciklust, amíg a PPU ``endOfFrame``-el nem jelez.<br>
**Szinkronizáció**: Az PPU háromszor gyorsabban, mint a CPU.
1. Futtat egy CPU instrukciót (TickCPU).
2. Megnézi, hány órajelbe telt az utasítás.
3. Annak háromszorosát futtatja le a PPU-n (TickPPU ciklusban).

A frame végén meghívja a ``DrawSprites`` függvényt (egyszerűsített sprite renderelés).
Meghívja a ``DrawBackgroundColor`` függvényt a háttérszín kitöltéséhez.

## Cart.h

```c
typedef struct Cart {
	// fejléc adatok
	unsigned int PRG_size;
	unsigned int CHR_size;
	int nametableArrangement; 

	// ROM
	uint8_t *PRG;
	uint8_t *CHR;

	// mapper emuláció
	int internalMapperNum;
} Cart;
```

``PRG_size``<br>
A programkód (PRG ROM) mérete bájtokban.

``CHR_size``<br>
A grafikai adatok (CHR ROM) mérete bájtokban. Ha ez 0, akkor a kazetta CHR-RAM-ot használ.

``nametableArrangement``<br>
A nametable tükrözés (mirroring) típusa, amelyet az iNES fejlécből olvasunk ki:
+ 0: Vertikális tükrözés
+ 1: Horizontális tükrözés

``PRG``, ``CHR``<br>
Mutatók a dinamikusan lefoglalt memóriaterületekre, ahová a fájlból betöltöttük a ROM tartalmát.

``internalMapperNum``<br>
A kazettához tartozó Mapper (memóriavezérlő) belső emulátor szerinti azonosítója.

## Cart.c

```c
Cart* InsertCart(const char* path)
```

Megnyitja a megadott útvonalon található .nes fájlt, és létrehoz belőle egy Cart struktúrát.<br>
Működése:
1. Fejléc olvasása: Beolvassa a fájl első 16 bájtját (iNES fejléc).
2. Méret számítás:
    + A PRG méretet 16 KB-os egységekben tárolja a fejléc 4. bájtja (``iNES[4] * 0x4000``).
    + A CHR méretet 8 KB-os egységekben tárolja a fejléc 5. bájtja (``iNES[5] * 0x2000``).
3.  Tükrözés: A 6. bájt legalsó bitje határozza meg a tükrözést (``iNES[6] & 0b1``).
4. Mapper detektálás: A 6. és 7. bájtok felső 4 bitjének összefűzésével kapjuk meg a Mapper ID-t. Ha az emulátor nem támogatja az adott mappert (``UNKNOWN_MAPPER``), a betöltés meghiúsul.
5. Adatok betöltése:
    + Lefoglalja a memóriát a PRG ROM-nak és beolvassa őket.
    + Lefoglalja a memóriát a CHR ROM-nak (ha ``CHR_size`` nem 0) és beolvassa őket. Ha 0, a mutató ``NULL`` marad.

Visszatérési értéke a létrehozott Cart struktúra, vagy hiba esetén ``NULL``.

---

```c
void FreeCart(Cart* cart)
```

Felszabadítja a kazettához tartozó erőforrásokat:<br>
A PRG ROM számára foglalt területet.<br>
A CHR ROM számára foglalt területet.<br>
Magát a Cart struktúrát.

## Palette.c

```c
uint8_t default_palette[] = {
    0x62, 0x62, 0x62, 0x00, ...
}
```

Egy statikus tömb, amely a *2C02G* típusú PPU chip alapértelmezett színeit tartalmazza RGB24 formátumban (pixelenként 3 bájt: R, G, B). A tömb mérete 192 bájt (64 lehetséges NES szín × 3 bájt színkomponens).

---

```c
uint8_t* GetDefPalette()
```

Visszaadja az alapértelmezett ``default_palette`` tömb mutatóját.

---

```c
uint8_t* MallocPalette(const char* path)
```

Egy külső, bináris palettafájl (.pal) betöltését végzi el.
+ Megnyitja a fájlt bináris olvasásra.
+ Beolvas 192 bájtot (64 színt) egy átmeneti bufferbe.
+ Ha az olvasás sikeres, dinamikusan foglal (``malloc``) egy 192 bájtos területet a memóriában, és átmásolja oda az adatokat.
+ Visszatérési értéke az új paletta mutatója, vagy hiba esetén ``NULL``.

---

```c
void FreeCustomPalette(PPU* ppu)
```

Felszabadítja az egyénileg betöltött paletta memóriaterületét.
+ Ellenőrzi a PPU ``using_default_palette`` flagjét.
+ Ha a flag hamis (tehát egyéni palettát használunk), meghívja a ``free`` függvényt a ppu->palette mutatón.

## Mapper.h

```c
enum {
	UNKNOWN_MAPPER = -1,
	NROM,
	MMC3,
	...
};
```

Felsorolja a támogatott mappereket. 
```c
typedef int Mapper;
```

## Mapper.c

```c
typedef struct MapperAndNumber
{
	Mapper mapper;
	uint8_t mapperNumber;
} MapperAndNumber;
const MapperAndNumber mappers[] = {
	{NROM, 0},
	{MMC3, 4},
	{MMC1, 1}
};
```

Hozzárendeli az internális mapper számot az iNES specifikációban lévő mapper számokhoz.

---

```c
typedef struct MapperFunctions {
	Mapper mapper;
	uint8_t(*ReadCpuByte)(CPU* cpu, uint16_t);
	void   (*WriteCpuByte)(CPU* cpu, uint16_t, uint8_t);
	uint8_t(*ReadChrByte)(PPU* ppu, uint16_t);
} MapperFunctions;
const MapperFunctions mapperFns[] = {
	{NROM, NROM_Read, NROM_Write, NROM_CHR},
	{MMC3, MMC3_Read, MMC3_Write, MMC3_CHR},
	...
};
```

Ez a struktúra fogja össze egy adott mapperhez tartozó implementációs függvényeket. A ``mapperFns`` tömb tartalmazza a konkrét hozzárendeléseket (pl. NROM -> ``NROM_Read``, ``NROM_Write``, ``NROM_CHR``). A tömb sorrendje az Mapper.h-ban lévő enumeráció sorrendjét kell, hogy kövesse.  

---

```c
Mapper GetMapper(uint8_t mapperNumber, int* internalMapperNum)
```

A kapott iNES mapper szám alapján megkeresi a belső Mapper enum értékét.<br>
Beállítja az ``internalMapperNum`` mutatót a ``mapperFns`` tömb indexére, hogy a későbbi hívások gyorsan elérjék a függvényeket.
Ha a mapper nem támogatott, ``UNKNOWN_MAPPER`` értékkel tér vissza.

---

```c
uint8_t ReadCpuMemViaMapper(CPU* cpu, uint16_t address)
```

A CPU memóriatérképének felső felét (kazetta terület) kezeli.
Ha a cím kisebb, mint 0x8000, debug értéket (0xcd) ad vissza (hibás logikát jelezhet). Egyébként meghívja az adott mapper ``(*ReadCpuByte)(CPU* cpu, uint16_t)`` függvényét a ``mapperFns`` tömbből, és annak ``uint8_t`` értékével tér vissza.

---

```c
uint8_t ReadChrMemViaMapper(PPU* ppu, uint16_t address)
```

A PPU grafikus memóriaolvasását kezeli.<br>
CHR-RAM kezelés: Ha a kazetta ``CHR_size`` értéke 0, akkor a PPU belső memóriáját (CHR-RAM) olvassa közvetlenül, mapper hívás nélkül.<br>
Egyébként (CHR-ROM esetén) meghívja az adott mapper ``(*ReadChrByte)(PPU* ppu, uint16_t)`` függvényét, és annak ``uint8_t`` értékével tér vissza.

---

```c
void WriteCpuMemViaMapper(CPU* cpu, uint16_t address, uint8_t value)
```

A CPU írási műveleteit továbbítja a kazettára. Mivel a ROM írásvédett, ezeket az írásokat a mapperek általában konfigurációra (pl. bank switch) használják. Meghívja az adott mapper ``(*WriteCpuByte)(CPU* cpu, uint16_t, uint8_t)`` függvényét.

---

## Generic.c

Általános segédfüggvények a bankváltások kezeléséhez. Mivel a mapperek logikája gyakran hasonló (memóriabankok cserélgetése), a közös funkciók ebbe a fájlba kerültek.

---

```c
uint8_t ReadDataInBank(uint8_t* memory, int memorySize, uint16_t address, uint16_t bankSize, int bankNumber)
```

Kiszámolja a fizikai címet a ROM tömbben, és az ott lévő értékkel tér vissza.<br>
Negatív bank indexelés:  A -1 az utolsó bankot, a -2 az utolsó előttit jelenti. Ez hasznos az olyan mappereknél, ahol az utolsó bank fixen be van drótozva a CPU memória végére.<br>
Túlcsordulás kezelés: A ``bankNumber``-t a ROM mérete alapján modulózza, hogy elkerülje a memórián kívüli olvasást.


---

```c
uint8_t ReadPrgInBank(CPU* cpu, uint16_t address, uint16_t bankSize, int bankNumber)
```

Meghívja a ``ReadDataInBank`` függvényt egy CPU-s olvasásra,
és azzal tér vissza.

---

```c
uint8_t ReadChrInBank(PPU* ppu, uint16_t address, uint16_t bankSize, int bankNumber)
```

Meghívja a ``ReadDataInBank`` függvényt egy PPU-s olvasásra,
és azzal tér vissza.

## NROM.c

A legegyszerűbb, mapper chip nélküli kazetta implementációja.

Olvasás: A CPU címteréből (0x8000-0xFFFF) kivonja a 0x8000 eltolást. Ha a PRG ROM mérete csak 16 KB, akkor a címet maszkolja (& 0x3fff), így a memória tükröződik a 0xC000-0xFFFF tartományban is.

Írás: Mivel az NROM-nak nincsenek regiszterei, az írás művelet nem csinál semmit.

## MMC1.c (Mapper 1)

Az első elterjedt ASIC mapper, amely soros adatátvitelt használ a konfiguráláshoz.

Shift Regiszter Mechanizmus: A CPU adatbusza 8 bites, de az MMC1 kevés lábbal rendelkezett, ezért az adatokat bitenként kell elküldeni neki.<br>
Minden írásnál az adat legalsó bitje bekerül a belső ``shiftReg``-be.<br>
Az 5. írás után a ``shiftReg`` tartalma átmásolódik a célregiszterbe, amelyet a cím határoz meg ("Control", CHR bankok, PRG bank).
Ha az írt érték MSB-je 1, akkor a ``shiftReg`` alaphelyzetbe állítódik.

PRG: Támogatja a 32 KB-os módot, illetve a "fix első" vagy "fix utolsó" bank módot 16 KB-os lapozással.<br>
CHR: Képes bankolni 8 KB-os vagy 4 KB-os módban.

Részletesebb specifikáció: https://www.nesdev.org/wiki/MMC1

## MMC3.c (Mapper 4)

Az egyik legnépszerűbb mapper.

Regiszterek elérése: Az MMC3 írási logikája a cím páros vagy páratlan voltán alapul:

Páros cím (pl. 0x8000): A parancsregiszter írása. Kiválasztja, hogy melyik belső regisztert akarjuk módosítani a következő írással, valamint beállítja a bankok elrendezésének módját.

Páratlan cím (pl. 0x8001): Adat írása a kiválasztott regiszterbe (a ``R`` tömb egyik elemébe).

PRG-ROM: A ``prgRomBankMode`` változó határozza meg, hogy a 0x8000~0x9FFF-es vagy a 0xC000~0xDFFF-es cím fix, és melyik cserélhető. A 0xE000-es tartomány mindig az utolsó bankra mutat. A 0xA000~0xBFFF tartomány minden módban bankolható.

CHR-ROM: A ``chrA12inversion`` bit megcseréli a 2x2KB és a 4x1KB bankok helyét a memóriában.

Részletesebb specifikáció: https://www.nesdev.org/wiki/MMC3

## Controller.h 

```c
typedef struct Controller {
	int a;
	int b;
	int select;
	int start;
	int up;
	int down;
	int left;
	int right;
	bool strobeBit;
	int nextBitToRead;
} Controller;
```

``a``, ``b``, ``select``, ``start``, ``up``, ``down``, ``left``, ``right``<br>
Ezek a változók tárolják, hogy az adott gomb le van-e nyomva (1 vagy 0).

``strobeBit``<br>
A vezérlő "kapuzó" jele. Ha ez aktív (1), a belső állapot folyamatosan az első gomb (A) értékére áll vissza.

``nextBitToRead``<br>
Egy belső számláló, amely nyilvántartja, hogy a soros olvasás során melyik gomb állapotát kell következőnek visszaadni a CPU-nak.

```c
#define CONTROLLER_BIT_OF_A		 0
#define CONTROLLER_BIT_OF_B		 1
#define CONTROLLER_BIT_OF_SELECT 2
#define CONTROLLER_BIT_OF_START	 3
#define CONTROLLER_BIT_OF_UP	 4
#define CONTROLLER_BIT_OF_DOWN	 5
#define CONTROLLER_BIT_OF_LEFT	 6
#define CONTROLLER_BIT_OF_RIGHT	 7
```

A header meghatározza a kontroller olvasás sorrendjét.<br>
A -> B -> Select -> Start -> Fel -> Le -> Bal -> Jobb

## Controller.c

```c
Controller* CreateController()
```

Lefoglalja a memóriát a ``Controller`` struktúrának, és nullázza azt.

---

```c
void FreeController(Controller* controller)
```

Felszabadítja a ``controller`` struktúrát.

---

```c
void WritingToControllerReg(Controller* controller, uint8_t value)
```

A CPU írási műveletét kezeli a 0x4016-os címen. <br>
Ez a függvény állítja be a ``strobeBit`` értékét a bemenő adat legalsó bitje alapján.<br>

---

```c
uint8_t ReadingFromControllerReg(Controller* controller)
```

A CPU olvasási műveletét kezeli a 0x4016-os címen.<br>
A ``nextBitToRead`` számláló alapján visszaadja az aktuális gomb állapotát.<br>
Olvasás után növeli ezt a számlálót, így a következő olvasás a következő gomb állapotát adja vissza (soros kommunikáció).<br>
Meghívja a ``CheckStrobe`` függvényt.

---

```c
void CheckStrobe(Controller* controller)
```

Ha a ``strobeBit`` aktív, akkor a ``nextBitToRead`` nullázódik.
