/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.145.0
        Device            :  dsPIC33CH512MP508
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.36b
        MPLAB 	          :  MPLAB X v5.25
*/

/*
    (c) 2019 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/** @file main.cpp
 * \brief modbus_qdrtu "Quick Dirty Modbus RTU implementation 
 *
 * This implementation will NOT use code from FreeModBus
 * and it is my intention to licence it under the
 * MIT license not LGPL
 *
 * As of 13/2/2015 this version implements  commands
 *
 * - Command 1 Read Coils
 * - Command 2 Read Inputs (alias)
 * - Command 3 Read Holding registers
 * - Command 4 Read Input registers (alias)
 * - Command 5 Force Single Coil
 * - Command 6 Force Single Register
 * - Command 15 Force Multiple Coils
 * - Command 16 Force Multiple Registers
 *
 * A "Coil" is essentially an output
 *
 * Offset issue:
 *
 * Historically PLC physical connections were numbered from 1
 * but internally Coil 1 is bit 0
 * Internally we will number from 0 as that is the convention 
 * that qmodmaster uses
 */


/**
  Section: Included Files
*/
#include "mcc_generated_files/mcc.h"

#define FCY _XTAL_FREQ
#include <libpic30.h>
 
#define DATA_UNDER_TEST 0xAAAA

/* The following CRC function is lifted from modbus_protocol.pdf */
/* It is also possible to make a smaller but slower bit-mangling version */
/* Table of CRC values for high-order byte */
static unsigned char auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
/* Table of CRC values for low-order byte */
static char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;

unsigned short CRC16(unsigned char *puchMsg , unsigned short usDataLen)
 /** Calculates a modbus CRC ($A001)
 * @param puchmsg   message to calculate CRC upon 
 * @param usDataLen quantity of bytes in message 
 * @returns byte swapped CRC value */
{
 unsigned char uchCRCHi = 0xFF ; /* high CRC byte initialized */
 unsigned char uchCRCLo = 0xFF ; /* low CRC byte initialized */
 unsigned uIndex ; /* will index into CRC lookup table */
 while (usDataLen--) /* pass through message buffer */
 {
 uIndex = uchCRCHi ^ *(puchMsg++) ; /* calculate the CRC */
 uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
 uchCRCLo = auchCRCLo[uIndex] ;
 }
 return (uchCRCHi << 8 | uchCRCLo) ;
}
//variables used by modbus implementation
char test;
unsigned short count, count2, coil,coil2, num, timer;
unsigned char inpacket[20];
unsigned char outpacket[20];
unsigned short CRC;

