# # Compiler
# CC = gcc 

# # Flags
# CFlags = -Wall -Wextra -g

# # The .c files (sources)
# CFile = encoder.c

# # Sets the names of the .o files
# OFile = $(CFile:.c=.o)

# # Set the executable names
# Ex = encoder

# # Declare as phony target, so the computer will differ them from files with the same name
# .PHONY: all clean

# all: $(Ex)

# $(Ex): $(OFile)
# 	$(CC) $(CFlags) -o $@ $^

# %.o: %.c
# 	$(CC) $(CFlags) -c -o $@ $<

# clean:
# 	rm -f $(OFile) $(Ex)

# setSpace :
# 	chmod 700 -R ~

# space : 
# 	du -a | sort -n

all: loader

loader: task2.c
	gcc -m32 -c task2.c -o task2.o
	ld  -o loader task2.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

# .PHONY: clean
# clean:
# 	rm -rf ./*.o loader


# CC = gcc
# LD = ld

# CFLAGS = -Wall -Wextra -m32
# LDFLAGS = -m elf_i386 -o lab5 task1.o startup.o start.o -T linking_script --dynamic-linker /lib/ld-linux.so.2 -lc

# TARGET = lab5

# SRCS = task2.c
# OBJS = $(SRCS:.c=.o)

# all: $(TARGET) loader

# loader: lab5 start.o
# 	gcc -m32 -c task2.c -o task2.o
# 	ld  -o loader task2.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

# $(TARGET): $(OBJS)
# 	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# start.o: start.s
# 	nasm -f elf -o start.o start.s

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJS) $(TARGET)
# 	rm -rf ./*.o loader

# .PHONY: all clean
