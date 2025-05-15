/**
 * mips.c - Source code file for a MIPS-lite simulation
 *
 * @authors:    Evan Brown (evbr2@pdx.edu)
 * 				Louis-David Gendron-Herndon (loge2@pdx.edu)
 *				Ameer Melli (amelli@pdx.edu)
 *				Anthony Le (anthle@pdx.edu)
 *
 *
 * @date:       May 9, 2025
 * @version:    1.0
 *
 *
 * MODES:		
 *				NORMAL:		Print instruction statistics at the end of the program.
 *							
 *
 *				DEBUG:		Normal mode + extra prints for decoded addresses
 *							and instructions executed.
 *							
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "mips.h"

// struct to hold decoded line information
typedef struct decoded_line_information {
	int instruction;
	int32_t dest_register;
	int32_t first_reg_val;
	int32_t second_reg_val;
	int32_t immediate;
	int pipe_stage;
} decodedLine;

// struct to hold pipline informatoin
typedef struct pipeline_status_and_instruction {
	int line_in_pipe;
	int stage;
} pipeline;

// Initialize memory array - Initialize Register array
int32_t registers[NUM_REGISTERS] = {0};
int32_t memory[MEMORY_SIZE];

// Stores all of the line's information in one array
decodedLine program_store[MEMORY_SIZE];

// Global variable to count transactions
int rtype_count = 0;
int itype_count = 0;
int arith_count = 0;
int logic_count = 0;
int memacc_count = 0;
int cflow_count = 0;
int total_inst_count = 0;

// Program run more
int mode;

// File pointer
FILE *file;

// Program Counter
int pc = 0;

bool rtype = 0;
bool was_control_flow = 0;



int main(int argc, char *argv[]) {
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
        if (mode == DEBUG)
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

        // Convert to binary string and begin binary manip -- CHARACTER 01 VERSION
		hex_to_binary_string(line, binary_string);
	
		// this is converting the intake to an integer
		uint32_t rawHex = StringToHex(line);

		// bit shift the intruction param by twenty six
		int32_t instruction_param = rawHex>>26;

		program_store[line_number - 1].instruction = instruction_param;


		// If - instruction is register based : Else - instruction is immediate based
		//		This block populates the instruction struct with appropriate register values and immediates based on instruction type
		if (instruction_param <= 0xB && instruction_param % 2 == 0){
			int32_t dest_reg = (rawHex << 6) >> 27;
			int32_t first_reg = (rawHex << 11) >> 27;
			int32_t second_reg = (rawHex << 16) >> 27;
			program_store[line_number - 1].dest_register = dest_reg;
			program_store[line_number - 1].first_reg_val = first_reg;
			program_store[line_number - 1].second_reg_val = second_reg;
		}
		// Note - Even if some instructions don't use all the register/immediate values, they are still populated here
		else if ( instruction_param <= 0x11){
			int32_t dest_reg = (rawHex << 6) >> 27;
			int32_t first_reg = (rawHex << 11) >> 27;
			int32_t immediate_val = (rawHex << 16) >> 16;
			program_store[line_number - 1].dest_register = dest_reg;
			program_store[line_number - 1].first_reg_val = first_reg;
			program_store[line_number - 1].immediate = immediate_val;
			
		}
		
		else {
			if (mode == DEBUG) {
            printf("Line %d\n", line_number);
           //  printf("Binary: %u\n", program_store[line_number - 1]);
            printf("%d does not map to a valid instruction\n\n", program_store[line_number - 1].instruction);
        }
			continue;
		}
		

        // DEBUG: print each binary string
        if (mode == DEBUG) {
            printf("Line %d\n", line_number);
           //  printf("Binary: %u\n", program_store[line_number - 1]);
            printf("Hex Number: %X\n\n", rawHex);
        }
		

        // CONTINUE
    }
	

	/*

	AGREED_METHOD
		PRE-DECODE
			Store info using line structs

		Have a master clock
			Cycles push thing through the pipiline and advance the program

		Pipes
			Pipes are struct that keep what instruction/line they're on and the stage
		

	*/

	
	if(mode == DEBUG)
		printf("No HALT instruction found- ending program");
	
	
	end_program();


	return 99;
}

