#include <Wire.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

//2 Actuators
//North/South
const int RELAY_1_A = 24;
const int RELAY_1_B = 25;

//East/West
const int RELAY_2_A = 26;
const int RELAY_2_B = 27;

//Photoresistors
const int pResistorN = A2; // N
const int pResistorE = A3; // E
const int pResistorS = A4; // S
const int pResistorW = A5; // W

//feedback actuator position
//Yellow to 5v
//White to GND
//Blue to analog pin
const int NSmeasure = A0;
const int EWmeasure = A1;
double EWvalue;
double NSvalue;

//Actuator Limits
int NSextendedLimit = 555;
int NSretractedLimit = 388;
int EWextendedLimit = 490;
int EWretractedLimit = 378;
int EWCenter = 448;
const int actDiff = 3;

//Actuator potentiometer difference
const int EWDiff = 10;
const int NSDiff = 3;

//Variables
// Store value from photoresistor (0-1023) (Current & Past)
int cValueN, cValueE, cValueS, cValueW;


//Check photoresistor values every 10 min
const int pDiff = 17;

// defines pins numbers
const int dirPin = 42;
const int stepPin = 44;
const int enPin = 40;

// Pins for brush motor
const int dirPin1 = 51;
const int stepPin1 = 53;
const int enPin1 = 49;

// Pins for Limit Switches
const int top_switch = 52;

int i = 0;

void setup() {
  // put your setup code here, to run once:
  //Serial Monitor init
  Serial.begin(9600);
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");



  //Photoresistors initialization
  pinMode(pResistorN, INPUT);
  pinMode(pResistorE, INPUT);
  pinMode(pResistorS, INPUT);
  pinMode(pResistorW, INPUT);

  cValueN = analogRead(pResistorN);
  cValueE = analogRead(pResistorE);
  cValueS = analogRead(pResistorS);
  cValueW = analogRead(pResistorW);

  //Actuator initialization (relays)
  //North/South
  pinMode(RELAY_1_A, OUTPUT);
  pinMode(RELAY_1_B, OUTPUT);
  //East/West
  pinMode(RELAY_2_A, OUTPUT);
  pinMode(RELAY_2_B, OUTPUT);

  digitalWrite(RELAY_1_A, HIGH);
  digitalWrite(RELAY_1_B, HIGH);
  digitalWrite(RELAY_2_A, HIGH);
  digitalWrite(RELAY_2_B, HIGH);

  pinMode(NSmeasure, INPUT);
  pinMode(EWmeasure, INPUT);
  NSvalue = analogRead(NSmeasure);
  EWvalue = analogRead(EWmeasure);

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);

  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(enPin1, OUTPUT);
  digitalWrite(enPin1, LOW);

  pinMode(top_switch, INPUT);

  digitalWrite(dirPin, LOW);
  digitalWrite(dirPin1, HIGH); // Enables the motor to move in a particular direction

}

