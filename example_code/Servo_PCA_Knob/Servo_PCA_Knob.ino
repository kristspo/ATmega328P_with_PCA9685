/*
 Example sketch for ATmega 328P development board with 16-channel PCA9685 PWM controller.
 https://github.com/kristspo/ATmega328P_with_PCA9685
 
 Servo 'Knob' example sketch changed to use Adafruit PWM Servo Driver Library.
 Controls one or more servos attached to PCA9685 from potentiometer ADC readings.
 
 Connections for Arduino Uno/Nano/Pro:
 Potentiometer from GND to VCC with middle pin attched to
 ** A0, A1, A2 or A3 as defined below
 PCA9685 attached to I2C
 ** SDA - A4
 ** SCL - A5
 One or more servos attached to on board servo pins starting from channel 0
*/

#include <Adafruit_PWMServoDriver.h>

// use A0, A1, A2, A3 to set analog pin where potentiometer is attached
#define READ_PIN A0
// number of servos channels to control
#define SERVO_COUNT 8
// enable (1) or disable (0) sending ADC readings and PCA9685 pwm values to Serial (57600 bps baud rate)
#define PRINTVAL 1
// potentiometer value and servos update rate in ms
#define UPDATEMS 50

// min and max servo pulse length in micro seconds - to find servo limits, change in small steps or use values from servo datasheet
#define SERVO_PULSE_MIN 1000
#define SERVO_PULSE_MAX 2000
// PCA9685 I2C address - it can be changed using on board dip switch (0x40 to 0x47)
#define I2CADDR 0x40


Adafruit_PWMServoDriver PCA = Adafruit_PWMServoDriver(I2CADDR);

unsigned long previousMillis;
unsigned long interval = UPDATEMS;
unsigned set_pwm;
// min and max PCA9685 pwm counter values for 50 Hz pwm period (20 ms) 
unsigned int pwm_min = round(SERVO_PULSE_MIN * 0.00005 * 4096) - 1;
unsigned int pwm_max = round(SERVO_PULSE_MAX * 0.00005 * 4096) - 1;

void setup() {
  // Start I2C and setup PCA9685 PWM frequency
  PCA.begin();
  // Set PCA9685 internal oscillator value that will be used for pwm frequency setup
  // Internal oscillator is typically 25 Mhz but seems to be somewhat faster
  // This impacts how accurate expected min/max pulse length matches servo control pulse
  PCA.setOscillatorFrequency(26500000); 
  PCA.setPWMFreq(50);
  
  if (PRINTVAL) {
    // Open serial communications and wait for port to open
    Serial.begin(57600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.print("Servo count: ");
    Serial.println(SERVO_COUNT, DEC);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned int pot, pwm;
  unsigned char z;
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
     
    // read value of the potentiometer (between 0 and 1023) and map to pwm values to use with PCA9685
    pot = analogRead(READ_PIN);
    pwm = map(pot, 0, 1023, pwm_min, pwm_max);
 
    if (set_pwm != pwm) {
      set_pwm = pwm;
      // update PCA9685 pwm values starting from channel 0
      for (z = 0; z < SERVO_COUNT; z++) {
        PCA.setPWM(z, 0, pwm);
      }

      // output ADC readings and PCA9685 pwm values to Serial
      if (PRINTVAL) {
        Serial.print(pot, DEC);
        Serial.write('\t');
        Serial.println(pwm, DEC);
      }
    }
  }
}
