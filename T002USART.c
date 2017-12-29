/*
 * File:   T002USART.c
 * Author: Daniel Marquina
 *
 * Tutorial/reminder of USART module for PIC18F4550.
 * 1.The archive "usart.h" had to be modified:
 *      - #include <pconfig.h> --> #include "pconfig.h"
 *  "pconfig.h" couldn't be found with the first line.
 * 2.USART Library had to be recompiled because of compatibility:
 *      - Create new library project
 *      - Add header and sources from Legacy library
 *      - Include those files in compiler's directories
 *      - Compile and use new library
 * 3.115200 bauds will be used, with BRG16 enabled.
 * Created on 3/10/2017
 */


#include "T002USART.h"

#define SPBRG_115200 42 // Refer to PIC18F4550's datasheet, page 250, for 115200

// Variables to test
char message[] = "Hi there."; // To be sent
float fNum = -1.65687;// To be sent, Little Endian
unsigned int uiNum = 12345; // To be sent using sprintf
char inChar; // Incoming char
char outString[6]; // Outgoing string, 1 more char than uiNum because of sprintf
unsigned char len = 5; // Length of outgoing string, without null character

void main(void) {    
    
    // Interrupt configuration
    RCONbits.IPEN = 0; // Disable priority feature
    IPR1bits.RCIP = 0; // Set low priority, useless when IPEN = 0 
    INTCONbits.GIE_GIEH = 1; // Enable global interrupts 
    INTCONbits.PEIE_GIEL = 1; // Enable peripheral interrupts
    
    // Serial port initialization
    OpenUSART(USART_TX_INT_OFF &
            USART_RX_INT_ON & // Enable USART's receiver interrupt
            USART_BRGH_HIGH & 
            USART_CONT_RX & // Enables receiver in asynchronous mode
            USART_EIGHT_BIT & 
            USART_ASYNCH_MODE & 
            USART_ADDEN_OFF, SPBRG_115200);
    BAUDCONbits.BRG16 = 1; // More resolution
    
    // Initialization signal
    TRISD = 0x00;
    PORTD = 0xFF;
    __delay_ms(500);
    PORTD = 0x00;
    
    // Infinite Loop
    while(1){
        // Send a message when a pull-up button is pressed
        if(BUT_1 != 1){
            sendStrUSART(message, 9); // Sends a string of '9' bytes
            __delay_ms(300);
        }
        // Send a float number byte by byte when a pull-up button is pressed
        if(BUT_2 != 1){
            // Char pointer to float number, points to a byte 
            char* pF = (char*)(&fNum);      
            for(char i = 0; i < 4; i++){
                while(BusyUSART());
                putcUSART(*(pF + i));
            }
            __delay_ms(300);
        }
        // Send an int number as an array when a pull-up button is pressed
        if(BUT_3 != 1){
            // Conversion to array 
            sprintf(outString,"%u",uiNum); // Adds a null character at the end
            sendStrUSART(outString, len);
            memset(&outString[0], 0, sizeof(outString));
            __delay_ms(300);
        }
    }
}

void interrupt ISR(void){
    
    if (RCIF && RCIE){
        // The only way to clear this flag (RCIF) bit is by reading RCREG 
        // (receiver buffer), otherwise the interrupt will never be cleared
        // Retransmits any received character
        inChar = getcUSART();
        putcUSART(inChar);
        return; // Useful when dealing with many interrupt's sources
    }
}