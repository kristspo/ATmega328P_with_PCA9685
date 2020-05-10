/*
 Example sketch for ATmega 328P development board with 16-channel PCA9685 PWM controller.
 https://github.com/kristspo/ATmega328P_with_PCA9685
 
 Controls servos attached to PCA9685 with values from data file on SD card.
 Uses Adafruit PWM Servo Driver Library and Arduino SD Library.
 
 Values from data file are mapped to servo control pulse length. Sample
 data files LOG4.TXT, LOG8.TXT, LOG16.TXT are provided with values for
 4, 8 or 16 servos.
 
 Connections for Arduino Uno/Nano/Pro:
 SD card attached to SPI
 ** CS   - 4
 ** MOSI - 11
 ** MISO - 12
 ** CLK  - 13
 PCA9685 attached to I2C
 ** SDA - A4
 ** SCL - A5
 One or more servos attached to on board servo pins starting from channel 0
*/

#include <SPI.h>
#include <SD.h>
#include <Adafruit_PWMServoDriver.h>

// data file to read from SD card
#define FILENAME "LOG8.TXT"
// enable (1) or disable (0) rewinding file at the end
#define LOOPFILE 1
// enable (1) or disable (0) printing files list at startup to Serial (57600 bps baud rate)
#define PRINTDIR 1
// enable (1) or disable (0) printing PCA9685 pwm values to Serial (57600 bps baud rate)
#define PRINTVAL 1
// data file reading and servos update rate in ms
#define UPDATEMS 40

// min and max servo pulse length in micro seconds - to find servo limits, change in small steps or use values from servo datasheet
#define SERVO_PULSE_MIN 1000
#define SERVO_PULSE_MAX 2000
// max value in data file - values from 0 to 1023 would be expected if Arduino ADC readings were logged
#define DATA_MAX 1023
// set PCA9685 I2C address - it can be changed using on board dip switch (0x40 to 0x47)
#define I2CADDR 0x40


Adafruit_PWMServoDriver PCA = Adafruit_PWMServoDriver(I2CADDR);
File DataFile;

unsigned long previousMillis;
unsigned long interval = UPDATEMS;
unsigned int servoVal[16];
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
  
  // Open serial communications and wait for port to open
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.print("PCA9685 pwm values range: ");
  Serial.print(pwm_min, DEC);
  Serial.print(" to ");
  Serial.print(pwm_max, DEC);
  Serial.println();
  
  // Open SD card
  Serial.println("Reading SD card...");
  if (!SD.begin(4)) {
    Serial.println("SD card failed!");
    interval = 0;
    return;
  }
 
  // List files
  if (PRINTDIR) {
    File root = SD.open("/");
    ListFiles(root);
  }
 
  // Open data file
  DataFile = SD.open(FILENAME, FILE_READ);
  if (DataFile) {
    Serial.print("Open file ");
    Serial.println(FILENAME);
  } else {
    Serial.print("File open failed ");    
    Serial.println(FILENAME);
    interval = 0;
  }
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned int val = 0;
  char ch, index = 0;
  
  if (interval && (currentMillis - previousMillis >= interval)) {
    previousMillis = currentMillis;
 
    // read and parse line from data file
    while (1) {
      ch = DataFile.read();
      // add digit value
      if (ch >= '0' && ch <= '9') {
        val *= 10;
        val += (ch - 48);
      }
      // save to servoVal
      if ((ch == ',' || ch == '\n') && index < 16) {
        val = constrain(val, 0, DATA_MAX);
        servoVal[index] = map(val, 0, DATA_MAX, pwm_min, pwm_max);
        val = 0; // clear temporary value
        index++;
      }
      // check for end of line or end of file
      if ((ch == '\n') || (ch == -1)) {
        break; // stop reading
      }
    }
   
    // update PCA9685 pwm values from servoVal
    ch = 0;
    while (ch < index) {
      PCA.setPWM(ch, 0, servoVal[ch]);
      ch++;
    }
 
    // print servoVal values to Serial
    if (PRINTVAL) {
      ch = 0;
      while (ch < index) {
        Serial.print(servoVal[ch], DEC);
        ch++;
        if (ch < index) {
          Serial.write('\t');
        } else {
          Serial.write('\n');
        }
      }
    }
   
    // loop reading from file if neccessary
    if (! DataFile.available()) {
      if (LOOPFILE) {
        Serial.println("Loop file");
        DataFile.seek(0);
      } else {
        Serial.println("Close file");
        DataFile.close();
        interval = 0;
      }
    }
  }
}

void ListFiles(File dir) {
  while (1) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // end of files list
      break;
    }
    Serial.print(entry.name());
    Serial.write(' ');
    Serial.print(entry.size(), DEC);
    Serial.println(" bytes");
    entry.close();
  }
}
