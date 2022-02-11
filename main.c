#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "lib/HD44780/HD44780.c"
#include "lib/DS18B20/DS18B20.c"
#include <string.h>
#include "lib/INITIALIZATION/INITIALIZATION.c"

#define F_CPU 1000000UL
#define read_eeprom_array(address,value_p,length) eeprom_read_block ((void *)value_p, (const void *)address, length)
#define write_eeprom_array(address,value_p,length) eeprom_write_block ((const void *)value_p, (void *)address, length)

int times_count = 7;
uint8_t statuses[7] = {1,0,1,0,1,0,1};
uint8_t EEMEM times_eeprom[7] = {60,60,10,50,10,50,180};

int delay = 0;
int volatile start = 0;
int volatile setup = 0;
int volatile setupStep = 0;

// stop work
ISR(INT0_vect)
{
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
void LCD_print_value(char *text, int value)
{
  char tmp[16]; 
  char str[4];
  str[0] = '\0';
  tmp[0] = '\0';
  sprintf(str, "%d", value);
  strcat(tmp, text);
  strcat(tmp, str);
  LCD_WriteText(tmp);
}

void showTimes(int currentStep, uint8_t statuses[], uint8_t times[])
{
  LCD_Clear();
  LCD_Home();
  LCD_print_value("Step: ", currentStep + 1);
  LCD_GoTo(0, 7);
  LCD_print_value("/", 7);
  LCD_GoTo(0, 10);
  LCD_print_value("Sta: ", statuses[currentStep]);
 
  uint8_t secundsLeft = times[currentStep];
  while (0 < secundsLeft ) {
     LCD_GoTo(1,0);

     if (secundsLeft < 10) {
      LCD_print_value("Time to end:   ", secundsLeft);
     }else if (secundsLeft >= 10 && secundsLeft < 100) {
      LCD_print_value("Time to end:  ", secundsLeft);
     } else {
       LCD_print_value("Time to end: ", secundsLeft);
     }
     
    if (start == 1) {
      _delay_ms(1000);
      secundsLeft -= 1;
    } else {
      break;
    }
  }
}

void work(uint8_t times[])
{
  int i;
  for (i = 0; i <times_count; i++ ) {
    
    if (statuses[i] == 1) {
      MOTOR_ON;
    } else {
      MOTOR_OFF;
    }
    showTimes(i, statuses, times);
  }
  start = 0;
}

int main()
{
  uint8_t times[7];
  read_eeprom_array(&times_eeprom, times, 7);
  initialize_ports();
  sei();

  LCD_Initalize();

  while(1) {

    if (start == 1) {
      work(times);
    } else {
      MOTOR_OFF;
      LCD_Clear();
      LCD_Home();
      LCD_WriteText("End, press:");
      LCD_GoTo(1,0);
      LCD_WriteText("3-setup, 4-start");
      _delay_ms(50);

      // show times testup menu
      if (bit_is_clear(PIND, 3)) {
        while(1) {
          
          LCD_Clear();
          LCD_Home();

          LCD_print_value("Krok: ", setupStep + 1);
          LCD_GoTo(1,0);
          LCD_print_value("Czas: ", times[setupStep]);

          // del 10s
          if (bit_is_clear(PIND, 6)) {
            _delay_ms(50);
            times[setupStep] = times[setupStep] - 10;
            if (times[setupStep] < 0) {
              times[setupStep] = 0;
            }
            write_eeprom_array(times_eeprom, times, sizeof(times_eeprom));
            _delay_ms(200);
          }

          // add 10s 
          if (bit_is_clear(PIND, 7)) {
            _delay_ms(50);
            times[setupStep] = times[setupStep] + 10;
            if (times[setupStep] >= 250) {
              times[setupStep] = 0;
            }
            write_eeprom_array(times_eeprom, times, sizeof(times_eeprom));
            _delay_ms(200);
          }

          // next setting
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
          _delay_ms(50);
          
        }
      }
    }
  }
  return 0;
}
