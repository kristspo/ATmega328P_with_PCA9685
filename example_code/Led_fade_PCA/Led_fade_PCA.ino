/*
 Example sketch for ATmega 328P development board with 16-channel PCA9685 PWM controller.
 https://github.com/kristspo/ATmega328P_with_PCA9685
 
 Fade leds attached to PCA9685 pwm channels by writing PCA9685 registers using Arduino Wire.
 Uses linear or adjusted fading curve for more natural led brightness at low levels.
 
 Connections for Arduino Uno/Nano/Pro:
 On board LED
 ** pin 13
 PCA9685 attached to I2C
 ** SDA - A4
 ** SCL - A5
 LEDs attached from on board pwm channel outputs to GND
*/

#include <Wire.h>

// PCA9685 start channel and channel count to update. LED_CH_START index starts at 0 (0 to 15)
#define LED_CH_START  0
#define LED_CH_COUNT  8
// enable (1) or disable (0) adjusted fading curve for more natural brightness at low levels
#define LED_FADE_LOG  1
// PCA9685 I2C address - it can be changed using on board dip switch (0x40 to 0x47)
#define PCA_I2CADDR   0x40


unsigned char count;
unsigned char fadeout;

// adjusted fade curve values - used if LED_FADE_LOG is 1
const unsigned int pwm_log[] PROGMEM = {0,1,4,9,16,25,36,49,64,81,100,121,144,169,196,225,256,289,324,361,400,441,484,529,576,625,676,729,784,841,900,961,1024,1089,1156,1225,1296,1369,1444,1521,1600,1681,1764,1849,1936,2025,2115,2208,2303,2400,2499,2600,2703,2808,2915,3024,3135,3248,3363,3480,3599,3720,3843,3968,4095};

void setup() {
	// Serial.begin(57600);

	// onboard led pin output
	pinMode(LED_BUILTIN, OUTPUT);

	Wire.begin();
	// reset every PCA9685 on I2C bus
	Wire.beginTransmission(0x00); // general call address
	Wire.write(0x06); // reset byte
	Wire.endTransmission();
	delay(20);
	// setup PCA9685
	Wire.beginTransmission(PCA_I2CADDR);
	Wire.write(0x00); // register address
	Wire.write(0x20); // register auto-increment bit set; sleep mode bit clear
	Wire.endTransmission();
}

void loop() {
  unsigned int pwm;
  char z, reg_addr;

  delay(15);
  if (count < 64) {
    count ++;

    // update PCA9685 pwm value (0 to 4095)
    if (LED_FADE_LOG) {
      // use adjusted fading curve
      if (fadeout) {
        pwm = pgm_read_word(& pwm_log[64 - count]);
      } else {
        pwm = pgm_read_word(& pwm_log[count]);
      }
    } else {
      // use linear fading curve
      if (fadeout) {
        pwm = 4096 - (count * 64);
      } else {
        pwm = (count * 64) - 1;
      }
    }
  } else {
    count = 0;
    fadeout = !fadeout;
    pwm = (fadeout) ? 4095 : 0;
  }
  // blink onboard led shortly ath the end of the cycle
  digitalWrite(LED_BUILTIN, (count == 64));

  // set PCA9685 start channel register address
  reg_addr = 0x06 + (LED_CH_START << 2);

  Wire.beginTransmission(PCA_I2CADDR);
  // send register values - four registers sets pwm value for one channel
  // Arduino Wire can transmit up to 32 bytes in one transmission
  for (z = 0; (z < LED_CH_COUNT && z < (16 - LED_CH_START)); z++) {
  	// update four channels (16 registers) at a time
    if ((z & 0x03) == 0) { // z value of 0, 4, 8, 12 
      if (z > 0) {
        Wire.endTransmission();
        Wire.beginTransmission(PCA_I2CADDR);
      }
	  // send PCA9685 register address
	  Wire.write(reg_addr);
	  reg_addr += 16;
  	}
    if (pwm == 0) {
      // led off
      Wire.write(0x00);
      Wire.write(0x00);
      Wire.write(0x00);
      Wire.write(0x10);      
    } else {
      // led pwm value
      Wire.write(0x00);
      Wire.write(0x00);
      // 8 lsb pwm bits
      Wire.write(pwm);
      // 4 msb pwm bits 
      Wire.write((pwm >> 8) & 0x0F);
    }
  }
  Wire.endTransmission();

  // Serial.println(pwm, DEC);
}
