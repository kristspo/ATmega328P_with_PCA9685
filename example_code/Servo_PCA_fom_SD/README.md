### Servo_PCA_fom_SD

This code controls servos attached to PCA9685 with values from data file on SD card. Values from data file are mapped to min and max servo control pulse length using SERVO_PULSE_MIN, SERVO_PULSE_MAX and DATA_MAX defines.

Code uses Adafruit PWM Servo Driver Library to control PCA9685 and Arduino SD Library to read from SD card. Sample data files (log4.txt, log8.txt, log16.txt) need to be copied to SD card.

Example code is provided "AS IS", without warranty of any kind that it will be useful for your purpose.