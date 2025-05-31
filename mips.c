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
	uint32_t rawHexVal;
} decodedLine;

// struct to hold pipline informatoin
typedef struct pipeline_status_and_instruction {
	int line_in_pipe;
	int stage;
} pipeline;

// Initialize memory array - Initialize Register array
int32_t registers[NUM_REGISTERS] = {0};
int usedRegisters[NUM_REGISTERS] = {0}; // Flag for register if used
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
int maxline = 0;
// Program run more
int mode;

// File pointer
FILE *file;

// Program Counter
int pc = 0;
int global_clk[CLOCK_SIZE]; // Currently doesn't do anything

bool rtype = 0;
bool was_control_flow = 0;
//////////////////////////
// MAIN
//////////////////////////
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
	else if (strcmp(argv[1], "DEBUG_EXTRA") == 0)
		mode = DEBUG_EXTRA;
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
	
<<<<<<< Updated upstream
    
	if (mode == DEBUG_EXTRA) {
		printf("-- Going through testcase lines --\n");
	}
=======
    // initialize pipeline slots empty
    decodedLine empty = {.instruction=NOP, .dest_register=-1, .first_reg_val=-1, .second_reg_val=-1, .immediate=0, .pipe_stage=0};
    pipe.pipe1 = empty; pipe.pipe2=empty; pipe.pipe3=empty; pipe.pipe4=empty; pipe.pipe5=empty;
    
>>>>>>> Stashed changes
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

		// this is converting the intake to an integer
		uint32_t rawHex = StringToHex(line);

		/*
		Instruction Formatting
		R-type
		┌────────┬──────┬──────┬──────┬────────────┐
		│ Opcode │  Rs  │  Rt  │  Rd  │  Unused    │
		├────────┼──────┼──────┼──────┼────────────┤
		│  6 bit │ 5bit │ 5bit │ 5bit │  11 bits   │
		└────────┴──────┴──────┴──────┴────────────┘
		Used by: ADD, SUB, MUL, OR, AND, XOR

		I-type
		┌────────┬──────┬──────┬──────────────────┐
		│ Opcode │  Rs  │  Rt  │    Immediate     │
		├────────┼──────┼──────┼──────────────────┤
		│  6 bit │ 5bit │ 5bit │     16 bits      │
		└────────┴──────┴──────┴──────────────────┘
		Used by: ADDI, SUBI, MULI, ORI, ANDI, XORI, LDW, STW, BZ, BEQ
		*/

		// bit shift the intruction param by twenty six
<<<<<<< Updated upstream
		int32_t instruction_param = rawHex>>26;
		maxline = line_number - 1;
		program_store[line_number - 1].instruction = instruction_param;
=======
		int32_t opcode = rawHex>>26;

		// Set instruction param
		program_store[line_number - 1].instruction = opcode;
>>>>>>> Stashed changes


