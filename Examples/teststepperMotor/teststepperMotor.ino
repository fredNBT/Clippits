#define Stepper_DIR 12
#define Stepper_STEP 14

void setup() {
  // put your setup code here, to run once:
  pinMode(Stepper_STEP, OUTPUT);  // Set the pin as an output
  pinMode(Stepper_DIR, OUTPUT);  // Set the pin as an output
  digitalWrite(Stepper_DIR, LOW); 

  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
    Serial.println("stepper test");


    for (int i = 0; i < 1000; i++) { // Toggle the pin 10 times
    digitalWrite(Stepper_STEP, HIGH); // Turn the pin on
    delayMicroseconds(100);                     // Wait for 500 milliseconds
    digitalWrite(Stepper_STEP, LOW);  // Turn the pin off
    delayMicroseconds(100);                     // Wait for another 500 milliseconds
  }

}
