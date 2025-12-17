#include <Adafruit_seesaw.h>
#include <ArduinoJson.h>
#include "MeConducteur.h"
#include "Sonar.h"
#include "MeAnneau.h"
#include <ezBuzzer.h>

#define BUZZER_PIN 45

bool debugMode = false;
bool firstAuto = true;
bool firstFollow = true;
bool firstVigilance = true;

unsigned long countdownTimer = 0;
unsigned long chronoStart = 0;
unsigned long chronoEnd = 0;
bool greenLedActive = false;
bool firstMission = true;
int countdownValue = 3;
int countCheckpoint = 0;
bool newCheckpoint = false;
bool hasPackage = false;
bool checkpointLocked = false;
int turnDelay = 300;
int noLineDelay = 150;
int distUntil = 10; 
int firstLed = 5;

bool packageReady = true;
bool intersectionArmed = true;
bool firstFind = true;
bool firstPackage = true;


float normalSpeed = 100;
float slowSpeed = 50;
float turnSpeed = 70;
float minSpeed = 50;
float maxSpeed = 255;
int slowDist = 50;
int grabPackageDist = 20;
int dist;
int printDelay = 1000;

const int blinkRate = 500;
const int beepRate = 500;
const int backwardPWM = 200;
const int klaxonFreq = 420;

double trackerKp = 0.5;
double trackerKi = 0;
double trackerKd = 0;

double gyroKp = 9;
double gyroKi = 1;
double gyroKd = 3;


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

enum MissionState {
  COUNTDOWN,
  FIND_LINE,
  M_ON_LINE,
  PACKAGE_SEARCH,
  DELIVER_PACKAGE,
  MANUAL,
  UNTIL,
  ARRIVED
};
MissionState mState = MANUAL;

enum FindState {
  INIT,
  WAITING,
  TURNING
};
FindState findState = INIT;

enum PackageSearchState {
  PS_INIT,
  PS_APPROACH,
  PS_RETURN
};
PackageSearchState psState = PS_INIT; 


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

  mState = MANUAL;
  
}

void loop() {
  currentTime = millis();
  if(mState != ARRIVED) chronoEnd = currentTime;

  

  sonar.Update();
  
  dist = sonar.GetDist(); 
  conducteur.Update(dist);

  speedUpdate();

  missionUpdate();

  buzzer.loop();

  ledTask();
  serialTask();
  if(debugMode) debugTask();
}


void ledTask() {
  if (!greenLedActive) return;

  if (currentTime - chronoStart >= 1000) {
    anneau.offLeds();
    greenLedActive = false;
  }
}


void missionUpdate() {
  switch (mState) {
    case COUNTDOWN:         
      missionCountdown();      
      break;
    case FIND_LINE:           
      missionFindLine();       
      break;
    case M_ON_LINE:         
      missionOnLine();         
      break;
    case PACKAGE_SEARCH:    
      missionPackageSearch();  
      break;
    case DELIVER_PACKAGE:   
      missionDeliverPackage(); 
      break;
    case MANUAL:  
      if(firstMission) {
        firstMission = false;
        conducteur.SetSpeed(maxSpeed);
      }     
      break;
    case UNTIL:
      missionDriveUntil();
      break;
    case ARRIVED:
      anneau.RainbowRing();
      break;
  }
}

void missionCountdown() {
  if (firstMission) {
    firstMission = false;
    countdownValue = 3;
    countdownTimer = currentTime;

    conducteur.SetSpeed(normalSpeed);
    conducteur.SetTurnSpeed(turnSpeed);

    anneau.fullLeds(10, 0, 0);
    Serial.println("3");
  }

  if (currentTime - countdownTimer < 1000) return;
  countdownTimer = currentTime;
  countdownValue--;

  if (countdownValue == 2) {
    Serial.println("2");
  }
  else if (countdownValue == 1) {
    anneau.fullLeds(10, 10, 0);
    Serial.println("1");
  }
  else if (countdownValue == 0) {
    anneau.fullLeds(0, 10, 0);
    Serial.println("GO!");

    greenLedActive = true;
    chronoStart = currentTime;

    SetMState(FIND_LINE);
  }
}

