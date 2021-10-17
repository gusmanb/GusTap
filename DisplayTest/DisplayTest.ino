/*
 Name:		DisplayTest.ino
 Created:	08/10/2021 12:41:14
 Author:	Gus
*/

#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>

#define I2C_ADDRESS 0x3C

SSD1306AsciiAvrI2c display;

// the setup function runs once when you press reset or power the board
void setup() {
	display.begin(&Adafruit128x32, I2C_ADDRESS);
	display.setFont(System5x7);
	display.clear();
	display.setInvertMode(false);
	display.println("12345678901234567890123456789012345678901234567890");
	display.setInvertMode(true);
	display.println("line!");
	display.println("line!");
	display.println("line!");
}

// the loop function runs over and over again until power down or reset
void loop() {
  
}
