/**
 * mips.c - Source code file for a MIPS-lite simulation
 *
 * @authors:    Evan Brown (evbr2@pdx.edu)
 * 				Louis-David Gendron-Herndon (loge2@pdx.edu)
 *				Ameer Melli (amelli@pdx.edu)
 *				Anthony Le (anthle@pdx.edu)
 *
 *
 * @date:       April 17, 2025
 * @version:    0.1
 *
 *
 * MODES:		
 *				NORMAL:		Print current cache state on 9's,
 *							and usage statistics at the end.
 *
 *				VERBOSE:	Print everything from NORMAL mode,
 *							plus details on L2-involved operations.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "mips.h"


int mode;


int main(int argc, char *argv[]) {
	FILE *file;
	char line[LINE_BUFFER_SIZE];
    char binary_string[ADDRESS_BITS + 1];
	int line_number = 0;
	
    // Check for at least two arguments: mode and filename
    if (argc < 3) {
        printf("Usage: %s <MODE> <TRACE_FILE>\n", argv[0]);
        return EXIT_FAILURE;
    }
	
	// Set mode specified in the first argument
	if (strcmp(argv[1], "DEBUG") == 0)
		mode = DEBUG;
	else if (strcmp(argv[1], "NORMAL") == 0)
		mode = NORMAL;
	else {
		printf("\nInvalid mode. Defaulting to NORMAL.\n");
		mode = NORMAL;
	}
	
	// Open the trace file specified in the second argument
    file = fopen(argv[2], "r");
    if (file == NULL) {
        perror("Error opening trace file");
        return EXIT_FAILURE;
    }
	
    
    
    // Read each line of the trace file
    while (fgets(line, sizeof(line), file)) {
		line_number++;
		
        // Remove newlines
        line[strcspn(line, "\r\n")] = '\0';

        // Skip empty lines
        if (strlen(line) == 0)
            continue;
		
		// Validate line length
        if (strlen(line) != HEX_STRING_LENGTH) {
            if (mode == DEBUG)
                printf("Error: Invalid instruction length at line %d (%zu characters). Exiting.\n", line_number, strlen(line));
			
			fclose(file);
            return EXIT_FAILURE; // End the program if incorrect length
        }

        // Convert to binary string and begin binary manip
		hex_to_binary_string(line, binary_string);
        opcode_master(binary_string);

        // DEBUG: print each binary string
        /*if (mode == DEBUG) {
            printf("Line %d\n", line_number);
            printf("Hex: %s\n", line);
            printf("Binary: %s\n\n", binary_string);
        }*/
		

        // CONTINUE
    }
	

    // Close the file
    fclose(file);
	
	return 0;
}



void hex_to_binary_string(const char *hex_string, char *binary_string) {
    unsigned int value;
    sscanf(hex_string, "%x", &value);

    for (int i = ADDRESS_BITS - 1; i >= 0; i--) {
        binary_string[ADDRESS_BITS - 1 - i] = (value & (1U << i)) ? '1' : '0';
    }
    binary_string[ADDRESS_BITS] = '\0'; // null-terminate
}




void opcode_master(const char *binary_string) {
    unsigned int opcode 	= 0;
	unsigned int rs 		= 0;
	unsigned int rt 		= 0;
	unsigned int rd 		= 0;
	unsigned int immediate 	= 0;

    // Build opcode by shifting in the first 6 bits of the string
    for (int i = 0; i < 6; i++) {
        opcode = (opcode << 1) | (binary_string[i] - '0');
    }

	// rs = bits 6..10
	for (int i = 6; i < 11; i++) {
		rs = (rs << 1) | (binary_string[i] - '0');
	}

	// rt = bits 11..15
	for (int i = 11; i < 16; i++) {
		rt = (rt << 1) | (binary_string[i] - '0');
	}

	// rd = bits 16..20
	for (int i = 16; i < 21; i++) {
		rd = (rd << 1) | (binary_string[i] - '0');
	}

	// immediate = bits 16..31
	for (int i = 16; i < 32; i++) {
		immediate = (immediate << 1) | (binary_string[i] - '0');
	}


    switch(opcode) {
		case ADD:
			printf("ADD\n");
			break;
			
		case ADDI:
			printf("ADDI\n");
			break;
			
		case SUB:
			printf("SUB\n");
			break;
			
		case SUBI:
			printf("SUBI\n");
			break;
			
		case MUL:
			printf("MUL\n");
			break;
			
		case MULI:
			printf("MULI\n");
			break;
			
		case OR:
			printf("OR\n");
			break;
			
		case ORI:
			printf("ORI\n");
			break;
			
		case AND:
			printf("AND\n");
			break;
			
		case ANDI:
			printf("ANDI\n");
			break;
			
		case XOR:
			printf("XOR\n");
			break;
			
		case XORI:
			printf("XORI\n");
			break;
			
		case LDW:
			printf("LDW\n");
			break;
			
		case STW:
			printf("STW\n");
			break;
			
		case BZ:
			printf("BZ\n");
			break;
			
		case BEQ:
			printf("BEQ\n");
			break;
			
		case JR:
			printf("JR\n");
			break;
			
		case HALT:
			printf("HALT: END PROGRAM\n"); // see below DEBUG block
			break;
			
		default:
			printf("Error: Unknown opcode 0x%02X. Exiting.\n", opcode);
			exit(EXIT_FAILURE);
	}
	
	
	if (mode == DEBUG) {
        
        printf("\nOpcode: ");
        for (int b = 5; b >= 0; b--) {
            putchar(((opcode >> b) & 1) ? '1' : '0');
        }
        
        printf("  (0x%X / %u)\n\n", opcode, opcode);
		
		printf("  rs: %2u   rt: %2u   rd: %2u\n  immediate: %2u\n\n", rs, rt, rd, immediate);
    }
	
	printf("\n\n\n\n");
	
	if (opcode == HALT)
		exit(EXIT_SUCCESS);
}