int32_t StringToHex(char *hex_string){
    uint32_t temp;
    int32_t signedInt;

    // Convert the hex string to a 32-bit unsigned integer
    temp = (uint32_t)strtoul(hex_string, NULL, 16);

    // Cast the unsigned value to signed (handles two's complement conversion)
    signedInt = (int32_t)temp;

	return signedInt;
}

void end_program(){
	// Close the file
    fclose(file);
	print_stats();
	exit(EXIT_SUCCESS);
}

void hex_to_binary_string(const char *hex_string, char *binary_string) {
    unsigned int value;
    sscanf(hex_string, "%x", &value);

    for (int i = ADDRESS_BITS - 1; i >= 0; i--) {
        binary_string[ADDRESS_BITS - 1 - i] = (value & (1U << i)) ? '1' : '0';
    }
    binary_string[ADDRESS_BITS] = '\0'; // null-terminate
}



void print_stats(){
    printf("\nInstruction Count Statistics:\n"); 
    printf("  Total Instructions:	%d\n", total_inst_count);
    printf("  R-Type:		%d\n", rtype_count);
    printf("  I-Type:		%d\n", itype_count);
    printf("  Arithmetic:		%d\n", arith_count);
    printf("  Logical:		%d\n", logic_count);
    printf("  Memory Access:	%d\n", memacc_count);
    printf("  Control Flow:		%d\n", cflow_count);
	
	return;
}


void opcode_master(const char *binary_string) {

	//
    unsigned int opcode 	= 0;
	unsigned int rs 		= 0;
	unsigned int rt 		= 0;
	unsigned int rd 		= 0;
	unsigned int immediate 	= 0;
	rtype = 0;
	was_control_flow = 0;




	// Decoding the address:
	{
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
	}
	
	if (mode == DEBUG){
		// Print opcode in binary
		printf("\nOpcode: ");
		for (int b = 5; b >= 0; b--) {
			putchar(((opcode >> b) & 1) ? '1' : '0');
		}

		// Print the opcode and its value in hex and decimal
		printf("  (0x%X / %u)\n", opcode, opcode);
		
		printf("Instruction: ");
		
	}
	
	
    switch(opcode) {	
		// Arithmetic Instructions:
		{
		case ADD:
			if (mode == DEBUG) printf("ADD\n");
			addfunc(rd, rs, rt, false);
			break;
			
		case ADDI:
			if (mode == DEBUG) printf("ADDI\n");
			addfunc(rt, rs, immediate, true);
			break;
			
		case SUB:
			if (mode == DEBUG) printf("SUB\n");
			subfunc(rd, rs, rt, false);
			break;
			
		case SUBI:
			if (mode == DEBUG) printf("SUBI\n");
			subfunc(rt, rs, immediate, true);
			break;
			
		case MUL:
			if (mode == DEBUG) printf("MUL\n");
			mulfunc(rd, rs, rt, false);
			
			break;
			
		case MULI:
			if (mode == DEBUG) printf("MULI\n");
			mulfunc(rt, rs, immediate, true);
			break;
		}
		
		
		// Logical Instructions:
		{
		case OR:
			if (mode == DEBUG) printf("OR\n");
			orfunc(rd, rs, rt, false);
			break;
			
		case ORI:
			if (mode == DEBUG) printf("ORI\n");
			orfunc(rt, rs, immediate, true);
			break;
			
		case AND:
			if (mode == DEBUG) printf("AND\n");
			andfunc(rd, rs, rt, false);
			break;
			
		case ANDI:
			if (mode == DEBUG) printf("ANDI\n");
			andfunc(rt, rs, immediate, true);
			break;
			
		case XOR:
			if (mode == DEBUG) printf("XOR\n");
			xorfunc(rd, rs, rt, false);
			break;
			
		case XORI:
			if (mode == DEBUG) printf("XORI\n");
			xorfunc(rt, rs, immediate, true);
			break;	
		}
		
		
		// Memory Access Instructions:
		{
		case LDW:
			if (mode == DEBUG) printf("LDW\n");
			ldwfunc(rt, rs, immediate);
			break;
			
		case STW:
			if (mode == DEBUG) printf("STW\n");
			stwfunc(rt, rs, immediate);
			break;
		}
		
		
		// Control Flow Instructions:
		{
		case BZ:
			if (mode == DEBUG) printf("BZ\n");
			bzfunc(rs, immediate);
			break;
			
		case BEQ:
			if (mode == DEBUG) printf("BEQ\n");
			beqfunc(rs, rt, immediate);
			break;
			
		case JR:
			if (mode == DEBUG) printf("JR\n");
			jrfunc(rs);
			break;
			
		case HALT:
			if (mode == DEBUG) printf("HALT: ENDING PROGRAM\n\n\n\n\n");
			haltfunc();
			break;
		}
		
	
		default:
			if (mode == DEBUG) printf("Error: Unknown opcode 0x%02X. Exiting.\n", opcode);
			exit(EXIT_FAILURE);
	}
	
	
	// Displays decoded sections of the address
	if (mode == DEBUG) {

		// Differentiate between R-Type and I-Type
		if (rtype == 1)
			printf("R-Type:    rs: %2u   rt: %2u   rd: %2u\n", rs, rt, rd);
		
		else
			printf("I-Type:    rs: %2u   rt: %2u   imm: %6d\n", rs, rt, (int16_t)immediate);
		
		printf("PC: %d\n", pc);

		printf("\n\n\n\n");
	}
	
	
	// Unless control flow instruction modified pc directly, increment by default
	if (was_control_flow == 0)
		pc++;
	
}


