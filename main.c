#include <msp430.h>
#include <legacymsp430.h>
#include <stdbool.h>
#include <stdio.h>
#include "utilities.h"
#include "yy_defines.h"


//#define LEFT_ANT  SD16AE2
//#define RIGHT_ANT SD16AE4



//UART
#define RXD   (1 << 1)
#define TXD   (1 << 5)

//57600 baud rate from SMCLK = 16MHz
#define Bittime_5   139
#define Bittime     278

#define OFFSET      4
#define THRESHOLD   0x0B


//function prototypes
void init (void);


unsigned  char  out_bitcount;
unsigned  int   out_data;

unsigned  char  in_bitcount;
unsigned  char  in_byte;

int             ant_num;




//array to store conversions
static unsigned char left_ant = 0x00;
static unsigned char right_ant = 0x00;

int main (void){
  bool up = false;
  unsigned char previous = 0;
  unsigned char max = 0;
  unsigned char min = 0;
  unsigned short timeout = TIMEOUT;

  init();

  P1OUT &= ~GREEN_BIT;
  //led_off(GREEN_BIT);
  //led_off(LEFT_EYE_BIT);
  //led_off(RIGHT_EYE_BIT);
  _BIS_SR(GIE);

  for (;;)
  {
    //volatile unsigned long i;
    //toggle LED
    //led_enable(GREEN_BIT, flag);
    //led_enable(LEFT_EYE_BIT, flag);
    //led_enable(RIGHT_EYE_BIT, flag);
    //flag = !flag;
   
    //P1OUT = 0xFF;
    //P1OUT ^= (GREEN | LEFT_EYE | RIGHT_EYE);

    //i = 99999;  

    //do (i--);
    //while (i != 0);

    //start a conversion for the left antena

    SD16CCTL0 |= SD16SC;
    _BIS_SR(LPM0_bits + GIE); 
    //_BIS_SR(GIE); 

    //printf ("L: 0x%04X R: 0x%04X\r\n", left_ant, right_ant);
    max = (left_ant > right_ant) ? left_ant : right_ant; 
    min = (left_ant > right_ant) ? right_ant: left_ant;
    if ((max - min) > THRESHOLD) {
      
      max = 0xFF;
      fade (previous, max);
      previous = max;
      timeout = TIMEOUT;
    }
    else {
      if (timeout > 1){
        if (up) {
          if (previous == 0xFF){
            up = false;
            fade (previous, previous);
          }
          else {
            fade (previous, previous + 1);
            previous++;
          }
        }
        else {
          if (previous == 0x00){
            up = true;
            fade (previous, previous);
          }
          else {
            fade (previous, previous - 1);
            previous--;
          }
        }
        timeout--;
      }
      else if (timeout == 1) {
        fade(previous, 0);
        previous = 0;
        timeout--;
      }
    }
  }
}


void init (void) {
  //stop the watchdog timer
  WDTCTL = WDTPW + WDTHOLD; 

  //Setup ADC
  //use internal voltage reference
  SD16CTL     = SD16REFON | SD16SSEL_1;

  //set the channel to 1, and 2
  SD16INCTL0  = SD16INCH_2;
  SD16CCTL0   = SD16SNGL | SD16UNI | SD16IE | SD16XOSR | SD16OSR_1024;
  SD16AE      = SD16AE2;
  ant_num     = 0;


  //set up the LED outputs
  //P1DIR |= (GREEN | LEFT_EYE | RIGHT_EYE);
  P1DIR |= (LEFT_EYE | RIGHT_EYE);


  //setup UART
  P1SEL   |= (TXD | RXD);
  P1DIR   |= TXD;

  //set the clock to 16MHz
  BCSCTL1 = CALBC1_16MHZ;
  DCOCTL  = CALDCO_16MHZ;

  CCTL0   = OUT;
  TACTL   = TASSEL_2 + MC_2;

  _EINT();
}

int putchar(int c){
  //write putchar here

  //load bit count (8 data + 1 start + 1 stop)
  out_bitcount  = 0x0A;

  //prevent ayn capture

//DON'T CARE ABOUT THIS CAUSE WE'RE ONLY WRITING
//  while (CCR0 != TAR){
    //current state of TA counter
    CCR0 = TAR;
//  }
  //set the time before the first bit
  CCR0 += Bittime;  
  //sets the bit high when done
  out_data  = 0x100 | (c & 0xFF);
  //add the start bit
  out_data  = out_data << 1;
  CCTL0 = CCIS0 + OUTMOD0 + CCIE;

  //finished
  while (CCTL0 & CCIE);

  return 0;
}

void read_byte(){
  //reading a byte from in_byte
  in_bitcount = 0x08;
  //Sync, negative edge, capture
  CCTL0 = SCS + OUTMOD0 + CM1 + CAP + CCIE;
}

//Timer A0 Interrupt Service Routine
interrupt(TIMERA0_VECTOR) Timer_A (void){
  CCR0  +=  Bittime;

  //led_on(GREEN_BIT);
  //TX
  if (CCTL0 & CCIS0){ 
    //TX on CCIOB?
    if (out_bitcount == 0) {
      //all bits transmitted, disable interrupt
      CCTL0 &= ~CCIE;
    }
    else {
      CCTL0 |=  OUTMOD2;      //space
      if (out_data & 0x01){
        CCTL0 &=  ~OUTMOD2;
      };
      out_data = out_data >> 1;
      out_bitcount--;
    }
  }

/*
  //RX
  if (CCTL0 & CAP){ 
    //Capture mode = start bit edge
    CCTL0 &= ~CAP;
    CCR0  += Bittime_5;
  }
  else {
    in_byte = in_byte >> 1;
    if (CCTL0 & SCCI) {
      //get bit waiting in the receive latch
      in_byte |= 0x80;
    }
    in_bitcount--;
    if (in_bitcount == 0){
      read_byte();
      CCTL0 &=  ~CCIE;
    }
  }
*/
}

interrupt(SD16_VECTOR) SD16ISR (void){
  switch (SD16IV){
    case (2):
      break;
    case (4):
      if (ant_num == 0) {
        left_ant    =   ((SD16MEM0 & 0x7FFF) >> OFFSET);
        SD16AE      &=  ~SD16AE2;
        SD16INCTL0  &=  ~SD16INCH_2;
    
        SD16INCTL0  |=  SD16INCH_4;
        SD16AE      |=  SD16AE4;
    
        SD16CCTL0   |=  SD16SC;
        ant_num     =   1;
      }
      else {
        right_ant   =   ((SD16MEM0 & 0x7FFF) >> OFFSET);
        SD16AE      &=  ~SD16AE4;
        SD16INCTL0  &=  ~SD16INCH_4;
    
    
        SD16AE      |=  SD16AE2;
        SD16INCTL0  |=  SD16INCH_2;
        ant_num     =   0;
    
        //exit low power mode
       _BIC_SR_IRQ(LPM0_bits);
    }
  }
}



