/**
 @file main.c
 @brief Lab 3 Starter Code
 @version .01
 @mainpage Lab 3 Starter Code

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
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "diskio.h"

/** Constants */
#define F_CPU 1000000UL
#define DEBUG 1

/// Success error code
#define ERR_NONE 0x00

/// Failed to mount SDC/MMC error code
#define ERR_FMOUNT 0x01

/// No SDC/MMC present error code
#define ERR_NODISK 0x02

/// Unable to initialize FAT file system error code
#define ERR_NOINIT 0x03

/// MMC/SDC write protected error code
#define ERR_PROTECTED 0x04

#define ERR_FOPEN 0x05
#define ERR_TIMER 0x06
#define ERR_FWRITE 0x07
#define ERR_FULL 0x08
#define ERR_FCLOSE 0x09

/** Global Variables */


/** Functions */

#if DEBUG == 1
/** This function needs to setup the variables used by the UART to enable the UART and transmit at 9600bps. This
 function should always return 0. Remember, by default the Wunderboard runs at 1mHz for its system clock.*/
uint8_t initializeUART(void) {
	/* Set baud rate */
	UBRR1H = 0;
	UBRR1L = 12;

	/* Set the U2X1 bit */
	UCSR1A = (1 << U2X1);

	/* Enable transmitter */
	UCSR1B |= (1 << TXEN1) | (1 << RXEN1);

	/* Set frame format: 8data, 1stop bit */
	UCSR1C |= (1 << UCSZ10) | (1 << UCSZ11);
	UCSR1C &= ~(1 << USBS1);

	return 0;
}

/** This function needs to write a single byte to the UART. It must check that the UART is ready for a new byte
 and return a 1 if the byte was not sent.
 @param [in] data This is the data byte to be sent.
 @return The function returns a 1 or error and 0 on successful completion.*/
uint8_t SendByteUART(unsigned char data) {

	if (!(UCSR1A & (1 << UDRE1))) {
		return 1;
	} else {
		UDR1 = data;
	}
	return 0;
}

/** This function needs to writes a string to the UART. It must check that the UART is ready for a new byte and
 return a 1 if the string was not sent.
 @param [in] str This is a pointer to the data to be sent.
 @return The function returns a 1 or error and 0 on successful completion.*/
uint8_t SendStringUART(unsigned char *data) {

	uint8_t length = strlen((const char *) data);
	uint8_t i;
	if (SendByteUART(data[0]) == 1) {
		return 1;
	} else {
		for (i = 1; i < length; i++) {
			while (SendByteUART(data[i]))
				;
		}
	}
	return 0;
}

uint8_t checkReceiveByteUART(void){
	return ((UCSR1A & (1 << RXC1)));
}

uint8_t ReceiveByteUART(void) {
	while (!(UCSR1A & (1 << RXC1)));
	return UDR1;
}

void printErrorUART(uint8_t err) {
	switch (err) {
	case ERR_FMOUNT:
		while (SendStringUART("ERROR: Could not mount SDC/MMC\r\n") == 1);
		break;
	case ERR_NODISK:
		while (SendStringUART("ERROR: No SDC/MMC present\r\n") == 1);
		break;
	case ERR_NOINIT:
		while (SendStringUART("ERROR: Unable to initialize FAT file system\r\n")== 1);
		break;
	case ERR_PROTECTED:
		while (SendStringUART("ERROR: SDC/MMC is write protected\r\n") == 1);
		break;
	case ERR_FOPEN:
		while (SendStringUART("ERROR: Unable to open file\r\n") == 1);
		break;
	case ERR_TIMER:
		while (SendStringUART(
				"ERROR: Clock selector for TIMER/COUNTER0 is invalid\r\n") == 1);
		break;
	case ERR_FWRITE:
		while (SendStringUART("ERROR: Unable to write to file\r\n") == 1);
		break;
	case ERR_FULL:
		while (SendStringUART("ERROR: File system is full\r\n") == 1);
		break;
	case ERR_FCLOSE:
		while (SendStringUART("ERROR: Unable to close file\r\n") == 1);
		break;
	default:
		while (SendStringUART("ERROR: Unknown\r\n") == 1);
		break;
	}
}

#else
#define initializeUART()
#define SendByteUART( data)
#define SendStringUART(str)
#define checkReceiveByteUART()
#define ReceiveByteUART()
#define printErrorUART(err)
#endif // DEBUG
/** The clearArray() function turns off all LEDS on the Wunderboard array. It accepts no inputs and returns nothing*/
void clearArray(void) {
	PORTC = 0x00;
	PORTB |= (1 << PB6) | (1 << PB7); /** Enable latches*/
	PORTB &= ~((1 << PB6) | (1 << PB7)); /** Disable latches*/
}

void setArrayAmber(unsigned char rows) {
	clearArray();
	PORTC = rows;
	PORTB |= (1 << PB6) | (1 << PB7);
	PORTB &= ~(1 << PB6) | (1 << PB7);
}

void setArrayGreen(unsigned char rows) {
	clearArray();
	PORTC = rows;
	PORTB |= (1 << PB7);
	PORTB &= ~(1 << PB7);
}

