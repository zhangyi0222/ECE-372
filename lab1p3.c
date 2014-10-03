//******************//
//Names: Matthew Burger, Frank Holler, Yi Zhang
//Date: 10/3/14
//*****************//


// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

#include "p24fj64ga002.h"
#include <stdio.h>
#include "lcd.h"

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



volatile int state = 2; //state is used to determine where to go for the switch statement (starts at 2
                        //because that is the reset state and sets the intial time

//These represent the characters in the manner "m2m1:s2s1.f2f1"
/**********************************************************/
volatile unsigned char f1 = 0;
volatile unsigned char f2 = 0;
volatile unsigned char s1 = 0;
volatile unsigned char s2 = 0;
volatile unsigned char m1 = 0;
volatile unsigned char m2 = 0;
/**********************************************************/


//We use flags to indicate in the CN ISR that the button has pressed. Flag = 1
//means a button was pressed
/**********************************************************/
volatile int flag1 = 0;
volatile int flag2 = 0;
volatile int flag3 = 0;
volatile int flag4 = 0;
/**********************************************************/

int main(void)
{
	// ****************************************************************************** //

	// Set the switch to be digital
        AD1PCFGbits.PCFG4 = 1;  //Set the switch to be digital

	//Configures the LEDs to be outputs
        TRISAbits.TRISA0 = 0;   //Right (green/run)
        TRISAbits.TRISA1 = 0;   //Left (red/stop)


	//Set the green light to be on first and the red one to be off
        LATAbits.LATA0 = 1;     //Turn Right (green/run) one off
        LATAbits.LATA1 = 0;     //Turn Left (red/stop) one on


        //Giving the LEDs ODCs
        ODCAbits.ODA0 = 1;
        ODCAbits.ODA1 = 1;

        //Configuring switches to be inputs
        TRISBbits.TRISB2 = 1;   //switch not on board to be input connected to IO5. START/STOP BUTTON
        TRISBbits.TRISB5 = 1;   //switch on board to be input. RESET BUTTON

        //Enables the change notification for the switch on the board
        CNEN2bits.CN27IE = 1;       //Enables change notification for switch on board
        CNEN1bits.CN6IE = 1;        //Enables change notification for switch not on board
        IFS1bits.CNIF = 0;
        IEC1bits.CNIE = 1;

	// Gives the switch not on the board a pull up resistor
        CNPU1bits.CN6PUE = 1;

        //Timer 1 used for ISR to increment
        T1CONbits.TCS=0;
        T1CONbits.TCKPS0=1;
        T1CONbits.TCKPS1=1;
        T1CONbits.TON = 0;
        IFS0bits.T1IF = 0;
        IEC0bits.T1IE = 1;
        TMR1 = 0;
        PR1 = 575;  //timer 1 period value for 10ms
        

        LCDInitialize();
	while(1)
	{
//            mm = counter
		// TODO: For each distinct button press, alternate which
		// LED is illuminated (on).
            switch(state) {

                //Stopped and waiting for button to be pressed
                case 0:
                    LCDMoveCursor(0,0);
                    LCDPrintString("Stopped");
                    LATAbits.LATA0 = 1;     //Turn Right (green/run) one off
                    LATAbits.LATA1 = 0;     //Turn Left (red/stop) one on
                    break;

                //Running and updating time
                case 1:
                    LCDMoveCursor(0,0);
                    LCDPrintString("Running:");
                    LCDMoveCursor(1,0);
                    LATAbits.LATA0 = 0;     //Turn Right (green/run) one on
                    LATAbits.LATA1 = 1;     //Turn Left (red/stop) one off

                    //Prints the time
                    LCDMoveCursor(1,7);
                    LCDPrintChar(f1+'0');
                    LCDMoveCursor(1,6);
                    LCDPrintChar(f2+'0');

                    LCDMoveCursor(1,4);
                    LCDPrintChar(s1+'0');

                    LCDMoveCursor(1,3);
                    LCDPrintChar(s2+'0');

                    LCDMoveCursor(1,1);
                    LCDPrintChar(m1+'0');

                    LCDMoveCursor(1,0);
                    LCDPrintChar(m2+'0');
                    break;

                //Reset button is pressed and timer goes to 0
                case 2:

                    //Sets row 2 to be 0 on the LCD and resets all the values
                    LCDMoveCursor(1,0);
                    LCDPrintString("00:00.00");
                    f1 = 0;
                    f2 = 0;
                    s1 = 0;
                    s2 = 0;
                    m1 = 0;
                    m2 = 0;
                    state = 0;
                    break;


            }



	}
	return 0;
}


void __attribute__((interrupt,auto_psv)) _T1Interrupt(void){
    IFS0bits.T1IF = 0;
	if(f1==9 && f2==9) {                           //.99 and going to roll over to 1 second
            if(s2 == 5 && s1 == 9) {                   //:59 seconds and going to roll over to 1 minute
		if(m1 == 9) {                          //m1 is going to roll over to m2
                    m1 = 0;
                    m2 = m2 + 1;
		}

		else {                                 //m1 won't roll over to m2 so we increment
                    m1 = m1 + 1;
		}
                    s1 = 0;                               //reset the seconds to be 0, i.e. :00
                    s2 = 0;
		}

            else if(s1 == 9 && s2 !=9) {             //only s1 is rolling over so we reset s1 and increment s2
		s1 = 0;
		s2 = s2 +1;
            }

            else {                                   //nothing is rolling over in the seconds, so we increment s1
		s1 = s1 + 1;
            }
            f1 = 0;                                 //f1 and f2 rolled over in this if statement so we reset the values to 0
            f2 = 0;
	}

	else if(f1 == 9 && f2 != 9) {               //only f1 rolls over so we increment f2 and reset f1
            f1 = 0;
            f2 = f2 + 1;
	}

	else {                                     //nothing is rolling over so we increment f1
            f1 = f1 + 1;
	}


}

void __attribute__((interrupt,auto_psv)) _CNInterrupt(void)
{
	// Clear interrupt flag
	IFS1bits.CNIF = 0;

        //Stopped and reset is pressed
        if(state == 0 && PORTBbits.RB5 == 0 && PORTBbits.RB2 == 1) {
            flag1 = 1;
        }

        else if(flag1 == 1) {
            TMR1 = 0;
            state = 2;
            flag1 = 0;
        }

        //Stopped and start/stopped is pressed
        else if(state == 0 && PORTBbits.RB5 == 1 && PORTBbits.RB2 == 0) {
            flag2 = 1;
        }

        else if(flag2 == 1) {
            T1CONbits.TON = 1;
            state = 1;
            flag2 = 0;
        }


        //Running and start/stopped is pressed
        else if(state == 1 && PORTBbits.RB5 == 1 && PORTBbits.RB2 == 0) {
            T1CONbits.TON = 0;
            flag3 = 1;
        }

        else if(flag3 == 1) {
            T1CONbits.TON = 0;
            state = 0;
        }

        //Running and the reset button is pressed
        else if(state == 1 && PORTBbits.RB5 == 0)
            flag4 = 1;

        else if(flag4 == 1) {
            state = 1;
        }
}

// *******************************************************************************************

