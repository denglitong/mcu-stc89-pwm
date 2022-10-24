#include "common.h"

unsigned char T0HighRH = 0;
unsigned char T0HighRL = 0;
unsigned char T0LowRH = 0;
unsigned char T0LowRL = 0;
unsigned long PeriodCnt = 0;
unsigned char T1RH = 0;
unsigned char T1RL = 0;

unsigned char PERCENTS[] = {5,  10, 15, 20, 25, 30, 35, 40, 45, 50,
                            55, 60, 65, 70, 75, 80, 85, 90, 95};
unsigned char PER_LEN = sizeof(PERCENTS) / sizeof(char);
unsigned char PER_DIRECTION = 0;
unsigned char PER_INDEX = 0;

void turn_on_led_master_switch();
void turn_on_all_leds();
void turn_off_all_leds();
void turn_on_single_led(unsigned char i);
int is_all_leds_turn_on();

void ConfigPWM_T0(unsigned int frequency, unsigned int low_percent);
void ClosePWM_T0();
void interrupt_time0() __interrupt(1);

void ConfigT1(unsigned int ms);
void interrupt_time1() __interrupt(3);

int main() {
  turn_on_led_master_switch();
  turn_on_all_leds();

  ConfigPWM_T0(100, 5);
  ConfigT1(50);  // change percent every 50ms

  while (1) {
  }
}

void ConfigPWM_T0(unsigned int frequency, unsigned int low_percent) {
  PeriodCnt = 11059200 / 12 / frequency;  // 计算一个周期所需的计数值
  unsigned int high_time_beats = PeriodCnt * low_percent / 100;
  unsigned low_time_beats = PeriodCnt - high_time_beats;
  high_time_beats = 65536 - high_time_beats;
  low_time_beats = 65536 - low_time_beats;

  T0HighRH = (unsigned char)(high_time_beats >> 8);
  T0HighRL = (unsigned char)(high_time_beats);
  T0LowRH = (unsigned char)(low_time_beats >> 8);
  T0LowRL = (unsigned char)(low_time_beats);

  EA = 1;        // enable global interrupt
  ET0 = 1;       // enable Timer0 interrupt
  TMOD &= 0xF0;  // 清除 T0 的控制位
  TMOD |= 0x01;  // set Timer0 mode TH0-TL0 16 bits timer
  // setup TH0 TL0 initial value
  TH0 = T0HighRH;
  TL0 = T0HighRL;
  TR0 = 1;  // start Timer0

  turn_on_all_leds();
}

void ClosePWM_T0() {
  EA = 0;
  ET0 = 0;
  TR0 = 0;
}

void interrupt_time0() __interrupt(1) {
  if (is_all_leds_turn_on()) {
    TH0 = T0LowRH;
    TL0 = T0LowRL;
    turn_off_all_leds();
  } else {
    TH0 = T0HighRH;
    TL0 = T0HighRL;
    turn_on_all_leds();
  }
}

void ConfigT1(unsigned int ms) {
  unsigned long tmp;
  tmp = 11059200 / 12 / 1000 * ms;  // 定时器计算所需的计数值
  tmp = 65536 - tmp;                // 定时器重载初值
  T1RH = (unsigned char)(tmp >> 8);
  T1RL = (unsigned char)(tmp);

  TMOD &= 0x0F;
  TMOD |= 0x10;
  TH1 = T1RH;
  TL1 = T1RL;
  ET1 = 1;  // 使能 T1 中断
  TR1 = 1;  // 启动 T1
}

void interrupt_time1() __interrupt(3) {
  // reload t1
  TH1 = T1RH;
  TL1 = T1RL;

  // assign frequency to timer0
  unsigned int high, low;
  high = PeriodCnt * PERCENTS[PER_INDEX] / 100;
  low = PeriodCnt - high;
  high = 65536 - high;
  low = 65536 - low;

  T0HighRH = (unsigned char)(high >> 8);
  T0HighRL = (unsigned char)(high);
  T0LowRH = (unsigned char)(low >> 8);
  T0LowRL = (unsigned char)(low);

  PER_DIRECTION == 0 ? PER_INDEX++ : PER_INDEX--;
  if (PER_INDEX == PER_LEN - 1 || PER_INDEX == 0) {
    PER_DIRECTION ^= 1;  // switch direction
  }
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
