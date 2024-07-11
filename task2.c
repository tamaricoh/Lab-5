#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

#define BUF_SZ 4096

void print_phdr_info(Elf32_Phdr *phdr) {
    printf("Type: 0x%x\n", phdr->p_type);
    printf("Offset: 0x%x\n", phdr->p_offset);
    printf("Virtual Address: 0x%x\n", phdr->p_vaddr);
    printf("Physical Address: 0x%x\n", phdr->p_paddr);
    printf("File Size: 0x%x\n", phdr->p_filesz);
    printf("Memory Size: 0x%x\n", phdr->p_memsz);
    printf("Flags: 0x%x\n", phdr->p_flags);
    printf("Alignment: 0x%x\n", phdr->p_align);
    printf("\n");
}

void load_phdr(Elf32_Phdr *phdr, int fd, void *map_start) {
    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    
    for (int i = 0; i < header->e_phnum; i++) {
        Elf32_Phdr *current_phdr = &phdr[i];
        
        if (current_phdr->p_type == PT_LOAD) {
            printf("Loading program header:\n");
            print_phdr_info(current_phdr);
            
            int prot_flags = 0;
            if (current_phdr->p_flags & PF_R)
                prot_flags |= PROT_READ;
            if (current_phdr->p_flags & PF_W)
                prot_flags |= PROT_WRITE;
            if (current_phdr->p_flags & PF_X)
                prot_flags |= PROT_EXEC;
            
            void *addr = (void *)(uintptr_t)current_phdr->p_vaddr;
            off_t offset = current_phdr->p_offset;
            size_t filesz = current_phdr->p_filesz;
            
            // Map the segment into memory
            void *mapped_addr = mmap(addr, filesz, prot_flags, MAP_PRIVATE | MAP_FIXED, fd, offset);
            
            if (mapped_addr == MAP_FAILED) {
                perror("mmap failed");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int startup(int argc, char **argv, void (*start)());

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <program>\n", argv[0]);
        return 1;
    }

    int elf_fd = open(argv[1], O_RDONLY);
    if (elf_fd == -1)
    {
        perror("Error opening file");
        return 1;
    }

    void *map_start = mmap(NULL, BUF_SZ, PROT_READ | PROT_WRITE, MAP_PRIVATE, elf_fd, 0);
    if (map_start == MAP_FAILED)
    {
        perror("Error mapping file");
        close(elf_fd);
        return 1;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr_table = (Elf32_Phdr *)((char *)map_start + header->e_phoff);

    load_phdr(phdr_table, elf_fd, map_start);
    startup(argc - 1, argv + 1, (void *)header->e_entry);
    munmap(map_start, BUF_SZ);
    close(elf_fd);

    return 0;
}
