#include "o3.h"
#include "gpio.h"
#include "systick.h"

/**************************************************************************//**
 * @brief Konverterer nummer til string 
 * Konverterer et nummer mellom 0 og 99 til string
 *****************************************************************************/
void int_to_string(char *timestamp, unsigned int offset, int i) {
    if (i > 99) {
        timestamp[offset]   = '9';
        timestamp[offset+1] = '9';
        return;
    }

    while (i > 0) {
	    if (i >= 10) {
		    i -= 10;
		    timestamp[offset]++;
		
	    } else {
		    timestamp[offset+1] = '0' + i;
		    i=0;
	    }
    }
}

/**************************************************************************//**
 * @brief Konverterer 3 tall til en timestamp-string
 * timestamp-argumentet mÃ¥ vÃ¦re et array med plass til (minst) 7 elementer.
 * Det kan deklareres i funksjonen som kaller som "char timestamp[7];"
 * Kallet blir dermed:
 * char timestamp[7];
 * time_to_string(timestamp, h, m, s);
 *****************************************************************************/
void time_to_string(char *timestamp, int h, int m, int s) {
    timestamp[0] = '0';
    timestamp[1] = '0';
    timestamp[2] = '0';
    timestamp[3] = '0';
    timestamp[4] = '0';
    timestamp[5] = '0';
    timestamp[6] = '\0';

    int_to_string(timestamp, 0, h);
    int_to_string(timestamp, 2, m);
    int_to_string(timestamp, 4, s);
}


port_pin_t led;
port_pin_t pb0;
port_pin_t pb1;
typedef struct {int h, m, s;} time_t;
time_t time = {0};
char time_string[8] = "0000000\0";
static int state = SETSEC;


volatile GPIO* gpio = (GPIO*)GPIO_BASE;
volatile SYSTICK* systick = (SYSTICK*)SYSTICK_BASE;



void GPIO_ODD_IRQHandler(void){
/* Interrupt for pinne med oddetall (1,3,5,...) */
    gpio->IFC = 1 << pb0.pin;
    switch (state){
    	case SETSEC: {
    		if(time.s < 59)
    			time.s++;
    		else
    			time.s = 0;
    		updateDisplay();
    	}break;
    	case SETMIN: {
    		if(time.m < 59)
    			time.m++;
    		else
    			time.m = 0;
    		updateDisplay();
    	}break;
    	case SETHR: {
    		if(time.h < 99)
    			time.h++;
    		else
    			time.h = 0;
    		updateDisplay();
    	}break;
    	case ALARM: {
    		// Nothing
    	}break;
    	case COUNTDOWN: {
    		// Nothing
    	}break;
    }

}
void GPIO_EVEN_IRQHandler(void){
/* Interrupt for pinne med partall (0,2,4,...) */
    gpio->IFC = 1 << pb1.pin;
    switch (state){
    	case SETSEC: {
    		state = SETMIN;
    	}break;
    	case SETMIN: {
    		state = SETHR;
    	}break;
    	case SETHR: {
    		state = COUNTDOWN;
    		startClock();
    	}break;
    	case ALARM: {
    		state = SETSEC;
			gpio->ports[led.port].DOUTCLR = 1 << led.pin;
    	}break;
    	case COUNTDOWN: {
    		// Nothing
    	}break;
    }

}
void SysTick_Handler(void){
/* Interrupt fra SysTick */
	tickTime();
	updateDisplay();
}

// Start clock and systick interrupts
void startClock() {
	systick->VAL = systick->LOAD; // set startvalue
	systick->CTRL |= SysTick_CTRL_ENABLE_Msk; // enable tick bit
}

// Stop clock and hence halt systick interrupts
void stopClock() {
	systick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk); // disable EN bit
}

void setFlag(volatile word* w, int pin, word flag) {
	*w &= ~(0b1111 << (pin * 4)); // clear
	*w |= flag << (pin*4); // set
}

// Initialize modes, interrupt information
void initializeIO() {
	// Set led0 to output mode
	setFlag(&gpio->ports[led.port].MODEL, led.pin, GPIO_MODE_OUTPUT);

	// Create interrupt for pb0 on fall
	setFlag(&gpio->ports[pb0.port].MODEH, pb0.pin-8, GPIO_MODE_INPUT);
	setFlag(&gpio->EXTIPSELH, pb0.pin-8, 0b0001);
	gpio->EXTIFALL |= 1 << pb0.pin;
	gpio->IEN |= 1 << pb0.pin;

	//Create interrupt for pb1 on fall
	setFlag(&gpio->ports[pb1.port].MODEH, pb1.pin-8, GPIO_MODE_INPUT);
	setFlag(&gpio->EXTIPSELH, pb1.pin-8, 0b0001);
	gpio->EXTIFALL |= 1 << pb1.pin;
	gpio->IEN |= 1 << pb1.pin;

	// Set information about systick interrupt, interrupt started in startClock()
	systick->LOAD = FREQUENCY;
	systick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;

	updateDisplay();
}

void updateDisplay(){
	time_to_string(time_string, time.h, time.m, time.s);
	lcd_write(time_string);
}

void tickTime(){
	if(time.s <= 0){
		if(time.m <= 0){
			if(time.h <= 0){
				stopClock();
				state = ALARM;
				gpio->ports[led.port].DOUTSET = 1 << led.pin;
				return;
			}
			time.h--;
			time.m = 60;
		}
		time.m--;
		time.s = 60;
	}
	time.s--;
}

int main(void) {
    init();

    led.port = GPIO_PORT_E;
    led.pin = 2;

    pb0.port = GPIO_PORT_B;
    pb0.pin = 9;

    pb1.port = GPIO_PORT_B;
    pb1.pin = 10;

    gpio->ports[led.port].DOUTCLR = (1 << led.pin);

    initializeIO();

    return 0;
}