void setArrayRed(unsigned char rows) {
	clearArray();
	PORTC = rows;
	PORTB |= (1 << PB6);
	PORTB &= ~(1 << PB6);
}

/** The initialize() function initializes all of the Data Direction Registers for the Wunderboard. Before making changes to DDRx registers, ensure that you have read the peripherals section of the Wunderboard user guide.*/
void initialize(void) {
	/** Port A is the switches and buttons. They should always be inputs. ( 0 = Input and 1 = Output )*/
	DDRA = 0b00000000;

	/** Port B has the LED Array color control, SD card, and audio-out on it. Leave DDRB alone. ( 0 = Input and 1 = Output )*/
	DDRB = 0b11000111;

	/** Port C is for the 'row' of the LED array. They should always be outputs. ( 0 = Input and 1 = Output )*/
	DDRC = 0b11111111;

	/** Port D has the Serial on it. Leave DDRB alone. ( 0 = Input and 1 = Output )*/
	DDRD = 0b00000000;

	/** Port E has the LED Array Column control out on it. Leave DDRE alone. ( 0 = Input and 1 = Output )*/
	DDRE = 0b00000111;

	/** Port F has the accelerometer and audio-in on it. Leave DDRF alone. ( 0 = Input and 1 = Output )*/
	DDRF = 0b00000000;
}

unsigned char read_adc(uint8_t channel){

	unsigned char test;

	ADMUX = 0x60 | channel; // Set the channel to the one we want
	ADCSRA = 0b11000110; // Start a new sample.
	while ((ADCSRA & 0b00010000) == 0 ); // Wait for a Valid Sample
	ADCSRA |= 0b00010000; // Tell ADC you have the sample you want.
	ADCSRA |= 0b01000000; // Start a new sample.
	while ((ADCSRA & 0b00010000) == 0 ); // Wait for a Valid Sample
	ADCSRA |= 0b00010000; // Tell ADC you have the sample you want.

	test = ADCH;
	ADCSRA = 0x00; // Disable the ADC

	return (test);
}

/**
 * @brief Initializes the SDC/MMC and mounts the FAT filesystem, if it exists.
 *
 * Initializes the connected SDC/MMC and mounts the FAT filesystem, if it
 * exists. Returns error code on failure.
 *
 * @param fs Pointer to FATFS structure
 * @return Returns ERR_FMOUNT, ERR_NODISK, ERR_NOINIT, or ERR_PROTECTED on error, or ERR_NONE on success.
 */
uint8_t initializeFAT(FATFS* fs) {
	DSTATUS driveStatus;

	// Mount and verify disk type
	if (f_mount(0, fs) != FR_OK) {
		// Report error
		return ERR_FMOUNT;
	}

	driveStatus = disk_initialize(0);

	// Verify that disk exists
	if (driveStatus & STA_NODISK) {
		// Report error
		return ERR_NODISK;
	}

	// Verify that disk is initialized
	if (driveStatus & STA_NOINIT) {
		// Report error
		return ERR_NOINIT;
	}

	// Verify that disk is not write protected
	if (driveStatus & STA_PROTECT) {
		// Report error
		return ERR_PROTECTED;
	}

	return ERR_NONE;
}

/** This function needs to setup the variables used by TIMER0 Compare Match (CTC) mode with 
 a base clock frequency of clk/1024. This function should return a 1 if it fails and a 0 if it
 does not. Remember, by default the Wunderboard runs at 1mHz for its system clock.
 @return This function returns a 1 is unsuccessful, else return 0.*/
uint8_t initializeTIMER0(void) {
	/* Set the CTC mode */
	TCCR0A = (2 << WGM00);

	/* Set the Clock Frequency */
	TCCR0B = (5 << CS20);

	/* Set initial count value */
	OCR0A = 0;

	return 0;
}

/** This function checks if TIMER0 has elapsed. 
 @return This function should return a 1 if the timer has elapsed, else return 0*/
uint8_t checkTIMER0(void) {

	if (TIFR0 & (1 << OCF0A)) {
		TIFR0 |= (1 << OCF0A);
		return 1;
	}
	return 0;

}

/** This function takes two values, clock and count. The value of count should be copied into OCR0A
 *  and the value of clock should be used to set CS02:0. The TCNT0 variable should also be reset to 0
 *  so that the new timer rate starts from 0.
 @param [in] clock Insert Comment
 @param [in] count Insert Comment
 @return The function returns a 1 or error and 0 on successful completion.*/
uint8_t setTIMER0(uint8_t clock, uint8_t count) {

	if (clock < 1 || clock > 5) { //makes sure there is a clock and uses the built in clock.
		return 1;
	}

	OCR0A = count;
	
	TCCR0B &= ~(7 << CS20);
	TCCR0B |= (clock << CS00);

	TCNT0 = 0;

	return 0;
}

