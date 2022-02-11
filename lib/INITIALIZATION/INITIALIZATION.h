#define MOTOR (1<<PB0)
#define MOTOR_ON PORTB &= ~MOTOR	//makro załączające diodę LED
#define MOTOR_OFF PORTB |= MOTOR	//makro do wyłączania diody LED