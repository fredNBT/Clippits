#include <SoftwareSerial.h>
SoftwareSerial Thermal(10, 9); //Soft RX from printer on D2, soft TX out to printer on D3

#define FALSE  0
#define TRUE  1
int printOnBlack = FALSE;
int printUpSideDown = FALSE;

int ledPin = 13;
int heatTime = 80; //80 is default from page 23 of datasheet. Controls speed of printing and darkness
int heatInterval = 255; //2 is default from page 23 of datasheet. Controls speed of printing and darkness
char printDensity = 15; //Not sure what the defaut is. Testing shows the max helps darken text. From page 23.
char printBreakTime = 15; //Not sure what the defaut is. Testing shows the max helps darken text. From page 23.

void setup() {
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600); //Use hardware serial for debugging
  Thermal.begin(19200); //Setup soft serial for ThermalPrinter control

  printOnBlack = FALSE;
  printUpSideDown = FALSE;

  //Modify the print speed and heat
  Thermal.write(27);
  Thermal.write(55);
  Thermal.write(7); //Default 64 dots = 8*('7'+1)
  Thermal.write(heatTime); //Default 80 or 800us
  Thermal.write(heatInterval); //Default 2 or 20us

  //Modify the print density and timeout
  Thermal.write(18);
  Thermal.write(35);
  int printSetting = (printDensity<<4) | printBreakTime;
  Thermal.write(printSetting); //Combination of printDensity and printBreakTime

  Serial.println();
  Serial.println("Parameters set");
}

