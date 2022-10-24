#include "common.h"

#define PWMOUT P0_0;

unsigned char HighRH = 0;
unsigned char HighRL = 0;
unsigned char LowRH = 0;
unsigned char LowRL = 0;

unsigned char HEART_IMAGE_IDX = 0;
const unsigned char HEART_IMAGE[] = {0b00000000, 0b01100110, 0b11111111,
                                     0b11111111, 0b01111110, 0b00111100,
                                     0b00011000, 0b00000000};

void turn_on_led_master_switch();
void turn_on_all_leds();
void turn_off_all_leds();
void turn_on_single_led(unsigned char i);
int is_all_leds_turn_on();

void ConfigPWM_T0(unsigned int frequency, unsigned int low_percent);
void ClosePWM_T0();

void interrupt_time0() __interrupt(1);

int main() {
  turn_on_led_master_switch();
  turn_on_all_leds();

  unsigned int i, j;

  while (1) {
    for (j = 5; j < 100; j += 5) {
      ConfigPWM_T0(100, j);
      for (i = 0; i < 5000; i++) {
      }
      ClosePWM_T0();
    }
    j -= 5;
    for (; j > 0; j -= 5) {
      ConfigPWM_T0(100, j);
      for (i = 0; i < 5000; i++) {
      }
      ClosePWM_T0();
    }
  }
}

void ConfigPWM_T0(unsigned int frequency, unsigned int low_percent) {
  unsigned long total_time_beats = 11059200 / 12 / frequency;
  unsigned int high_time_beats = total_time_beats * low_percent / 100;
  unsigned low_time_beats = total_time_beats - high_time_beats;
  high_time_beats = 65536 - high_time_beats;
  low_time_beats = 65536 - low_time_beats;

  HighRH = (unsigned char)(high_time_beats >> 8);
  HighRL = (unsigned char)(high_time_beats);
  LowRH = (unsigned char)(low_time_beats >> 8);
  LowRL = (unsigned char)(low_time_beats);

  EA = 1;       // enable global interrupt
  ET0 = 1;      // enable Timer0 interrupt
  TMOD = 0x01;  // set Timer0 mode TH0-TL0 16 bits timer
  // setup TH0 TL0 initial value
  TH0 = HighRH;
  TL0 = HighRL;
  TR0 = 1;  // start Timer0

  turn_on_all_leds();
}

void ClosePWM_T0() {
  EA = 0;
  ET0 = 0;
  TR0 = 0;
}

// 教学板子 LED_SINGLE 总开关
void turn_on_led_master_switch() {
  enable_u3_74hc138();
  // 110 LEDS6 为低电平，三极管导通，LED 总开关打开
  ADDR_2 = 1;
  ADDR_1 = 1;
  ADDR_0 = 0;
}

void turn_on_all_leds() { LED_LINE = 0x00; }

void turn_off_all_leds() { LED_LINE = 0xff; }

void turn_on_single_led(unsigned char i) {
  // turn on the i-th led from low-high
  LED_LINE &= ~(0x01 << i);
}

int is_all_leds_turn_on() { return LED_LINE == 0x00; }

void interrupt_time0() __interrupt(1) {
  if (is_all_leds_turn_on()) {
    TH0 = LowRH;
    TL0 = LowRL;
    turn_off_all_leds();
  } else {
    TH0 = HighRH;
    TL0 = HighRL;
    turn_on_all_leds();
  }
}
