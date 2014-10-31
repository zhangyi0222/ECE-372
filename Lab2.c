//******************//
//Names: Matthew Burger, Frank Holler, Yi Zhang
//Date: 10/17/14
//*****************//



// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

#include "p24fj64ga002.h"
#include <stdio.h>
#include "lcd.h"
#include "keypad.h"

// ******************************************************************************************* //
// Configuration bits for CONFIG1 settings. 
//
// Make sure "Configuration Bits set in code." option is checked in MPLAB.
//
// These settings are appropriate for debugging the PIC microcontroller. If you need to 
// program the PIC for standalone operation, change the COE_ON option to COE_OFF.

_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & 
		 BKBUG_ON & COE_ON & ICS_PGx1 & 
		 FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )

// ******************************************************************************************* //
// Configuration bits for CONFIG2 settings.
// Make sure "Configuration Bits set in code." option is checked in MPLAB.

_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
		 IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )

// ******************************************************************************************* //

// Varible used to indicate that the current configuration of the keypad has been changed,
// and the KeypadScan() function needs to be called.

volatile char scanKeypad;
volatile int state = 0;         //used to tell the state
volatile int i = 0;      //flag
volatile char key;
// ******************************************************************************************* //

int main(void)
{       //char keyholder;
    int k = 0, b=0;                         //variables used to iterate through for loops
        char holder[4];                     //holds the password in set mode to transfer to database
        char digits[4];                     //holds the password in program mode
        //char passWord[]="1234";
		char database[10][4];       //stores all the passwords into a database
		int database_row = 0;       //keeps track of how many passwords in database
        int flag = 0;                       //another flag used to see if password is good
	char key;
	
	// TODO: Initialize and configure IOs, LCD (using your code from Lab 1), 
	// UART (if desired for debugging), and any other configurations that are needed.
            //enable the change notification for the pins and enable the change notification in general

        T4CONbits.T32 = 1; //enables 32bit timer mode
        T4CONbits.TON = 0;
        T4CONbits.TCS=0;
        T4CONbits.TCKPS0 = 1; //sets the prescaler to be 256
        T4CONbits.TCKPS1 = 1;


        TMR4 = 0;
        TMR5 = 0;

        PR4 = 0xC1FF;       //Timer set to be 2 seconds
        PR5 = 0x1;

        IFS1bits.T5IF = 0;  //Sets the flag for the timer to be 0



	LCDInitialize();
	KeypadInitialize();
        IFS1bits.CNIF = 0;  //Sets the flag to 0
        IEC1bits.CNIE = 1;  //Enables change notification

	
	// TODO: Initialize scanKeypad variable.
        scanKeypad = 0;
        LCDPrintString("Enter");
		database[0][0] = '1';       //stores the default password
		database[0][1] = '2';
		database[0][2] = '3';
		database[0][3] = '4';
	while(1)
	{
		// TODO: Once you create the correct keypad driver (Part 1 of the lab assignment), write
		// the C program that use both the keypad and LCD drivers to implement the 4-digit password system.
//            switch(state) {
		switch(state) {

		//start of user mode. Waits for first button to be pressed
			case 0:
				LCDMoveCursor(1,0);
				if(scanKeypad == 1) {
					key = KeypadScan();
					if(key != '#' && key != -1) {
                                                LCDPrintChar(key);
						digits[0] = key;

						if(key == '*') {
							i = 1;      //set flag to indicate first char is *
						}
						state = 1;
					}
                                        else if(key == -1) {        //More than one key is pressed
						state = 0;
					}
					else {
						state = 4;
					}
					scanKeypad = 0;
                                        LATB &= 0x0FFF;             //check to see if button is pressed
				}
				break;

		//first button has been pressed and was not '#'. Waiting for second digit
			case 1:
				LCDMoveCursor(1,1);
				if(scanKeypad == 1) {
					key = KeypadScan();
					if(i == 1 && key == '*') {      //indicates 2 *s are pressed
						LCDClear();
						LCDPrintString("Set Mode");
						LCDMoveCursor(1,0);
						state = 5;
						i = 0;
                                                k = 0;
					}
                                        else if(key != '#' && key != -1 && key != '*' && i == 0) {
						LCDPrintChar(key);
						digits[1] = key;
						state = 2;
					}
					else if(key == -1) {
						state = 1;
					}
					else {
						state = 4;
					}
					scanKeypad = 0;
                                        LATB &= 0x0FFF;
				}
				break;

		//second button has been pressed and was a valid number. Waiting for third digit
			case 2:
				LCDMoveCursor(1,2);
				if(scanKeypad == 1) {
					key = KeypadScan();
					if(key != '#' && key != '*' && key != -1) {
						LCDPrintChar(key);
						digits[2] = key;
						state = 3;
					}
					else if(key == -1) {
						state = 2;
					}
                                        else {                  //if * or #, bad state
						state = 4;
					}
					scanKeypad = 0;
                                        LATB &= 0x0FFF;
				}
				break;

		//third button was pressed and was valid. Waiting for final digit
			case 3:
				LCDMoveCursor(1,3);
				if(scanKeypad == 1) {
					key = KeypadScan();
					if(key != '#' && key != '*' && key != -1) {
						LCDPrintChar(key);
						digits[3] = key;
						for(b = 0; b <= database_row; b++) {    //scan up to total passwords in database
                                                    flag=0;
							for(k = 0; k < 4; k++) {
								if(digits[k] == database[b][k])
                                                                {
                                                                    flag++;
								}
							}
                                                        if(flag==4)             //if all 4 characters match, stop looking
                                                            break;
						}
						if(flag == 4) {                 //go to good state
                                                    state = 6;
                                                    flag = 0;
                                                }
                                                else
                                                    state = 4;
					}
					else if(key == -1) {
						state = 3;
					}
					else {
						state = 4;
					}
					scanKeypad = 0;
                                        LATB &= 0x0FFF;
				}
				break;

		//"Bad" state. Invalid password was pressed while in user mode
			case 4:
				LCDClear();
				LCDPrintString("Bad");
				T4CONbits.TON = 1;              //start timer for 2 seconds
                                while(IFS1bits.T5IF != 1);
                                IFS1bits.T5IF = 0;
                                T4CONbits.TON = 0;
                                TMR4 = 0;
                                TMR5 = 0;
                                state = 0;
                                LCDClear();
                                LCDPrintString("Enter");
                                LATB &= 0x0FFF;
				break;

		//Program Mode. Enter five digits.
			case 5:
				if(scanKeypad == 1) {
					key = KeypadScan();
					if((key == '#' || key == '*') && k <= 4) {
                                            if (k<4)
                                                i = 1;              //set flag if # or * is seen for first 4 presses
                                            LCDPrintChar(key);
                                            k++;

					}
					else if(key == -1) {
						state = 5;
					}
					else if((key != '#' && key != '*') && k <= 4) {
						holder[k] = key;        //stores the first 4 characters pressed
                                                LCDPrintChar(key);
                                                k++;

					}
					scanKeypad = 0;
                                        LATB &= 0x0FFF;
                                        if(k > 4) {
                                            if(i == 0 && key == '#' ) {     //Last character in key should be #
                                                for(k=0; k<4; k++)
                                                    database[database_row+1][k]=holder[k];//set database to store password because valid
                                                state = 7;       //Move to valid state
                                                database_row++;  //Increment database_row because we have a new password
                                                k=0;
                                            }
                                            else {
						state = 8;  //Move to invalid state
						i = 0;
                                            }
                                        }
                                        
				}

				break;

		//"Good" state. A valid password was entered while in user mode.
			case 6:
				LCDClear();
				LCDPrintString("Good");
                                T4CONbits.TON = 1;
                                while(IFS1bits.T5IF != 1);  //Wait 2 seconds
                                IFS1bits.T5IF = 0;
                                T4CONbits.TON = 0;
                                TMR4 = 0;
                                TMR5 = 0;
                                state = 0;
				LCDClear();
                                LCDPrintString("Enter");
                                LATB &= 0x0FFF;
				break;

		//"Valid" state. A valid password was entered while in Program mode and was recorded.
			case 7:
				LCDClear();
				LCDPrintString("Valid");
                                T4CONbits.TON = 1;
                                while(IFS1bits.T5IF != 1);  //Wait 2 seconds
                                IFS1bits.T5IF = 0;
                                T4CONbits.TON = 0;
                                TMR4 = 0;
                                TMR5 = 0;
                                state = 0;
				LCDClear();
                                LCDPrintString("Enter");
                                LATB &= 0x0FFF;
				break;

		//"Invalid" state. An invalid password was entered while in Program mode.
			case 8:
				LCDClear();
				LCDPrintString("Invalid");
                                T4CONbits.TON = 1;
                                while(IFS1bits.T5IF != 1);  //Wait 2 seconds
                                IFS1bits.T5IF = 0;
                                T4CONbits.TON = 0;
                                TMR4 = 0;
                                TMR5 = 0;
                                state = 0;
				LCDClear();
                                LCDPrintString("Enter");
                                LATB &= 0x0FFF;
				break;
		}
	}
	return 0;
}

