/**
 @file main.c
 @brief Lab 2 Starter Code
 @version .01
 @mainpage Lab 2 Starter Code

 @section intro Code Overview
 
 @section hw Hardware Pin Out
 Port A:
 A0 - A3 : Push Buttons
 A4 - A7 : Slide Switches

 Port B:
 B0 - B3 : SPI (SD Card)
 B4		: Nothing
 B5		: Audio Out
 B6 		: Red Enable
 B7 		: Green Enable

 Port C:
 C0 - C7 : LED Array (Row)

 Port D:
 D0 - D1 : Nothing
 D2		: Serial RX
 D3		: Serial TX
 D4 - D7	: Nothing

 Port E:
 E0 - E2 : LED Array (Column)
 E3		: USB (UID)
 E4 - E5	: Nothing
 E6		: Relay
 E7		: Nothing

 Port F:
 F0 		: ADC Channel 0
 F1 		: ADC Channel 1
 F2 		: ADC Channel 2
 F3 		: ADC Channel 3
 F4		: ADC Channel 4 (Audio In)
 F5 		: ADC Channel 5 (Accel X Axis)
 F6 		: ADC Channel 6 (Accel Y Axis)
 F7 		: ADC Channel 7 (Accel Z Axis (if installed))

 */

/** Includes */
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

/** Constants */
#define F_CPU 1000000U
#define DEBUG

/** Global Variables */

/** Functions */

/** The initialize() function initializes all of the Data Direction Registers for the Wunderboard. Before making changes to DDRx registers, ensure that you have read the peripherals section of the Wunderboard user guide.*/
void initialize(void) {
	/** Port A is the switches and buttons. They should always be inputs. ( 0 = Input and 1 = Output )*/
	DDRA = 0b00000000;

	/** Port B has the LED Array color control, SD card, and audio-out on it. Leave DDRB alone. ( 0 = Input and 1 = Output )*/
	DDRB = 0b11000000;

	/** Port C is for the 'row' of the LED array. They should always be outputs. ( 0 = Input and 1 = Output )*/
	DDRC = 0b11111111;

	/** Port D has the Serial on it. Leave DDRB alone. ( 0 = Input and 1 = Output )*/
	DDRD = 0b00000000;

	/** Port E has the LED Array Column control out on it. Leave DDRE alone. ( 0 = Input and 1 = Output )*/
	DDRE = 0b00000111;

	/** Port F has the accelerometer and audio-in on it. Leave DDRF alone. ( 0 = Input and 1 = Output )*/
	DDRF = 0b00000000;
}

/** The clearArray() function turns off all LEDS on the Wunderboard array. It accepts no inputs and returns nothing*/
void clearArray(void) {
	PORTC = 0x00;
	PORTB |= (1 << PB6) | (1 << PB7); /** Enable latches*/
	PORTB &= ~((1 << PB6) | (1 << PB7)); /** Disable latches*/
}
#ifdef DEBUG
/** This function needs to setup the variables used by the UART to enable the UART and tramsmit at 9600bps. This 
 function should always return 0. Remember, by defualt the Wunderboard runs at 1mHz for its system clock.*/
unsigned char InitializeUART(void) {

	//unsigned int baud = 6; //f_osc = 1MHz, baud = 9600 -> set 12

	/* Set baud rate */
	UBRR1H = 0;
	UBRR1L = 12;
	//UBRR1H = (unsigned char) (baud >> 8);
	//UBRR1L = (unsigned char) baud;
	/* Set the U2X1 bit */
	UCSR1A = (1 << U2X1);
	//UCSR1A = (1 << U2X1);
	/* Enable transmitter */
	UCSR1B |= (1 << TXEN1) | (1 << RXEN1);
	//UCSR1B = (1 << RXEN1) | (1 << TXEN1);
	/* Set frame format: 8data, 1stop bit */
	UCSR1C |= (1 << UCSZ10) | (1 << UCSZ11);
	UCSR1C &= ~(1 << USBS1);
	//UCSR1C = (1 << USBS1) /*1 stop bit*/ | (3 << UCSZ10) /*8-bit frame*/ | (0b00 << UPM10) /*Disable parity*/ ;

	return 0;
}

/** This function needs to write a single byte to the UART. It must check that the UART is ready for a new byte 
 and return a 1 if the byte was not sent.
 @param [in] data This is the data byte to be sent.
 @return The function returns a 1 or error and 0 on successful completion.*/

unsigned char SendByteUART(unsigned char data) {


	if (!(UCSR1A & (1<<UDRE1))) {
		return 1;
	} else {
		UDR1 = data;
	}
	return 0;
}

/** This function needs to write a string to the UART. It must check that the UART is ready for a new byte and
 return a 1 if the string was not sent.
 @param [in] data This is a pointer to the data to be sent.
 @return The function returns a 1 or error and 0 on successful completion.*/

unsigned char SendStringUART(unsigned char *data) {

	uint8_t length = strlen((const char *) data);
	uint8_t i;
	if (SendByteUART(data[0]) == 1){
		return 1;
	} else {
		for (i = 1; i < length; i++){
			while (SendByteUART(data[i]));
		}
	}
	return 0;
}
#endif

/** Main Function */

int main(void) {
	/** Local Variables */

	initialize();
	InitializeUART();
	clearArray();
	char string[13];
	int num;
	PORTB = 0b10000000;

	while (1) {
		if (PINA == 0b00000000) {
			PORTB = 0b10000000;
			PORTE = 7;
			PORTC = ~(PINA);
		} else {
			PORTB = 0b11000000;
			PORTE = 0;
			PORTC = PINA;
		}
		num = PINA;
		sprintf(string, "Port A is: %d\n", num);
		while (SendStringUART((unsigned char *) string) == 1);
	}
	return 0;

}//main
