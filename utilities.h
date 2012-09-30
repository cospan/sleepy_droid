#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <stdbool.h>

int get_adc(int bit);
void led_toggle(int bit);
void led_on(int bit);
void led_off(int bit);
void led_enable(int bit, bool enable);

void fade(int from, int to);
bool  do_fade(unsigned char a, unsigned char b);



#endif
