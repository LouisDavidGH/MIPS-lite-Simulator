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

        // Convert to binary string
        hex_to_binary_string(line, binary_string);

        // DEBUG: print each binary string
        if (mode == DEBUG) {
            printf("Line %d\n", line_number);
            printf("Hex: %s\n", line);
            printf("Binary: %s\n\n", binary_string);
        }
		

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