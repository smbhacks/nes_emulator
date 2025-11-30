# NES Emulátor felhasználói dokumentáció

Az emulátor megnyitása után láthatunk egy fekete ablakot és egy menüsort.

# Menüsor

A menüsorban a következő opciók érhetőek el: File, Video.


## File

### Open iNES file..

Megnyit egy iNES fájlt (``.nes`` fájlkiterjesztés). 
Ha nem támogatott iNES fájlt nyit meg, akkor egy hibaüzenetet ír ki.

### Close ROM

Bezárja az iNES fájlt.

### Reset

Emulálja a konzolon a RESET gomb megnyomását. Általában újraindítja a játékot.

### Exit

Kilép az emulátorból.

## Video

### Load .pal file

Az alapértelmezett NES palettát ki lehet cserélni ezzel a menügomb segítségével. Be lehet tölteni egy ``.pal`` fájlt, amely megadja az NES 00~3F indexű színeit RGB24 formátumban (Szóval a fájlméretnek 64 * 3 = 192 bájtnak kell lennie).

### Use default palette

Az alapértelmezett NES palettára vált.

# Irányítás

Ha sikerült elindítani egy kazettát, akkor azt a következő gombokkal
lehet irányítani:

| Billentyűzet | NES kontroller megfelelője |
| ------------ | -------------------------- |
| Bal irány    | Bal                        |
| Jobb irány   | Jobb                       |
| Fel irány    | Fel                        | 
| Le irány     | Le                         |
| X            | B                          |
| C            | A                          |
| Szóköz       | Select                     |
| Enter        | Start                      |

