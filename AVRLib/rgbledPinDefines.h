// R Melpignano 18 DEC 2017
// ------------------------
// Pin defines for RGBLED Project with pushbutton inputs

// General pins controlling balance of RGB of LEDs

#define RLED 						PD5
#define GLED 						PD3
#define BLED						PD6

#define LED_PORT					PORTD
#define LED_PIN                 	PIND
#define LED_DDR                 	DDRD

#define RBUTTON						PC2
#define GBUTTON						PC1
#define BBUTTON						PC3

#define BUTTON_PORT					PORTC
#define BUTTON_PIN					PINC
#define BUTTON_DDR					DDRC

#define CHANGE_PORT					PORTC
#define CHANGE_PIN					PINC
#define CHANGE_DDR					DDRC
#define CHANGE_P					PC3

#define STATUS_LED					PB0
#define ERROR_LED					PC0

// Debug LEDs
#define DEBUG_0						PD2
#define DEBUG_1						PD4
#define DEBUG_2						PB6
#define DEBUG_3						PB7
#define DEBUG_01_DDR				DDRD
#define DEBUG_23_DDR				DDRB
#define DEBUG_01_PORT				PORTD
#define DEBUG_23_PORT				PORTB

#define DEBUG_0_ON					DEBUG_01_PORT |= (1 << DEBUG_0)
#define DEBUG_0_OFF					DEBUG_01_PORT &= ~(1 << DEBUG_0)
#define DEBUG_1_ON					DEBUG_01_PORT |= (1 << DEBUG_1)
#define DEBUG_1_OFF					DEBUG_01_PORT &= ~(1 << DEBUG_1)
#define DEBUG_2_ON					DEBUG_23_PORT |= (1 << DEBUG_2)
#define DEBUG_2_OFF					DEBUG_23_PORT &= ~(1 << DEBUG_2)
#define DEBUG_3_ON					DEBUG_23_PORT |= (1 << DEBUG_3)
#define DEBUG_3_OFF					DEBUG_23_PORT &= ~(1 << DEBUG_3)


// SPI port defines

#define SPI_SS                     	PB2
#define SPI_MOSI                    PB3
#define SPI_MISO                    PB4
#define SPI_SCK                     PB5

#define SPI_PORT               		PORTB
#define SPI_PIN                		PINB
#define SPI_DDR                		DDRB

