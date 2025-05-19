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


// Mode values
#define DEBUG 1
#define NORMAL 0


// Buffer sizes
#define HEX_STRING_LENGTH 8             // 8 hex characters = 32 bits
#define BINARY_STRING_LENGTH 32         // 32 binary characters
#define LINE_BUFFER_SIZE 20             // line buffer for reading lines



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



typedef struct decoded_line_information decodedLine;

typedef struct pipeline_status_and_instruction pipeline;

void opcode_master(decodedLine line);


void print_stats();


void end_program();

void addfunc(int dest, int src1, int src2, bool is_immediate);

void subfunc(int dest, int src1, int src2, bool is_immediate);

void mulfunc(int dest, int src1, int src2, bool is_immediate);

void orfunc(int dest, int src1, int src2, bool is_immediate);

void andfunc(int dest, int src1, int src2, bool is_immediate);

void xorfunc(int dest, int src1, int src2, bool is_immediate);

void ldwfunc(int rt, int rs, int imm);

void stwfunc(int rt, int rs, int imm);

void bzfunc(int rs, int imm);

void beqfunc(int rt, int rs, int imm);

void jrfunc(int rs);

void haltfunc();





#endif