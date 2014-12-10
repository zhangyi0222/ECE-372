//******************//
//Names: Matthew Burger, Frank Holler, Yi Zhang
//Date: 12/10/14
//*****************//
// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

#include "p24fj64ga002.h"
#include <stdio.h>
#include "lcd.h"

#define MAX_SPEED 900

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
volatile int barcode_timer = 0;
volatile int barcode_state = 0;
volatile int state = 0;
volatile int flag = 0;
volatile int sec = 0;
int main(void)
{
    int delay = 0;
    int barcode_state = 0;
    int barcode_count = 0;
    char barcode[4] = "0000";
    int test = 0;
    int remote_state = 0;
    int i = 0;
    int total  = 0;
    int count = -1;
    int start_flag = 0;
    int black_flag = 0;
    int red_flag = 0;
    int white_flag = 0;
    unsigned int ADC_value_left, ADC_value_middle, ADC_value_right;
    unsigned int ADC_value_barcode, ADC_value_remote;
    char value_remote[8];
    char value_barcode[8];
    char value_left[8]; //left phototransistor
    char value_right[8]; //right phototransistor
    char value_middle[8]; //middle phototransistorw




    RPOR4bits.RP8R = 18;    //right wheel
    RPOR4bits.RP9R = 19;    //left wheel
    RPOR0bits.RP0R = 21;    //right wheel ground
    RPOR1bits.RP2R = 20;    //left wheel ground


    /********************Settings for the wheels, starts off*******************/
    OC1R = 0;
    OC1RS = 0;        

    OC2R = 0;
    OC2RS = 0;        

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
    /**************************************************************************/

    /*******************Timer settings for the laser point*********************/
    T1CONbits.TCS = 0;
    T1CONbits.TCKPS0 = 1;
    T1CONbits.TCKPS1 = 1;
    T1CONbits.TON = 0;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    TMR1 = 0;
    PR1 = 57599;
    /**************************************************************************/


    /********************Timer setting for the barcode*************************/
    T4CONbits.TCS = 0;
    T4CONbits.TCKPS0 = 1;
    T4CONbits.TCKPS1 = 1;
    T4CONbits.TON = 0;
    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 1;
    TMR4 = 0;
    PR4 = 57599;
    /**************************************************************************/

    //input pins for sensor
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISBbits.TRISB3 = 1;
    TRISBbits.TRISB15 = 1;
    TRISBbits.TRISB14 = 1;



    LCDInitialize();


    
    //Analog input pins being used
    AD1PCFGbits.PCFG0 = 0;
    AD1PCFGbits.PCFG1 = 0;
    AD1PCFGbits.PCFG5 = 0;
    AD1PCFGbits.PCFG9 = 0;


    AD1CON2 = 0x0000;
    AD1CON3 = 0x0101;
    AD1CON1 = 0x00E0;


    AD1CSSL = 0;            //Channel scanning, want to use manaul scanning


    AD1CON1bits.ADON = 1;   //Turn on A/D converter
    IFS0bits.AD1IF = 0;     //Turn on interrupt flag
    while(1)
    {
/************Left ADC value******************/
        //LCDMoveCursor(1,4);
        AD1CHS = 0;
        AD1CON1bits.SAMP = 1;           //Start sample
        DelayUs(2000);
        while(IFS0bits.AD1IF == 0);
        AD1CON1bits.SAMP = 0;
        ADC_value_left = ADC1BUF0;
        sprintf(value_left, "%4d", ADC_value_left);
        //LCDPrintString(value_left);

/************Middle ADC value******************/
        AD1CHS = 1;
        AD1CON1bits.SAMP = 1;           //Start sample
        DelayUs(2000);
        while(IFS0bits.AD1IF == 0);
        AD1CON1bits.SAMP = 0;
        ADC_value_middle = ADC1BUF0;
        sprintf(value_middle, "%4d", ADC_value_middle);
        //LCDPrintString(value_middle);

/************Right ADC value******************/
        AD1CHS = 5;
        AD1CON1bits.SAMP = 1;           //Start sample
        DelayUs(2000);
        while(IFS0bits.AD1IF == 0);
        AD1CON1bits.SAMP = 0;
        ADC_value_right = ADC1BUF0;
        sprintf(value_right, "%4d", ADC_value_right);
        //LCDPrintString(value_right);

/************Barcode value******************/
        //LCDMoveCursor(1,4);
        AD1CHS = 9;
        AD1CON1bits.SAMP = 1;           //Start sample
        DelayUs(2000);
        while(IFS0bits.AD1IF == 0);
        AD1CON1bits.SAMP = 0;
        ADC_value_barcode = ADC1BUF0;
        sprintf(value_barcode, "%4d", ADC_value_barcode);
        //LCDPrintString(value_barcode);

/************Remote value******************/
	//LCDMoveCursor(1,4);
        AD1CHS = 10;
        AD1CON1bits.SAMP = 1;           //Start sample
        DelayUs(2000);
        while(IFS0bits.AD1IF == 0);
        AD1CON1bits.SAMP = 0;
        ADC_value_remote = ADC1BUF0;
        sprintf(value_remote, "%4d", ADC_value_remote);
	//LCDPrintString(value_remote);


        //Default ADC_value for the remote it 1023, only changes if light/laser
        //is shined on the phototransistor.
        //Timer is put in so that it takes 2 seconds before it is allowed to
        //change between on/off again.
    	if(ADC_value_remote < 950) {
            switch(remote_state) {
                //turn timer on and change to state 2
                case 0:
                    TMR1 = 0;
                    T1CONbits.TON = 1;
                    remote_state = 2;
                    break;
                //waits 3 seconds before allowing another change of state
		case 1:
                    if(sec > 3) {
                    remote_state = 0;
                    sec = 0;
                    T1CONbits.TON = 0;
                    }
                    break;
                //picks the state to change to based on the current state
		case 2:
                    if(state == 0) {
                        OC1RS = MAX_SPEED;
                        OC2RS = MAX_SPEED;
                        state = 1;
                    }
                    else if(state == 1) { 
                        state = 0;
                    }
                    remote_state = 1;
                    break;
		}
	}

        //State = 0 is off
        //State = 1 is running with the barcode working
        switch(state) {

            case 0:
                OC1RS = 0;
                OC2RS = 0;
                break;
                
            //Fast Mode: basically goes straight as long as the middle sensor
            //is reading black( which is above 600 for us)
            case 1:
                if(ADC_value_middle >= 600) {//on the line
                    OC1RS = MAX_SPEED;
                    OC2RS = MAX_SPEED;
                }
                else {

                    //hard left turn if left sensor is picked up
                    if(ADC_value_left > 300) {
                        OC1RS = MAX_SPEED;
                        OC2RS = 0;
                    }

                    //hard right turn ifi right sensor is picked up
                    else if(ADC_value_right > 300) {
                        OC1RS = 0;
                        OC2RS = MAX_SPEED;
                    }
                }

                //Does the barcode reading
                switch(barcode_state) {

                    //start state: waits for black to be detected
                    case 0:
                        if(ADC_value_barcode > 700) {
                            barcode_state = 4; //state where we wait for new line
                            start_flag = 1;
                        }
                        break;

                     //white state: basically if it doesn't leave this state
                     //after a second, then we reset the barcode reading and reset
                     //all the variables back to start
                    case 1:
                        if(barcode_timer > 2) {
                                if(total == 1) { //total keeps track of how many barcodes we've read
                                    LCDClear();
                                    LCDPrintChar(barcode[0]);
                                    LCDPrintChar(barcode[1]);
                                    LCDPrintChar(barcode[2]);
                                    LCDPrintChar(barcode[3]);
                                    LCDMoveCursor(1,0);
                                }
                                else
                                    LCDClear();
                                T4CONbits.TON = 0;
                                TMR4 = 0;
                                barcode_timer = 0;
                                start_flag = 0;
                                white_flag = 0;
                                barcode_state = 0;
                                count = -1;
                                barcode_count = 0;
                        }
                        else if(ADC_value_barcode > 600 && barcode_timer < 2) {
                            barcode_state = 5;
                            T4CONbits.TON = 0;
                            TMR4 = 0;
                        }
                        break;

                    //black state: prints to the LCD and stores the value
                    case 2:
                        barcode[barcode_count]='0';
                        LCDPrintChar('0');
                        barcode_count++;
                        barcode_state = 4;  //waits for new value
                        break;

                    //red state: prints to the LCD and stores the value
                    case 3:
                        barcode[barcode_count]='1';
                        LCDPrintChar('1');
                        barcode_count++;
                        barcode_state = 4;
                        break;

                    //wait state: waits for another color other than white
                    case 4:
                        //the start flag is not necessary but it's there from old code
                        if(start_flag == 1) {

                            //less than 650 means it is white and count keeps track of
                            //how many barcode values are read
                            if(ADC_value_barcode < 650 && count < 3) {
                                    count++;
                                    barcode_state = 1;
                                    white_flag = 1;
                                    TMR4 = 0;
                                    barcode_timer = 0;
                                    T4CONbits.TON = 1; //timer for seeing how long we are reading white
                                }

                            //if we read a full barcode, we'll go to the next line
                            else if(count == 3) {
                                    total = 1;
                                    count = -1;
                                    start_flag = 0;
                                    LCDMoveCursor(1,0);
                                    barcode_state = 0;
                             }
                        }
                        break;

                    //pick black or red state
                    case 5:
                        //Puts a minor delay to avoid the transition between colors and then polls again
                        DelayUs(10000);
                            AD1CHS = 9;
                            AD1CON1bits.SAMP = 1;           //Start sample
                            DelayUs(2000);
                            while(IFS0bits.AD1IF == 0);
                            AD1CON1bits.SAMP = 0;
                            ADC_value_barcode = ADC1BUF0;

                            //go black state
                            if(ADC_value_barcode > 800) {
                                barcode_state = 2;
                            }

                            //go red state
                            else if(ADC_value_barcode > 700 && ADC_value_barcode < 800)
                            {
                                barcode_state = 3;
                            }

                        break;
                }
                
                break;

        }
   }
    return 0;
}


//we were originally using the button on the board to change the state but now we use
//the laser, so we don't really need the change notification ISR anymore
void __attribute__((interrupt,auto_psv)) _CNInterrupt(void){
	IFS1bits.CNIF = 0;
        if(PORTBbits.RB5 == 0 && state == 1)
            state = 0;
        else if(PORTBbits.RB5 == 0 && state == 0) {
            state = 1;
            OC1RS = MAX_SPEED;
            OC2RS = MAX_SPEED;
        }
}


//Used to keep track of how long the laser pointer
void __attribute__((interrupt,auto_psv)) _T1Interrupt(void){

    // Clear Timer 1 interrupt flag to allow another Timer 1 interrupt to occur.
	IFS0bits.T1IF = 0;

	//increment sec every second
	sec = sec + 1;


}

//Used to keep track of the barcode
void __attribute__((interrupt,auto_psv)) _T4Interrupt(void){
    
	IFS1bits.T4IF = 0;
        barcode_timer = barcode_timer + 1;


}