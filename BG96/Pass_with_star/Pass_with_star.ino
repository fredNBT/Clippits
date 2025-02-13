#define BG96_PWR 13
#define TXD2 17  // Change this to your desired TX pin
#define RXD2 16  // Change this to your desired RX pin



void setup() {
  Serial.begin(115200); // open the serial port at 9600 bps:  
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  
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
      Serial2.write(26); // replaces the * with CTRL+Z
    }else {
    Serial2.write(incomingByte);
    }
    
  }
  if (Serial2.available()) { // if anything arrives from the modem send it to BG96
   Serial.write(Serial2.read());
  }
}