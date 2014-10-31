//******************//
//Names: Matthew Burger, Frank Holler, Yi Zhang
//Date: 10/17/14
//*****************//


// ******************************************************************************************* //

#include "p24fj64ga002.h"
#include "keypad.h"
#include "lcd.h"

// ******************************************************************************************* //

void KeypadInitialize() {
	
	// TODO: Configure IOs and Change Notificaiton interrupt for keypad scanning. This 
	// configuration should ensure that if any key is pressed, a change notification interrupt 
	// will be generated.
    //Inputs pins that represent the columns
    TRISBbits.TRISB2 = 1;
    TRISBbits.TRISB3 = 1;
    TRISBbits.TRISB11 = 1;

    //enable pull up resistors
    CNPU1bits.CN6PUE = 1;
    CNPU1bits.CN7PUE = 1;
    CNPU1bits.CN15PUE = 1;

    //Enable change notification for the pins
    CNEN1bits.CN6IE = 1;
    CNEN1bits.CN7IE = 1;
    CNEN1bits.CN15IE = 1;


    AD1PCFGbits.PCFG4 = 1;
    AD1PCFGbits.PCFG5 = 1;
    
    //Pins to be used for the rows set as outputs
    TRISBbits.TRISB12 = 0;
    TRISBbits.TRISB13 = 0;
    TRISBbits.TRISB14 = 0;
    TRISBbits.TRISB15 = 0;

    //set open drain configuration for the output pins
    ODCBbits.ODB12 = 1;
    ODCBbits.ODB13 = 1;
    ODCBbits.ODB14 = 1;
    ODCBbits.ODB15 = 1;


    //set all outputs to be 0 to see if button is pressed
    LATB = (LATB & 0x0FFF)|(0x0<<12);


}

// ******************************************************************************************* //

char KeypadScan() {

	char key = -1;
        int flag = 0;   //check to see if more than one button is pressed
        int row = 0;    //variable used to check the rows
	// TODO: Implement the keypad scanning procedure to detect if exactly one button of the 
	// keypad is pressed. The function should return:
	//
	//      -1         : Return -1 if no keys are pressed.
	//      '0' - '9'  : Return the ASCII character '0' to '9' if one of the 
	//                   numeric (0 - 9) keys are pressed.
	//      '#'        : Return the ASCII character '#' if the # key is pressed. 
	//      '*'        : Return the ASCII character '*' if the * key is pressed. 
	//       -1        : Return -1 if more than one key is pressed simultaneously.
	// Notes: 
	//        1. Only valid inputs should be allowed by the user such that all invalid inputs 
	//           are ignored until a valid input is detected.
	//        2. The user must release all keys of the keypad before the following key press
	//           is processed. This is to prevent invalid keypress from being processed if the 
	//           users presses multiple keys simultaneously.
	//

        for (row = 0 ; row <= 3; row++) {
            DelayUs(1);
            if( row == 0) {
            //Check row 0
                 LATB = (LATB & 0x0FFF)|(0xE<<12);  //Setting to check the first row

                 DelayUs(5000); //Debouncing the buttons
                if(PORTBbits.RB2 == 0 && PORTBbits.RB3 == 1 && PORTBbits.RB11 == 1) {
                    key = '1';
                    flag++;
                }
                else if(PORTBbits.RB2 == 1 && PORTBbits.RB3 == 0 && PORTBbits.RB11 == 1) {
                    key = '2';
                    flag++;
                }
                else if(PORTBbits.RB2 == 1 && PORTBbits.RB3 == 1 && PORTBbits.RB11 == 0){
                    key = '3';
                    flag++;
                }
            }

            else if(row == 1) {
                //Check row 1
                LATB = (LATB & 0x0FFF)|(0xD<<12);   //setting to check the second row

                 DelayUs(5000); //Debouncing the buttons
                if(PORTBbits.RB2 == 0 && PORTBbits.RB3 == 1 && PORTBbits.RB11 == 1){
                    key = '4';
                    flag++;
                }
                else if(PORTBbits.RB2 == 1 && PORTBbits.RB3 == 0 && PORTBbits.RB11 == 1){
                    key = '5';
                    flag++;
                }
                else if(PORTBbits.RB2 == 1 && PORTBbits.RB3 == 1 && PORTBbits.RB11 == 0){
                    key = '6';
                    flag++;
                }
            }

            else if(row == 2) {
                //Check row 2
                LATB = (LATB & 0x0FFF)|(0xB<<12);   //setting to check the third row

                 DelayUs(5000); //Debouncing the buttons
                if(PORTBbits.RB2 == 0 && PORTBbits.RB3 == 1 && PORTBbits.RB11 == 1){
                    key = '7';
                    flag++;
                }
                else if(PORTBbits.RB2 == 1 && PORTBbits.RB3 == 0 && PORTBbits.RB11 == 1){
                    key = '8';
                    flag++;
                }
                else if(PORTBbits.RB2 == 1 && PORTBbits.RB3 == 1 && PORTBbits.RB11 == 0){
                    key = '9';
                    flag++;
                }


            }

            else if(row == 3) {
                //Check row 3
                LATB = (LATB & 0x0FFF)|(0x7<<12);   //setting to check the last row

                 DelayUs(5000); //Debouncing the buttons
                if(PORTBbits.RB2 == 0 && PORTBbits.RB3 == 1 && PORTBbits.RB11 == 1){
                    key = '*';
                    flag++;
                }
                else if(PORTBbits.RB2 == 1 && PORTBbits.RB3 == 0 && PORTBbits.RB11 == 1){
                    key = '0';
                    flag++;
                }
                else if(PORTBbits.RB2 == 1 && PORTBbits.RB3 == 1 && PORTBbits.RB11 == 0){
                    key = '#';
                    flag++;
                }

            }

        }


        //check to see if more than one button is pressed or no button is pressed
        //flag should only be 1 if ONLY one button is pressed
        if (flag!=1)
            key=-1;

        LATB &= 0x0FFF; //Clear the rows to start checking if a button is pressed again
        while(PORTBbits.RB2 == 0);  //Wait for button to be released
        while(PORTBbits.RB3 == 0);  //Wait for button to be released
        while(PORTBbits.RB11 == 0); //Wait for button to be released

        return key;
}

// ******************************************************************************************* //
