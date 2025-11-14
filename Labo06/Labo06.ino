#include <Adafruit_seesaw.h>
#include "MeConducteur.h"
#include "Sonar.h"



float normalSpeed = 60;
float slowSpeed = 100;
float turnSpeed = 100;
float minSpeed = 50;
float maxSpeed = 255;
int slowDist = 50;

unsigned long currentTime = 0;

Conducteur conducteur;
Sonar sonar;

enum CruzeControlState {
    NORMAL,
    VIGILANCE
};
CruzeControlState speedState = NORMAL;


void setup() {
    Serial.begin(115200);

    conducteur.Setup();
    conducteur.SetGyroPID(0, 0, 0);
    conducteur.SetTrackerPID(0.75, 0, 0);

    conducteur.SetTurnSpeed(turnSpeed);
    conducteur.SetMinSpeed(minSpeed);
    conducteur.SetMaxSpeed(maxSpeed);
    conducteur.SetSpeed(normalSpeed);
    conducteur.SetState(CALIBRATE);
}

void loop() {
    currentTime = millis();

    speedUpdate();
    sonar.Update();
    conducteur.Update();
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

    bool transition = sonar.GetDist() < slowDist;
    if(transition) firstTime = true; speedState = VIGILANCE;
}

void slowState() {
    static bool firstTime = true;

    if(firstTime) {
        firstTime = false;

        conducteur.SetSpeed(slowSpeed);
    }

    bool transition = sonar.GetDist() > slowDist;
    if(transition) firstTime = true; speedState = NORMAL;
}

