#include <Adafruit_seesaw.h>
#include "MeConducteur.h"
#include "Sonar.h"



float normalSpeed = 100;
float slowSpeed = 60;
float turnSpeed = 100;
float minSpeed = 50;
float maxSpeed = 255;

unsigned long currentTime = 0;

Conducteur conducteur;

CruzeControlState {
    NORMAL,
    VIGILANCE
}
CruzeControlState speedState = NORMAL;


void setup() {
    Serial.begin(115200);

    conducteur.Setup();
    conducteur.SetGyroPID(0, 0, 0);
    conducteur.SetTrackerPID(0.9, 0, 0);

    conducteur.SetTurnSpeed(turnSpeed);
    conducteur.SetMinSpeed(minSpeed);
    conducteur.SetMaxSpeed(maxSpeed);
    conducteur.SetSpeed(speed);
    conducteur.SetState(CALIBRATE);
}

void loop() {
    currentTime = millis();

    speedUpdate();
    conducteur.Update();
}

void speedUpdate() {
    switch(speedState) {
        case NORMAL:
            conducteur.SetSpeed(normalSpeed);
            break;
        case VIGILANCE:
            conducteur.SetSpeed(slowSpeed);
            break;
    }
}

