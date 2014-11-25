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
    unsigned int ADC_value_left, ADC_value_middle, ADC_value_right;
    char value_left[8]; //left phototransistor
    char value_right[8]; //right phototransistor
    char value_middle[8]; //middle phototransistorw

    RPOR4bits.RP8R = 18;    //right wheel
    RPOR4bits.RP9R = 19;    //left wheel
    RPOR0bits.RP0R = 21;    //right wheel ground
    RPOR1bits.RP2R = 20;    //left wheel ground

    OC1R = 0;
    OC1RS = 1023;        //Cruising speed

    OC2R = 0;
    OC2RS = 1023;        //Cruising speed

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

    //input pins for sensor
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISBbits.TRISB3 = 1;

    TRISBbits.TRISB5  = 1;
    CNEN2bits.CN27IE = 1;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;

//    CNEN2bits.CN27IE = 1; //for switch but not using anymore
//    IFS1bits.CNIF = 0;
//    IEC1bits.CNIE = 1;

    LCDInitialize();
    //LCDPrintChar('0');


    
    //Analog input pins being used
    AD1PCFGbits.PCFG0 = 0;
    AD1PCFGbits.PCFG1 = 0;
    AD1PCFGbits.PCFG5    = 0;


    AD1CON2 = 0x0000;
    AD1CON3 = 0x0101;
    AD1CON1 = 0x00E0;


    AD1CSSL = 0;            //Channel scanning


    AD1CON1bits.ADON = 1;   //Turn on A/D converter
    IFS0bits.AD1IF = 0;     //Turn on interrupt flag

    while(1)
    {
        LCDMoveCursor(0,0);

/************Left ADC value******************/
        AD1CHS = 0;
        AD1CON1bits.SAMP = 1;           //Start sample
        DelayUs(2000);
        while(IFS0bits.AD1IF == 0);
        AD1CON1bits.SAMP = 0;
        ADC_value_left = ADC1BUF0;
        sprintf(value_left, "%4d", ADC_value_left);
        LCDPrintString(value_left);

/************Middle ADC value******************/
        AD1CHS = 1;
        AD1CON1bits.SAMP = 1;           //Start sample
        DelayUs(2000);
        while(IFS0bits.AD1IF == 0);
        AD1CON1bits.SAMP = 0;
        ADC_value_middle = ADC1BUF0;
        sprintf(value_middle, "%4d", ADC_value_middle);
        LCDPrintString(value_middle);

/************Right ADC value******************/
        LCDMoveCursor(1,4);
        AD1CHS = 5;
        AD1CON1bits.SAMP = 1;           //Start sample
        DelayUs(2000);
        while(IFS0bits.AD1IF == 0);
        AD1CON1bits.SAMP = 0;
        ADC_value_right = ADC1BUF0;
        sprintf(value_right, "%4d", ADC_value_right);
        LCDPrintString(value_right);


        




        switch(state) {
            case 0:
                OC1RS = 0;
                OC2RS = 0;
                break;
                
            //Fast Mode
            case 1:
                if(ADC_value_middle >= 600) {//on the line
                    OC1RS = 1023;
                    OC2RS = 1023;
                }
                else {

                    //hard left turn
                    if(ADC_value_left > 300) {
                        OC1RS = 1023;
                        OC2RS = 0;
                    }

                    //hard right turn
                    else if(ADC_value_right > 300) {
                        OC1RS = 0;
                        OC2RS = 1023;
                    }
                }
                break;

            //slow mode
            case 2:
                if(ADC_value_middle >= 600) {//on the line
                    OC1RS = 900;
                    OC2RS = 900;
                }
                else {

                    //hard left turn
                    if(ADC_value_left > 300) {
                        OC1RS = 900;
                        OC2RS = 0;
                    }

                    //hard right turn
                    else if(ADC_value_right > 300) {
                        OC1RS = 0;
                        OC2RS = 900;
                    }
                }
        }
   }
    return 0;
}

void __attribute__((interrupt,auto_psv)) _CNInterrupt(void){
	IFS1bits.CNIF = 0;
        if(PORTBbits.RB5 == 0 && state == 2)
            state = 0;
        else if(PORTBbits.RB5 == 0 && state == 0) {
            state = 2;
            OC1RS = 1023;
            OC2RS = 1023;
        }
        else if(PORTBbits.RB5 == 0 && state == 1

                ) {
            state = 1;
            OC1RS = 900;
            OC2RS = 900;
        }
}