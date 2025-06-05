/**
 * mips.c - Source code file for a MIPS-lite simulation
 *
 *
 * @authors:	Evan Brown (evbr2@pdx.edu)
 * 				Louis-David Gendron-Herndon (loge2@pdx.edu)
 *				Ameer Melli (amelli@pdx.edu)
 *				Anthony Le (anthle@pdx.edu)
 *
 *
 *
 *
 * @date:       June 5, 2025
 * @version:    4.0
 *
 *
 *
 *
 *
 * FUNCTIONAL MODES:		
 *
 *				NO_PIPE:	Works through the trace file, line by line,
 *							with no pipeline functionality.
 *							
 *
 *				NO_FWD:		Adds pipeline functionality with hazard stalling,
 *							but no forwarding.
 *
 *
 *				FWD:		Adds forwarding to the pipeline functionality.
 *
 *
 *
 *
 * DEBUG MODES:		
 *
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
#include <inttypes.h>
#include "mips.h"

// struct to hold decoded line information
typedef struct decoded_line_information {
	int32_t instruction;
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
int32_t registers[NUM_REGISTERS];
bool register_used[NUM_REGISTERS];

int32_t memory[MEMORY_SIZE];
bool memory_used[MEMORY_SIZE];

// Stores all of the line's information in one array
decodedLine program_store[MEMORY_SIZE+1];
uint32_t rawHex_array[MEMORY_SIZE];

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
int total_stalls = 0;

// Program run more
int mode;

// Function mode
int functional_mode;

// File pointer
FILE *file;

// Program Counter
int pc = 0;
int cycle_counter = 0;

bool rtype = 0;
bool was_control_flow = 0;
bool was_jrfunc_for_nopipe = 0;
bool halt_executed = false;
bool ready_to_end = false;

// Hazard and newline loaded variables
bool hazard = false;
bool newInstAdded = true;
bool end_of_fetch = false;

uint32_t rawHex;
uint8_t opcode;
uint8_t rs;
uint8_t rt;
uint8_t rd;
int16_t imm16;




int main(int argc, char *argv[]) {
	char line[LINE_BUFFER_SIZE];
    char binary_string[ADDRESS_BITS + 1];
	int line_number = 0;
	
    // Check for at least two arguments: mode and filename
    if (argc < 4) {
        printf("Usage: %s <DEBUG/NORMAL> <NO_PIPE/NO_FWD/FWD> <TRACE_FILE>\n", argv[0]);
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
	
	
	
	// Set mode specified in the first argument
	if (strcmp(argv[2], "NO_PIPE") == 0)
		functional_mode = NO_PIPE;
	else if (strcmp(argv[2], "NO_FWD") == 0)
		functional_mode = NO_FWD;
	else if (strcmp(argv[2], "FWD") == 0)
		functional_mode = FWD;
	else {
		printf("\nInvalid mode. Defaulting to Non-Pipelined (NO_PIPE).\n");
		functional_mode = NO_PIPE;
	}
	
	
	
	// Open the trace file specified in the second argument
    file = fopen(argv[3], "r");
    if (file == NULL) {
        if (mode == DEBUG)
			perror("Error opening trace file");
        return EXIT_FAILURE;
    }
	
	// Initialize all registers and memory to zero.
	for (int i = 0; i<NUM_REGISTERS; i++){
		register_used[i] = false;
		registers[i] = 0;
	}
	
	for (int i = 0; i<MEMORY_SIZE; i++){
		memory_used[i] = false;
		memory[i] = 0;
	}
	
	
	
	
	
	
    // initialize pipeline slots empty
    decodedLine empty = {.instruction=NOP, .dest_register=-1, .first_reg_val=-1, .second_reg_val=-1, .immediate=0, .pipe_stage=0};
    pipe.pipe1 = empty; pipe.pipe2=empty; pipe.pipe3=empty; pipe.pipe4=empty; pipe.pipe5=empty;
    
	decodedLine newinst = empty;
	
	
	// fill program_store array
	while (fgets(line, sizeof(line), file) != NULL){
		
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
		
		program_store[line_number - 1] = empty;
		
		// this is converting the intake to an integer
		rawHex = StringToHex(line);
		
		rawHex_array[line_number - 1] = rawHex;
		
		// bit shift the intruction param by twenty six
		opcode = (rawHex >> 26) & 0x3F;

		// Set instruction param
		program_store[line_number - 1].instruction = opcode;

		// Set pipline stage to zero
		program_store[line_number - 1].pipe_stage = 0;

		// Getting registers by bit shifting and masking
		rs = (rawHex >> 21) & 0x1F;
		rt = (rawHex >> 16) & 0x1F;
		rd = (rawHex >> 11) & 0x1F;
		imm16 = rawHex & 0xFFFF;
		int32_t  imm    = (int32_t) imm16;

		switch (opcode) {
		  //-----------------------------------
		  // R-type arithmetic/logical:
		  case ADD: case SUB: case MUL:
		  case OR:  case AND: case XOR:
			program_store[line_number - 1].dest_register    = rd;
			program_store[line_number - 1].first_reg_val     = rs;
			program_store[line_number - 1].second_reg_val    = rt;
			break;

		  //-----------------------------------
		  // I-type 2-reg + immediate:
		  case ADDI: case SUBI: case MULI:
		  case ORI:  case ANDI: case XORI:
		  case LDW:  case STW:
			program_store[line_number - 1].dest_register    = rt;
			program_store[line_number - 1].first_reg_val     = rs;
			program_store[line_number - 1].immediate         = imm;
			break;

		  //-----------------------------------
		  // Branches with two regs + offset
		  case BEQ:
			program_store[line_number - 1].first_reg_val     = rs;
			program_store[line_number - 1].second_reg_val    = rt;
			program_store[line_number - 1].immediate         = imm;
			break;

		  // Branch-zero: 1 reg + offset
		  case BZ:
			program_store[line_number - 1].first_reg_val     = rs;
			program_store[line_number - 1].immediate         = imm;
			break;

		  //-----------------------------------
		  // Jump-register (uses only rs)
		  case JR:
			program_store[line_number - 1].first_reg_val     = rs;
			break;

		  //-----------------------------------
		  // HALT, NOP, EOP have no operands
		  case HALT:
		  case NOP:
		  case EOP:
			// nothing to fill
			break;

		  //-----------------------------------
		  default:
			if (mode == DEBUG) {
			  printf("Line %d: opcode 0x%X not a valid instruction\n",
					 line_number, opcode);
			}
			exit(EXIT_FAILURE);
		}
		
	}
	
	// Mark the end of the trace file in program_store
	program_store[line_number] = empty;
	program_store[line_number].instruction = EOP;
	
	pc = -1; // will be incremented first thing to pc=0 AKA the first trace file line
	
	
	
	
	if (functional_mode == NO_PIPE){
		for (pc = 0; pc <= line_number; pc++){
			//DEBUG: print each binary string
			if ((mode == DEBUG) && (rawHex_array[pc] > 0x0)) {
				printf("\n\n-------------------------------------------------------\n");
				printf("\n---Line %d---\n", pc + 1);
			   //  printf("Binary: %u\n", program_store[pc]);
				printf("Hex Number:\t\t0x%X\n", rawHex_array[pc]);
				printf("Instruction:\t\t0x%X, %d\n", program_store[pc].instruction, program_store[pc].instruction);
				printf("Destination register:\t%d\n", program_store[pc].dest_register);
				printf("1st source register:\t%d\n", program_store[pc].first_reg_val);
				if (opcode <= 0xB && opcode % 2 == 0) printf ("2nd source register:\t\t%d\n\n", program_store[pc].second_reg_val);
				else printf("Immediate value:   %6d\n\n", (int16_t)program_store[pc].immediate);
			}
			
			if (opcode_master(program_store[pc])){
				if(!was_jrfunc_for_nopipe)
					pc+=2;
				else if (was_jrfunc_for_nopipe)
					was_jrfunc_for_nopipe = 0;
			}
			cycle_counter += 5; // 5 cycles per instruction
			
			if (!was_control_flow && program_store[pc].instruction == HALT){
				end_program();
			}
		}
	}
	
	else if ((functional_mode == NO_FWD) || (functional_mode == FWD)){
		while (1) {
			// if a new instruction is added to the pipeline 
			// in the previous iteration of the while loop,
			// then get a NEW new instruction from the trace file.
			if (newInstAdded){
				if (mode == DEBUG) printf("Loading new line from trace file\n\n");
				
				pc++;
				
				if (program_store[pc].instruction == EOP){
					pc--;
					newinst = empty;
					end_of_fetch = true;
				}	
				
				else {
					newinst = program_store[pc];
					newInstAdded = false;
				}
				
			}


			// Array of decodedLines which serves as pipes
			decodedLine *slots[5] = {&pipe.pipe1, &pipe.pipe2, &pipe.pipe3, &pipe.pipe4, &pipe.pipe5};

			// 3 decodedLine variables to hold the line that is in a particular stage
			decodedLine *inIF = NULL, *inID = NULL, *inEX = NULL, *inMEM = NULL, *inWB = NULL;
			int inIFindex = 0;
			int inIDindex = 0;
			int inEXindex = 0;
			int inMEMindex = 0;
			int inWBindex = 0;
			

			// Re-check which lines are in which stages
			for (int i = 0; i < 5; i++) {
				if (slots[i]->pipe_stage == 1) {
					inIF = slots[i];
					inIFindex = i;
				}
				
				if (slots[i]->pipe_stage == 2) {
					inID = slots[i];
					inIDindex = i;
				}
				
				if (slots[i]->pipe_stage == 3) {
					inEX = slots[i];
					inEXindex = i;
				}
				
				if (slots[i]->pipe_stage == 4) {
					inMEM = slots[i];
					inMEMindex = i;
				}
				
				if (slots[i]->pipe_stage == 5) {
					inWB = slots[i];
					inWBindex = i;
				}
			}


			// FORWARDING 
			if (inID && inIF && findHazard(inID, inIF) && (functional_mode == FWD)) { // Checking for "IF-ID" hazards, effectively one less than an ID-MEM hazard
				hazard = true;
				cycle_counter++;
				
				// DEBUG
				if (mode == DEBUG) printf("\n\n\n\nStall at cycle %d: IF-ID hazard detected\n\n\n\n", cycle_counter);

				// Iterate through pipes and stall as appropriate
				for (int i = 0; i < 5; i++) {
					if (slots[i]->pipe_stage > 1 && slots[i]->pipe_stage < 5) { // The secondary difference is pushing ID stages until MEM compared to EX until WB
						if (slots[i]->pipe_stage == 3) {
							if(opcode_master(*slots[i])){
								*slots[inIFindex] = empty;
								*slots[inIDindex] = empty;
							}
						}
						slots[i]->pipe_stage++;
					}
					// Write-back logic once a line is pushed through its respective pipe
					else if (slots[i]->pipe_stage == 5){
						if (mode == DEBUG) printf("\n\nWriting back data from pipe %d\n\n", i+1);
						if (halt_executed && (slots[i]->instruction == HALT)){
							cycle_counter--;
							end_program();
						}
						
						*slots[i] = empty;
					}
					
					// Load a new instruction in the pipe
					if (!end_of_fetch && (slots[i]->pipe_stage == 0) && !newInstAdded){
						// determine if there's an empty 
						bool already_have_fetch_inst = 0;
						for (int j=0; j<5; j++) {
							if (slots[j]->pipe_stage == 1)
								already_have_fetch_inst = 1;
						}
							
						if ((already_have_fetch_inst == 0) && (!was_control_flow)){
							newinst.pipe_stage = 1;
							*slots[i] = newinst;
							newInstAdded = true;
							if (mode == DEBUG) printf("New instruction added to pipeline\n\n");
						}
						
						else if ((already_have_fetch_inst == 0) && (was_control_flow)){
							newinst = program_store[pc];
							newinst.pipe_stage = 1;
							*slots[i] = newinst;
							newInstAdded = true;
							if (mode == DEBUG) printf("New instruction added to pipeline\n\n");
						}
					}
					
				}
			}


			// ID-EX hazard handling
			if (inID && inEX && findHazard(inEX, inID) && functional_mode == NO_FWD) {
				hazard = true;
				cycle_counter++;
				total_stalls++;
				
				// DEBUG
				if (mode == DEBUG) printf("\n\n\n\nStall at cycle %d: EX-ID hazard detected\n\n\n\n", cycle_counter);

				// Iterate through pipes and stall as appropriate
				for (int i = 0; i < 5; i++) {
					if (slots[i]->pipe_stage > 2 && slots[i]->pipe_stage < 5) {
						if (slots[i]->pipe_stage == 3) {
							if(opcode_master(*slots[i])){
								*slots[inIFindex] = empty;
								*slots[inIDindex] = empty;
							}
						}
						slots[i]->pipe_stage++;
					}
					// Write-back logic once a line is pushed through its respective pipe
					else if (slots[i]->pipe_stage == 5){
						if (mode == DEBUG) printf("\n\nWriting back data from pipe %d\n\n", i+1);
						if (halt_executed && (slots[i]->instruction == HALT)){
							cycle_counter--;
							end_program();
						}
						
						*slots[i] = empty;
					}
					
					// Load a new instruction in the pipe
					if (!end_of_fetch && (slots[i]->pipe_stage == 0) && !newInstAdded){
						// determine if there's an empty 
						bool already_have_fetch_inst = 0;
						for (int j=0; j<5; j++) {
							if (slots[j]->pipe_stage == 1)
								already_have_fetch_inst = 1;
						}
							
						if ((already_have_fetch_inst == 0) && (!was_control_flow)){
							newinst.pipe_stage = 1;
							*slots[i] = newinst;
							newInstAdded = true;
							if (mode == DEBUG) printf("New instruction added to pipeline\n\n");
						}
						
						else if ((already_have_fetch_inst == 0) && (was_control_flow)){
							newinst = program_store[pc];
							newinst.pipe_stage = 1;
							*slots[i] = newinst;
							newInstAdded = true;
							if (mode == DEBUG) printf("New instruction added to pipeline\n\n");
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


				//DEBUG: print each binary string
				if ((mode == DEBUG) && (rawHex_array[pc] > 0x0)) {
					printf("---Line %d---\n", pc + 1);
				   //  printf("Binary: %u\n", program_store[pc]);
					printf("Hex Number:\t\t0x%X\n", rawHex_array[pc]);
					printf("Instruction:\t\t0x%X, %d\n", program_store[pc].instruction, program_store[pc].instruction);
					printf("Destination register:\t%d\n", program_store[pc].dest_register);
					printf("1st source register:\t%d\n", program_store[pc].first_reg_val);
					if (opcode <= 0xB && opcode % 2 == 0) printf ("2nd source register:\t\t%d\n\n", program_store[pc].second_reg_val);
					else printf("Immediate value:   %6d\n\n", (int16_t)program_store[pc].immediate);
				}
			}


			// Re-check which lines are in which stages
			for (int i = 0; i < 5; i++) {
				if (slots[i]->pipe_stage == 1) {
					inIF = slots[i];
					inIFindex = i;
				}
				
				if (slots[i]->pipe_stage == 2) {
					inID = slots[i];
					inIDindex = i;
				}
				
				if (slots[i]->pipe_stage == 3) {
					inEX = slots[i];
					inEXindex = i;
				}
				
				if (slots[i]->pipe_stage == 4) {
					inMEM = slots[i];
					inMEMindex = i;
				}
				
				if (slots[i]->pipe_stage == 5) {
					inWB = slots[i];
					inWBindex = i;
				}
			}


			// memory access instructions - MEM-ID hazard handling
			if (inID && inMEM && findHazard(inMEM, inID) && functional_mode == NO_FWD) {
				hazard = true;
				cycle_counter++;
				total_stalls++;
				
				// DEBUG
				if (mode == DEBUG) printf("\n\n\n\nStall at cycle %d: MEM-ID hazard detected\n\n\n\n", cycle_counter);
				
				// Iterate through pipes and stall as appropriate
				for (int i = 0; i < 5; i++) {
					if (slots[i]->pipe_stage > 2 && slots[i]->pipe_stage < 5) {
						if (slots[i]->pipe_stage == 3) {
							if(opcode_master(*slots[i])){
								*slots[inIFindex] = empty;
								*slots[inIDindex] = empty;
							}
						}
						slots[i]->pipe_stage++;
					}
					// Write-back logic once a line is pushed through its respective pipe
					else if (slots[i]->pipe_stage == 5){
						if (mode == DEBUG) printf("\n\nWriting back data from pipe %d\n\n", i+1);
						if (halt_executed && (slots[i]->instruction == HALT)){
							cycle_counter--;
							end_program();
						}
						
						*slots[i] = empty;
					}

					// Load a new instruction in the pipe
					if (!end_of_fetch && (slots[i]->pipe_stage == 0) && !newInstAdded){
						// determine if there's an empty 
						bool already_have_fetch_inst = 0;
						for (int j=0; j<5; j++) {
							if (slots[j]->pipe_stage == 1)
								already_have_fetch_inst = 1;
						}
							
						if ((already_have_fetch_inst == 0) && (!was_control_flow)){
							newinst.pipe_stage = 1;
							*slots[i] = newinst;
							newInstAdded = true;
							if (mode == DEBUG) printf("New instruction added to pipeline\n\n");
						}
						
						else if ((already_have_fetch_inst == 0) && (was_control_flow)){
							newinst = program_store[pc];
							newinst.pipe_stage = 1;
							*slots[i] = newinst;
							newInstAdded = true;
							if (mode == DEBUG) printf("New instruction added to pipeline\n\n");
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


				//DEBUG: print each binary string
				if ((mode == DEBUG) && (rawHex_array[pc] > 0x0)) {
					printf("---Line %d---\n", pc + 1);
				   //  printf("Binary: %u\n", program_store[pc]);
					printf("Hex Number:\t\t0x%X\n", rawHex_array[pc]);
					printf("Instruction:\t\t0x%X, %d\n", program_store[pc].instruction, program_store[pc].instruction);
					printf("Destination register:\t%d\n", program_store[pc].dest_register);
					printf("1st source register:\t%d\n", program_store[pc].first_reg_val);
					if (opcode <= 0xB && opcode % 2 == 0) printf ("2nd source register:\t\t%d\n\n", program_store[pc].second_reg_val);
					else printf("Immediate value:   %6d\n\n", (int16_t)program_store[pc].immediate);
				}
			}


			
			// No-hazard case
			if (!hazard) {
				cycle_counter++;
				
				// Execute on each instruction once they're in the EX stage
				for (int i = 0; i < 5; i++) {
					if (slots[i]->pipe_stage > 0 && slots[i]->pipe_stage < 5) {
						if (slots[i]->pipe_stage == 3) {
							if(opcode_master(*slots[i])){
								*slots[inIFindex] = empty;
								*slots[inIDindex] = empty;
							}
						}
						slots[i]->pipe_stage++;
					}
					// Print Write-backs
					else if (slots[i]->pipe_stage == 5){
						if (mode == DEBUG) printf("\n\nWriting back data from pipe %d\n\n", i+1);
						if (halt_executed && (slots[i]->instruction == HALT)){
							cycle_counter--;
							end_program();
						}
						
						*slots[i] = empty;
					}
					
					// Load a new instruction in the pipe
					if (!end_of_fetch && (slots[i]->pipe_stage == 0) && !newInstAdded){
						// determine if there's an empty 
						bool already_have_fetch_inst = 0;
						for (int j=0; j<5; j++) {
							if (slots[j]->pipe_stage == 1)
								already_have_fetch_inst = 1;
						}
							
						if ((already_have_fetch_inst == 0) && (!was_control_flow)){
							newinst.pipe_stage = 1;
							*slots[i] = newinst;
							newInstAdded = true;
							if (mode == DEBUG) printf("New instruction added to pipeline\n\n");
						}
						
						else if ((already_have_fetch_inst == 0) && (was_control_flow)){
							newinst = program_store[pc];
							newinst.pipe_stage = 1;
							*slots[i] = newinst;
							newInstAdded = true;
							if (mode == DEBUG) printf("New instruction added to pipeline\n\n");
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


				//DEBUG: print each binary string
				if ((mode == DEBUG) && (rawHex_array[pc] > 0x0)) {
					printf("---Line %d---\n", pc + 1);
				   //  printf("Binary: %u\n", program_store[pc]);
					printf("Hex Number:\t\t0x%X\n", rawHex_array[pc]);
					printf("Instruction:\t\t0x%X, %d\n", program_store[pc].instruction, program_store[pc].instruction);
					printf("Destination register:\t%d\n", program_store[pc].dest_register);
					printf("1st source register:\t%d\n", program_store[pc].first_reg_val);
					if (opcode <= 0xB && opcode % 2 == 0) printf ("2nd source register:\t\t%d\n\n", program_store[pc].second_reg_val);
					else printf("Immediate value:   %6d\n\n", (int16_t)program_store[pc].immediate);
				}
				
			}
			
			
			
			// PLEASE DO NOT GET RID OF THIS IT MAKES IT RUN FOREVER TRUST ME
			hazard = false;
			
			// once we've hit EOF *and* every stage is empty, we're done
			if (end_of_fetch
			  && pipe.pipe1.pipe_stage == 0
			  && pipe.pipe2.pipe_stage == 0
			  && pipe.pipe3.pipe_stage == 0
			  && pipe.pipe4.pipe_stage == 0
			  && pipe.pipe5.pipe_stage == 0) {
				end_program();   // prints stats + exit
			}
			
		}
	}


	
	if(mode == DEBUG) printf("No HALT instruction found- ending program");
	
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


int32_t StringToHex(char *hex_string) {
    uint32_t temp;
    int32_t signedInt;

    // Convert the hex string to a 32-bit unsigned integer
    temp = (uint32_t)strtoul(hex_string, NULL, 16);

    // Cast the unsigned value to signed (handles two's complement conversion)
    signedInt = (int32_t)temp;

	return signedInt;
}


void end_program() {
	
	print_stats();
	exit(EXIT_SUCCESS);
}


void print_stats() {
	
	printf("\n\n\n Registers Used:\n"); 
	printf("================================\n");
	bool atleast_one_register_printed = 0;
	for (int i = 0; i<NUM_REGISTERS; i++){
		if (register_used[i]){
			printf(" R[%d]: ",i);
			printf("%" PRIi32 "\n", registers[i]);
			atleast_one_register_printed = 1;
		}
	}
	if (!atleast_one_register_printed)
			printf("No registers used.");
	printf("================================\n");
	
	
	
	printf("\n\n\n Memory Used:\n"); 
	printf("================================\n");
	bool atleast_one_memory_printed = 0;
	for (int i = 0; i<MEMORY_SIZE; i++){
		if (memory_used[i]){
			if (atleast_one_memory_printed) printf("--------------------------------\n");
			printf(" Address:   %" PRIi32 "\n Contents:  %d\n", i, memory[i]);
			atleast_one_memory_printed = 1;
		}
	}
	if (!atleast_one_memory_printed)
			printf("No memory addresses used.\n");
	printf("================================\n");
	
	
	
    printf("\n\n\n Instruction Count Statistics:\n"); 
	printf("================================\n");
    printf(" Total Instructions:	%d\n", total_inst_count);
	printf("--------------------------------\n");
    printf(" R-Type:		%d\n", rtype_count);
    printf(" I-Type:		%d\n", itype_count);
	printf("--------------------------------\n");
    printf(" Arithmetic:		%d\n", arith_count);
    printf(" Logical:		%d\n", logic_count);
    printf(" Memory Access:		%d\n", memacc_count);
    printf(" Control Flow:		%d\n", cflow_count);
	printf("--------------------------------\n");
	printf(" Cycles:		%d\n", cycle_counter);
	printf("--------------------------------\n");
	printf(" Program Counter:	%d\n", pc);
	printf("================================\n");
	
	return;
}


bool opcode_master(decodedLine line) {

	rtype = 0;
	was_control_flow = 0;
	

	
    switch(line.instruction) {	
		// Arithmetic Instructions:
		{
		case ADD:
			if (mode == DEBUG) printf("\nADD Instruction Executed\n");
			addfunc(line.dest_register, line.first_reg_val, line.second_reg_val, false);
			break;
			
		case ADDI:
			if (mode == DEBUG) printf("\nADDI Instruction Executed\n");
			addfunc(line.dest_register, line.first_reg_val, line.immediate, true);
			break;
			
		case SUB:
			if (mode == DEBUG) printf("\nSUB Instruction Executed\n");
			subfunc(line.dest_register, line.first_reg_val, line.second_reg_val, false);
			break;
			
		case SUBI:
			if (mode == DEBUG) printf("\nSUBI Instruction Executed\n");
			subfunc(line.dest_register, line.first_reg_val, line.immediate, true);
			break;
			
		case MUL:
			if (mode == DEBUG) printf("\nMUL Instruction Executed\n");
			mulfunc(line.dest_register, line.first_reg_val, line.second_reg_val, false);
			
			break;
			
		case MULI:
			if (mode == DEBUG) printf("\nMULI Instruction Executed\n");
			mulfunc(line.dest_register, line.first_reg_val, line.immediate, true);
			break;
		}
		
		
		// Logical Instructions:
		{
		case OR:
			if (mode == DEBUG) printf("\nOR Instruction Executed\n");
			orfunc(line.dest_register, line.first_reg_val, line.second_reg_val, false);
			break;
			
		case ORI:
			if (mode == DEBUG) printf("\nORI Instruction Executed\n");
			orfunc(line.dest_register, line.first_reg_val, line.immediate, true);
			break;
			
		case AND:
			if (mode == DEBUG) printf("\nAND Instruction Executed\n");
			andfunc(line.dest_register, line.first_reg_val, line.second_reg_val, false);
			break;
			
		case ANDI:
			if (mode == DEBUG) printf("\nANDI Instruction Executed\n");
			andfunc(line.dest_register, line.first_reg_val, line.immediate, true);
			break;
			
		case XOR:
			if (mode == DEBUG) printf("\nXOR Instruction Executed\n");
			xorfunc(line.dest_register, line.first_reg_val, line.second_reg_val, false);
			break;
			
		case XORI:
			if (mode == DEBUG) printf("\nXORI Instruction Executed\n");
			xorfunc(line.dest_register, line.first_reg_val, line.immediate, true);
			break;	
		}
		
		
		// Memory Access Instructions:
		{
		case LDW:
			if (mode == DEBUG) printf("\nLDW Instruction Executed\n");
			ldwfunc(line.dest_register, line.first_reg_val, line.immediate);
			break;
			
		case STW:
			if (mode == DEBUG) printf("\nSTW Instruction Executed\n");
			stwfunc(line.dest_register, line.first_reg_val, line.immediate);
			break;
		}
		
		
		// Control Flow Instructions:
		{
		case BZ:
			if (mode == DEBUG) printf("\nBZ Instruction Executed\n");
			bzfunc(line.first_reg_val, line.immediate);
			break;
			
		case BEQ:
			if (mode == DEBUG) printf("\nBEQ Instruction Executed\n");
			beqfunc(line.first_reg_val, line.second_reg_val, line.immediate);
			break;
			
		case JR:
			if (mode == DEBUG) printf("\nJR Instruction Executed\n");
			jrfunc(line.first_reg_val);
			break;
			
		case HALT:
			if (mode == DEBUG) printf("HALT INSTRUCTION EXECUTED: FINISHING PROGRAM...\n\n\n\n\n");
			haltfunc();
			break;
		}
		
	
		default:
			if (opcode == NOP) {
				if (mode == DEBUG) 
					printf("\nNOP Instruction Executed\n");
			}
			else {
				if (mode == DEBUG)
					printf("Error: Unknown opcode 0x%02X. Exiting.\n", line.instruction);
					
				exit(EXIT_FAILURE);
			}
			
			
			
	}
	
	
	// Unless control flow instruction modified pc directly, increment by default
	if (was_control_flow == 0)
		return false;
	
	else
		return true;
}


void addfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate) {
	if(mode == DEBUG) printf(
            "[DEBUG] addfunc called with dest=%" PRIi32
            ", src1=%" PRIi32
            ", src2=%" PRIi32
            ", is_immediate=%d\n",
            dest,
            src1,
            src2,
            is_immediate
        );
	
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[(int)src1];
    int32_t val2 = is_immediate ? src2 : registers[(int)src2]; // sign-extend imm
    registers[(int)dest] = val1 + val2;
	register_used[(int)dest] = 1;
	register_used[(int)src1] = 1;
	if (rtype) register_used[(int)src2] = 1;

    arith_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void subfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate) {
	if(mode == DEBUG) printf(
            "[DEBUG] subfunc called with dest=%" PRIi32
            ", src1=%" PRIi32
            ", src2=%" PRIi32
            ", is_immediate=%d\n",
            dest,
            src1,
            src2,
            is_immediate
        );
		
		
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[(int)src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[(int)src2];
    registers[(int)dest] = val1 - val2;
	register_used[(int)dest] = 1;
	register_used[(int)src1] = 1;
	if (rtype) register_used[(int)src2] = 1;

    arith_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void mulfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate) {
	if(mode == DEBUG) printf(
            "[DEBUG] mulfunc called with dest=%" PRIi32
            ", src1=%" PRIi32
            ", src2=%" PRIi32
            ", is_immediate=%d\n",
            dest,
            src1,
            src2,
            is_immediate
        );
		
		
	if (!is_immediate) rtype = 1;
	int32_t val1 = registers[(int)src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[(int)src2];
    registers[(int)dest] = val1 * val2;
	register_used[(int)dest] = 1;
	register_used[(int)src1] = 1;
	if (rtype) register_used[(int)src2] = 1;

    arith_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void orfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate) {
	if(mode == DEBUG) printf(
            "[DEBUG] orfunc called with dest=%" PRIi32
            ", src1=%" PRIi32
            ", src2=%" PRIi32
            ", is_immediate=%d\n",
            dest,
            src1,
            src2,
            is_immediate
        );
		
		
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[(int)src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[(int)src2];
    registers[(int)dest] = val1 | val2;
	register_used[(int)dest] = 1;
	register_used[(int)src1] = 1;
	if (rtype) register_used[(int)src2] = 1;

    logic_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void andfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate) {
	if(mode == DEBUG) printf(
            "[DEBUG] andfunc called with dest=%" PRIi32
            ", src1=%" PRIi32
            ", src2=%" PRIi32
            ", is_immediate=%d\n",
            dest,
            src1,
            src2,
            is_immediate
        );
		
		
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[(int)src2];
    registers[(int)dest] = val1 & val2;
	register_used[(int)dest] = 1;
	register_used[(int)src1] = 1;
	if (rtype) register_used[(int)src2] = 1;

    logic_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void xorfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate) {
	if(mode == DEBUG) printf(
            "[DEBUG] xorfunc called with dest=%" PRIi32
            ", src1=%" PRIi32
            ", src2=%" PRIi32
            ", is_immediate=%d\n",
            dest,
            src1,
            src2,
            is_immediate
        );
		
		
	if (!is_immediate) rtype = 1;
    int32_t val1 = registers[(int)src1];
    int32_t val2 = is_immediate ? (int16_t)src2 : registers[(int)src2];
    registers[(int)dest] = val1 ^ val2;
	register_used[(int)dest] = 1;
	register_used[(int)src1] = 1;
	if (rtype) register_used[(int)src2] = 1;

    logic_count++;
    if (is_immediate)
        itype_count++;
    else
        rtype_count++;

    total_inst_count++;
}


void ldwfunc(int32_t rt, int32_t rs, int32_t imm) {
	
	if(mode == DEBUG) printf(
            "[DEBUG] ldwfunc called with rt=%" PRIi32
            ", rs=%" PRIi32
            ", imm=%" PRIi32 "\n",
            rt,
            rs,
            imm
        );
	
    int32_t addr = registers[(int)rs] + (int16_t)imm;
	/*
    if (addr % 4 != 0 || addr / 4 < 0 || addr / 4 >= MEMORY_SIZE) {
        printf("Memory load error: invalid address 0x%X\n", addr);
        exit(EXIT_FAILURE);
    }*/

    registers[(int)rt] = memory[((int)addr % MEMORY_SIZE)];
	
	memory_used[((int)addr % MEMORY_SIZE)] = 1;
	register_used[(int)rt] = 1;
	register_used[(int)rs] = 1;
	
    memacc_count++;
    itype_count++;
    total_inst_count++;
}