void loop() {
  char option;

  while(1) {
    Serial.println();
    Serial.println("Thermal Printer Testing");
    Serial.println("1) Print Hello World!");
    Serial.println("2) Type to print");
    Serial.println("3) Dull boy");
    Serial.println("4) Printer status");
    Serial.println("5) Reverse white/black");
    Serial.println("6) Print characters upside down");
    Serial.println("7) Adjust heat settings");
    Serial.println("8) Adjust print settings");
    Serial.println("9) Print factory test page");
    Serial.println("a) Print entire character set");
    Serial.println("b) Print numberic barcode");
    Serial.println("c) Print alpha barcode");
    Serial.println("d) Bitmap test");
    Serial.print(": ");

    while(1) {
      while(!Serial.available());
      option = Serial.read();
      if(isalnum(option)) break;
    }

    if(option == '1') {
      Serial.println("Hello World!");

      Thermal.println("Hello World!");
      Thermal.println("12345678901234567890123456789012");
      Thermal.write(10); //Sends the LF to the printer, advances the paper
      Thermal.write(10);
    }
    else if (option == '2') {
      Serial.println("Turn on line endings! Press $ to exit. Tell me what's on your mind.");
      //Free mode
      while(1) {
        while(Serial.available() == 0) ; //Wait for incoming characters

        while(Serial.available() > 0) {
          option = Serial.read(); //Take a character from serial port and check what it is

          if(option == '$') 
            break; //Break if we see the money
          else if(option == 13) 
            Thermal.write(10); //Line feed on CR
          else {
            Thermal.write(option); //Push this character to printer
            Serial.print(option);
            delay(10);
          }
        }

        if(option == '$') break; //Break if we see ctrl+z
      }
    }
    else if (option == '3') {
      //The Shining
      for(int x = 0 ; x < 15 ; x++)
        Thermal.println("All work and no play makes Jack a dull boy");
      Thermal.write(10);
      Thermal.write(10);
      Thermal.write(10);
    }
    else if(option == '4')  {
      Serial.println("Printer Status:");

      Thermal.write(27); //ESC
      Thermal.write(118); //v

      while(Thermal.available() < 8) ; //Wait for response
      char printerStatus[8];
      for(int x = 0 ; x < 8 ; x++) {
        printerStatus[x] = Thermal.read();
      }

      if(printerStatus[0] == 'P') { //Check to make sure the array contains what we expect
        if(printerStatus[1] == 1) 
          Serial.println("Paper detected");
        else
          Serial.println("No paper!");
        Serial.print("System Voltage: ");
        Serial.print(printerStatus[3]);
        Serial.print(".");
        Serial.print(printerStatus[4]);
        Serial.println("V");
        Serial.print("Head temp: ");
        Serial.print(printerStatus[6]);
        Serial.print(printerStatus[7]);
        Serial.println("C");
      }
      else 
        Serial.print("Status Read Error");
    }
    else if(option == '5') {
      //Select background color
      Thermal.write(29); //GS-ESC
      Thermal.write(66); //B

      if(printOnBlack == FALSE) {
        printOnBlack = TRUE;
        Serial.println("Print on black");
        Thermal.write(1); //Prints text on black background
      }
      else {
        printOnBlack = FALSE;
        Serial.println("Print on white");
        Thermal.write((byte)0); //Prints text on white background
      }
    }
    else if(option == '6') {
      //Select text orientation
      Thermal.write(27); //ESC
      Thermal.write(123); //{

      if(printUpSideDown == FALSE) {
        printUpSideDown = TRUE;
        Serial.println("Print upside down");
        Thermal.write(1); //Prints upside down
      }
      else {
        printUpSideDown = FALSE;
        Serial.println("Print normal upside");
        Thermal.write((byte)0); //Prints normal
      }
    }

    else if(option == '7') {
      //Change settings
      Serial.println();
      Serial.println("Turn on Newline endings!");

      char buffer[10];
      while(1) {
        Serial.print("Current heat time:");
        Serial.print(heatTime, DEC);
        Serial.print(" New heat time (default is 80):");

        read_line(buffer, sizeof(buffer)); //Read user's response
        heatTime = atoi(buffer); //Convert response to integer
        if(heatTime < 256 && heatTime > 3) break;
      }

      while(1) {
        Serial.print("Current heat interval:");
        Serial.print(heatInterval, DEC);
        Serial.print(" New heat interval (default is 2):");

        read_line(buffer, sizeof(buffer)); //Read user's response
        heatInterval = atoi(buffer); //Convert response to integer
        if(heatInterval < 256 && heatInterval > 2) break;
      }

      //Control Parameters
      Thermal.write(27);
      Thermal.write(55);
      Thermal.write(7); //Default 64 dots = 8*('7'+1)
      Thermal.write(heatTime); //Default 80 or 800us
      Thermal.write(heatInterval); //Default 2 or 20us

      Serial.println();
      Serial.println("Parameters set");
    }
    else if (option == '8') {
      //Change more settings
      Serial.println();
      Serial.println("Turn on Newline endings!");

      char buffer[10];
      while(1) {
        Serial.print("Current print density:");
        Serial.print(printDensity, DEC);
        Serial.print(" New print density (max is 15):");

        read_line(buffer, sizeof(buffer)); //Read user's response
        printDensity = atoi(buffer); //Convert response to integer
        if(printDensity < 16 && printDensity > 0) break;
      }

      while(1) {
        Serial.print("Current print break time:");
        Serial.print(printBreakTime, DEC);
        Serial.print(" New print break time (max is 15):");

        read_line(buffer, sizeof(buffer)); //Read user's response
        printBreakTime = atoi(buffer); //Convert response to integer
        if(printBreakTime < 16 && printBreakTime > 0) break;
      }

      int printSetting = printDensity << 4;
      printSetting |= printBreakTime;

      //Control Parameters
      Thermal.write(18); //DC2
      Thermal.write(35); //#
      Thermal.write(printSetting);

      Serial.println();
      Serial.println("Parameters set");
    }
    else if (option == '9') {
      //Print test page
      Thermal.write(18); //DC2
      Thermal.write(84); //T
    }
    else if (option == 'a') {
      //Print the entire character set
      for(int x = 0 ; x < 256 ; x++)
        Thermal.write(x);
      Thermal.write(10);
      Thermal.write(10);
    }
    else if (option == 'b') {
      Serial.println("Barcode printing");
      //Print barcode example
      Thermal.print(29); //GS
      Thermal.print(107); //k
      Thermal.print(0); //m = 0
      Thermal.print('2'); //Data
      Thermal.print('9');
      Thermal.print('1');
      Thermal.print('9');
      Thermal.print('3');
      Thermal.print('9');
      Thermal.print('1');
      Thermal.print('9');
      Thermal.print('3');
      Thermal.print('9');
      Thermal.print('1');
      Thermal.print('9'); //12 characters
      Thermal.write((byte)0); //Terminator

      delay(3000); //For some reason we can't immediately have line feeds here
      Thermal.write(10); //Paper feed
      Thermal.write(10); //Paper feed
    }
    else if (option == 'c') {
      //Print barcode example
      Thermal.write(29); //GS
      Thermal.write(107); //k
      Thermal.write(4); //m = 4, fancy bar code
      Thermal.print('W'); //Data, cap letters, $, ., % acceptable
      Thermal.print('W');
      Thermal.print('W');
      Thermal.print('.');
      Thermal.print('S');
      Thermal.print('P');
      Thermal.print('A');
      Thermal.print('R');
      Thermal.print('K');
      Thermal.print('F');
      Thermal.print('U');
      Thermal.print('N');
      Thermal.print('.');
      Thermal.print('C');
      Thermal.print('O');
      Thermal.print('M'); //16 characters
      Thermal.write((byte)0); //Terminator

      delay(3000); //For some reason we can't immediately have line feeds here
      Thermal.write(10); //Paper feed
      Thermal.write(10); //Paper feed
    }
    else if(option == 'd')  {
      //Bitmap example
      Serial.println("Print bitmap image");
      Thermal.write(18); //DC2
      Thermal.write(86); //V
      Thermal.write(50); //nL = 10
      Thermal.write((byte)0); //nH = 0

      //Now we need 10 rows
      for(int y = 0 ; y < 50 ; y++) {
        //Now we need 48 bytes total
        for(int x = 0 ; x < 48 ; x++)
          Thermal.write(0xAA); //0b.1010.1010
      }

      Thermal.write(10); //Paper feed
      Thermal.write(10); //Paper feed

      Serial.println("Print bitmap done");
    }    
    else {
      Serial.print("Choice = ");
      Serial.println(option, DEC);
    }
  }
}

//Reads a line until the \n enter character is found
uint8_t read_line(char* buffer, uint8_t buffer_length) {
  memset(buffer, 0, buffer_length);

  while(Serial.available()) Serial.read(); //Clear out any characters that may be sitting in the buffer

  uint8_t read_length = 0;
  while(read_length < buffer_length - 1) {
    while (!Serial.available()); //We've destroyed the standard way Arduino reads characters so this no longer works
    uint8_t c = Serial.read();

    if(c == 0x08 || c == 0x7f) { //Backspace characters
      if(read_length < 1)
        continue;

      --read_length;
      buffer[read_length] = '\0';

      Serial.print((char)0x08);
      Serial.print(' ');
      Serial.print((char)0x08);

      continue;
    }

    Serial.print((char)c);

    if(c == '\n' || c == '\r') {
      buffer[read_length] = '\0';
      break;
    }
    else {
      buffer[read_length] = c;
      ++read_length;
    }
  }

  return read_length;
}