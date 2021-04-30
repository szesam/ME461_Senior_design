
/**************************/
int pumpPin = 2;
int solenoidInlet = 3;
int solenoidExhaust = 4;
const int buttonPin = 5;
const int buttonPin2 = 6;
int buttonState = 0;
int buttonState2 = 0;
/******************************************************************************/

void setup() {
  pinMode(pumpPin, OUTPUT);
  pinMode(solenoidInlet,OUTPUT);
  pinMode(solenoidExhaust,OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(buttonPin2, INPUT);
  //Initial deflate routine that will run once. Will depressurize any air that's
  //already in the line. 
  digitalWrite(solenoidExhaust, HIGH); //open exhaust valve
  digitalWrite(solenoidInlet,LOW);
  delay(5000);                         //wait 5 seconds for system to deflate
}
void loop() {
  //continously read button states
  buttonState = digitalRead(buttonPin);
  buttonState2 = digitalRead(buttonPin2);
  if (buttonState == HIGH) //button 1 depressed
  {
    digitalWrite(solenoidExhaust, LOW); //close outlet
    delay(100);
    digitalWrite(solenoidInlet, HIGH); //open inlet
    delay(100);
    digitalWrite(pumpPin, HIGH); //starting pumping
  }
  else 
  {
    digitalWrite(pumpPin, LOW);
    delay(100);
    digitalWrite(solenoidInlet, LOW);
    delay(100);
    digitalWrite(solenoidExhaust,LOW);
  }
  if (buttonState2 == HIGH)
  {
    digitalWrite(solenoidExhaust, HIGH); //Open outlet
    delay(100);
  }
  delay(1000);
}