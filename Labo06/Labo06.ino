#include <Adafruit_seesaw.h>
#include "MeConducteur.h"
#include "Sonar.h"



float normalSpeed = 100;
float slowSpeed = 120;
float turnSpeed = 70;
float minSpeed = 50;
float maxSpeed = 255;
int slowDist = 50;
int dist;

unsigned long currentTime = 0;

Conducteur conducteur;
Sonar sonar;

enum CruzeControlState {
    NORMAL,
    VIGILANCE
};
CruzeControlState speedState = NORMAL;

enum IntersectionState {
    NONE,
    I_IDLE,
    I_TURN_90,
    I_CHECK_1,
    I_TURN_180,
    I_CHECK_2
};
IntersectionState iState = I_IDLE;
bool deliveryDone = false;


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

    

    sonar.Update();
    
    dist = sonar.GetDist(); 
    conducteur.Update(dist);

    speedUpdate();
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

