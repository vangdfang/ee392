/************************************************************
 * File:    ee392.h                                         *
 * Project: Wireless Bridge Monitor                         *
 * Author:  Doug Kelly                                      *
 * Group:   Matt Bloom, Steve Jia, Doug Kelly, Liz Stahlman *
 ************************************************************/
#ifndef EE392_H
#define EE392_H

/* Defined constants */
#define DEBUG 0

/* Static variables */
static int float_level = 0;
static unsigned int ID = 4242;
/* NOTE: if DEVICE/DEST are identical,
 * XBee will not have a destination.
 * The XBee would report an error if attempted.
 */
static unsigned int DEVICE = 1;
static unsigned int DEST = 1;
static unsigned int TIMER0_TICK;
static unsigned int TIMER0_COUNT = 0xFDAB;

/* Function prototypes */
static void timer0_isr(void);
static void readxbee(void);
void initserial();
unsigned int get_timer0_tick();
void msleep(int msec);
void initxbee();
void writexbee(char *str);
void writeserial(char *str);
void readfloat();

#endif