void missionFindLine() {

  switch(findState) {
    case INIT:
      findInit();      
      break;
    case WAITING:
      findWaiting();
      break;
    case TURNING:
      findTurning();
      break;
  }
}
void findInit() {
  if(firstFind) {
    firstFind = false;

    conducteur.SetTrackerPID(0,0,0);
    conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
    conducteur.SetDriveMode(FREE);
    conducteur.SetState(FORWARD);
    conducteur.SetSpeed(normalSpeed);
  }

  if (conducteur.IsIntersection()) {
    SetFindState(WAITING);
  }
}
void findWaiting() {
  static unsigned long lastTime = 0;

  if(firstFind) {
    firstFind = false;

    lastTime = currentTime;
  }

  if(currentTime - lastTime < turnDelay) return;

  conducteur.SetState(STOP);
  SetFindState(TURNING);
}
void findTurning() {
  if(firstFind) {
    firstFind = false;

    conducteur.SetAngle(90);
    conducteur.SetDriveMode(DISTANCE);
    conducteur.SetState(RTURNING);
  }

  if(conducteur.GetState() == STOP) {
    conducteur.SetTrackerPID(trackerKp, trackerKi, trackerKd);
    conducteur.SetGyroPID(0, 0, 0);

    firstFind = true;
    missionCheckpoint();
    SetMState(M_ON_LINE);
  }
}








void missionCheckpoint() {
  newCheckpoint = true;
  anneau.SetFirstLed(firstLed);
  anneau.SetLastLed(firstLed + countCheckpoint);
  countCheckpoint++;
  anneau.halfLeds(50, 0, 50);
}

void missionOnLine() {
  if(firstMission) {
    firstMission = false;

    conducteur.SetState(FOLLOW);
  }

  if(dist < slowDist && packageReady) {
    conducteur.SetState(STOP);
    packageReady = false;
    SetMState(PACKAGE_SEARCH);
  }

  if(conducteur.IsIntersection() && intersectionArmed) {
    intersectionArmed = false;
    missionCheckpoint();
  }
  if(!conducteur.IsIntersection()) intersectionArmed = true;

  if(conducteur.IsDeliveryDone()) {
    conducteur.SetState(STOP);
    SetMState(DELIVER_PACKAGE);
  }
}

void missionPackageSearch() {
  switch (psState) {
    case PS_INIT:
      psInit();
      break;
    case PS_APPROACH:
      psApproach();
      break;
    case PS_RETURN:
      psReturn();
      break;
  }
}
void psInit() {
  if (firstPackage) {
    firstPackage = false;

    conducteur.SetTrackerPID(0,0,0);
    conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
    conducteur.SetDriveMode(FREE);
    conducteur.SetState(FORWARD);
  }

  if (conducteur.IsStableNoLine(noLineDelay)) {
    psState = PS_APPROACH;
    firstPackage = true;
  }
}
void psApproach() {
  if (dist < grabPackageDist) {
    conducteur.SetState(STOP);
    psState = PS_RETURN;
    firstPackage = true;

    anneau.SetFirstLed(11);
    anneau.SetLastLed(5);
    anneau.halfLeds(20, 10, 0);
  }
}
void psReturn() {
  if (firstPackage) {
    firstPackage = false;

    conducteur.SetAngle(180);
    conducteur.SetDriveMode(DISTANCE);
    conducteur.SetState(LTURNING);
  }

  if (!conducteur.GetTurnState()) return;

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(FORWARD);

  if (conducteur.IsCentered()) {
    conducteur.SetTrackerPID(trackerKp, trackerKi, trackerKd);
    conducteur.SetGyroPID(0, 0, 0);
    conducteur.SetState(FOLLOW);

    missionCheckpoint();
    SetMState(M_ON_LINE);
    firstPackage = true;
  }
}

void missionDeliverPackage() {
  if(firstMission) {
    firstMission = false;

    conducteur.SetSpeed(slowSpeed);
    conducteur.SetAngle(90);
    conducteur.SetDriveMode(DISTANCE);
    conducteur.SetState(RTURNING);
  }

  if(!conducteur.GetTurnState()) return;

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(BACKWARD);

  if(!conducteur.IsNoLine()) return;

  anneau.SetFirstLed(11);
  anneau.SetLastLed(5);
  anneau.halfLeds(0, 10, 0);

  missionCheckpoint();

  conducteur.SetState(STOP);
  SetMState(MANUAL);
}

void missionDriveUntil() {
  bool transition = sonar.GetDist() <= distUntil;

  if (firstMission) {
    firstMission = false;

    missionCheckpoint();

    conducteur.SetDriveMode(FREE);
    conducteur.SetState(FORWARD);
  }

  if (transition) {
    Serial.println("STOP");
    conducteur.SetState(STOP);
    SetMState(ARRIVED);
  }
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
  if(transition) { firstTime = true; speedState = VIGILANCE; }
}

void slowState() {
    if(mState == MANUAL) return;
    static bool firstTime = true;

    if(firstTime) {
        firstTime = false;

        conducteur.SetSpeed(slowSpeed);
    }

    bool transition = dist > slowDist;
    if(transition) { firstTime = true; speedState = NORMAL; }
}

