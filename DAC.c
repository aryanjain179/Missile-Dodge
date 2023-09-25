// DAC.c
// This software configures DAC output
// Lab 6 requires 6 bits for the DAC
// Runs on LM4F120 or TM4C123
// Program written by: Avinash Bhaskaran, Aryan Jain
// Date Created: 3/6/17 
// Last Modified: 3/22/22 
// Lab number: 6
// Hardware connections
// Key 3-0 = PA5-2, 370 Hz(GF0), 329.6 Hz(E0), 277.2(DF0) Hz, 220 Hz (A7) | DAC on PB5-0

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data


// **************DAC_Init*********************
// Initialize 6-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x2;             // Turn on PortB clock
  while((SYSCTL_PRGPIO_R&0x2) == 0){};  // Wait until clock has stabilized
    
  GPIO_PORTB_DIR_R |= 0x3F;             // Set PB5-0 as digital outputs
  GPIO_PORTB_DEN_R |= 0x3F;
  GPIO_PORTB_DR8R_R |= 0x3F;            // Allow for more current
}

// **************DAC_Out*********************
// output to DAC
// Input: 6-bit data, 0 to 63 
// Input=n is converted to n*3.3V/63
// Output: none
void DAC_Out(uint32_t data){
  GPIO_PORTB_DATA_R = data;             // This is the purpose of the DAC
}
