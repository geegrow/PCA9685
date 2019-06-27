#ifndef _PCA9685_H
#define _PCA9685_H

#include <Arduino.h>
#include <Wire.h>

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD

/*
 * Because PCA9685 can't produce lower frequency..
 * ...and more than 1526 Hz also
 */
#define MINIMUM_ALLOWED_FREQUENCY 24 // Hz
#define MAXIMUM_ALLOWED_FREQUENCY 1526 // Hz


class PCA9685 {
	public:
		PCA9685(uint8_t addr = 0x40, uint16_t frequency = 50);
		void begin(void);
		void reset(void);
		void setPWMFreq(uint16_t frequency);
		void setPWM(uint8_t num, uint16_t on, uint16_t off);
		void setPWM(uint8_t num, uint8_t);
		void setPin(uint8_t num, uint16_t val, bool invert=false);

	private:
		// Physical I2C address of PCA9685
		uint8_t deviceAddr;
		uint16_t frequency;

		uint8_t read(uint8_t registerAddress);
		void write(uint8_t registerAddress, uint8_t data);
};

#endif
