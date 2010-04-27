#ifndef EE392_H
#define EE392_H

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