void debugTask() {
    static unsigned long lastDebug = 0;
    if(currentTime - lastDebug < printDelay) return;
    lastDebug = currentTime;

    conducteur.DebugPrint();
}



void avance() {
  conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
  conducteur.SetTrackerPID(0, 0, 0);

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(FORWARD);
}

void leftState() {
  conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
  conducteur.SetTrackerPID(0, 0, 0);

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(LTURNING);
}

void rightState() {
  conducteur.SetGyroPID(gyroKp, gyroKi, gyroKd);
  conducteur.SetTrackerPID(0, 0, 0);

  conducteur.SetDriveMode(FREE);
  conducteur.SetState(RTURNING);
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

  conducteur.SetState(STOP);
  analogWrite(BUZZER_PIN, 0);
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
      if(mState != MANUAL) return

      Serial.print(F("Commande FORWARD reçue"));

      avance();
      break;

    case 'B':  //Commande BACKWARD
      if(mState != MANUAL) return
      Serial.print(F("Commande BACKWARD reçue"));

      reculerState();

      break;

    case 'R':  //Commande RIGHT
      if(mState != MANUAL) return
      Serial.print(F("Commande RIGHT reçue"));

      rightState();

      break;

    case 'L':  //Commande LEFT
      if(mState != MANUAL) return
      Serial.print(F("Commande LEFT reçue"));

      leftState();

      break;

    case 'S':
      if(mState != MANUAL) return
      Serial.print(F("Commande STOP reçue"));

      arreterState();

      break;

    case 'C':
      Serial.print(F("Commande CALIBRATION reçue"));
      resetMission();
      conducteur.SetGyroPID(0, 0, 0);
      conducteur.SetTrackerPID(trackerKp, trackerKi, trackerKd);
      conducteur.SetState(CALIBRATE);
      break;

    case 'G':
      Serial.println(F("Commande GO reçue"));
      Serial.println(F("Début du chrono"));
      SetMState(COUNTDOWN);
      break;

    case 'U':
      Serial.println(F("Commande UNTIL reçue"));
      SetMState(UNTIL);
      break;

    case 'M':
      Serial.println(F("Commande Manual reçu"));
      SetMState(MANUAL);
      conducteur.SetState(STOP);
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
    case 'p':
      Serial.print(F("Commande Changement de vitesse reçue avec paramètres : "));
      Serial.println(params);

      normalSpeed = params.toFloat();
      conducteur.SetSpeed(normalSpeed);
      break;

    case 'l':  
      Serial.print(F("Commande LIGHT reçue avec paramètres : "));
      Serial.println(params);

      commandLight(params);
      break;
    
    case 'P':
      Serial.print(F("Commande Set Kp reçue avec paramêtre : "));
      Serial.println(params);

      trackerKp = params.toDouble();
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




void SetMState(MissionState s) {
  if (mState == s) return;

  mState = s;
  firstMission = true;
}

void SetFindState(FindState s) {
    if(findState == s) return;

    findState = s;
    firstFind = true;
}

void resetMission() {
  unsigned long countdownTimer = 0;
  unsigned long chronoStart = 0;
  unsigned long chronoEnd = 0;
  bool greenLedActive = false;
  bool firstMission = true;
  int countdownValue = 3;
  int countCheckpoint = 0;
  bool newCheckpoint = false;
  bool hasPackage = false;
  bool checkpointLocked = false;
  int turnDelay = 300;
  int noLineDelay = 150;
  int distUntil = 10; 
  int firstLed = 5;

  bool packageReady = true;
  bool intersectionArmed = true;
  bool firstFind = true;
  bool firstPackage = true;
  anneau.offLeds();
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

void serialTask() {
  static unsigned long lastPrint = 0;
  if(currentTime - lastPrint < printDelay) return;
  lastPrint = currentTime;

  static StaticJsonDocument<128> doc;
  doc["ts"] = currentTime;
  doc["chrono"] = chronoEnd - chronoStart;
  doc["gz"] = conducteur.GetAngleZ();
  doc["etat"] = mState;
  if(newCheckpoint) {
    newCheckpoint = false;
    doc["cp"] = countCheckpoint;
  }

  static JsonObject pwm = doc.createNestedObject("pwm");
  pwm["l"] = conducteur.GetLeftPwm();
  pwm["r"] = conducteur.GetRightPwm();

  static JsonArray capt = doc.createNestedArray("capt");
  capt.add(conducteur.GetTrackerVal(0));
  capt.add(conducteur.GetTrackerVal(1));
  capt.add(conducteur.GetTrackerVal(2));
  capt.add(conducteur.GetTrackerVal(3));
  capt.add(conducteur.GetTrackerVal(4));


  static char output[256];
  serializeJson(doc, output);
  Serial.println(output);
}