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
  INIT_1,
  TRAJET_1,
  INIT_2,
  TRAJET_2,
  DONE
};

LivreurState state;
AllerState allerStep;

void setup() {
  // put your setup code here, to run once:
  attachInterrupt(conducteur.GetPinRight(), Conducteur::GetIsrRight, RISING);
  attachInterrupt(conducteur.GetPinLeft(), Conducteur::GetIsrLeft, RISING);

  anneau.Setup();
  conducteur.Setup();
  sonar.Setup();

  distX = 100;
  distY = 150;
  turnAngle = 90;
  uTurnAngle = 180;
  turnSpeed = 100;
  speed = 100;
  route = 0;

  blinkDelay = 500;
  deliveryTimer = 3000;

  state = ALLER;

  conducteur.SetState(STOP);
  conducteur.SetTurnSpeed(turnSpeed);
  conducteur.SetSpeed(speed);

  delay(WAITING_DELAY);
}

void loop() {
  // put your main code here, to run repeatedly:
}

bool isEven(int number) {
  return (number % 2) == 0;
}

void pivotState() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    conducteur.SetAngle(turnAngle);

    if (isEven(DA[6])) {
      conducteur.SetState(LTURNING);
    } else {
      conducteur.SetState(RTURNING);
    }
  }

  if (conducteur.GetTurnState()) {
    firstTime = true;
    driveBackDist = sonar.GetDist() - distY;
    state = ALLER;
  }
}

void allerState() {
  switch (allerStep) {
    case TRAJET_1:
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
  bool transition = conducteur.GetState() == STOP;

  if (firstTime) {
    firstTime = false;

    conducteur.SetDistance(distX);
    conducteur.SetDriveMode(DISTANCE);
    conducteur.SetState(FORWARD);
  }

  if (transition) {
    firstTime = true;

    conducteur.SetAngle(turnAngle);
    conducteur.SetState(isEven(DA[6]) ? LTURNING : RTURNING);
    allerStep = TRAJET_2;
    state = PIVOT;
  }
}

void secondRouteTask() {
  static bool firstTime = true;
  bool transition = sonar.GetDist() <= distY;

  if (firstTime) {
    firstTime = false;

    conducteur.SetDriveMode(LIBRE);
    conducteur.SetState(FORWARD);
  }

  if (transition) {
    firstTime = true;

    conducteur.SetState(STOP);
    allerStep = DONE;
    state = LIVRAISON;
  }
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
  // static bool firstTime = true;
  // bool hasTurned = conducteur.GetTurnState();


  // if (firstTime) {
  //   firstTime = false;
  //   conducteur.SetAngle(uTurnAngle);
  //   conducteur.SetState(RTURNING);
  //   // animationRetour();
  // }

  // if (!hasTurned) return;

  // conducteur.Drive(driveBackDist);

  // if (fabs(conducteur.GetDistToGo()) >= 2) return;


  // conducteur.SetAngle(turnAngle);
  // if (isEven(DA[6])) {
  //   conducteur.SetState(RTURNING);
  // } else {
  //   conducteur.SetState(LTURNING);
  // }

  // if (!hasTurned) return;
  // //marche pas, hasTurned va return au premier et va entrer en boucle infini
  // //trouver une autre solution car code devient trop impropre, trop de flag
}

void Update() {
  currentTime = millis();


  conducteur.Update();
  sonar.Update();

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
