#include <stdio.h>
#include <string.h>
#include <stdbool.h> 


int main(int argc, char *argv[]) {

    char *DbugMode = "on";
    char *encryptionKey = NULL;
    int AddOrSub = 0; // 1 = add , -1 = sub
    FILE *infile = stdin;
    FILE *outfile = stdout;

    for (int i = 1; i < argc; i++) {

        // argv[i] == "-D" compares addresses and not values. thats why i used strcmp
        if (strcmp(argv[i], "-D") == 0) {
            DbugMode = "off";
        } else if (strcmp(argv[i], "+D") == 0) {
            DbugMode = "on";
        } else {
            // Check if we have encryption key & save the key (as char array)
            if (strncmp(argv[i], "+e", 2) == 0 || strncmp(argv[i], "-e", 2) == 0) { 
                AddOrSub = (strncmp(argv[i], "+e", 2) == 0)? 1 : -1; 
                encryptionKey = argv[i] + 2;
            } else
            // Check if there is a specific input file
            if (strncmp(argv[i], "-I", 2) == 0) {
                // "r" stands for "reading"
                infile = fopen(argv[i] + 2, "r");
                if (infile == NULL) {
                    fprintf(stderr, "Error: Cannot open input file '%s'\n", argv[i] + 2);
                    return 1;
                }
            } else
            // Check if there is a specific output file
            if (strncmp(argv[i], "-O", 2) == 0) {
                // "w" stands for "writing"
                outfile = fopen(argv[i] + 2, "w");
                if (outfile == NULL) {
                    fprintf(stderr, "Error: Cannot open output file '%s'\n", argv[i] + 2);
                    return 1;
                }
            } else if (strcmp(DbugMode, "on") == 0) {
                // %s\n ---> %s means argument is a string. \n tells the printer to add new line after printing the argument.
                fprintf(stderr, "%s\n", argv[i]);
            }
        }
    }


    int ch;
    int index_InEncryptionKey = 0;
    bool lowerCase = false;
    bool digit = false;

    // fgetc : reads a single character from the specified input stream.
    // It returns the character read as an unsigned char cast to an int, 
    // or EOF if an error occurs or if the end of the file is reached.
    while ((ch = fgetc(infile)) != EOF) {
        lowerCase = (ch >= 'a' && ch <= 'z')? true: false; // Single if with two condition
        digit = (ch >= '0' && ch <= '9')? true: false; // Single if with two condition
        // if (strcmp(DbugMode, "on") == 0) {
        if (AddOrSub != 0) {
            if (ch != '\n' && (lowerCase | digit)) {
                // Convert char to int :
                int digit_InEncryptionKey = encryptionKey[index_InEncryptionKey] - '0'; 
                if (strcmp(DbugMode, "on") == 0) {
                    fprintf(stderr, "Debug: Input character '%c', Encryption key '%d'\n", ch, digit_InEncryptionKey); // Optional part
                }
                if (AddOrSub == 1) {
                    // Encode
                    ch += digit_InEncryptionKey;
                } else {
                    // Decode
                    ch -= digit_InEncryptionKey;
                }
                // Cases where z+1=a, a-1=z, etc.
                if (lowerCase){
                    if (ch > 'z')
                        ch -= 26;
                    else if (ch < 'a')
                        ch += 26;
                }
                if (digit){
                    if (ch > '9')
                        ch -= 10;
                    else if (ch < '0')
                        ch += 10;
                }

                
            }
            // Move :
            index_InEncryptionKey = (index_InEncryptionKey + 1) % strlen(encryptionKey); // >=0 because (index_InEncryptionKey + 1)>=0

            // }

            // fputc : writes a single character to the specified output stream.
            // It takes an int argument representing the character to write and returns the 
            // character written as an unsigned char cast to an int, or EOF if an error occurs
            fputc(ch, outfile);
        }
    }

    // fclose :  closes the file associated with the specified stream.
    // It flushes any buffered output and releases any resources associated with the file.
    // After closing a file, further operations on the stream (e.g., reading or writing) are not allowed.
    fclose(outfile);
    fclose(infile);


    return 0;
}