// ADC.c
// Runs on TM4C123
// Provide functions that initialize ADC0
// Last Modified: 4/12/22  
// Student names: Avinash Bhaskaran, Aryan Jain
// Last modification date: 4/12/22
// Labs 8 and 9 specify PD2
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function using PD2 
// Input: none
// Output: none
void ADC_Init(void){ 
SYSCTL_RCGCADC_R |= 0x0001;   // 1) activate ADC0 
                                  // 2) activate clock for Port D
  SYSCTL_RCGCGPIO_R |= 0x08;
  while((SYSCTL_PRGPIO_R&0x08) != 0x08){};
  GPIO_PORTD_DIR_R &= ~0x04;      // 3) make PD2 input
  GPIO_PORTD_AFSEL_R |= 0x04;     // 4) enable alternate function on PD2
  GPIO_PORTD_DEN_R &= ~0x04;      // 5) disable digital I/O on PD2
  GPIO_PORTD_AMSEL_R |= 0x04;     // 6) enable analog functionality on PD2
    
//  while((SYSCTL_PRADC_R&0x0001) != 0x0001){};    // good code, but not yet implemented in simulator


  ADC0_PC_R &= ~0xF;              // 7) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x0123;          // 8) Sequencer 3 is highest priority
  ADC0_ACTSS_R &= ~0x0008;        // 9) disable sample sequencer 3
  ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
  ADC0_SSMUX3_R &= ~0x000F;       // 11) clear SS3 field
  ADC0_SSMUX3_R += 5;             //    set channel
  ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
  ADC0_IM_R &= ~0x0008;           // 13) disable SS3 interrupts
  ADC0_SAC_R = 6;  
  ADC0_ACTSS_R |= 0x0008;         // 14) enable sample sequencer 3
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	// 1) initiate SS3
  // 2) wait for conversion done
  // 3) read result
  // 4) acknowledge completion
  uint32_t data;
  
  ADC0_PSSI_R = 0x8;
  while((ADC0_RIS_R&0x8) == 0){};
  data = ADC0_SSFIFO3_R&0xFFF;
  ADC0_ISC_R = 0x8;
  return data;
}