<<<<<<< Updated upstream
		// If - instruction is register based : Else - instruction is immediate based
		//		This block populates the instruction struct with appropriate register values and immediates based on instruction type
		if (instruction_param <= 0xB && instruction_param % 2 == 0){
			int32_t first_reg= (rawHex << 6) >> 27;		// rs
			int32_t second_reg = (rawHex << 11) >> 27;  // rt
			int32_t dest_reg= (rawHex << 16) >> 27;     // rd

			program_store[line_number - 1].first_reg_val = first_reg;	// rs
			program_store[line_number - 1].second_reg_val = second_reg; // rt
			program_store[line_number - 1].dest_register = dest_reg;	// rd
			program_store[line_number - 1].rawHexVal = rawHex;
		}
		// Note - Even if some instructions don't use all the register/immediate values, they are still populated here
		else if ( instruction_param <= 0x11){
			int32_t first_reg = (rawHex << 6) >> 27;					// rs
			int32_t second_reg = (rawHex << 11) >> 27;					// rt
			int32_t immediate_val = (rawHex << 16) >> 16;				// immediate
			program_store[line_number - 1].first_reg_val = first_reg;	// rs
			program_store[line_number - 1].second_reg_val = second_reg;	// rt/rd
			program_store[line_number - 1].immediate = immediate_val;	// immediate
			program_store[line_number - 1].rawHexVal = rawHex;
		}
		// Am I missing some logic for getting other instructions?
		else {
			if (mode == DEBUG) {
            printf("Line %d\n", line_number);
            printf("%d does not map to a valid instruction\n\n", program_store[line_number - 1].instruction);
        }
=======
		decodedLine newinst = empty;
        newinst.instruction = opcode;

        int rs = (rawHex >> 21) & 0x1F;
        int rt = (rawHex >> 16) & 0x1F;
        int rd = (rawHex >> 11) & 0x1F;

        if (opcode % 2 == 0 && opcode <= 0x0A) {
            // R-type
            newinst.dest_register = rd;
            newinst.first_reg_val = rs;
            newinst.second_reg_val = rt;
			program_store[line_number - 1].dest_register = rd;
			program_store[line_number - 1].first_reg_val = rs;
			program_store[line_number - 1].second_reg_val = rt;
        } 
		
		else if (opcode <= 0x10){
            // I-type
            newinst.dest_register = rt;
            newinst.first_reg_val = rs;
            newinst.immediate = rawHex & 0xFFFF;
			program_store[line_number - 1].dest_register = rt;
			program_store[line_number - 1].first_reg_val = rs;
			program_store[line_number - 1].immediate = rawHex & 0xFFFF;
        }
		
		else {
			if (mode == DEBUG) {
				printf("Line %d\n", line_number);
				//  printf("Binary: %u\n", program_store[line_number - 1]);
				printf("%d does not map to a valid instruction\n\n", program_store[line_number - 1].instruction);
			}
>>>>>>> Stashed changes
			continue;
		}
		// if (mode == DEBUG_EXTRA) {
		// 	print_line(program_store[line_number - 1], line_number - 1);
		// }

    }

	if (mode == DEBUG_EXTRA) {
		printf("\n-- Going through the instructions --\n");
	}
	int Running = 1;
	// THIS SECTION IS THE PROGRAM COUNTER KEEPING TRACK OF EACH INSTRUCTION UNTIL HALTED
	while(Running) {
		
<<<<<<< Updated upstream
		if (pc == maxline + 1) {
			printf("Error not valid area\n");
			break;
		}
		// print_line(program_store[pc], pc);
		// bit shift the intruction param by twenty six
		int32_t instruction_param = program_store[pc].rawHexVal>>26;
=======







		decodedLine *slots[5] = {&pipe.pipe1, &pipe.pipe2, &pipe.pipe3, &pipe.pipe4, &pipe.pipe5};
        decodedLine *inID = NULL, *inEX = NULL, *inMEM = NULL;
        for (int i = 0; i < 5; i++) {
            if (slots[i]->pipe_stage == 2) inID = slots[i];
            if (slots[i]->pipe_stage == 3) inEX = slots[i];
            if (slots[i]->pipe_stage == 4) inMEM = slots[i];
        }

		bool hazard = false;





		// memory access instructions
		if (inID && inMEM && findHazard(inMEM, inID)) {
			hazard = true;
			cycle_counter++;
			bool newInstAdded = false;
			
			if (mode == DEBUG) 
				printf("\n\n\n\nStall at cycle %d: EX-ID hazard detected\n\n\n\n", cycle_counter);

            for (int i = 0; i < 5; i++) {
                if (slots[i]->pipe_stage > 2 && slots[i]->pipe_stage < 5) {
					if (slots[i]->pipe_stage == 3) {
						if (!opcode_master(*slots[i]))
							pc++;
					}
                    slots[i]->pipe_stage++;
                }
				else if (slots[i]->pipe_stage == 5){
					printf("\n\nWriting back data from pipe %d\n\n", i+1);
					*slots[i] = empty;
				}
				if ((slots[i]->pipe_stage == 0) && !newInstAdded) {
					newinst.pipe_stage = 1;
					*slots[i] = newinst;
					newInstAdded = true;
				}
            }
		}




		if (inID && inEX && findHazard(inEX, inID)) {
			hazard = true;
			cycle_counter++;
			bool newInstAdded = false;
			
			if (mode == DEBUG) 
				printf("\n\n\n\nStall at cycle %d: EX-ID hazard detected\n\n\n\n", cycle_counter);

            for (int i = 0; i < 5; i++) {
                if (slots[i]->pipe_stage > 2 && slots[i]->pipe_stage < 5) {
					if (slots[i]->pipe_stage == 3) {
						if (!opcode_master(*slots[i]))
							pc++;
					}
                    slots[i]->pipe_stage++;
                }
				else if (slots[i]->pipe_stage == 5){
					printf("\n\nWriting back data from pipe %d\n\n", i+1);
					*slots[i] = empty;
				}
				if ((slots[i]->pipe_stage == 0) && !newInstAdded) {
					newinst.pipe_stage = 1;
					*slots[i] = newinst;
					newInstAdded = true;
				}
            }
		}




		if (!hazard) {
			cycle_counter++;
			bool newInstAdded = false;
			
            for (int i = 0; i < 5; i++) {
                if (slots[i]->pipe_stage > 0 && slots[i]->pipe_stage < 5) {
					if (slots[i]->pipe_stage == 3) {
						if (!opcode_master(*slots[i]))
							pc++;
					}
                    slots[i]->pipe_stage++;
                }
				else if (slots[i]->pipe_stage == 5){
					printf("\n\nWriting back data from pipe %d\n\n", i+1);
					*slots[i] = empty;
				}
				
				if ((slots[i]->pipe_stage == 0) && !newInstAdded) {
					newinst.pipe_stage = 1;
					*slots[i] = newinst;
					newInstAdded = true;
				}
			}	
		}



		
		






















		

		// DEBUG: pipe cycle debug
		if (mode == DEBUG) {
				printf("***************PIPE CYCLE DEBUG**************\n\n");
				printf("Pipe 1: %d\n", slots[0]->pipe_stage);
				printf("Pipe 2: %d\n", slots[1]->pipe_stage);
				printf("Pipe 3: %d\n", slots[2]->pipe_stage);
				printf("Pipe 4: %d\n", slots[3]->pipe_stage);
				printf("Pipe 5: %d\n", slots[4]->pipe_stage);
				printf("*********************************\n");
		}

		// opcode_master(program_store[line_number - 1]);
>>>>>>> Stashed changes

        // DEBUG: print each binary string
        if (mode == DEBUG) {
            printf("---Line %d---\n", pc);
           //  printf("Binary: %u\n", program_store[line_number - 1]);
<<<<<<< Updated upstream
            printf("Hex Number: %X\n", program_store[pc].rawHexVal);
			printf("Instruction: 0x%X, %d\n", program_store[pc].instruction, program_store[pc].instruction);
			printf("Destination register: %d\n", program_store[pc].dest_register);
			printf("1st source register: %d\n", program_store[pc].first_reg_val);
			if (instruction_param <= 0xB && instruction_param % 2 == 0) printf ("2nd source register: %d\n\n", program_store[pc].second_reg_val);
			else printf("Immediate value: %d\n\n", program_store[pc].immediate);
        }

		// Using the program counter to go through the list.
		opcode_master(program_store[pc]);
		if (program_store[pc].instruction == HALT) {
			break;
		}
		
	}
