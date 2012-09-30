#include <msp430x20x3.h>
//#include <stdio.h>

#include "yy_defines.h"
#include "utilities.h"


#define DELAY   0x010
#define PERIOD  0xFF


//int get_adc(int bit) {
//  return 0;
//}
/*
void led_on(int bit) {
  P1OUT |= (1 << bit);
}
void led_off(int bit) {
  P1OUT &= ~(1 << bit);
}
void led_toggle(int bit){
  unsigned char value;
  value = P1IN & (1 << bit);
  if (value > 0) {
    led_off(bit);
  }
  else {
    led_on(bit);
  }
}

void led_enable(int bit, bool enable){
  if (enable) {
    led_on(bit);
  }
  else {
    led_off(bit);
  }
}
*/

void fade(int from, int to){
  int i = 0;
  bool up;
  int position = 0;
  unsigned short delay_count = 0;

  if (from > to) {
    up = false;
  }
  else {
    up = true;
  }

  position = from;
  for (i = 0; i < PERIOD; i++){
    if (i < position) {
      P1OUT |= (LEFT_EYE | RIGHT_EYE); 
    }
    else {
      P1OUT &= ~(LEFT_EYE | RIGHT_EYE);
    }
  }


  while (position != to){
    delay_count = DELAY; 
    while (delay_count > 0) {
      for (i = 0; i < PERIOD; i++){
        if (i < position) {
          P1OUT |= (LEFT_EYE | RIGHT_EYE); 
        }
        else {
          P1OUT &= ~(LEFT_EYE | RIGHT_EYE);
        }
      }
      delay_count--;
    }

    if (up){
      position++;
    }
    else {
      position--;
    }
  }
}