/** Main Function */
int main(void) {
	/** Local Variables */
	enum states {init, start, idle, interrupt, time, clean, stop} state;
	state = init;
	uint8_t count = 0;
	uint8_t adcval, len;
	char string[8];

	FATFS fs;
	FIL log;
	uint8_t result;
	UINT bytesWritten;

	while (1){
		switch (state){
			case init :
				initialize();
				clearArray();
				initializeUART();
				setArrayAmber(0b00000001);

				while (SendStringUART("Initializing devices...\r\n") == 1);
				result = initializeFAT(&fs);
			
				_delay_ms(15);
				setArrayAmber(0b00000011);
				_delay_ms(25);
				setArrayAmber(0b00000111);
				_delay_ms(45);
				setArrayAmber(0b00001111);
				_delay_ms(75);
				setArrayAmber(0b00011111);
				_delay_ms(115);
				setArrayAmber(0b00111111);
				_delay_ms(165);
				setArrayAmber(0b01111111);
				_delay_ms(225);
				setArrayAmber(0b11111111);
				initializeTIMER0();

				//SendStringUART("Initializing MMC/SDC and FAT file system\r\n");

				if (result != ERR_NONE)
				{
					// Report error
					printErrorUART(result);
					while (SendStringUART("Error! Shutting down...\r\n") == 1);
					setArrayRed(result);
					state = stop;
					while (1);
				}
				//_delay_ms(500);
				state = start;
				break;

			case start :
				if(f_open(&log, "/log.txt", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
				{
					// Report error
					printErrorUART(ERR_FOPEN);
					while (SendStringUART("Error! Shutting down...\r\n") == 1);
					setArrayRed(ERR_FOPEN);
					state = stop;
					while (1);
				}
				while (SendStringUART("File log.txt opened\r\n") == 1);
				while (SendStringUART("Press \'s\' key to start and press again to stop.\r\n") == 1);
				while (SendStringUART("Going into idle...\n\r") == 1);
				//PORTE = 1;
				setArrayGreen(0xff);
				state = idle;
				break;

			case idle :
				if (checkReceiveByteUART()){
					if(ReceiveByteUART() == 's'){
						state = time;
						clearArray();
						setTIMER0(5, 255);
						while (SendStringUART("Setting timer...\n\r") == 1);
						while (SendStringUART("Running program...\r\n") == 1);
						while (SendStringUART("\r\nBeginning log...\r\n") == 1);
						if (f_write(&log, "Beginning log set:\r\n", 20, &bytesWritten) != FR_OK) {
						printErrorUART(ERR_FWRITE);
						while (SendStringUART("Error! Shutting down...\r\n") == 1);
						state = stop;
						setArrayRed(ERR_FWRITE);
					}
					}
				}
				break;

			case time :
				if (checkTIMER0() == 1){
					clearArray();
					count++;
				}
				if (count > 1){
					setArrayAmber(3);
					adcval = read_adc(PF5);
					sprintf(string, "%d\r\n", adcval);
					len = strlen(string);
					if (f_write(&log, string, len, &bytesWritten) != FR_OK){
						printErrorUART(ERR_FWRITE);
						while (SendStringUART("Error! Shutting down...\r\n") == 1);
						state = stop;
						setArrayRed(ERR_FWRITE);
					}
					while (SendStringUART("Logging...\r\n") == 1);
					count = 0;
				}
				if (checkReceiveByteUART()){
					if (ReceiveByteUART() == 's'){
						state = clean;
						setArrayGreen(0xff);
						if (f_write(&log, "Ending log set!\r\n", 17, &bytesWritten) != FR_OK) {
						printErrorUART(ERR_FWRITE);
						while (SendStringUART("Error! Shutting down...\r\n") == 1);
						state = stop;
						setArrayRed(ERR_FWRITE);
					}
					}
				}
				break;

			case clean :
				//clearArray();
				if (f_close(&log) != FR_OK) /*close file*/
				{
					printErrorUART(ERR_FCLOSE);
					while (SendStringUART("Error! Shutting down...\r\n") == 1);
					state = stop;
					setArrayRed(ERR_FCLOSE);
					while (1);
				}

				f_mount(0,0); /*unmount disk*/

				while (SendStringUART("Everything has closed...\r\n") == 1);
				while (SendStringUART("Shutting down...\r\n") == 1);
				setArrayGreen(0xff);
				_delay_ms(15);
				setArrayGreen(0b11111110);
				_delay_ms(25);
				setArrayGreen(0b11111100);
				_delay_ms(45);
				setArrayGreen(0b11111000);
				_delay_ms(75);
				setArrayGreen(0b11110000);
				_delay_ms(115);
				setArrayGreen(0b11100000);
				_delay_ms(165);
				setArrayGreen(0b11000000);
				_delay_ms(225);
				setArrayGreen(0b10000000);
				_delay_ms(305);
				setArrayGreen(0);
				//while(1);
				//clearArray();
				SendStringUART("Goodbye!\r\n");
				state = stop;
				break;

			case stop :
				_delay_ms(500);
				clearArray();
				if (checkReceiveByteUART()){
					if (ReceiveByteUART() == 'r'){
						state = init;
					}
				}
				break;

			default :
				while (SendStringUART("I don't know what happened...\r\n") == 1);
				PORTC = ~PORTC;
				break;

		}
	}

	return 0;
}