void stwfunc(int32_t rt, int32_t rs, int32_t imm) {
	
	
		
	if(mode == DEBUG) printf(
            "[DEBUG] stwfunc called with rt=%" PRIi32
            ", rs=%" PRIi32
            ", imm=%" PRIi32 "\n",
            rt,
            rs,
            imm
        );
	
    int32_t addr = registers[(int)rs] + (int16_t)imm;
	
	/*
    if (addr % 4 != 0 || addr / 4 < 0 || addr / 4 >= MEMORY_SIZE) {
        if (mode == DEBUG) printf("Memory store error: invalid address 0x%X\n", addr);
        exit(EXIT_FAILURE);
    }*/
	
	memory[((int)addr % MEMORY_SIZE)] = registers[(int)rt];
	memory_used[((int)addr % MEMORY_SIZE)] = 1;
	
	register_used[(int)rt] = 1;
	register_used[(int)rs] = 1;

    memacc_count++;
    itype_count++;
    total_inst_count++;
}


void bzfunc(int32_t rs, int32_t imm) {
	
	if(mode == DEBUG) printf(
            "[DEBUG] bzfunc called with rs=%d, imm=%d (signed offset=%d), "
            "reg[%d]=%d, pc_before=%d, pc_target=%d\n",
            rs, imm, (int16_t)imm,
            rs, registers[rs],
            pc, pc+(int16_t)imm
        );
	
	
    cflow_count++;
    itype_count++;
    total_inst_count++;
	register_used[(int)rs] = 1;

    if (registers[(int)rs] == 0) {
		pc-=2;
        pc += (int16_t)imm;
        was_control_flow = 1;
    }
}


