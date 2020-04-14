#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* All values have been calculated for 1MHz clock speed */

#define MIN_OCR1A		70 // Counter value for -90 degrees = time_ms / time_of_clk_tick
#define MAX_OCR1A		300 // Counter value for 90 degrees
#define SHAFT_STEP		1
#define PWM_PIN_DDR		DDRB
#define PWM_PIN			PORTB1
#define BUTTONS_DDR		DDRC
#define BUTTON1			PORTC5
#define BUTTON2			PORTC4
#define EN_LED_PORT_DDR	DDRB
#define EN_LED			PORTB0

// Timer0 compare interrupt
ISR(TIMER0_COMPA_vect)
{
	// Counter = 0
	TCNT0 = 0;
	// Toggle EN_LED pin output
	if((PORTB & (1 << EN_LED)) == 0)
		PORTB |= (1 << EN_LED);
	else
		PORTB &= ~(1 << EN_LED);
}

static void init_timer0(void)
{
	/* Normal mode */
	// About 203ms delay, a little less than 5 blinks a second
	OCR0A = 200;
	// Counter = 0
	TCNT0 = 0;
	// Compare TCNT0 with OCR0A
	TIMSK0 |= (1 << OCIE0A);
	// Prescaler = 1024
	TCCR0B |= (1 << CS00) | (1 << CS02);
	// Enable global interrupts
	sei();
}

static void init_pwm(void)
{
	PWM_PIN_DDR |= (1 << PWM_PIN);
	// Timer1 count = 0
	TCNT1 = 0;
	// TOP count for timer1 = 2499 for 50Hz PWM cycle
	ICR1 = 2499;
	// Enable fast PWM mode
	// Look for TOP in ICR1, prescaler = 8
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);
	// Initial shaft position at 0 degrees
	OCR1A = 175;
}

static void bt_control_servo(void)
{
	// Rotate the shaft at button press
	// Increase angle
	if((PINC & (1 << BUTTON1)) == 0)
	{
		if(OCR1A < MAX_OCR1A && OCR1A + SHAFT_STEP < MAX_OCR1A)
		{
			_delay_ms(2);
			OCR1A += SHAFT_STEP;
		}
	}
	// Decrease angle
	else if((PINC & (1 << BUTTON2)) == 0)
	{
		if(OCR1A > MIN_OCR1A && OCR1A - SHAFT_STEP > MIN_OCR1A)
		{
			_delay_ms(2);
			OCR1A -= SHAFT_STEP;
		}
	}
}

int main(void)
{
	init_pwm();
	
	// Initialize buttons
	// Buttons as inputs
	BUTTONS_DDR &= ~(1 << BUTTON1) | ~(1 << BUTTON2);
	// Activate pull-up resistors
	PORTC |= (1 << BUTTON1) | (1 << BUTTON2);
	
	// Set EN_LED as output
	EN_LED_PORT_DDR |= (1 << EN_LED);
	// Initialize EN_LED to 0
	PORTB &= ~(1 << EN_LED);
	init_timer0();
	
    while (1) 
    {
		bt_control_servo();
    }
}

