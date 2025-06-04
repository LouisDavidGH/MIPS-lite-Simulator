/**
 * mips.h - Header file for a MIPS-lite simulation
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
 */



#ifndef _MIPS_H
#define _MIPS_H


// MIPS system specifications
#define ADDRESS_BITS 32
#define NUM_REGISTERS 32
#define MEMORY_SIZE 1024
#define CLOCK_SIZE 5

// Mode values
#define DEBUG_EXTRA 2
#define DEBUG 1
#define NORMAL 0


// Buffer sizes
#define HEX_STRING_LENGTH 8             // 8 hex characters = 32 bits
#define BINARY_STRING_LENGTH 32         // 32 binary characters
#define LINE_BUFFER_SIZE 20             // line buffer for reading lines

// Commands
#define ADD 0x00  // Opcode: 000000 
#define ADDI 0x01 // Opcode: 000001	
#define SUB 0x02  // Opcode: 000010
#define SUBI 0x03 // Opcode: 000011
#define MUL 0x04  // Opcode: 000100
#define MULI 0x05 // Opcode: 000101
#define OR 0x06   // Opcode: 000110
#define ORI 0x07  // Opcode: 000111
#define AND 0x08  // Opcode: 001000
#define ANDI 0x09 // Opcode: 001001
#define XOR 0x0A  // Opcode: 001010
#define XORI 0x0B // Opcode: 001011
#define LDW 0x0C  // Opcode: 001100
#define STW 0x0D  // Opcode: 001101
#define BZ 0x0E   // Opcode: 001110
#define BEQ 0x0F  // Opcode: 001111
#define JR 0x10   // Opcode: 010000
#define HALT 0x11 // Opcode: 010001

// Forward declaration of a struct named decoded_line_information,
// and creating an alias 'decodedLine' for easier use.
typedef struct decoded_line_information decodedLine;

// Forward declaration of a struct named pipeline_status_and_instruction,
// and creating an alias 'pipeline' for easier use.
typedef struct pipeline_status_and_instruction pipeline;

// Function prototype for handling instructions based on opcode.
// Takes a decodedLine struct representing the decoded instruction.
void opcode_master(decodedLine line);

// Function prototype to print program or pipeline statistics.
void print_stats();

// Function prototype to end the program execution, possibly cleanup.
void end_program();

// Arithmetic and logic functions with parameters:
// dest - destination register index,
// src1, src2 - source register indices,
// is_immediate - flag indicating whether the second operand is an immediate value.
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

// Performs addition operation.
void addfunc(int dest, int src1, int src2, bool is_immediate);

// Performs subtraction operation.
void subfunc(int dest, int src1, int src2, bool is_immediate);

// Performs multiplication operation.
void mulfunc(int dest, int src1, int src2, bool is_immediate);

// Performs bitwise OR operation.
void orfunc(int dest, int src1, int src2, bool is_immediate);

// Performs bitwise AND operation.
void andfunc(int dest, int src1, int src2, bool is_immediate);

// Performs bitwise XOR operation.
void xorfunc(int dest, int src1, int src2, bool is_immediate);

// Load word function: loads data into rt from memory address computed from rs + imm offset.
void ldwfunc(int rt, int rs, int imm);

// Store word function: stores data from rt into memory address computed from rs + imm offset.
void stwfunc(int rt, int rs, int imm);

// Branch if zero function: branches if register rs equals zero, using imm as offset.
void bzfunc(int rs, int imm);

// Branch if equal function: branches if registers rt and rs are equal, using imm as offset.
void beqfunc(int rt, int rs, int imm);

/*
    ┌────────┬──────┬─────────────────────────┐
    │ Opcode │  Rs  │      Unused (26-bit)    │
    ├────────┼──────┼─────────────────────────┤
    │  6 bit │ 5bit │         21 bits         │
    └────────┴──────┴─────────────────────────┘
    Used by: JR

*/
// Jump register function: jumps to the address contained in register rs.
void jrfunc(int rs);

/*
    ┌────────┬───────────────────────────────┐
    │ Opcode │         Unused (26-bit)       │
    ├────────┼───────────────────────────────┤
    │  6 bit │           26 bits             │
    └────────┴───────────────────────────────┘
    Used by: HALT
*/
// Halt function: stops program execution.
void haltfunc();

int32_t StringToHex(char *hex_string);
void print_line(decodedLine line, int index);
void print_registers();



#endif