void beqfunc(int32_t rs, int32_t rt, int32_t imm) {
	
	if(mode == DEBUG) printf(
            "[DEBUG] beqfunc called with rs=%d, rt=%d, imm=%d (signed offset=%d)\n"
            "        reg[%d]=%d, reg[%d]=%d\n"
            "        pc_before=%d, pc_target=%d\n",
            rs, rt, imm, (int16_t)imm,
            rs, registers[(int)rs],
            rt, registers[(int)rt],
            pc, pc+(int16_t)imm
        );
	
	
    cflow_count++;
    itype_count++;
    total_inst_count++;
	register_used[(int)rs] = 1;
	register_used[(int)rt] = 1;

    if (registers[(int)rs] == registers[(int)rt]) {
		pc-=2;
        pc += (int16_t)imm;
        was_control_flow = 1;
    }
}


void jrfunc(int32_t rs) {
    cflow_count++;
    itype_count++;
    total_inst_count++;
	was_control_flow = 1;
	was_jrfunc_for_nopipe = 1;

    pc = registers[(int)rs];  // Assume PC holds instruction index, not byte address
	register_used[(int)rs] = 1;
}


void haltfunc() {
	
	cflow_count++;
	itype_count++;
	total_inst_count++;
	
	// Close the file
    fclose(file);
	
	halt_executed = true;
}