/************************************************************
 * File:    ee392.c                                         *
 * Project: Wireless Bridge Monitor                         *
 * Author:  Doug Kelly                                      *
 * Group:   Matt Bloom, Steve Jia, Doug Kelly, Liz Stahlman *
 ************************************************************/
#include <reg952.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ee392.h"

/* timer0_isr()
 * Timer 0 ISR; based heavily on APNT_105 from Keil Software:
 * http://www.keil.com/appnotes/files/apnt_105.pdf
 */
static void timer0_isr(void) interrupt 1 using 1
{
    TR0 = 0;
    TH0 = (TIMER0_COUNT >> 8);
    TL0 = (TIMER0_COUNT & 0x00FF);
    TIMER0_TICK++;
    TR0 = 1;
}

/* readxbee()
 * UART1 ISR (RX): used to process character on input buffer
 * and immediately send character to computer.
 */
static void readxbee(void) interrupt 17 using 1
{
    char buf[3];
    EA=0;
    if((S1CON & RI_1_MSK)==1) {
        /* received character from XBee */
        sprintf(buf, "%c", S1BUF);
        writeserial(&buf);
        S1CON ^= RI_1_MSK;
    }
    EA=1;
}

/* initserial()
 * Set UARTs appropriately (mode, baud rate), and ports as input/output.
 */
void initserial()
{
    P1M1 &= 0xFC;    /* set port to I/O mode */
    P4M1 &= 0xF3;
    P4M1 |= 0x01;    /* set P4.0 to input-only */
    P4M2 &= ~0x01;
    P4 = 0x0D;       /* must set up P4.0/P4.2/P4.3 high to enable TX/RX */

    S0CON = 0x52;    /* SCON: mode 1, 8-bit UART, enable rcvr */
    S1CON = 0x52;
    BRGR1_0 = 0x02;  /* BRGR?_?: 9600 baud: (7.373MHz/9600)-16 */
    BRGR0_0 = 0xF8;  /* should be 0xf0, but 0xf8 seems to work better */
    BRG1_1 = 0x02;
    BRG0_1 = 0xF8;
    BRGCON_0 = 3;    /* enable internal baud rate generator */
    BRGCON_1 = 3;
    IEN2 |= 0x04;    /* enable RX interrupt (UART1) */
    return;
}

/* get_timer0_tick()
 * Used to determine amount of time in sleep.
 */
unsigned int get_timer0_tick()
{
    unsigned int t;
    EA = 0;
    t = TIMER0_TICK;
    EA = 1;
    return t;
}

/* msleep(int)
 * Sleep for n milliseconds (roughly) using timer
 */
void msleep(int msec)
{
    EA = 0;
    /* reset tick counter */
    TIMER0_TICK=0;
    /* set timer mode to 16-bit mode */
    TMOD &= ~0x0F;
    TMOD |= 0x01;
    /* load timer value (one tick -- ~1msec) */
    TH0 = (TIMER0_COUNT >> 8);
    TL0 = (TIMER0_COUNT & 0x00FF);
    /* enable the timer and interrupts */
    ET0 = 1;
    TR0 = 1;
    EA = 1;

    /* wait for timer to expire */
    while(get_timer0_tick() < msec);

    return;
}

/* initxbee()
 * Send commands to configure XBee module parameters (ID, destination)
 * Ideally, this would also check for errors, but does not.
 * These messages are caught by the serial interrupt.
 */
void initxbee()
{
    char msg[50];

    /* delay */
    msleep(2000);

    sprintf(msg, "+++");
    writexbee(&msg);

    /* delay */
    msleep(20000);

    /* XXX - check for OK */

    /* set parameters */
    sprintf(msg, "ATID%d,NID%d\r", ID, DEVICE);
    writexbee(&msg);
    msleep(2000);

    sprintf(msg, "ATAC,DND%d,CN\r", DEST);
    writexbee(&msg);
    msleep(2000);

    /* XXX - check for OK */

    return;
}

void writexbee(char *str)
{
    int i;
    for(i=0; i<strlen(str); i++) {
        /* ensure previous character sent */
        while((S1CON & TI_1_MSK)==0);
        S1BUF=str[i];
        /* clear interrupt flags */
        S1CON ^= TI_1_MSK;
    }
    return;
}

/* writeserial(char*)
 * Write a string out to the serial port (UART0)
 */
void writeserial(char *str)
{
    int i;
    for (i=0; i<strlen(str); i++) {
        /* ensure previous character sent */
        while(TI_0==0);
        S0BUF=str[i];
        /* clear interrupt flags */
        TI_0=0;
    }
    return;
}

/* readfloat()
 * Read current float switch status, send message if changed.
 */
void readfloat()
{
    char msg[5];
    /* P4 is not bit addressable. */
    /* must use bitwise AND here. */
    if((P4 & 0x01) != float_level) {
        /* float level is different; send update to XBee */
        float_level = (P4 & 0x01);
        sprintf(msg, "D%d,%d\r", DEVICE, float_level);
        writexbee(&msg);
    }

    #if DEBUG
    /* extra debugging output to serial UART */
    if(P4 & 0x01) {
        sprintf(msg, "ON\r");
    }
    else {
        sprintf(msg, "OFF\r");
    }
    writeserial(&msg);
    #endif

    return;
}

/* main()
 * Main program loop. Inititalize and read float switch periodically.
 */
void main()
{
    initserial();
    initxbee();
    while(1)
    {
        readfloat();
        /* delay for a bit */
        msleep(100000);
    }
}

