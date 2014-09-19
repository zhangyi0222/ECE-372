// ******************************************************************************************* //
//  Yi Zhang
//  9/19/2014
// File:         lab0.c   
// Date:         08-20-2010
// Authors:      Roman Lysecky 
// Revised:		 08-27-2013 (R. Thamvichai)
//
// Description:  Software code for Lab 0 assignment for ECE 372 Fall 2010. Sample code toggles 
//               a user specified LED on the Microchip 16-bit 28-pin starter board. The user
//               specified LED is received using the PIC's UART.
//
// Requirements: This softwrae code requires the MPLAB C30 Compiler or MPLAB XC16 (MPLAB C Compiler  
//               for PIC24 MCUs) 
//
// Credits:      Software code based upon sample code provided with Microchip 16-bit 28-pin 
//               Development board.
//
// ******************************************************************************************* //

// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

#include "p24fj64ga002.h"
#include <stdio.h>

// ******************************************************************************************* //
// Configuration bits for CONFIG1 settings. 
//
// Make sure "Configuration Bits set in code." option is checked in MPLAB.
// This option can be set by selecting "Configuration Bits..." under the Configure
// menu in MPLAB.
//
// These settings are appropriate for debugging the PIC microcontroller. If you need to 
// program the PIC for standalone operation, change the COE_ON option to COE_OFF.

_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & 	
          BKBUG_ON & COE_ON & ICS_PGx1 & 
          FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )


// ******************************************************************************************* //
// Configuration bits for CONFIG2 settings.
// Make sure "Configuration Bits set in code." option is checked in MPLAB. 
// This option can be set by selecting "Configuration Bits..." under the Configure
// menu in MPLAB.

_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
          IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )

// ******************************************************************************************* //
// Defines to simply UART's baud rate generator (BRG) regiser
// given the osicllator freqeuncy and PLLMODE.

#define XTFREQ          7372800         	  // On-board Crystal frequency
#define PLLMODE         4               	  // On-chip PLL setting (Fosc)
#define FCY             (XTFREQ*PLLMODE)/2    // Instruction Cycle Frequency (Fosc/2)

#define BAUDRATE         115200       
#define BRGVAL          ((FCY/BAUDRATE)/16)-1 

// ******************************************************************************************* //

// Global variables can be accessed by both the main application code and the interrupt 
// service routines. Descriptions should be provided to clearly indicate what the variable
// is used for. Well chosen variables names are also benficial.

// Variable used to determine which LED will be toggled by the Timer 1 interrupt.
// This variable will be set by commands recieved from the UART.
// By default we will toggle LED4. 
int ledToToggle = 4;

// ******************************************************************************************* //

