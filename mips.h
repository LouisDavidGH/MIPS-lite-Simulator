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
















#endif