void loop() {

  tmElements_t tm;
  i = 0;
  EWvalue = analogRead(EWmeasure);
  cValueN = analogRead(pResistorN);
  cValueE = analogRead(pResistorE);
  cValueS = analogRead(pResistorS);
  cValueW = analogRead(pResistorW);
  Serial.print("Current: N: ");
  Serial.print(cValueN);
  Serial.print(" S: ");
  Serial.print(cValueS);
  Serial.print(" E: ");
  Serial.print(cValueE);
  Serial.print(" W: ");
  Serial.println(cValueW);

  //Morning position///////////////////////////////////////////////

  if (RTC.read(tm) && (  (tm.Hour == 5) && (tm.Minute == 0))) {
    //morning position///////////////////////////////////////////////////////////////////////////////////
    //move NS position
    while ((abs(NSvalue - NSretractedLimit)) > actDiff) {
      NSvalue = analogRead(NSmeasure);
      Serial.println(NSvalue);

      if ((NSvalue - NSretractedLimit) > 0) {
        //retract actuator
        digitalWrite(RELAY_1_A, HIGH);
        digitalWrite(RELAY_1_B, LOW);
        Serial.println("retracting NS actuator");
        Serial.println(NSvalue);
        Serial.println(NSvalue - NSretractedLimit);
      }
      delay(200);
      if ((NSvalue - NSretractedLimit) < 0) {
        //extend actuator
        digitalWrite(RELAY_1_A, LOW);
        digitalWrite(RELAY_1_B, HIGH);
        Serial.println("extending NS actuator");
        Serial.println(NSvalue);
      }

    }


    digitalWrite(RELAY_1_A, HIGH);
    digitalWrite(RELAY_1_B, HIGH);
    Serial.println("stopping NS actuator");
    Serial.println(NSvalue);



    //move EW position
    while ((abs(EWvalue - EWretractedLimit)) > actDiff) {
      NSvalue = analogRead(NSmeasure);
      EWvalue = analogRead(EWmeasure);
      if (EWvalue > EWretractedLimit) {
        //retract acutuator
        digitalWrite(RELAY_2_A, LOW);
        digitalWrite(RELAY_2_B, HIGH);
        Serial.println("retracting EW actuator");

        Serial.println(EWvalue);
      }
      if (EWvalue < EWretractedLimit) {
        //extend actuator
        digitalWrite(RELAY_2_A, HIGH);
        digitalWrite(RELAY_2_B, LOW);
        Serial.println("extending EW actuator");

        Serial.println(EWvalue);
      }

    }

    digitalWrite(RELAY_2_A, HIGH);
    digitalWrite(RELAY_2_B, HIGH);
    Serial.println("stopping EW actuator");

    Serial.println(EWvalue);
  }




  if (RTC.read(tm) && (  tm.Minute == 10 ||  tm.Minute == 20 || tm.Minute == 30 || tm.Minute == 40  || tm.Minute == 50 || tm.Minute == 00) && (tm.Hour < 17) && (tm.Hour > 6)) {
    /////////////////////Solar tracking///////////////////
    //////////////////////////////////////////////////////////////////////////
    //Check if diff b/w E and W value is > 17
    while (abs(cValueE - cValueW) > pDiff && (EWvalue < EWextendedLimit)) {
      EWvalue = analogRead(EWmeasure);
      cValueN = analogRead(pResistorN);
      cValueE = analogRead(pResistorE);
      cValueS = analogRead(pResistorS);
      cValueW = analogRead(pResistorW);
      Serial.print("Current: N: ");
      Serial.print(cValueN);
      Serial.print(" S: ");
      Serial.print(cValueS);
      Serial.print("EWvalue: ");
      Serial.print(EWvalue);
      NSvalue = analogRead(NSmeasure);
      Serial.print(" NS value: ");
      Serial.println(NSvalue);


      //Move E/W actuator until within 17 difference
      //if..and EWpos is less than
      if ((cValueE - cValueW) > 0) {
        //retract actuator
        Serial.print("Retract E/W actuator ");
        Serial.print("Current: E: ");
        Serial.print(cValueE);
        Serial.print(" W: ");
        Serial.println(cValueW);

        digitalWrite(RELAY_2_A, LOW);
        digitalWrite(RELAY_2_B, HIGH);
      }
      //if..and EWpos is less than
      if ((cValueE - cValueW) < 0 ) {
        Serial.print("Extend E/W actuator ");
        Serial.print("Current: E: ");
        Serial.print(cValueE);
        Serial.print(" W: ");
        Serial.println(cValueW);


        digitalWrite(RELAY_2_A, HIGH);
        digitalWrite(RELAY_2_B, LOW);
      }

    }


    //leave E/W actuator off if values are similar
    digitalWrite(RELAY_2_A, HIGH);
    digitalWrite(RELAY_2_B, HIGH);
    Serial.println("leave E/W actuator Off");


    delay(3000);
    //Check if diff b/w N and S value is > pDiff
    while ((abs(cValueN - cValueS)) > pDiff && (NSvalue < NSextendedLimit) ) {
      EWvalue = analogRead(EWmeasure);
      NSvalue = analogRead(NSmeasure);
      cValueN = analogRead(pResistorN);
      cValueE = analogRead(pResistorE);
      cValueS = analogRead(pResistorS);
      cValueW = analogRead(pResistorW);
      Serial.print("Current: N: ");
      Serial.print(cValueN);
      Serial.print(" S: ");
      Serial.print(cValueS);
      Serial.print(" EWvalue: ");
      Serial.print(EWvalue);

      //Move N/S actuator until within 17 difference
      //if..and NSpos is less than
      if ((cValueN - cValueS) > 0) {
        //extend actuator
        Serial.print("Retract N/S actuator ");
        Serial.print("Current: N: ");
        Serial.print(cValueN);
        Serial.print(" S: ");
        Serial.println(cValueS);

        digitalWrite(RELAY_1_A, HIGH);
        digitalWrite(RELAY_1_B, LOW);

      }
      //if..and NSpos is less than
      if ((cValueN - cValueS) < 0) {

        Serial.print("Extend N/S actuator ");
        Serial.print("Current: N: ");
        Serial.print(cValueN);
        Serial.print(" S: ");
        Serial.println(cValueS);

        digitalWrite(RELAY_1_A, LOW);
        digitalWrite(RELAY_1_B, HIGH);

      }

    }

    //leave N/S actuator off if values are similar
    digitalWrite(RELAY_1_A, HIGH);
    digitalWrite(RELAY_1_B, HIGH);
    Serial.println("leave N/S actuator Off");
    delay(50000);
  }

  if (RTC.read(tm) && (  (tm.Hour == 20) && (tm.Hour == 00))) {
    //move to cleaning mechanism position
    //Cleaning mechanism position/////////////////////////
    //Move EW first
    //while EWvalue-EWCenter is greater than EWDiff (10)
    while (abs(EWvalue - EWCenter) > EWDiff) {
      EWvalue = analogRead(EWmeasure);
      Serial.println(EWvalue);
      //if EWvaue-EWCenter is greater than 10
      if ((EWvalue - EWCenter) > EWDiff) {
        //retract actuator
        digitalWrite(RELAY_2_A, LOW);
        digitalWrite(RELAY_2_B, HIGH);
        Serial.println("retracting EW actuator");

        Serial.println(EWvalue);
      }
      //if EWvalue -EWcenter is less than 0
      if ((EWvalue - EWCenter) < 0) {
        //extend actuator
        digitalWrite(RELAY_2_A, HIGH);
        digitalWrite(RELAY_2_B, LOW);
        Serial.println("extending EW actuator");
        Serial.println(EWvalue);
      }
    }

    digitalWrite(RELAY_2_A, HIGH);
    digitalWrite(RELAY_2_B, HIGH);
    Serial.println(EWvalue);


    Serial.println(NSvalue);
    delay(3000);
    //then NS
    while (abs(NSvalue - NSextendedLimit) > NSDiff) {
      NSvalue = analogRead(NSmeasure);
      Serial.println(NSvalue);
      //if NSvalue -NSextendedLimit is less than 3
      //extend actuator
      //if NSvalue - NSextendedLimit is greater than 3
      delay(200);
      if ((NSvalue - NSextendedLimit) < NSDiff) {
        digitalWrite(RELAY_1_A, LOW);
        digitalWrite(RELAY_1_B, HIGH);
        Serial.println("extending NS actuator");
        Serial.println(NSvalue);
      }
      delay(200);
      if ((NSvalue - NSextendedLimit) > 0) {
        digitalWrite(RELAY_1_A, HIGH);
        digitalWrite(RELAY_1_B, LOW);
        Serial.println("retracting NS actuator");
        Serial.println(NSvalue);
      }
    }

    digitalWrite(RELAY_1_A, HIGH);
    digitalWrite(RELAY_1_B, HIGH);
    Serial.println("stopping NS actuator");

    Serial.println(NSvalue);

    //run cleaning mechanism//////////////////////////////////////////////
    while (i < 1) {
      int topSw = digitalRead(top_switch);

      if (topSw == HIGH && (digitalRead(dirPin) == LOW)) {
        motorStep(1);
      }

      if (topSw == LOW && (digitalRead(dirPin) == LOW)) {
        digitalWrite(dirPin, HIGH);
        digitalWrite(dirPin1, HIGH);

        for (int x = 0; x < 19000; x++) {
          digitalWrite(stepPin, HIGH);
          digitalWrite(stepPin1, HIGH);
          delayMicroseconds(500);
          digitalWrite(stepPin, LOW);
          digitalWrite(stepPin1, LOW);
          delayMicroseconds(500);
        }
        digitalWrite(enPin, HIGH);
        digitalWrite(enPin1, HIGH);
        i++;
      }
    }
  }

}

void motorStep( int MAX) {

  for (int x = 0; x < MAX; x++) {
    digitalWrite(stepPin, HIGH);
    digitalWrite(stepPin1, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    digitalWrite(stepPin1, LOW);
    delayMicroseconds(500);
  }
}