//placeholder for board LEDS (since we only have 2 and one is unavailable)
unsigned char myleds;
/*
                         Main application
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize();
 
    //Program and enable slave
    SLAVE1_Program();
    SLAVE1_Start();
 
    ProtocolA_DATA dataSend;
    ProtocolB_DATA dataReceive;
 
    dataSend.ProtocolA[0] = DATA_UNDER_TEST;
    dataReceive.ProtocolB[0] = 0;         //Initializing to known value.
 
    //Mailbox write
    SLAVE1_ProtocolAWrite((ProtocolA_DATA*)&dataSend);
 
    //Issue interrupt to slave
    SLAVE1_InterruptRequestGenerate();
    while(!SLAVE1_IsInterruptRequestAcknowledged());
    SLAVE1_InterruptRequestComplete();
    while(SLAVE1_IsInterruptRequestAcknowledged());
 
    //Wait for interrupt from slave
    while(!SLAVE1_IsInterruptRequested());
    SLAVE1_InterruptRequestAcknowledge();
    while(SLAVE1_IsInterruptRequested());
    SLAVE1_InterruptRequestAcknowledgeComplete();   
 
    //Mailbox read
    SLAVE1_ProtocolBRead((ProtocolB_DATA*)&dataReceive);
 
    //Glow LED on data match
    if(dataReceive.ProtocolB[0] == DATA_UNDER_TEST)
    {
        LED_MASTER_SetHigh();
    }
    else
    {
        LED_MASTER_SetLow();
    }
    myleds = 0;
    count = 0;
    timer = 0;
    
    // simple block-read routine
    // expects a character within 7 counts (3.5ms) of previous one
    // if a character is not recieved it assumes the end of a block
    while(1) {
        if  (!UART1_IsRxReady())
        {
            timer=0;
            inpacket[count]=UART1_Read();
            if (count<20)
            {
                count++;
            };
        }
        else
        timer++;
        if (timer<7)
        {
            __delay_us(500);
        }
        else
        {
/* Action code tests for valid CRC then attempts to process packet 
 * note that the previous code will return zero-length packets 
 * so we must test the byte count to filter them out */                
                    
            if (count>3 && CRC16(inpacket,count)==0) {
                        outpacket[0]=inpacket[0];
                        outpacket[1]=inpacket[1]|0x80;
                        outpacket[2]=1; //illegal function error
                        count2=3;
                switch(inpacket[1]) {
                    case 1: //read coils
                    case 2: //read inputs
                        coil=inpacket[2]<<8 | inpacket[3];
                        num=inpacket[4]<<8 | inpacket[5];
                        if(coil+num < 5) {
                           outpacket[0]=inpacket[0];
                           outpacket[1]=inpacket[1];
                           outpacket[2]=0;
                           count2=2;
                           for (coil2=0;coil2<num;coil2++)
                           {
                              if ((coil2 & 7) ==0) { count2++; outpacket[2]++; outpacket[count2]=0; }
                              if (myleds&(1<<(coil+coil2))) {outpacket[count2] |= 1<<(coil2 & 7);}
                           }
                           count2++;
                           }
                           break;
                           
                    case 5: //force single coil
                        coil=inpacket[2]<<8 | inpacket[3];
                        if(coil < 4 && inpacket[5]==0) {
                           if(inpacket[4]) { myleds|=(1<<coil); }
                           else { myleds &=~(1<<coil);}
                           //myleds[coil]=inpacket[4];
                           outpacket[0]=inpacket[0];
                           outpacket[1]=inpacket[1];
                           outpacket[2]=inpacket[2];
                           outpacket[3]=inpacket[3];
                           outpacket[4]=inpacket[4];
                           outpacket[5]=inpacket[5];
                           count2=6;
                           }
                           break;
                            
                    case 3: //read holding registers
                    case 4: //read input registers (alias)
                        coil=inpacket[2]<<8 | inpacket[3];
                        num=inpacket[4]<<8 | inpacket[5];
                        if(coil==0 && num == 1) {
                           outpacket[0]=inpacket[0];
                           outpacket[1]=inpacket[1];
                           outpacket[2]=0;
                           count2=3;
                           for (coil2=0;coil2<num;coil2++)
                           {
                              outpacket[count2++]=0;
                              outpacket[count2++]=myleds;
                              outpacket[2]+=2;
                           }
                           }
                           break;
                            
                    case 6: //force single register
                        coil=inpacket[2]<<8 | inpacket[3];
                        if(coil == 0) {
                           myleds=inpacket[5];
                           outpacket[0]=inpacket[0];
                           outpacket[1]=inpacket[1];
                           outpacket[2]=inpacket[2];
                           outpacket[3]=inpacket[3];
                           outpacket[4]=inpacket[4];
                           outpacket[5]=inpacket[5];
                           count2=6;
                           }
                           break;
                           
                    case 15: //force multiple coils
                        coil=inpacket[2]<<8 | inpacket[3];
                        num=inpacket[4]<<8 | inpacket[5];
                        // inpacket[6] = byte count
                        if(coil+num < 5) {
                           count2=6;
                           for (coil2=0;coil2<num;coil2++)
                           {
                              if ((coil2 & 7) ==0) { count2++; }
                              if ( inpacket[count2] & (1<<(coil2 & 7)))
                              {myleds |= (1<<(coil+coil2));}
                              else {myleds &= ~(1<<(coil+coil2));}
                           //   myleds[coil+coil2] = ( inpacket[count2] & (1<<(coil2 & 7)));
                           }
                           count2++;
                           outpacket[0]=inpacket[0];
                           outpacket[1]=inpacket[1];
                           outpacket[2]=inpacket[2];
                           outpacket[3]=inpacket[3];
                           outpacket[4]=inpacket[4];
                           outpacket[5]=inpacket[5];
                           count2=6;
                           }
                           break;
                            
                    case 16: //force multiple registers
                        coil=inpacket[2]<<8 | inpacket[3];
                        num=inpacket[4]<<8 | inpacket[5];
                        // inpacket[6] = byte count
                        if(coil==0 && num ==1) {
                           count2=7;
                           for (coil2=0;coil2<num;coil2++)
                           {
                             count2++;
                             myleds=inpacket[count2++];
                           }
                           outpacket[0]=inpacket[0];
                           outpacket[1]=inpacket[1];
                           outpacket[2]=inpacket[2];
                           outpacket[3]=inpacket[3];
                           outpacket[4]=inpacket[4];
                           outpacket[5]=inpacket[5];
                           count2=6;
                           }
                           break;
                            
                }
                        CRC=CRC16(outpacket,count2);
                        outpacket[count2++]=CRC >> 8;
                        outpacket[count2++]=CRC & 0xff;
                for(count=0; count<count2; count++) {
                    UART1_Write(outpacket[count]);
                }
                __delay_ms(3);
            }
            
            timer=0;
            count=0;
        }
    }
 
    while (1)
    {
        // Add your application code
        if (!UART1_IsRxReady()) 
        {
            UART1_Write(UART1_Read());
        }
    }
    return 1; 
}
/**
 End of File
*/


