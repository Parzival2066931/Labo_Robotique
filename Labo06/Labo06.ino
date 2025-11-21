#include <Adafruit_seesaw.h>
#include "MeConducteur.h"
#include "Sonar.h"
#include "MeAnneau.h"
#include <ezBuzzer.h>

#define BUZZER_PIN 45

bool debugMode = false;
bool firstAuto = true;

float distance = 0;
float normalSpeed = 70;
float slowSpeed = 100;
float turnSpeed = 70;
float minSpeed = 50;
float maxSpeed = 255;
int slowDist = 50;
int dist;
int printDelay = 1000;

const int blinkRate = 500;
const int beepRate = 500;
const int backwardPWM = 200;
const int klaxonFreq = 420;

double trackerKp = 0.75;
double trackerKi = 0;
double trackerKd = 0;

double gyroKp = 9;
double gyroKi = 1;
double gyroKd = 3;


enum avanceState {
  AUTO,
  LIBRE
};
avanceState aState;

unsigned long currentTime = 0;

Anneau anneau;
Conducteur conducteur;
Sonar sonar;
ezBuzzer buzzer(BUZZER_PIN);

enum CruzeControlState {
    NORMAL,
    VIGILANCE
};
CruzeControlState speedState = NORMAL;

void setup() {
    Serial.begin(115200);
    attachInterrupt(conducteur.GetPinRight(), Conducteur::GetIsrRight, RISING);
    attachInterrupt(conducteur.GetPinLeft(), Conducteur::GetIsrLeft, RISING);

    conducteur.Setup();
    anneau.Setup();
    sonar.Setup();

    conducteur.SetGyroPID(0, 0, 0);
    conducteur.SetTrackerPID(0, 0, 0);

    conducteur.SetTurnSpeed(turnSpeed);
    conducteur.SetMinSpeed(minSpeed);
    conducteur.SetMaxSpeed(maxSpeed);
    conducteur.SetSpeed(normalSpeed);
    conducteur.SetState(STOP);

    buzzer.stop();

    aState = LIBRE;
}

void loop() {
    currentTime = millis();

    

    sonar.Update();
    
    dist = sonar.GetDist(); 
    conducteur.Update(dist);
    

    speedUpdate();

    buzzer.loop();

    switch (aState) {
        case AUTO:
            avanceAuto();
            break;
        case LIBRE:
            break;
    }

    if(debugMode) debugTask();
}

void speedUpdate() {
    switch(speedState) {
        case NORMAL:
            normalState();
            break;
        case VIGILANCE:
            slowState();
            break;
    }
}

void normalState() {
    static bool firstTime = true;

    if(firstTime) {
        firstTime = false;

        conducteur.SetSpeed(normalSpeed);
    }

    bool transition = dist < slowDist;
    if(transition) firstTime = true; speedState = VIGILANCE;
}

void slowState() {
    static bool firstTime = true;

    if(firstTime) {
        firstTime = false;

        conducteur.SetSpeed(slowSpeed);
    }

    bool transition = dist > slowDist;
    if(transition) firstTime = true; speedState = NORMAL;
}

void debugTask() {
    static unsigned long lastDebug = 0;
    if(currentTime - lastDebug < debugDelay) return;

    conducteur.DebugPrint();
}

void avance() {
    conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
    conducteur.SetTrackerPID(0, 0, 0);

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(FORWARD);
}

void avanceAuto() {
    bool blinkState = switchBoolTask(blinkRate);
    conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
    conducteur.SetTrackerPID(0, 0, 0);

    if (firstAuto) {
        firstAuto = false;

        conducteur.SetDriveMode(DISTANCE);
        conducteur.SetDistance(distance);
        conducteur.SetState(FORWARD);

        Serial.print(F("[AUTO] Distance cible: "));
        Serial.println(distance);
    }

    blinkState ? anneau.fullLeds(10, 10, 0) : anneau.fullLeds(0, 0, 0);

    if (conducteur.GetState() == STOP) {
        Serial.println(F("[AUTO] Distance atteinte, arrêt automatique"));
        arreterState();
    }
}

void leftState() {
    bool blinkState = switchBoolTask(blinkRate);
    conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
    conducteur.SetTrackerPID(0, 0, 0);

    anneau.SetFirstLed(11);
    anneau.SetLastLed(2);

    conducteur.SetDriveMode(FREE);
    conducteur.SetState(LTURNING);

    blinkState ? anneau.partLeds(0, 10, 0) : anneau.fullLeds(0, 0, 0);
}

void rightState() {
    bool blinkState = switchBoolTask(blinkRate);
    conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
    conducteur.SetTrackerPID(0, 0, 0);

    anneau.SetFirstLed(2);
    anneau.SetLastLed(5);

    conducteur.SetDriveMode(FREE);
    conducteur.SetState(RTURNING);

    blinkState ? anneau.partLeds(0, 10, 0) : anneau.fullLeds(0, 0, 0);
}

void reculerState() {
    bool buzzerState = switchBoolTask(beepRate);
    conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
    conducteur.SetTrackerPID(0, 0, 0);

    conducteur.SetDriveMode(FREE);
    conducteur.SetState(BACKWARD);

    buzzerState ? analogWrite(BUZZER_PIN, backwardPWM) : analogWrite(BUZZER_PIN, 0);
}