void addfunc(int dest, int src1, int src2, bool is_immediate) {
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[src2]; // sign-extend imm
    registers[dest] = val1 + val2;

    arith_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void subfunc(int dest, int src1, int src2, bool is_immediate) {
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[src2];
    registers[dest] = val1 - val2;

    arith_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void mulfunc(int dest, int src1, int src2, bool is_immediate){
	if (!is_immediate) rtype = 1;
	int32_t val1 = registers[src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[src2];
    registers[dest] = val1 * val2;

    arith_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void orfunc(int dest, int src1, int src2, bool is_immediate) {
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[src2];
    registers[dest] = val1 | val2;

    logic_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void andfunc(int dest, int src1, int src2, bool is_immediate) {
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[src2];
    registers[dest] = val1 & val2;

    logic_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void xorfunc(int dest, int src1, int src2, bool is_immediate) {
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[src2];
    registers[dest] = val1 ^ val2;

    logic_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void ldwfunc(int rt, int rs, int imm) {
    int32_t addr = registers[rs] + (int16_t)imm;

    if (addr % 4 != 0 || addr / 4 < 0 || addr / 4 >= MEMORY_SIZE) {
        printf("Memory load error: invalid address 0x%X\n", addr);
        exit(EXIT_FAILURE);
    }

    registers[rt] = memory[addr / 4];

    memacc_count++;
    itype_count++;
    total_inst_count++;
}


void stwfunc(int rt, int rs, int imm) {
    int32_t addr = registers[rs] + (int16_t)imm;

    if (addr % 4 != 0 || addr / 4 < 0 || addr / 4 >= MEMORY_SIZE) {
        printf("Memory store error: invalid address 0x%X\n", addr);
        exit(EXIT_FAILURE);
    }

    memory[addr / 4] = registers[rt];

    memacc_count++;
    itype_count++;
    total_inst_count++;
}


void bzfunc(int rs, int imm) {
    cflow_count++;
    itype_count++;
    total_inst_count++;

    if (registers[rs] == 0) {
        pc += (int16_t)imm;
        was_control_flow = 1;
    }
}

void beqfunc(int rs, int rt, int imm) {
    cflow_count++;
    itype_count++;
    total_inst_count++;

    if (registers[rs] == registers[rt]) {
        pc += (int16_t)imm;
        was_control_flow = 1;
    }
}


void jrfunc(int rs) {
    cflow_count++;
    itype_count++;
    total_inst_count++;
	was_control_flow = 1;

    pc = registers[rs];  // Assume PC holds instruction index, not byte address
}


void haltfunc(){
	cflow_count++;
	itype_count++;
	total_inst_count++;
	was_control_flow = 1;
	
	end_program();
}