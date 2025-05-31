/**
 * mips.c - Source code file for a MIPS-lite simulation
 *
 * @authors:    Evan Brown (evbr2@pdx.edu)
 * 				Louis-David Gendron-Herndon (loge2@pdx.edu)
 *				Ameer Melli (amelli@pdx.edu)
 *				Anthony Le (anthle@pdx.edu)
 *
 *
 * @date:       May 31, 2025
 * @version:    2.0
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
typedef struct pipe_main {
	decodedLine pipe1;
	decodedLine pipe2;
	decodedLine pipe3;
	decodedLine pipe4;
	decodedLine pipe5;
} pipeline;

// Initialize memory array - Initialize Register array
int32_t registers[NUM_REGISTERS] = {0};
int32_t memory[MEMORY_SIZE];

// Stores all of the line's information in one array
decodedLine program_store[MEMORY_SIZE];

// Variable for our pipe struct
pipeline pipe;

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
int cycle_counter = 0;

bool rtype = 0;
bool was_control_flow = 0;
bool halt_executed = false;
bool ready_to_end = false;



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
	
	
    // initialize pipeline slots empty
    decodedLine empty = {.instruction=NOP, .dest_register=-1, .first_reg_val=-1, .second_reg_val=-1, .immediate=0, .pipe_stage=0};
    pipe.pipe1 = empty; pipe.pipe2=empty; pipe.pipe3=empty; pipe.pipe4=empty; pipe.pipe5=empty;
    
    // Read each line of the trace file
    while (1) {
		line_number++;
		fgets(line, sizeof(line), file);
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

		// bit shift the intruction param by twenty six
		int32_t opcode = rawHex>>26;

		// Set instruction param
		program_store[line_number - 1].instruction = opcode;

		// Set pipline stage to zero
		program_store[line_number - 1].pipe_stage = 0;

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
		
		else if (opcode <= 0x11){
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
			continue;
		}
		







		decodedLine *slots[5] = {&pipe.pipe1, &pipe.pipe2, &pipe.pipe3, &pipe.pipe4, &pipe.pipe5};
        decodedLine *inID = NULL, *inEX = NULL, *inMEM = NULL;
        for (int i = 0; i < 5; i++) {
            if (slots[i]->pipe_stage == 2) inID = slots[i];
            if (slots[i]->pipe_stage == 3) inEX = slots[i];
            if (slots[i]->pipe_stage == 4) inMEM = slots[i];
        }

		bool hazard = false;


		bool newInstAdded = false;


		// memory access instructions
		if (inID && inMEM && findHazard(inMEM, inID)) {
			hazard = true;
			cycle_counter++;
			
			
			if (mode == DEBUG) 
				printf("\n\n\n\nStall at cycle %d: MEM-ID hazard detected\n\n\n\n", cycle_counter);

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
					if (halt_executed && (slots[i]->instruction == HALT)){
						end_program();
					}
					
					*slots[i] = empty;
				}
				if ((slots[i]->pipe_stage == 0) && !newInstAdded) {
					newinst.pipe_stage = 1;
					*slots[i] = newinst;
					newInstAdded = true;
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

			// DEBUG: print each binary string
			if (mode == DEBUG) {
				printf("---Line %d---\n", line_number);
			   //  printf("Binary: %u\n", program_store[line_number - 1]);
				printf("Hex Number: %X\n", rawHex);
				printf("Instruction: 0x%X, %d\n", program_store[line_number - 1].instruction, program_store[line_number - 1].instruction);
				printf("Destination register: %d\n", program_store[line_number - 1].dest_register);
				printf("1st source register: %d\n", program_store[line_number - 1].first_reg_val);
				if (opcode <= 0xB && opcode % 2 == 0) printf ("2nd source register: %d\n\n", program_store[line_number - 1].second_reg_val);
				else printf("Immediate value: %d\n\n", program_store[line_number - 1].immediate);
			}
		}




		if (inID && inEX && findHazard(inEX, inID)) {
			hazard = true;
			cycle_counter++;
			
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
					if (halt_executed && (slots[i]->instruction == HALT)){
						end_program();
					}
					
					*slots[i] = empty;
				}
				
				if ((slots[i]->pipe_stage == 0) && !newInstAdded) {
					newinst.pipe_stage = 1;
					*slots[i] = newinst;
					newInstAdded = true;
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

			// DEBUG: print each binary string
			if (mode == DEBUG) {
				printf("---Line %d---\n", line_number);
			   //  printf("Binary: %u\n", program_store[line_number - 1]);
				printf("Hex Number: %X\n", rawHex);
				printf("Instruction: 0x%X, %d\n", program_store[line_number - 1].instruction, program_store[line_number - 1].instruction);
				printf("Destination register: %d\n", program_store[line_number - 1].dest_register);
				printf("1st source register: %d\n", program_store[line_number - 1].first_reg_val);
				if (opcode <= 0xB && opcode % 2 == 0) printf ("2nd source register: %d\n\n", program_store[line_number - 1].second_reg_val);
				else printf("Immediate value: %d\n\n", program_store[line_number - 1].immediate);
			}
		}




		if (!hazard) {
			cycle_counter++;
			
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
					if (halt_executed && (slots[i]->instruction == HALT)){
						end_program();
					}
					
					*slots[i] = empty;
				}
				
				if ((slots[i]->pipe_stage == 0) && !newInstAdded) {
					newinst.pipe_stage = 1;
					*slots[i] = newinst;
					newInstAdded = true;
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

			/* DEBUG: print each binary string
			if (mode == DEBUG) {
				printf("---Line %d---\n", line_number);
			   //  printf("Binary: %u\n", program_store[line_number - 1]);
				printf("Hex Number: %X\n", rawHex);
				printf("Instruction: 0x%X, %d\n", program_store[line_number - 1].instruction, program_store[line_number - 1].instruction);
				printf("Destination register: %d\n", program_store[line_number - 1].dest_register);
				printf("1st source register: %d\n", program_store[line_number - 1].first_reg_val);
				if (opcode <= 0xB && opcode % 2 == 0) printf ("2nd source register: %d\n\n", program_store[line_number - 1].second_reg_val);
				else printf("Immediate value: %d\n\n", program_store[line_number - 1].immediate);
			}*/
			
		}

		

		
	
	
    }

	



	
	if(mode == DEBUG)
		printf("No HALT instruction found- ending program");
	
	end_program();

	return 99;
}








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
	
	print_stats();
	exit(EXIT_SUCCESS);
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
	printf("  Cycles:		%d\n", cycle_counter);
	
	return;
}


bool opcode_master(decodedLine line) {

	//
    unsigned int opcode 	= line.instruction;
	unsigned int rd 		= line.dest_register;
	unsigned int rs 		= line.first_reg_val;
	unsigned int rt 		= line.second_reg_val;
	unsigned int immediate 	= line.immediate;
	rtype = 0;
	was_control_flow = 0;

	
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
			if (mode == DEBUG) printf("HALT: FINISHING PROGRAM...\n\n\n\n\n");
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
		return false;
	
	else
		return true;
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
	
	// Close the file
    fclose(file);
	
	halt_executed = true;
}