int main(void)
{
	// Varaible for character recived by UART.
	int receivedChar;

	//RPINR18 is a regsiter for selectable input mapping (see Table 10-2) for 
	// for UART1. U1RX is 8 bit value used to specifiy connection to which
	// RP pin. RP9 is used for this configuration. Physical Pin 18.
	RPINR18bits.U1RXR = 9;	

	// RPOR4 is a register for selctable ouput mapping (see Regsiter 1019) for
	// pins RP9 and RP8. The register for RP8 is assigned to 3 to connect 
	// the U1TX output for UART1 (see table 10-3). Physical Pin 17.
	RPOR4bits.RP8R = 3;		

	// Use LATB to write value to PORTB. This enables a Read-Modify-Write 
	// behavior used in the interrupt later. Set the current output to
	// 0 .
	LATB = 0;

	// TRISB controls direction for all PORTB pins, where 0 -> output, 1 -> input.
	// Configure RB15, RB14, RB13, and RB12 as outputs.
	TRISBbits.TRISB15 = 0;
	TRISBbits.TRISB14 = 0;
	TRISBbits.TRISB13 = 0;
	TRISBbits.TRISB12 = 0;

	// **TODO** SW1 of the 16-bit 28-pin Starter Board is connected to pin RB??. 
	// Assign the TRISB bit for this pin to configure this port as an input.

    TRISBbits.TRISB5 = 1;                                                      //set sw1(RB5) as input
	// Clear Timer value (i.e. current tiemr value) to 0
	TMR1 = 0;				

	// Set Timer 1's period value regsiter to value for 250ms. Please note 
	// T1CON's register settings below (internal Fosc/2 and 1:256 prescalar).
	// 
	//    Fosc     = XTFREQ * PLLMODE
	//             = 7372800 * 4
	//             = 29491200
	// 
	//    Fosc/2   = 29491200 / 2
	//             = 14745600
	//
	//    Timer 1 Freq = (Fosc/2) / Prescaler
	//                 = 14745600 / 256
	//                 = 57600
	//
	//    PR1 = 250 ms / (1 / (T1 Freq))
	//        = 250e-3 / (1 / 57600) 
	//        = 250e-3 * 57600
	//        = 14400 
	PR1 = 14400;			

	// Clear Timer 1 interrupt flag. This allows us to detect the 
	// first interupt.
	IFS0bits.T1IF = 0;		

	// Enable the interrupt for Timer 1
	IEC0bits.T1IE = 1;

	// Setup Timer 1 control register (T1CON) to:
 	//     TON           = 1     (start timer)
	//     TCKPS1:TCKPS2 = 11    (set timer prescaler to 1:256)
	//     TCS           = 0     (Fosc/2)
	T1CON = 0x8030;

	// Set UART1's baud rate generator register (U1BRG) to the value calculated above.
	U1BRG  = BRGVAL;

	// Set UART1's mode register to 8-bit data, no parity, 1 stop bit, enabled.
	//     UARTEN        = 1     (enable UART)
	//     PDSEL1:PDSEL0 = 00    (8-bit data, no parity)
	//     STSEL         = 0     (1 stop bit)
	U1MODE = 0x8000; 		

	// Set UART2's status and control register
	//     UTXISEL1:UTXISEL0 = 00    (U1TXIF set when character 
	//                                written to trasmit buffer)
	//     UTXEN             = 1     (trasnmit enabled)
	//     URXISEL1:URXISEL0 = 01    (U1RXIF set when any character 
	//                                is received in receive buffer)
	//     RIDLE             = 0     (Reciver is active)
	U1STA  = 0x0440; 		// Reset status register and enable TX & RX

	// Clear the UART RX interrupt flag. Althouhg we are not using a ISR for 
	// the UART receive, the UART RX interrupt flag can be used to deermine if 
	// we have recived a character from he UART. 
	IFS0bits.U1RXIF = 0;

	// printf by default is mapped to serial communication using UART1.
	// NOTES:
	//        1. You must specify a heap size for printf. This is required
	//           becuase printf needs to allocate its own memory, which is
	//           allocated on the heap. This can be set in MPLAB by:
	//           a.) Selecting Build Options...->Project from the Project menu.
	//           b.) Selecting the XC16 LINK (MPLABLINK30) Tab.
	//           c.) Entering the size of heap, e.g. 512, under Heap Size
	//        2. printf function is advanced and using printf may require 
	//           significant code size (6KB-10KB).   
	printf("\n\n\rkonnichiwa!\n\r");

	// Print a message requesting the user to select a LED to toggle.
	printf("Select LED to Toggle (4-7): ");

	// The main loop for your microcontroller should not exit (return), as
	// the program should run as long as the device is powered on. 
	while(1)
	{
		// **TODO** Modified the main loop of the software application such that 
		// whenever the SW1 is continuously pressed, the currently selected LED 
		// will blink twice as fast. When SW1 is released the LEDs will blink at 
		// the initially defined rate.


		// Use the UART RX interrupt flag to wait until we recieve a character.
		if(IFS0bits.U1RXIF == 1) {	

			// U1RXREG stores the last character received by the UART. Read this 
			// value into a local variable before processing.
			receivedChar = U1RXREG;

			// Echo the entered character so the user knows what they typed.
			printf("%c\n\r", receivedChar);

			// Check to see if the character value is between '4' and '7'. Be sure sure
			// use single quotation mark as the character '4' is not the same as the 
			// number 4.
			if( receivedChar <= '7' && receivedChar >= '4' ) {
				// Assign ledToToggle to the number corresponding to the number 
				// entered. We can do this by subtracting the value for 
				// the character '0'.
				ledToToggle = receivedChar - '0';

				// Print a confirmation message.
				printf("Toggling LED%d\n\r", ledToToggle);
			}
			else {
				// Display error message.
				printf("Invalid LED Selection!\n\r");
			}
    if(ledToToggle==4){
           LATBbits.LATB14 = 1;                                               //turn other leds off
           LATBbits.LATB13 = 1;
           LATBbits.LATB12 = 1;
       }
   if(ledToToggle==5){
           LATBbits.LATB15 = 1;                                               //turn other leds off
           LATBbits.LATB13 = 1;
           LATBbits.LATB12 = 1;
       }
   if(ledToToggle==6){
           LATBbits.LATB15 = 1;                                               //turn other leds off
           LATBbits.LATB14 = 1;
           LATBbits.LATB12 = 1;
       }
   if(ledToToggle==7){
           LATBbits.LATB15 = 1;                                               //turn other leds off
           LATBbits.LATB14 = 1;
           LATBbits.LATB13 = 1;
       }

			// Clear the UART RX interrupt flag to we can detect the reception
			// of another character.
			IFS0bits.U1RXIF = 0;	

			// Re-print the message requesting the user to select a LED to toggle.
			printf("Select LED to Toggle (4-7): ");
		}
	}

	return 0;
}

// ******************************************************************************************* //
// Defines an interrupt service routine that will execute whenever Timer 1's
// count reaches the specfied period value defined within the PR1 register.
// 
//     _ISR and _ISRFAST are macros for specifying interrupts that 
//     automatically inserts the proper interrupt into the interrupt vector 
//     table
//
//     _T1Interrupt is a macro for specifying the interrupt for Timer 1
//
// The functionality defined in an interrupt should be a minimal as possible
// to ensure additional interrupts can be processed. 
void _ISR _T1Interrupt(void)
{
	// Clear Timer 1 interrupt flag to allow another Timer 1 interrupt to occur.
	IFS0bits.T1IF = 0;		
	
	// Toggle the LED Specified by the User.
	LATB ^= ((0x1000)<<(7-ledToToggle));

            if(PORTBbits.RB5==0){                                          //when sw1 pressing
               PR1=14400/2;                                                //half PR1
               TMR1 = 0;                                                   //reset TMR1
            }
            else {                                     //when sw1 released
               PR1=14400;                                                  //reset PR1
               TMR1 = 0;                                                   //reset PR1
            }
}

// ******************************************************************************************* //