void arreterState() {
    conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
    conducteur.SetTrackerPID(0, 0, 0);

    setAState(LIBRE);
    conducteur.SetState(STOP);
    analogWrite(BUZZER_PIN, 0);


    anneau.SetFirstLed(5);
    anneau.SetLastLed(11);
    anneau.partLeds(10, 0, 0);
}



void serialEvent() {

  static String receivedData = "";

  if (!Serial.available()) return;

  receivedData = Serial.readStringUntil('\n');
  parseData(receivedData);
}

void parseData(String& receivedData) {
  bool isFromBLE = false;

  if (receivedData.length() >= 2) {

    if ((uint8_t)receivedData[0] == 0xFF && (uint8_t)receivedData[1] == 0x55) {
      isFromBLE = true;

      receivedData.remove(0, 2);
    }

    else if (receivedData.startsWith("!!")) {

      receivedData.remove(0, 2);
    } else {

      Serial.print(F("Données non reconnues : "));
      Serial.println(receivedData);
      return;
    }
  } else {
    Serial.print(F("Données trop courtes : "));
    Serial.println(receivedData);
    return;
  }


  if (debugMode) {
    conducteur.DebugPrint();
  }


  int firstComma = receivedData.indexOf(',');

  if (firstComma == -1) {

    handleCommand(receivedData);
  } else {

    String command = receivedData.substring(0, firstComma);
    String params = receivedData.substring(firstComma + 1);
    handleCommandWithParams(command, params);
  }
}

void handleCommand(String command) {
  // Utilisation d'un switch pour les commandes sans paramètres
  char cmd = command[0];
  switch (cmd) {
    case 'k':  // Commande "KLAXON"
      Serial.println(F("Commande KLAXON reçue"));
      klaxonAction();
      break;

    case 'd':  // Commande pour basculer le mode débogage
      debugMode = !debugMode;
      Serial.print(F("Mode débogage : "));
      Serial.println(debugMode ? F("activé") : F("désactivé"));
      break;

    case 'F':  //Commande FORWARD
      Serial.print(F("Commande FORWARD reçue"));

      setAState(LIBRE);
      avance();


      break;

    case 'B':  //Commande BACKWARD
      Serial.print(F("Commande BACKWARD reçue"));

      reculerState();

      break;

    case 'R':  //Commande RIGHT
      Serial.print(F("Commande RIGHT reçue"));

      rightState();

      break;

    case 'L':  //Commande LEFT
      Serial.print(F("Commande LEFT reçue"));

      leftState();

      break;

    case 'S':
      Serial.print(F("Commande STOP reçue"));

      arreterState();

      break;

    case 'C':
        Serial.print(F("Commande CALIBRATION reçue"));
        conducteur.SetGyroPID(0, 0, 0);
        conducteur.SetTrackerPID(trackerKp, trackerKi, trackerKd);
        conducteur.SetState(CALIBRATE);
        break;
    default:
      Serial.print(F("Commande inconnue sans paramètres : "));
      Serial.println(command);
      break;
  }
}

void handleCommandWithParams(String command, String params) {
  char cmd = command[0];
  switch (cmd) {
    case 'A':
      // if(command != "AUTO") return;

      Serial.print(F("Commande AUTO reçue avec paramètres : "));
      Serial.println(params);

      distance = params.toFloat();

      setAState(AUTO);

      break;

    case 'p':
      Serial.print(F("Commande Changement de vitesse reçue avec paramètres : "));
      Serial.println(params);

      normalSpeed = params.toFloat();

      conducteur.SetSpeed(normalSpeed);

      break;

    case 'l':  // Commande "LIGHT" pour définir la couleur de l'anneau LED
      Serial.print(F("Commande LIGHT reçue avec paramètres : "));
      Serial.println(params);
      commandLight(params);
      break;

    default:
      Serial.print(F("Commande inconnue avec paramètres : "));
      Serial.print(command);
      Serial.print(F(", "));
      Serial.println(params);
      break;
  }
}

void commandLight(String params) {
  int commaCount = countCharOccurrences(params, ',');

  if (commaCount == 2) {

    int r = params.substring(0, params.indexOf(',')).toInt();
    params = params.substring(params.indexOf(',') + 1);
    int g = params.substring(0, params.indexOf(',')).toInt();
    int b = params.substring(params.indexOf(',') + 1).toInt();

    anneau.fullLeds(r, g, b);
  } else if (commaCount == 3) {

    int idx = params.substring(0, params.indexOf(',')).toInt();
    params = params.substring(params.indexOf(',') + 1);
    int r = params.substring(0, params.indexOf(',')).toInt();
    params = params.substring(params.indexOf(',') + 1);
    int g = params.substring(0, params.indexOf(',')).toInt();
    int b = params.substring(params.indexOf(',') + 1).toInt();

    anneau.oneLed(idx, r, g, b);
  } else {
    Serial.println(F("Commande lumière invalide"));
  }
}





void klaxonAction() {
  analogWrite(BUZZER_PIN, 100);
}

int countCharOccurrences(const String& str, char ch) {
  int count = 0;
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == ch) {
      count++;
    }
  }
  return count;
}

void setAState(avanceState s) {
  static avanceState lastAState = LIBRE;
  if (lastAState == s) return;

  lastAState = s;
  aState = s;
  firstAuto = true;
}



bool switchBoolTask(int delay) {
  static unsigned long lastTime = 0;
  int rate = delay;
  static bool state = false;

  if (currentTime - lastTime >= rate) {
    lastTime = currentTime;
    state = !state;
  }

  return state;
}
