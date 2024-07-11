#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

#define BUF_SZ 4096

void print_phdr(Elf32_Phdr *phdr, int i) {
    printf("Program header number %d at address %p\n", i, (void *)phdr);
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    (void)arg;
    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;

    // Ensure the mapped file is a valid ELF file
    if (header->e_ident[EI_MAG0] != ELFMAG0 || 
        header->e_ident[EI_MAG1] != ELFMAG1 || 
        header->e_ident[EI_MAG2] != ELFMAG2 || 
        header->e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "Not a valid ELF file\n");
        return -1;
    }

    Elf32_Phdr *first_phdr = (Elf32_Phdr *)(map_start + header->e_phoff);
    for (size_t i = 0; i < header->e_phnum; i++) {
        Elf32_Phdr *current_phdr = first_phdr + i;
        func(current_phdr, i);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc!= 2)
    {
        printf("must have an argument\n");
        return 1;
    }
    int elf_file = open(argv[1], O_RDWR);
    if (elf_file == -1)
    {
        printf("could not open file\n");
        return 1;
    }
    printf("Type \t\tOffset   VirtAddr PhysAddr FileSiz  MemSiz      Flg\tAlign\n");
    void* map_start = mmap(NULL, BUF_SZ, PROT_READ | PROT_WRITE, MAP_PRIVATE, elf_file, 0);
    foreach_phdr(map_start, print_phdr, 0);
    munmap(map_start, BUF_SZ);
    close(elf_file);
    return 0;
    
    
}