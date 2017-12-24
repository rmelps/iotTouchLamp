// R Melpignano 18 DEC 2017
// ------------------------
// Pin defines for RGBLED Project with pushbutton inputs

// General pins controlling balance of RGB of LEDs

#define RLED 						PD6
#define GLED 						PD3
#define BLED						PD5

#define LED_PORT					PORTD
#define LED_PIN                 	PIND
#define LED_DDR                 	DDRD

#define RBUTTON						PC3
#define GBUTTON						PC2
#define BBUTTON						PC1

#define BUTTON_PORT					PORTC
#define BUTTON_PIN					PINC
#define BUTTON_DDR					DDRC

// SPI port defines

#define SPI_SS                     	PB2
#define SPI_MOSI                    PB3
#define SPI_MISO                    PB4
#define SPI_SCK                     PB5

#define SPI_PORT               		PORTB
#define SPI_PIN                		PINB
#define SPI_DDR                		DDRB
