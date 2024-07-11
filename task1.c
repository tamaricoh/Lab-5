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

void print_info(Elf32_Phdr *p_header, int arg) {
    (void)arg;

    // Determine segment type
    const char *type_str = "UNKNOWN";
    switch (p_header->p_type) {
        case PT_NULL:     type_str = "NULL";     break;
        case PT_LOAD:     type_str = "LOAD";     break;
        case PT_DYNAMIC:  type_str = "DYNAMIC";  break;
        case PT_INTERP:   type_str = "INTERP";   break;
        case PT_NOTE:     type_str = "NOTE";     break;
        case PT_SHLIB:    type_str = "SHLIB";    break;
        case PT_PHDR:     type_str = "PHDR";     break;
        case PT_TLS:      type_str = "TLS";      break;
    }

    // Calculate protection flags
    int prot = 0;
    if (p_header->p_flags & PF_R)
        prot |= PROT_READ;
    if (p_header->p_flags & PF_W)
        prot |= PROT_WRITE;
    if (p_header->p_flags & PF_X)
        prot |= PROT_EXEC;

    // Calculate mapping flags
    int flags = MAP_PRIVATE;

    // Print formatted output
    printf("%-8s  %#08x  %#08x  %#08x  %#08x  %#08x  %c%c%c   %#x   ",
           type_str,
           p_header->p_offset,
           p_header->p_vaddr,
           p_header->p_paddr,
           p_header->p_filesz,
           p_header->p_memsz,
           (p_header->p_flags & PF_R) ? 'R' : ' ',
           (p_header->p_flags & PF_W) ? 'W' : ' ',
           (p_header->p_flags & PF_X) ? 'E' : ' ',
           p_header->p_align);

    // Print protection flags and mapping flags only for PT_LOAD segments
    if (p_header->p_type == PT_LOAD) {
        printf("%#x   ", prot);
        printf("%#x\n", flags);
    } else {
        printf("N/A   N/A\n");
    }
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
    foreach_phdr(map_start, print_info, 0);
    munmap(map_start, BUF_SZ);
    close(elf_file);
    return 0;
}