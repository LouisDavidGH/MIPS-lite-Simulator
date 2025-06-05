/**
 * mips.h - Header file for a MIPS-lite simulation
 *
 * @authors:    Evan Brown (evbr2@pdx.edu)
 * 				Louis-David Gendron-Herndon (loge2@pdx.edu)
 *				Ameer Melli (amelli@pdx.edu)
 *				Anthony Le (anthle@pdx.edu)
 *
 *
 * @date:       June 5, 2025
 * @version:    4.0
 *		
 *
 */



#ifndef _MIPS_H
#define _MIPS_H


// MIPS system specifications
#define ADDRESS_BITS 32
#define NUM_REGISTERS 32
#define MEMORY_SIZE 1024


// Mode values
#define DEBUG 1
#define NORMAL 0

// Functional Mode values
#define NO_PIPE 2
#define NO_FWD 3
#define FWD 4


// Buffer sizes
#define HEX_STRING_LENGTH 8             // 8 hex characters = 32 bits
#define BINARY_STRING_LENGTH 32         // 32 binary characters
#define LINE_BUFFER_SIZE 20             // line buffer for reading lines

// Pipeline stages
#define IF 1
#define ID 2
#define EX 3
#define MEM 4
#define WB 5
#define NUMPIPES 5

// Instruction opcode values
#define ADD 0x00 
#define ADDI 0x01 
#define SUB 0x02
#define SUBI 0x03
#define MUL 0x04
#define MULI 0x05
#define OR 0x06
#define ORI 0x07
#define AND 0x08
#define ANDI 0x09
#define XOR 0x0A
#define XORI 0x0B
#define LDW 0x0C
#define STW 0x0D
#define BZ 0x0E
#define BEQ 0x0F
#define JR 0x10
#define HALT 0x11

// Special opcode values
#define EOP 0xFA 	// "End Of Program" to mark the program_store[i] 
					// instruction AFTER the last valid instruction
#define NOP 0xFF	// "No OPeration" used in pipelining



typedef struct decoded_line_information decodedLine;

// Not sure if this is needed anymore
typedef struct pipe_main pipeline;

// Switch statement to complete the appropriate
// function based on the opcode
bool opcode_master(decodedLine line);

// true = (wr destination == rd source)
// else false
bool findHazard(const decodedLine *wr, const decodedLine *rd);

// Prints the used registers, used memory, and instruction stats
void print_stats();

// Runs print_stats() and ends the program
void end_program();

// (src1 reg value) + (src2 reg value) = (dest register value)
// src1 = rs
// if is_immediate, src2 is immediate instead of rt
// ^ Same goes for the rest of the arithmetic and logical functions
void addfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate);

// (src1 reg value) - (src2 reg value) = (dest register value)
void subfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate);

// (src1 reg value) * (src2 reg value) = (dest register value)
void mulfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate);

// (src1 reg value) | (src2 reg value) = (dest register value)
void orfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate);

// (src1 reg value) & (src2 reg value) = (dest register value)
void andfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate);

// (src1 reg value) ^ (src2 reg value) = (dest register value)
void xorfunc(int32_t dest, int32_t src1, int32_t src2, bool is_immediate);

// Load the value from rt into memory[(rs+imm)%1024]
void ldwfunc(int32_t rt, int32_t rs, int32_t imm);

// Store the value from memory[(rs+imm)%1024] into rt
void stwfunc(int32_t rt, int32_t rs, int32_t imm);

// If the value in rs = 0, add imm to PC
void bzfunc(int32_t rs, int32_t imm);

// If rt's value = rs's value, add imm to PC
void beqfunc(int32_t rt, int32_t rs, int32_t imm);

// Branches the to the value in register RS
void jrfunc(int32_t rs);

// Sets a flag that allows the program to end
void haltfunc();

int32_t StringToHex(char *hex_string);




#endif