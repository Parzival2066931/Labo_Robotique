// #include <MeAuriga.h>
#include "MeConducteur.h"
#include "MeAnneau.h"
#include "MeSonar.h"

#define WAITING_DELAY 3000



Anneau anneau;
Conducteur conducteur;
Sonar sonar;

float distX;
float distY;
float driveBackDist;
int turnAngle;
int uTurnAngle;
int turnSpeed;
int speed;
int DA[7] = { 2, 0, 6, 6, 9, 3, 1 };
int route;

unsigned long currentTime;
unsigned long lastBlink;
int blinkDelay;
unsigned long lastDelivery;
int deliveryTimer;

enum LivreurState {
  ALLER,
  PIVOT,
  ARRET,
  LIVRAISON,
  RETOUR
};

enum AllerState {
  TRAJET_1,
  TRAJET_2,
  DONE
};

enum RetourState {
  UTURN,
  TRAJET_3,
  RETOUR_PIVOT,
  TRAJET_4,
  END
};

LivreurState state;
AllerState allerStep;
RetourState retourStep;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  attachInterrupt(conducteur.GetPinRight(), Conducteur::GetIsrRight, RISING);
  attachInterrupt(conducteur.GetPinLeft(), Conducteur::GetIsrLeft, RISING);

  anneau.Setup();
  conducteur.Setup();
  sonar.Setup();

  distX = 200;
  distY = 40;
  turnAngle = 90;
  uTurnAngle = 180;
  turnSpeed = 70;
  speed = 100;
  route = 0;

  blinkDelay = 500;
  deliveryTimer = 3000;

  state = ALLER;
  allerStep = TRAJET_1;
  retourStep = UTURN;

  sonar.SetPrintDelay(100);
  sonar.SetMinDist(0);
  sonar.SetMaxDist(400);

  conducteur.SetState(STOP);
  conducteur.SetTurnSpeed(turnSpeed);
  conducteur.SetSpeed(speed);
  conducteur.SetPID(6, 0.4, 4);

  anneau.fullLeds(10, 0, 0);

  delay(WAITING_DELAY);
  Serial.println("Setup completed");
}

void loop() {
  Update();
}

bool isEven(int number) {
  return (number % 2) == 0;
}

void pivotState() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    conducteur.SetAngle(turnAngle);
    conducteur.SetState(isEven(DA[6]) ? LTURNING : RTURNING);
  }

  if (conducteur.GetTurnState()) {
    firstTime = true;
    state = ALLER;
  }
}

void allerState() {
  switch (allerStep) {
    case TRAJET_1:
      progressBarTask();
      firstRouteTask();
      break;
    case TRAJET_2:
      secondRouteTask();
      break;
    case DONE:
      break;
  }
}

void firstRouteTask() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    conducteur.SetDistance(distX);
    conducteur.SetDriveMode(DISTANCE);
    conducteur.SetState(FORWARD);
  }

  if (conducteur.GetState() == STOP) {
    firstTime = true;


    allerStep = TRAJET_2;
    state = PIVOT;
  }
}

void secondRouteTask() {
  static bool firstTime = true;
  bool transition = sonar.GetDist() <= distY;
  static float startDist = 0;

  if (firstTime) {
    firstTime = false;


    startDist = conducteur.GetDistanceTraveled();
    conducteur.SetDriveMode(LIBRE);
    conducteur.SetState(FORWARD);
  }

  if (transition) {
    firstTime = true;
    driveBackDist = conducteur.GetDistanceTraveled() - startDist;
    conducteur.SetState(STOP);
    allerStep = DONE;
    state = LIVRAISON;
  }
}

void progressBarTask() {
  int lastLed = map(long(conducteur.GetDistanceTraveled()), 0, long(distX), 0, LEDNUM - 1);
  anneau.SetFirstLed(0);
  anneau.SetLastLed(lastLed);
  anneau.partLeds(0, 10, 0);
}

void livraisonState() {
  static bool firstTime = true;
  static bool ledState = false;


  if (firstTime) {
    firstTime = false;

    conducteur.SetState(STOP);
    lastBlink = currentTime;
    lastDelivery = currentTime;
    anneau.fullLeds(0, 0, 0);
  }

  if (currentTime - lastBlink < blinkDelay) return;
  lastBlink = currentTime;
  ledState = !ledState;


  if (ledState) {
    anneau.fullLeds(0, 10, 0);
  } else {
    anneau.fullLeds(0, 0, 0);
  }

  if (currentTime - lastDelivery >= deliveryTimer) {
    firstTime = true;
    ledState = false;

    state = RETOUR;
  }
}

void retourState() {

  returnAnimationTask();

  switch (retourStep) {
    case UTURN:
      uTurnTask();
      break;
    case TRAJET_3:
      returnRouteTask();
      break;
    case RETOUR_PIVOT:
      returnPivotTask();
      break;
    case TRAJET_4:
      returnRouteTask();
      break;
    case END:
      anneau.RainbowRing();
      break;
  }
}

void uTurnTask() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    conducteur.SetAngle(uTurnAngle);
    conducteur.SetState(RTURNING);
  }

  if (conducteur.GetTurnState()) {
    firstTime = true;
    conducteur.SetState(STOP);
    retourStep = TRAJET_3;
  }
}

void returnRouteTask() {
  static bool firstTime = true;


  if (firstTime) {
    firstTime = false;

    conducteur.SetDistance(retourStep == TRAJET_3 ? driveBackDist : distX);
    conducteur.SetDriveMode(DISTANCE);
    conducteur.SetState(FORWARD);
  }

  if (conducteur.GetState() == STOP) {
    firstTime = true;

    retourStep = retourStep == TRAJET_3 ? RETOUR_PIVOT : END;
  }
}

void returnPivotTask() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    conducteur.SetAngle(turnAngle);
    conducteur.SetState(isEven(DA[6]) ? RTURNING : LTURNING);
  }

  if (conducteur.GetTurnState()) {
    firstTime = true;
    retourStep = TRAJET_4;
  }
}

void returnAnimationTask() {
  static unsigned long lastTime = 0;
  const int rate = 100;

  if(currentTime - lastTime < rate) return;
  lastTime = currentTime;

  isEven(DA[5]) ? anneau.trailLed(0, 10, 0, true) : anneau.trailLed(0, 0, 10, false);
}


void Update() {
  currentTime = millis();

  conducteur.Update();
  sonar.Update();

  //Tests
  // allerState();


  switch (state) {
    case ALLER:
      allerState();
      break;
    case PIVOT:
      pivotState();
      break;
    case RETOUR:
      retourState();
      break;
    case LIVRAISON:
      livraisonState();
      break;
  }
}
