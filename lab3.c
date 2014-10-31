//******************//
//Names: Matthew Burger, Frank Holler, Yi Zhang
//Date: 10/31/14
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
volatile int state = 0;
volatile int flag = 0;
int main(void)
{
    unsigned int ADC_value;
    char value1[8]; //left wheel
    char value2[8]; //right wheel
    unsigned int temp1 = 0; //left wheel
    unsigned int temp2 = 0; //right wheel
    RPOR4bits.RP8R = 18;    //right wheel
    RPOR4bits.RP9R = 19;    //left wheel
    RPOR0bits.RP0R = 21;    //right wheel ground
    RPOR1bits.RP2R = 20;    //left wheel ground

    OC1R = 0;
    OC1RS = 512;        //50% duty cycle

    OC2R = 0;
    OC2RS = 512;        //50% duty cycle

    OC3R = 0;
    OC3RS = 0;

    OC4R = 0;
    OC4RS = 0;

    OC1CON = 0x000E;    //select timer 3
    OC2CON = 0x000E;    //select timer 3
    OC3CON = 0x000E;
    OC4CON = 0x000E;
    
    PR3 = 1023;         //Set the sampling to be around 15kHz

    TMR3 = 0;
    T3CON = 0x0800;
    T3CONbits.TON = 1;

    //input pins for switch and pot
    TRISAbits.TRISA0 = 1;
    TRISBbits.TRISB5 = 1;



    CNEN2bits.CN27IE = 1;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;

    LCDInitialize();



    
    //Analog input pins being used
    AD1PCFGbits.PCFG0 = 0;

    AD1CON2 = 0x0000;
    AD1CON3 = 0x0101;
    AD1CON1 = 0x00E0;
    AD1CHS = 0;             //Configure input channels,
    AD1CSSL = 0;            //Channel scanning
    AD1CON1bits.ADON = 1;   //Turn on A/D converter
    IFS0bits.AD1IF = 0;     //Turn on interrupt flag

    while(1)
    {
        AD1CON1bits.SAMP = 1;           //Start sample
        if (IFS0bits.AD1IF == 1) {      //go in once sampling completed
            IFS0bits.AD1IF = 0;         //clear flag
            AD1CON1bits.SAMP = 0;       //stop sampling
            LCDMoveCursor(0,0);
            ADC_value = ADC1BUF0;
            sprintf(value1, "%4d", ADC_value);      //Print adc value out
            LCDPrintString(value1);
            switch(state) {
                case 0:
                    OC1RS = 0;
                    OC2RS = 0;
                    temp1 = 0;
                    temp2 = 0;
                    break;
                case 1:
//                    //Change the period value of the pwm, thus changing the duty cycle
//                    //Pot in middle = both motors full speed
//                    //Pot turns to right = left motor full speed, right stopped
//                    //Pot turns to left = left motor stopped, right motor full speed
//                    //In between the above should be fractional values of the direct
                        RPOR4bits.RP8R = 18;
                        RPOR4bits.RP9R = 19;
                        RPOR0bits.RP0R = 21;
                        RPOR1bits.RP2R = 20;
                        if(ADC_value >= 512) {
                            temp1 = ((1023-ADC_value)*100)/512;     //percent left wheel
                            temp2 = 100;                            //percent right wheel
                            OC1RS = 1023;                           //set to be 100% duty cycle right wheel
                            OC2RS = 1023*(float)temp1/100;          //set to be % duty cycle found above left wheel
                            }
                        else {
                            temp1 = 100;                            //percent left wheel
                            temp2 = (ADC_value*100)/512;            //percent right wheel
                            OC1RS = 1023*(float)temp2/100;          //set to be % duty cycle found above right wheel
                            OC2RS = 1023;                           //set to be 100% duty cycle left wheel
                        }
                        flag = 1;
                        break;
                case 2:
                        RPOR4bits.RP8R = 21;
                        RPOR4bits.RP9R = 20;
                        RPOR0bits.RP0R = 18;
                        RPOR1bits.RP2R = 19;
                       if(ADC_value >= 512) {
                            temp1 = ((1023-ADC_value)*100)/512;
                            temp2 = 100;
                            OC1RS = 1023;
                            OC2RS = 1023*(float)temp1/100;
                        }
                        else {
                            temp1 = 100;
                            temp2 = (ADC_value*100)/512;
                            OC1RS = 1023*(float)temp2/100;
                            OC2RS = 1023;
                        }
                        flag = 0;
                        break;
             }
             LCDMoveCursor(1,0);
             sprintf(value1, "%3d", (int)temp1);
             LCDPrintString(value1);
             LCDMoveCursor(1,4);
            sprintf(value2, "%3d", (int)temp2);
            LCDPrintString(value2);
        }
   }
    return 0;
}

void __attribute__((interrupt,auto_psv)) _CNInterrupt(void){
	IFS1bits.CNIF = 0;

	if(PORTBbits.RB5 == 0) {
            if(state == 0) {
                if(flag == 0)       //indicates previous state was state 2
                    state = 1;
                else
                    state = 2;   
		}
            else
               state = 0;
	}
}