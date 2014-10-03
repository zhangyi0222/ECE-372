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

void DebounceDelay() {
    T1CONbits.TON = 1;                  //starts timer
    while(IFS0bits.T1IF == 0);          //waits until the flag is raised
    T1CONbits.TON = 0;                  //turn the timer off
    TMR1 = 0;                           //reset the timer value
    IFS0bits.T1IF = 0;                  //turn the flag back off
}

int main(void)
{
	// ****************************************************************************** //

	// TODO: Configure AD1PCFG register for configuring input pins between analog input
	// and digital IO.
        AD1PCFGbits.PCFG4 = 1;  //Set the switch to be digital

	// TODO: Configure TRIS register bits for Right and Left LED outputs.
        TRISAbits.TRISA0 = 0;   //Right (green/run)
        TRISAbits.TRISA1 = 0;   //Left (red/stop)


	// TODO: Configure LAT register bits to initialize Right LED to on.
        LATAbits.LATA0 = 0;     //Turn Right (green/run) one on
        LATAbits.LATA1 = 1;     //Turn Left (red/stop) one off


	// TODO: Configure ODC register bits to use open drain configuration for Right
	// and Left LED output.
        ODCAbits.ODA0 = 1;
        ODCAbits.ODA1 = 1;

	// TODO: Configure TRIS register bits for swtich input.
        TRISBbits.TRISB2 = 1;   //switch


	// TODO: Configure CNPU register bits to enable internal pullup resistor for switch
	// input.
        CNPU1bits.CN6PUE = 1;

	// TODO: Setup Timer 1 to use internal clock (Fosc/2).
        T1CONbits.TCS=0;

	// TODO: Setup Timer 1's prescaler to 1:256.
        T1CONbits.TCKPS0=1;
        T1CONbits.TCKPS1=1;

 	// TODO: Set Timer 1 to be initially off.
        T1CONbits.TON = 0;

	// TODO: Clear Timer 1 value and reset interrupt flag
        IFS0bits.T1IF = 0;
        TMR1 = 0;

	// TODO: Set Timer 1's period value register to value for 5 ms.
        PR1 = 287;  //timer 1 period value for 5ms

        int state = 0;  //state is used to determine where to go for the switch statement
	while(1)
	{
		// TODO: For each distinct button press, alternate which
		// LED is illuminated (on).
            switch(state) {
                //Button is not pressed and moves once it is pressed
                case 0:
                    if(PORTBbits.RB2 == 0) {        //First instance on button being pressed
                        DebounceDelay();            //Debouncing to make sure the button is still pressed
                        if(PORTBbits.RB2 == 0)      //Move states if still pressed after debounce
                            state = 1;
                    }
                    break;

                //Button is pressed and we are waiting for it to be released
                case 1:
                    if(PORTBbits.RB2 == 1) {                        //First instance to see if the button is released yet
                        DebounceDelay();                            //Debouncing to make sure the button is not pressed anymore
                        if(PORTBbits.RB2 == 1) {                    //Toggles the LEDS and moves states if released
                            LATAbits.LATA0 = !LATAbits.LATA0;       //Toggle the LEDS
                            LATAbits.LATA1 = !LATAbits.LATA1;       //Toggle the LEDS
                            state = 0;
                        }
                    }
                    break;
            }


		// TODO: Use DebounceDelay() function to debounce button press
		// and button release in software.

                //At the top

	}
	return 0;
}



// *******************************************************************************************