=======
            printf("Hex Number: %X\n", rawHex);
			printf("Instruction: 0x%X, %d\n", program_store[line_number - 1].instruction, program_store[line_number - 1].instruction);
			printf("Destination register: %d\n", program_store[line_number - 1].dest_register);
			printf("1st source register: %d\n", program_store[line_number - 1].first_reg_val);
			if (opcode <= 0xB && opcode % 2 == 0) printf ("2nd source register: %d\n\n", program_store[line_number - 1].second_reg_val);
			else printf("Immediate value: %d\n\n", program_store[line_number - 1].immediate);
        }
		
    }
>>>>>>> Stashed changes





	
	if(mode == DEBUG)
		printf("No HALT instruction found- ending program");
	
	end_program();

	return 99;
}

//////////////////////////
// END OF MAIN
//////////////////////////







// detect RAW hazard between two stages
bool findHazard(const decodedLine *wr, const decodedLine *rd) {
    // Both stages must hold an instruction
    if (wr->pipe_stage == 0 || rd->pipe_stage == 0) 
		return false;
	
    // Ignore NOP slots
    if (wr->instruction == NOP || rd->instruction == NOP) 
		return false;
	
    if (wr->dest_register < 0) 
		return false;
	
    // Check first source register
    if (wr->dest_register == rd->first_reg_val) 
		return true;
	
    // Only R-type opcodes (even, <=0x0A) have a second source
    if ((rd->instruction % 2 == 0 && rd->instruction <= 0x0A) && wr->dest_register == rd->second_reg_val)
        return true;
	
    return false;
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


void print_stats(){
	printf("---------------------------------");
    printf("\nInstruction Count Statistics:\n"); 
    printf("  Total Instructions:	%d\n", total_inst_count);
    printf("  R-Type:		%d\n", rtype_count);
    printf("  I-Type:		%d\n", itype_count);
    printf("  Arithmetic:		%d\n", arith_count);
    printf("  Logical:		%d\n", logic_count);
    printf("  Memory Access:	%d\n", memacc_count);
    printf("  Control Flow:		%d\n", cflow_count);
	printf("  Program Counter   	%d\n", pc);
	print_registers();
	return;
}

void print_registers() {
	printf("\nProgram counter: %d\n", pc);
	for (int i = 0; i < NUM_REGISTERS; i++) {
		if (usedRegisters[i] == 1) {
			printf("R%d: %d\n", i, registers[i]);
		}
	}
}

// Helper to print binary of a given width, with space every 4 bits
void printBinaryFixedWidth(unsigned int val, int width) {
    for (int i = width - 1; i >= 0; i--) {
        printf("%d", (val >> i) & 1);
        // Print space every 4 bits, except after the last group
        if (i % 4 == 0 && i != 0) {
            printf(" ");
        }
    }
}

void print_line(decodedLine line, int index) {
    unsigned int opcode     = line.instruction;
    unsigned int rd         = line.dest_register;
    unsigned int rs         = line.first_reg_val;
    unsigned int rt         = line.second_reg_val;
    unsigned int imm  		= line.immediate;

    printf("\n---------------------------------\n");
	if (opcode == BZ || opcode == BEQ || opcode == JR) {
    	printf("\nLine Number: [%d]\n", index - imm + 1);
	}
	else {
		printf("\nLine Number: [%d]\n", index);
	}
    printf("  Raw Hex: %08X\n", line.rawHexVal);
    
    printf("\nBinary: ");
    printBinaryFixedWidth(line.rawHexVal, 32);
    printf("\n");

    // Print instruction name based on opcode
	printf("  Instruction: ");
	switch (opcode) {
		// ----------------- R-type Instructions -----------------
		case ADD:
			printf("ADD   (R-type)\n");
			printf("    Format   : ADD rd, rs, rt\n");
			printf("    Equation : R[%d] = R[%d] + R[%d]\n", rd, rs, rt);
			break;

		case SUB:
			printf("SUB   (R-type)\n");
			printf("    Format   : SUB rd, rs, rt\n");
			printf("    Equation : R[%d] = R[%d] - R[%d]\n", rd, rs, rt);
			break;

		case MUL:
			printf("MUL   (R-type)\n");
			printf("    Format   : MUL rd, rs, rt\n");
			printf("    Equation : R[%d] = R[%d] * R[%d]\n", rd, rs, rt);
			break;

		case OR:
			printf("OR    (R-type)\n");
			printf("    Format   : OR rd, rs, rt\n");
			printf("    Equation : R[%d] = R[%d] | R[%d]\n", rd, rs, rt);
			break;

		case AND:
			printf("AND   (R-type)\n");
			printf("    Format   : AND rd, rs, rt\n");
			printf("    Equation : R[%d] = R[%d] & R[%d]\n", rd, rs, rt);
			break;

		case XOR:
			printf("XOR   (R-type)\n");
			printf("    Format   : XOR rd, rs, rt\n");
			printf("    Equation : R[%d] = R[%d] ^ R[%d]\n", rd, rs, rt);
			break;

		// ----------------- I-type Arithmetic Instructions -----------------
		case ADDI:
			printf("ADDI  (I-type)\n");
			printf("    Format   : ADDI rt, rs, imm\n");
			printf("    Equation : R[%d] = R[%d] + %d\n", rt, rs, imm);
			break;

		case SUBI:
			printf("SUBI  (I-type)\n");
			printf("    Format   : SUBI rt, rs, imm\n");
			printf("    Equation : R[%d] = R[%d] - %d\n", rt, rs, imm);
			break;

		case MULI:
			printf("MULI  (I-type)\n");
			printf("    Format   : MULI rt, rs, imm\n");
			printf("    Equation : R[%d] = R[%d] * %d\n", rt, rs, imm);
			break;

		case ORI:
			printf("ORI   (I-type)\n");
			printf("    Format   : ORI rt, rs, imm\n");
			printf("    Equation : R[%d] = R[%d] | %d\n", rt, rs, imm);
			break;

		case ANDI:
			printf("ANDI  (I-type)\n");
			printf("    Format   : ANDI rt, rs, imm\n");
			printf("    Equation : R[%d] = R[%d] & %d\n", rt, rs, imm);
			break;

		case XORI:
			printf("XORI  (I-type)\n");
			printf("    Format   : XORI rt, rs, imm\n");
			printf("    Equation : R[%d] = R[%d] ^ %d\n", rt, rs, imm);
			break;

		// ----------------- Load/Store Instructions -----------------
		case LDW:
			printf("LDW   (I-type)\n");
			printf("    Format   : LDW rt, offset(rs)\n");
			printf("    Equation : R[%d] = MEM[R[%d] + %d]\n", rt, rs, imm);
			break;

		case STW:
			printf("STW   (I-type)\n");
			printf("    Format   : STW rt, offset(rs)\n");
			printf("    Equation : MEM[R[%d] + %d] = R[%d]\n", rs, imm, rt);
			break;

		// ----------------- Branch Instructions -----------------
		case BZ:
			printf("BZ    (I-type)\n");
			printf("    Format   : BZ rs, offset\n");
			printf("    Equation : if (R[%d] == 0) PC += %d\n", rs, imm);
			break;

		case BEQ:
			printf("BEQ   (I-type)\n");
			printf("    Format   : BEQ rs, rt, offset\n");
			printf("    Equation : if (R[%d] == R[%d]) PC += %d\n", rs, rt, imm);
			break;

		// ----------------- Jump Register -----------------
		case JR:
			printf("JR    (R-type)\n");
			printf("    Format   : JR rs\n");
			printf("    Equation : PC = R[%d]\n", rs);
			break;

		// ----------------- Halt -----------------
		case HALT:
			printf("HALT\n");
			break;

		// ----------------- Unknown Instruction -----------------
		default:
			printf("UNKNOWN INSTRUCTION\n");
			break;
	}

    // Print opcode
    printf("\n  Opcode:       %d 	[", opcode);
    printBinaryFixedWidth(opcode, 6);
    printf("]\n");
	
    // For R-type instructions
    if (opcode == ADD || opcode == SUB || opcode == MUL || opcode == OR || opcode == AND || opcode == XOR || opcode == JR) {	
        printf("  rs:           %d 	[", rs);
        printBinaryFixedWidth(rs, 5);
        printf("]\n");

        printf("  rt:           %d 	[", rt);
        printBinaryFixedWidth(rt, 5);
        printf("]\n");

        printf("  rd:           %d 	[", rd);
        printBinaryFixedWidth(rd, 5);
        printf("]\n");
		printf("  Operands : rd = R[%d], rs = R[%d], rt = R[%d]\n", rd, rs, rt);
		
    }
    // For I-type instructions
    else if (opcode == ADDI || opcode == SUBI || opcode == MULI || opcode == ORI || opcode == ANDI || opcode == XORI ||
             opcode == LDW || opcode == STW || opcode == BZ || opcode == BEQ) {
        printf("  rd/rs:        %d 	[", rs);
        printBinaryFixedWidth(rs, 5);
        printf("]\n");

        printf("  rt:           %d 	[", rt);
        printBinaryFixedWidth(rt, 5);
        printf("]\n");

        printf("  immediate:    %d 	[", (int16_t)imm);
        printBinaryFixedWidth(imm & 0xFFFF, 16);
        printf("]\n");
		printf("  Operands : rt = R[%d], rs = R[%d], imm = %d\n", rt, rs, (int16_t)imm);
    }
    // For HALT or unknown
    else {
        printf("  rd:           %d 	[", rd);
        printBinaryFixedWidth(rd, 5);
        printf("]\n");

        printf("  rs:           %d 	[", rs);
        printBinaryFixedWidth(rs, 5);
        printf("]\n");

        printf("  rt:           %d 	[", rt);
        printBinaryFixedWidth(rt, 5);
        printf("]\n");

        printf("  immediate:    %d 	[", (int16_t)imm);
        printBinaryFixedWidth(imm & 0xFFFF, 16);
        printf("]\n");
		printf("  Operands : rd = R[%d], rs = R[%d], rt = R[%d]\n", rd, rs, rt);
    }
}

bool opcode_master(decodedLine line) {

    unsigned int opcode 	= line.instruction;
	unsigned int rd 		= line.dest_register;
	unsigned int rs 		= line.first_reg_val;
	unsigned int rt 		= line.second_reg_val;
	unsigned int immediate 	= line.immediate;
	rtype = 0;
	was_control_flow = 0;
	
	// if (mode == DEBUG_EXTRA) {
	// 	printf("\nOpcode: 0x%02X\n", opcode);
	// 	printf("RD: %u\n", rd);
	// 	printf("RS: %u\n", rs);
	// 	printf("RT: %u\n", rt);
	// 	printf("Immediate: %u\n", immediate);
	// }

    switch(opcode) {
		////////////////////////////////	
		// Arithmetic Instructions:
		////////////////////////////////
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
		
		////////////////////////////////
		// Logical Instructions:
		////////////////////////////////
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
		
		////////////////////////////////
		// Memory Access Instructions:
		////////////////////////////////
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
		
		////////////////////////////////
		// Control Flow Instructions:
		////////////////////////////////
		{
		case BZ:
			if (mode == DEBUG) printf("BZ\n");
			pc--;
			bzfunc(rs, immediate);
			break;
			
		case BEQ:
			if (mode == DEBUG) printf("BEQ\n");
			pc--;
			beqfunc(rs, rt, immediate);
	
			break;
			
		case JR:
			if (mode == DEBUG) printf("JR\n");
			pc--;
			jrfunc(rs);

			break;
			
		case HALT:
			if (mode == DEBUG) {
				printf("HALT: ENDING PROGRAM\n\n\n\n\n");
			}
			else if (mode == DEBUG_EXTRA) {
				print_line(line, pc);
			}
			haltfunc();
			pc--;

			break;
		}
	
		default:
			if (mode == DEBUG) printf("Error: Unknown opcode 0x%02X. Exiting.\n", opcode);
			exit(EXIT_FAILURE);
	}
	
	
	// Displays decoded sections of the address
	if (mode == DEBUG) {

		// Differentiate between R-Type and I-Type
		if (rtype == 1) {
			printf("R-Type:    rs: %2u   rt: %2u   rd: %2u\n", rs, rt, rd);
		}
		else {
			printf("I-Type:    rs: %2u   rt: %2u   imm: %6d\n", rs, rt, (int16_t)immediate);
		}
		printf("PC: %d\n", pc);

		printf("\n\n\n\n");
	}
	
	if (mode == DEBUG_EXTRA) {
			print_line(line, pc);
			printf("\n -- Registers Used -- :");
			print_registers();
	}
	// Unless control flow instruction modified pc directly, increment by default
	if (was_control_flow == 0)
		return false;
	
	else
		return true;
}

// addfunc(rt, rs, immediate, true);
void addfunc(int dest, int src1, int src2, bool is_immediate) {
    // Mark registers as used if not already marked
    // Note: For immediate instructions, src2 is not a register index
    if (usedRegisters[dest] == 0 || usedRegisters[src1] == 0) {
        usedRegisters[dest] = 1;
        usedRegisters[src1] = 1;
        if (!is_immediate) {
            usedRegisters[src2] = 1;  // Only mark src2 if it's a register (not immediate)
        }
    }

    // If instruction is R-type (register-register), set rtype flag
    if (!is_immediate) rtype = 1;

    // Retrieve value from source register 1
    int32_t val1 = registers[src1];

    // If immediate, sign-extend src2 (16-bit immediate), else get value from source register 2
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[src2];

    // Perform the addition and store result in destination register
    registers[dest] = val1 + val2;

    // Increment count of arithmetic instructions executed
    arith_count++;

    // Increment count based on instruction type (immediate or register)
    if (is_immediate)
        itype_count++;  // I-type (immediate) instruction count
    else
        rtype_count++;  // R-type instruction count

    // Increment total instruction count executed
    total_inst_count++;
}



void subfunc(int dest, int src1, int src2, bool is_immediate) {
	
	// Mark registers as used if not already marked
    // Note: For immediate instructions, src2 is not a register index
    if (usedRegisters[dest] == 0 || usedRegisters[src1] == 0) {
        usedRegisters[dest] = 1;
        usedRegisters[src1] = 1;
        if (!is_immediate) {
            usedRegisters[src2] = 1;  // Only mark src2 if it's a register (not immediate)
        }
    }
	
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
	// Mark registers as used if not already marked
    // Note: For immediate instructions, src2 is not a register index
    if (usedRegisters[dest] == 0 || usedRegisters[src1] == 0) {
        usedRegisters[dest] = 1;
        usedRegisters[src1] = 1;
        if (!is_immediate) {
            usedRegisters[src2] = 1;  // Only mark src2 if it's a register (not immediate)
        }
    }

	if (!is_immediate) rtype = 1;
	int32_t val1 = registers[src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[src2];
    registers[dest] = val1 * val2;

	usedRegisters[dest] = 1;
	usedRegisters[src1] = 1;
	usedRegisters[src2] = 1;
	
    arith_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void orfunc(int dest, int src1, int src2, bool is_immediate) {
	// Mark registers as used if not already marked
    // Note: For immediate instructions, src2 is not a register index
    if (usedRegisters[dest] == 0 || usedRegisters[src1] == 0) {
        usedRegisters[dest] = 1;
        usedRegisters[src1] = 1;
        if (!is_immediate) {
            usedRegisters[src2] = 1;  // Only mark src2 if it's a register (not immediate)
        }
    }

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
	// Mark registers as used if not already marked
    // Note: For immediate instructions, src2 is not a register index
    if (usedRegisters[dest] == 0 || usedRegisters[src1] == 0) {
        usedRegisters[dest] = 1;
        usedRegisters[src1] = 1;
        if (!is_immediate) {
            usedRegisters[src2] = 1;  // Only mark src2 if it's a register (not immediate)
        }
    }

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
	// Mark registers as used if not already marked
    // Note: For immediate instructions, src2 is not a register index
    if (usedRegisters[dest] == 0 || usedRegisters[src1] == 0) {
        usedRegisters[dest] = 1;
        usedRegisters[src1] = 1;
        if (!is_immediate) {
            usedRegisters[src2] = 1;  // Only mark src2 if it's a register (not immediate)
        }
    }

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
	usedRegisters[rt] = 1;

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
	usedRegisters[rt] = 1;

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
		usedRegisters[rs] = 1;
    }
}


void beqfunc(int rs, int rt, int imm) {
    cflow_count++;
    itype_count++;
    total_inst_count++;

    if (registers[rs] == registers[rt]) {
        pc += (int16_t)imm;
        was_control_flow = 1;
		usedRegisters[rs] = 1;
    }
}


void jrfunc(int rs) {
    cflow_count++;
    itype_count++;
    total_inst_count++;
	was_control_flow = 1;

    pc = registers[rs];  // Assume PC holds instruction index, not byte address
	usedRegisters[rs] = 1;
}


void haltfunc(){
	cflow_count++;
	itype_count++;
	total_inst_count++;
	was_control_flow = 1;
	
	end_program();
}