// ******************************************************************************************* //
// Defines an interrupt service routine that will execute whenever any enable
// input change notifcation is detected.
//
//     In place of _ISR and _ISRFAST, we can directy use __attribute__((interrupt))
//     to inform the compiler that this function is an interrupt.
//
//     _CNInterrupt is a macro for specifying the interrupt for input change
//     notification.
//
// The functionality defined in an interrupt should be a minimal as possible
// to ensure additional interrupts can be processed.
void __attribute__((interrupt,auto_psv)) _CNInterrupt(void)
{
        //DelayUs(5000);
	// TODO: Clear interrupt flag
	IFS1bits.CNIF = 0;
        //DelayUs(5000);
	// TODO: Detect if *any* key of the keypad is *pressed*, and update scanKeypad
	// variable to indicate keypad scanning process must be executed.
//        if(scanKeypad == 1 && (PORTBbits.RB2 == 0 || PORTBbits.RB3 == 0 || PORTBbits.RB11 == 0))
//            key = -1;
        if(PORTBbits.RB2 == 0 && T4CONbits.TON!=1)
            scanKeypad = 1;
	if(PORTBbits.RB3 == 0 && T4CONbits.TON!=1)
            scanKeypad = 1;
	if(PORTBbits.RB11 == 0 && T4CONbits.TON!=1)
            scanKeypad = 1;


}
// ******************************************************************************************* //
