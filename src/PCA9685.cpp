#include "PCA9685.h"
#include <I2CTransport.h>

// Set to true to print some debug messages, or false to disable them.
//#define ENABLE_DEBUG_OUTPUT


/**************************************************************************/
/*!
    @brief  Instantiates a new PCA9685 PWM driver chip with the I2C address on the Wire interface. On Due we use Wire1 since its the interface on the 'default' I2C pins.
    @param  addr The 7-bit I2C address to locate this chip, default is 0x40
*/
/**************************************************************************/
PCA9685::PCA9685(uint8_t address, uint16_t frequency) {
	this->deviceAddr = address;
	this->frequency = frequency;
}

/**************************************************************************/
/*!
	@brief  Setups the I2C interface and hardware
*/
/**************************************************************************/
void PCA9685::begin(void) {
	reset();
	setPWMFreq(this->frequency);
}

/**************************************************************************/
/*!
	@brief  Sends a reset command to the PCA9685 chip over I2C
*/
/**************************************************************************/
void PCA9685::reset(void) {
	write(PCA9685_MODE1, 0x80);
	delay(10);
}

/**************************************************************************/
/*!
	@brief  Sets the PWM frequency for the entire chip, up to ~1.6 KHz
	@param  freq Floating point frequency that we will attempt to match
*/
/**************************************************************************/
void PCA9685::setPWMFreq(uint16_t frequency) {
	frequency = constrain(frequency, MINIMUM_ALLOWED_FREQUENCY, MAXIMUM_ALLOWED_FREQUENCY);

	this->frequency = frequency;

	#ifdef ENABLE_DEBUG_OUTPUT
		Serial.print("Attempting to set freq ");
		Serial.println(frequency);
	#endif

	uint32_t prescaleval = 2500000000; //Instead float value 25000000.00
	prescaleval = (prescaleval / 4096) / frequency;
	prescaleval = (prescaleval * 10)/9;
	prescaleval -= 100;

	uint8_t prescale = (uint8_t)((prescaleval + 50)/100);

	#ifdef ENABLE_DEBUG_OUTPUT
		Serial.print("Estimated pre-scale: "); Serial.println(prescaleval);
		Serial.print("Final pre-scale: "); Serial.println(prescale);
	#endif

	uint8_t oldmode = read(PCA9685_MODE1);
	uint8_t newmode = (oldmode&0x7F) | 0x10; // sleep

	write(PCA9685_MODE1, newmode); // go to sleep
	write(PCA9685_PRESCALE, prescale); // set the prescaler
	write(PCA9685_MODE1, oldmode);

	delay(5);
	write(PCA9685_MODE1, oldmode | 0xa0);  //  This sets the MODE1 register to turn on auto increment.

	#ifdef ENABLE_DEBUG_OUTPUT
		Serial.print("Mode now 0x"); Serial.println(read(PCA9685_MODE1), HEX);
	#endif
}

/**************************************************************************/
/*!
	@brief  Sets the PWM output of one of the PCA9685 pins
	@param  num One of the PWM output pins, from 0 to 15
	@param  on At what point in the 4096-part cycle to turn the PWM output ON
	@param  off At what point in the 4096-part cycle to turn the PWM output OFF
*/
/**************************************************************************/
void PCA9685::setPWM(uint8_t num, uint16_t on, uint16_t off) {
	#ifdef ENABLE_DEBUG_OUTPUT
		Serial.print("Setting PWM "); Serial.print(num); Serial.print(": "); Serial.print(on); Serial.print("->"); Serial.println(off);
	#endif

	uint8_t buff[4];
    buff[0] = (uint8_t)on;
    buff[1] = on>>8;
    buff[2] = (uint8_t)off;
    buff[3] = off>>8;

    // Set preventFreezing = true when call I2C
    I2CTransport::writeBytes(deviceAddr, LED0_ON_L+(4*num), 4, buff, true);
}

void PCA9685::setPWM(uint8_t pinNum, uint8_t pwmValue) {
	volatile uint16_t off = 0;

	#ifdef ENABLE_DEBUG_OUTPUT
		Serial.print("Setting PWM "); Serial.print(pinNum); Serial.print(": "); Serial.print(pwmValue); Serial.print("%");
	#endif

	if (pwmValue > 0) {
		off = (uint16_t)((4096 * (uint32_t)pwmValue)/255);
		off = off -1;
	}

	setPWM(pinNum, 0, off);
}

/**************************************************************************/
/*!
	@brief  Helper to set pin PWM output. Sets pin without having to deal with
            on/off tick placement and properly handles a zero value as completely
            off and 4095 as completely on.  Optional invert parameter supports
            inverting the pulse for sinking to ground.
	@param  num One of the PWM output pins, from 0 to 15
	@param  val The number of ticks out of 4096 to be active, should be a value
            from 0 to 4095 inclusive.
	@param  invert If true, inverts the output, defaults to 'false'
*/
/**************************************************************************/
void PCA9685::setPin(uint8_t num, uint16_t val, bool invert)
{
	// Clamp value between 0 and 4095 inclusive.
	val = min(val, (uint16_t)4095);
	if (invert) {
		if (val == 0) {
			// Special value for signal fully on.
			setPWM(num, 4096, 0);
		}
		else if (val == 4095) {
			// Special value for signal fully off.
			setPWM(num, 0, 4096);
		}
		else {
			setPWM(num, 0, 4095-val);
		}
	} else {
		if (val == 4095) {
			// Special value for signal fully on.
			setPWM(num, 4096, 0);
		}
		else if (val == 0) {
			// Special value for signal fully off.
			setPWM(num, 0, 4096);
		}
		else {
			setPWM(num, 0, val);
		}
	}
}

/*******************************************************************************************/

uint8_t PCA9685::read(uint8_t registerAddress) {
	return I2CTransport::readByte(deviceAddr, registerAddress);
}

void PCA9685::write(uint8_t registerAddress, uint8_t data) {
	I2CTransport::writeByte(deviceAddr, registerAddress, data);
}
