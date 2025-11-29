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

Egy adott CPU címre ír egy értéket. Az address értéke bármi lehet. (0x0000-0xFFFF)

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

---

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

