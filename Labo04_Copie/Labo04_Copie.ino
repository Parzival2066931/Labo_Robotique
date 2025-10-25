#include "MeAnneau.h"
#include "MeConducteur.h"
#include "MeSonar.h"
#include <ezBuzzer.h>

#define BUZZER_PIN 45

Anneau anneau;
Conducteur conducteur;
Sonar sonar;
ezBuzzer buzzer(BUZZER_PIN);


unsigned long currentTime = 0;

bool debugMode = false;
bool firstAuto = true;

float distance = 0;
float speed = 100;
float turnSpeed = 100;
float minSpeed = 50;
float maxSpeed = 255;
const int blinkRate = 500;
const int beepRate = 500;
const int backwardPWM = 200;
const int klaxonFreq = 420;

enum avanceState {
  AUTO,
  LIBRE
};

avanceState aState;


void setup() {
  Serial.begin(115200);
  attachInterrupt(conducteur.GetPinRight(), Conducteur::GetIsrRight, RISING);
  attachInterrupt(conducteur.GetPinLeft(), Conducteur::GetIsrLeft, RISING);

  anneau.Setup();
  conducteur.Setup();
  sonar.Setup();

  conducteur.SetPID(9, 1, 3);
  conducteur.SetTurnSpeed(turnSpeed);
  conducteur.SetMinSpeed(minSpeed);
  conducteur.SetMaxSpeed(maxSpeed);
  conducteur.SetSpeed(speed);

  buzzer.stop();

  aState = LIBRE;
}

void loop() {
  currentTime = millis();

  update();
}

void update() {

  sonar.Update();
  conducteur.Update();
  buzzer.loop();

  switch (aState) {
    case AUTO:
      avanceAuto();
      break;
    case LIBRE:
      break;
  }
}

void avance() {

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(FORWARD);
}

void avanceAuto() {
  bool blinkState = switchBoolTask(blinkRate);

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

  anneau.SetFirstLed(11);
  anneau.SetLastLed(2);

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(LTURNING);

  blinkState ? anneau.partLeds(0, 10, 0) : anneau.fullLeds(0, 0, 0);
}

void rightState() {
  bool blinkState = switchBoolTask(blinkRate);

  anneau.SetFirstLed(2);
  anneau.SetLastLed(5);

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(RTURNING);

  blinkState ? anneau.partLeds(0, 10, 0) : anneau.fullLeds(0, 0, 0);
}

void reculerState() {
  bool buzzerState = switchBoolTask(beepRate);

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(BACKWARD);

  buzzerState ? analogWrite(BUZZER_PIN, backwardPWM) : analogWrite(BUZZER_PIN, 0);
}

void arreterState() {

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
    Serial.print(F("Reçu : "));
    Serial.println(receivedData);
    Serial.print(F("Source : "));
    Serial.println(isFromBLE ? F("BLE") : F("Moniteur Série"));
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

      speed = params.toFloat();

      conducteur.SetSpeed(speed);

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