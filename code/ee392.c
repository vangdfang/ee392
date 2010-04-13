#include <reg952.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ee392.h"

#define DEBUG 0

static int float_level = 0;
static unsigned int ID = 4242;
/* NOTE: if DEVICE/DEST are identical,
 * XBee will report an error on startup.
 * This is OK; it simply won't talk to itself.
 */
static unsigned int DEVICE = 1;
static unsigned int DEST = 1;
static unsigned int TIMER0_TICK;
static unsigned int TIMER0_COUNT = 0xFDAB;

/* Timer 0 ISR; based heavily on APNT_105 from Keil Software:
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
	/* Enable RX interrupt */
	IEN2 |= 0x04;
	return;
}

unsigned int get_timer0_tick()
{
	unsigned int t;
	EA = 0;
	t = TIMER0_TICK;
	EA = 1;
	return t;
}

void msleep(int msec)
{
    /* sleep for n milliseconds with timer */
	EA = 0;
	TIMER0_TICK=0;
	TMOD &= ~0x0F;
	TMOD |= 0x01;
	TH0 = (TIMER0_COUNT >> 8);
	TL0 = (TIMER0_COUNT & 0x00FF);
	ET0 = 1;
	TR0 = 1;
	EA = 1;

	/* wait for timer to expire */
	while(get_timer0_tick() < msec);

	/* done */
	return;
}

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

void readfloat()
{
    char msg[5];
    /* P4 is not bit addressable. */
    /* must use bitwise AND here. */
    if((P4 & 0x01) != float_level) {
        float_level = (P4 & 0x01);
        sprintf(msg, "D%d,%d\r", DEVICE, float_level);
        writexbee(&msg);
    }

	#if DEBUG
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

