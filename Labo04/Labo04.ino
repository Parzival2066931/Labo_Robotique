
#include "MeAnneau.h"
#include "MeConducteur.h"
#include "MeSonar.h"

#define BUZZER_PIN 45

Anneau anneau;
Conducteur conducteur;
Sonar sonar;
MeBuzzer buzzer;


unsigned long currentTime = 0;

bool debugMode = false;

float distance = 0;
float speed = 100;
float backwardSpeed = 50;
float turnSpeed = 70;
float minSpeed = 50;
float maxSpeed = 255;
const int blinkRate = 500;
const int beepRate = 500;




enum teleComState {
  AVANCER,
  PIVOTER,
  RECULER,
  ARRETER
};

enum pivotState {
  GAUCHE,
  DROITE
};

enum avanceState {
  AUTO,
  LIBRE
};

// enum ledModeState {
//   FULL,
//   ONE
// };

teleComState state;
pivotState pState;
avanceState aState;
// ledModeState lState;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  buzzer.setpin(BUZZER_PIN);

  anneau.Setup();
  conducteur.Setup();
  sonar.Setup();

  conducteur.SetPID(6, 1, 3);
  conducteur.SetTurnSpeed(turnSpeed);
  conducteur.SetMinSpeed(minSpeed);
  conducteur.SetMaxSpeed(maxSpeed);
  conducteur.SetSpeed(speed);

  state = ARRETER;
}

void loop() {
  currentTime = millis();

  update();
}

void update() {

  sonar.Update();
  conducteur.Update();

  switch (state) {
    case AVANCER:
      avancerState();
      break;
    case PIVOTER:
      pivoterState();
      break;
    case RECULER:
      reculerState();
      break;
    case ARRETER:
      arreterState();
      break;
  }
}

void avancerState() {
  switch (aState) {
    case AUTO:
      avanceAuto();
      break;
    case LIBRE:
      avance();
      break;
  }
}

void avance() {

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(FORWARD);
}
void avanceAuto() {
  static bool firstTime = true;
  bool blinkState = switchBoolTask(blinkRate);

  if (firstTime) {
    firstTime = false;

    conducteur.SetDriveMode(DISTANCE);
    conducteur.SetDistance(distance);
  }

  blinkState ? anneau.fullLeds(10, 10, 0) : anneau.fullLeds(0, 0, 0);

  if (conducteur.GetState() == STOP) {
    firstTime = true;
  }
}

void pivoterState() {
  switch (pState) {
    case GAUCHE:
      leftState();
      break;
    case DROITE:
      rightState();
      break;
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
  anneau.partLeds(0, 10, 0);

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(RTURNING);


  blinkState ? anneau.partLeds(0, 10, 0) : anneau.fullLeds(0, 0, 0);
}

void reculerState() {
  bool buzzerState = switchBoolTask(beepRate);

  conducteur.SetSpeed(backwardSpeed);
  conducteur.SetDriveMode(FREE);
  conducteur.SetState(BACKWARD);


  // buzzerState ? buzzer.tone(1000) : buzzer.noTone();
}

void arreterState() {

  conducteur.SetState(STOP);
  buzzer.noTone();

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
  //QUEL EST LA DIFFÉRENCE ENTRE LES COMMANDE REÇU AVEC OU SANS PARAMÈTRE POUR LES DIRECTIONS?
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

      aState = LIBRE;
      setState(AVANCER);

      break;

    case 'B':  //Commande BACKWARD
      Serial.print(F("Commande BACKWARD reçue"));

      setState(RECULER);

      break;

    case 'R':  //Commande RIGHT
      Serial.print(F("Commande RIGHT reçue"));

      pState = DROITE;
      setState(PIVOTER);

      break;

    case 'L':  //Commande LEFT
      Serial.print(F("Commande LEFT reçue"));

      pState = GAUCHE;
      setState(PIVOTER);

      break;

    case 'S':
      Serial.print(F("Commande STOP reçue"));

      setState(ARRETER);

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

      aState = AUTO;
      setState(AVANCER);

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
  buzzer.tone(3000);
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

void setState(teleComState s) {
  static teleComState lastState = ARRETER;
  if (lastState == s) return;

  lastState = state;
  state = s;
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