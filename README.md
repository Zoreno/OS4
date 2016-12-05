# OS4

X86 Operativsystem skrivet av mig. För närvarande endast stöd för VirtualBox och utveckling i linux.

# Nödvänding programvara

- NASM
- GCC
- Virtualbox

# Instruktioner

1. Kör make i root-directory.
2. Kör ```bash bash build_harddrive.sh```
3. Skapa en ny virtuell maskin i VBox.
4. Gå in på inställningar för maskinen och gå till tabben system.
5. Ändra boot-order till optical->hard disk->floppy.
6. Gå till tabben storage.
7. Ta bort alla existerande controllers.
8. Lägg till en SATA-controller och en optisk disk med avbildningen os.iso.
9. Lägg till en IDE-controller med en hårddisk med avbildningen hard_drive.vdi(skapad i steg 2)
10. Gå till tabben serial och aktivera COM1. Port mode ska vara "raw file". Path kan vara till exempel "com.out"
11. Starta upp virtuella maskinen. Undvik att trycka på tangentbordet innan kommandoprompten visas. (Detta ska åtgärdas :) )
12. OS4 ska nu kunna köras. (Det har dock inte så mycket funktionalitet än)

# TODO

Funderingar på saker man kan göra i framtiden. (Ingen hänsyn till ordning)

## ACPI

Främst för shutdown men skulle vara roligt att få till lite mer avancerade
funktioner. Dock verkar detta vara ett gigantiskt område.

- [x] Shutdown
- [ ] Power management

## PCI

- [x] Hämta komponentlista
- [ ] Dynamisk tilldelning av drivrutiner

## Real time clock

Hämta tid och sånt från CMOS.

- [x] Hämta tid från CMOS
- [ ] Utveckla time-framework(hänger samman med kernel runtime library/time)

## Floppy driver

För att kunna läsa och skriva data till floppy-disken.

- [x] DMA

- [x] Läsa
- [x] Skriva

## File system

Få till läsning/skrivning av filer. Finns tid över kan
man alltid kolla på NTFS, som är vanligt för HDDs.

- [ ] File system framework
- [ ] FAT32
- [ ] NTFS

## Utveckling av CLI

Möjlighet att registrera kommandon med en funktionspekare. Utveckla stöd för
inargument. Kanske hjälpsidor för kommandon. Detta dock under förutsättning
att text-mode graphics används

- [ ] Funktionspekare
- [ ] argc och argv (i likhet med executable)
- [ ] Working directory
- [ ] Hjälpsidor

## Minneshantering

Implementera kernel_malloc och kernel_free för användning i kernel. Görs med
någon form av heap eller SLAB allocator (eller liknande).

- [x] Physical memory manager
- [x] Virtual memory manager
- [x] Kernel memory allocator
- [ ] User space memory allocator.

## Kernel runtime library

Några exempel:

### Containers

- [ ] Linked List
- [ ] Double linked list
- [ ] Queue
- [ ] Stack
- [ ] String
- [ ] Map
- [ ] Priority Queue
- [ ] Sets

### Misc

- [ ] Algorithms
- [ ] Time
- [ ] Functions
- [ ] Random number generation

### Input och output

- [ ] Standard in och out
- [ ] Filhantering
- [ ] Serialization

### Multithreading

- [ ] Threads
- [ ] Mutex

### Matematik

- [ ] Komplexa tal
- [ ] Vektorer
- [ ] Floats

### Security

- [ ] Hashing

## Kommentera filer

Gärna i doxygen-format. Kan underlätta en del för att förstå koden.

## Köra exe-filer

Kunna läsa in PE-filer och kanske ELF-filer och köra dessa.

- [ ] PE parser
- [ ] ELF parser

## Multitasking

Kunna köra flera trådar/processer samtidigt med stöd för 
schemaläggningsalgoritmer och kommunikation mellan processer. Semaphores/mutex.

### Nya Processer

- [ ] Kunna skapa en ny process
- [ ] Eget address space
- [ ] Egen Heap
- [ ] Egen Stack

### Inter-process Communication

- [ ] Pipes mellan processer
- [ ] Message passing
- [ ] Shared memory

### Concurremcy

- [ ] Mutex
- [ ] Spinlock
- [ ] Semaphores

### Scheduling

- [ ] Task Switching

- [ ] Round Robin
- [ ] First Come First Served
- [ ] Multilevel Feedback Queue


## Userspace

Köra program i userspace. System calls.

## User input

Sätt för användaren att kommunicera med datorn.

- [x] Keyboard
- [ ] Stöd för svenskt tangentbord
- [ ] Mus

## Grafik

Bättre upplösning. Rita pixlar istället för tecken.
Användargränssnitt.

- [x] Text mode driver
- [ ] Pixel mode driver
- [x] Sätt att växla mellan modes
- [ ] Fonts

