#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>
#include <string.h>

#define PAGE_SIZE 0x1000

extern int startup(int argc, char **argv, void (*start)());

// Function to print the program header details along with mmap flags
void print_phdr(Elf32_Phdr *phdr, int index) {
    const char *type;
    switch (phdr->p_type) {
        case PT_NULL:    type = "NULL"; break;
        case PT_LOAD:    type = "LOAD"; break;
        case PT_DYNAMIC: type = "DYNAMIC"; break;
        case PT_INTERP:  type = "INTERP"; break;
        case PT_NOTE:    type = "NOTE"; break;
        case PT_SHLIB:   type = "SHLIB"; break;
        case PT_PHDR:    type = "PHDR"; break;
        default:         type = "UNKNOWN"; break;
    }

    // Determine protection flags
    int prot = 0;
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;

    // Determine mapping flags
    //#define MAP_ANONYMOUS 0x20
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;

    printf("%-8s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c 0x%x 0x%x 0x%x\n",
           type,
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           (phdr->p_flags & PF_R) ? 'R' : ' ',
           (phdr->p_flags & PF_W) ? 'W' : ' ',
           (phdr->p_flags & PF_X) ? 'E' : ' ',
           phdr->p_align,
           prot,
           flags);
}

// Function to iterate over program headers (from Task 0)
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start; // ELF header
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff); // Program header start
    int phnum = ehdr->e_phnum; // Number of program headers

    for (int i = 0; i < phnum; i++) {
        func(&phdr[i], i);
    }

    return 0;
}

void load_phdr(Elf32_Phdr *phdr, int fd) {

    if (phdr->p_type != PT_LOAD) {
        return;
    }

    // Determine protection flags
    int prot = 0;
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;

    // Align the virtual address and offset to PAGE_SIZE
    void *vaddr = (void *)(phdr->p_vaddr & ~(PAGE_SIZE - 1));
    off_t offset = phdr->p_offset & ~(PAGE_SIZE - 1);
    size_t padding = phdr->p_vaddr & (PAGE_SIZE - 1);
    size_t map_size = phdr->p_memsz + padding;

    // Map the segment into memory
    void *addr = mmap(vaddr, map_size, prot, MAP_PRIVATE | MAP_FIXED, fd, offset);
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Print the program header information
    print_phdr(phdr, 0);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf-executable>\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    Elf32_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
        perror("read");
        close(fd);
        return 1;
    }
    lseek(fd, 0, SEEK_SET);
    size_t map_size = ehdr.e_phoff + (ehdr.e_phentsize * ehdr.e_phnum);
    void *map_start = mmap(NULL, map_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
    printf("Type     Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align  Prt Mflag\n");
    printf("---------------------------------------------------------------------------\n");
    for (int i = 0; i < ehdr.e_phnum; i++) {
        Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr.e_phoff + i * sizeof(Elf32_Phdr));
        load_phdr(phdr, fd);
    }
    // Call startup function
    startup(argc - 1, argv + 1, (void (*)())ehdr.e_entry);
    munmap(map_start, ehdr.e_phentsize * ehdr.e_phnum);

    close(fd);
    return 0;
}

