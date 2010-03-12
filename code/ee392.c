#include <reg952.h>
#include <stdlib.h>
#include <stdio.h>

void initserial()
{
    P1M1 &= 0xFC;    /* set port to I/O mode */
    P4M1 &= 0xF3;
    P4 = 0x0C;       /* must set up P4.2/P4.3 high to enable TX/RX */

    S0CON = 0x52;    /* SCON: mode 1, 8-bit UART, enable rcvr */
    S1CON = 0x52;
    BRGR1_0 = 0x02;  /* BRGR?_?: 9600 baud: (7.373MHz/9600)-16 */
    BRGR0_0 = 0xF8;  /* should be 0xf0, but 0xf8 seems to work better */
    BRG1_1 = 0x02;
    BRG0_1 = 0xF8;
    BRGCON_0 = 3;    /* enable internal baud rate generator */
    BRGCON_1 = 3;
}

void writeserial()
{
    char rxbuf;
    char txbuf;

    if(RI_0==1) {
        /* received character from computer */
        rxbuf=S0BUF;
        /* ensure previous character sent */
         while((S1CON & TI_1_MSK)==0);
        S1BUF=rxbuf;
        /* clear interrupt flags */
        S1CON ^= TI_1_MSK;
        RI_0 = 0;
    }
    if((S1CON & RI_1_MSK)==1) {
        /* received character from XBee */
        txbuf=S1BUF;
        /* ensure previous character sent */
        while(TI_0==0);
        S0BUF=txbuf;
        /* clear interrupt flags */
        TI_0=0;
        S1CON ^= RI_1_MSK;
    }
}

void main()
{
    int i;
    int j;
    initserial();
    while(1)
    {
        writeserial();
        /* delay for a bit */
        for(i=0;i==100;i++){
            for(j=0;j==100;j++);
        }
    }
}

