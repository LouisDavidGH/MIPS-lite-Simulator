/**
 * mips.h - Header file for a MIPS-lite simulation
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
 *				NORMAL:		
 *
 *				DEBUG:		
 *
 */



#ifndef _MIPS_H
#define _MIPS_H


// MIPS instruction specifications
#define ADDRESS_BITS 32
#define NUM_REGISTERS 32


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













/**
  * hex_to_binary_string() - 		
  *									 
  *									
  *									
  *
  * @param	*hex_string			
  * @param	*binary_string		
  *
  */
void hex_to_binary_string(const char *hex_string, char *binary_string);




void opcode_master(const char *binary_string);











#endif