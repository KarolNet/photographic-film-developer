#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "lib/HD44780/HD44780.c"
#include "lib/DS18B20/DS18B20.c"
#include <string.h>

#define F_CPU 1000000UL
#define MOTOR (1<<PB0)		//definicja wyprowadzenia do którego podłączona jest dioda LED
#define MOTOR_ON PORTB &= ~MOTOR	//makro załączające diodę LED
#define MOTOR_OFF PORTB |= MOTOR	//makro do wyłączania diody LED
#define read_eeprom_array(address,value_p,length) eeprom_read_block ((void *)value_p, (const void *)address, length)
#define write_eeprom_array(address,value_p,length) eeprom_write_block ((const void *)value_p, (void *)address, length)

int times_count = 7;

uint8_t times[7] = {60,60,10,50,10,50,180};
uint8_t statuses[7] = {1,0,1,0,1,0,1};
// uint8_t EEMEM times_eeprom[7] = {60,60,10,50,10,50,180};
// uint8_t EEMEM lupa = 14;
int time[2];
int delay = 0;
int volatile start = 1;
int volatile setup = 0;
int volatile setupStep = 0;
char str[4];

void delay_s(int s) {
  while (0 < s) {
    if (start == 1) {
      _delay_ms(1000);
      s -= 1;
    } else {
      break;
    }
  }
}

ISR(INT0_vect){
  if (start == 0) {
    start = 1;
  } else {
    start = 0;
  }
}

// ISR(INT1_vect){
//  if (setup == 0) {
//     setup = 1;
//   } else {
//     // setup =0;
//     setupStep++;
//     // _delay_ms(10);
//     // if (setupStep >= times_count) {
//     //   setupStep = times_count;
//     // }
//   }
// }

ISR(BADISR_vect){}


void showTimes(int currentStep) {

  int i;
  int lcdIndex = 0;
  LCD_Clear();
  LCD_Home();

  for (i = 0; i <times_count; i++ ) {
    sprintf(str, "%d", statuses[i]);
    LCD_GoTo(0, lcdIndex);
    if (currentStep == i) {
       LCD_WriteText(">");
    }
    LCD_WriteText(str);
    lcdIndex = lcdIndex+2;
  }
  lcdIndex = 0;
  for (i = 0; i <times_count; i++ ) {
    sprintf(str, "%d", times[i]);
    LCD_GoTo(1, lcdIndex);
    LCD_WriteText(str);
    lcdIndex = lcdIndex + 2;
  }
  _delay_ms(50);
}

void work() {
  int i;
  for (i = 0; i <times_count; i++ ) {
    showTimes(i);
    if (statuses[i] == 1) {
      MOTOR_ON;
    } else {
      MOTOR_OFF;
    }

    delay_s(times[i]);
  }
  start = 0;
}
int main()
{
  char tmp[16]; 
  // write_eeprom_array(times_eeprom, times,sizeof(times_eeprom));
  // read_eeprom_array(times_eeprom, times, sizeof(times_eeprom));
  // times[0] = eeprom_read_byte((uint8_t*)7);
  // times[0] = eeprom_read_byte(&times_eeprom[1]);
  // times[1] = 20;
  // times[2] = 10;
  // times[3] = 20;
  // times[4] = 20;
  // times[5] = 20;
  // times[6] = eeprom_read_byte(&times_eeprom[1]);
  DDRB |= MOTOR;

  // przycisk start/stop
  GICR = (1<<INT0) | (1<<INT1); 
  MCUCR = _BV(ISC01);
  // MCUCR = 0;
  // MCUCR &= _BV(ISC01);
  // MCUCR &= ~_BV(ISC00);
  DDRD &= ~_BV(PD2);
  PORTD |= _BV(PD2);

  // przycisk setup
  // GICR = _BV(INT1);
  // MCUCR = _BV(ISC01);
  DDRD &= ~_BV(PD3);
  PORTD |= _BV(PD3);

// button 1
  DDRD &= ~_BV(PD6);
  PORTD |= _BV(PD6);
  // button 2
  DDRD &= ~_BV(PD7);
  PORTD |= _BV(PD7);
  sei();

  LCD_Initalize();

  while(1) {

    if (start == 1) {
      work();
    } else {
      MOTOR_OFF;
      LCD_Clear();
      LCD_Home();
      LCD_WriteText("Koniec, wcisnij:");
      LCD_GoTo(1,1);
      LCD_WriteText("4-start, 3-setup");
      _delay_ms(50);


      if (bit_is_clear(PIND, 3)) {
        while(1) {
          
          LCD_Clear();
          LCD_Home();

          str[0] = '\0';
          tmp[0] = '\0';
          sprintf(str, "%d", setupStep);
          strcat(tmp, "Krok: ");
          strcat(tmp, str);
          LCD_WriteText(tmp);

          LCD_GoTo(1,0);
          str[0] = '\0';
          tmp[0] = '\0';
          sprintf(str, "%d", times[setupStep]);
          strcat(tmp, "Czas: ");
          strcat(tmp, str);
          LCD_WriteText(tmp);

          if (bit_is_clear(PIND, 6)) {
            _delay_ms(20);
            times[setupStep] = times[setupStep] - 30;
            // write_eeprom_array(times_eeprom, times,sizeof(times_eeprom));
            // _delay_ms(200);
          }

          if (bit_is_clear(PIND, 7)) {
            _delay_ms(20);
            times[setupStep] = times[setupStep] + 30;
            if (times[setupStep] < 0) {
              times[setupStep] = 0;
            }
            // write_eeprom_array(times_eeprom, times,sizeof(times_eeprom));
            // _delay_ms(200);
          }
          if (bit_is_clear(PIND, 3)) {
            _delay_ms(20);
            setupStep++;
            if (setupStep >= times_count) {
              setupStep = 0;
            }
          }
          if (start) {
            break;
          }
          _delay_ms(100);
          
        }
      }
    }
  }
  return 0;
}