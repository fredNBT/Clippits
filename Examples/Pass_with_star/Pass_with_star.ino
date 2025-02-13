#include <SoftwareSerial.h>
#define BG96_PWR 8
SoftwareSerial BG96Serial(9, 10); // RX, TX

void setup() {
  Serial.begin(9600); // open the serial port at 9600 bps:  
  BG96Serial.begin(115200);
  pinMode(BG96_PWR, OUTPUT); // Set the BG96 PowerPin as an output
  digitalWrite(BG96_PWR, HIGH); //Pull the pin high 
  delay(100);            // wait 100 ms
  digitalWrite(BG96_PWR, LOW);  // Pull Low
  delay(2000);
}

void loop() {
  if (Serial.available()) {// If anything comes in Serial (USB), send it out to the modem
  	    char incomingByte = Serial.read();
    // Check if the incoming byte is an asterisk '*'
    if (incomingByte == '*') {
      BG96Serial.write(26); // replaces the * with CTRL+Z
    }else {
    BG96Serial.write(incomingByte);
    }
    
  }
  if (BG96Serial.available()) { // if anything arrives from the modem send it to BG96
   Serial.write(BG96Serial.read());
  }
}