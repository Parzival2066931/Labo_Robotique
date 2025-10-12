#include <MeAuriga.h>
#include "MeAnneau.h"
#include "MeConducteur.h"
#include "MeSonar.h"

Anneau anneau;
Conducteur conducteur;
Sonar sonar;
MeBuzzer buzzer;


unsigned long currentTime = 0;

bool debugMode = false;

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

enum ledModeState {
  FULL,
  ONE
};

teleComState state;
pivotState pState;
avanceState aState;
ledModeState lState;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  anneau.Setup();
  conducteur.Setup();
  sonar.Setup();


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
  switch(aState) {
    case AUTO:
      avanceAuto();
      break;
    case LIBRE:
      avance();
      break;
  }
}

void avance() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    conducteur.SetDriveMode(FREE);
    conducteur.SetState(FORWARD);
  }
}

void avanceAuto() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;
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
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    anneau.SetFirstLed(2);
    anneau.SetLastLed(5);

    conducteur.SetDriveMode(FREE);
    conducteur.SetState(LTURNING);
  }
}

void rightState() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    anneau.SetFirstLed(11);
    anneau.SetLastLed(2);

    conducteur.SetDriveMode(FREE);
    conducteur.SetState(RTURNING);
  }
}

void reculerState() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    conducteur.SetDriveMode(FREE);
    conducteur.SetState(BACKWARD);
  }
}

void arreterState() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    conducteur.SetState(STOP);
  }
}

void klaxonAction() {
  buzzer.tone(1000, 500);
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
    case 'b':  // Commande "BEEP"
      Serial.println(F("Commande BEEP reçue - exécuter le bip"));
      klaxonAction();
      break;

    case 'd':  // Commande pour basculer le mode débogage
      debugMode = !debugMode;
      Serial.print(F("Mode débogage : "));
      Serial.println(debugMode ? F("activé") : F("désactivé"));
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
    case 'F':  // Commande "FORWARD"
      Serial.print(F("Commande FORWARD reçue avec paramètres : "));
      Serial.println(params);

      commandForward(params);
      break;
    case 'R':
      Serial.print(F("Commande FORWARD reçue avec paramètres : "));
      Serial.println(params);

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
  } 
  else if (commaCount == 3) {
    
    int idx = params.substring(0, params.indexOf(',')).toInt();
    params = params.substring(params.indexOf(',') + 1);
    int r = params.substring(0, params.indexOf(',')).toInt();
    params = params.substring(params.indexOf(',') + 1);
    int g = params.substring(0, params.indexOf(',')).toInt();
    int b = params.substring(params.indexOf(',') + 1).toInt();
    
    anneau.oneLed(idx, r, g, b); 
  } 
  else {
    Serial.println(F("Commande lumière invalide"));
  }
}

void commandForward(String params) {
    // paramètre
    Serial.print(F("Paramètre : "));
    Serial.println(params);
    // Ajouter le code pour traiter la commande FORWARD avec ses paramètres
}




int countCharOccurrences(const String &str, char ch) {
  int count = 0;
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == ch) {
      count++;
    }
  }
  return count;
}