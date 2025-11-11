#include <Adafruit_seesaw.h>
#include "MeConducteur.h"




float speed = 100;
float turnSpeed = 100;
float minSpeed = 50;
float maxSpeed = 255;

unsigned long currentTime = 0;

Conducteur conducteur;


void setup() {
    Serial.begin(115200);

    conducteur.Setup();
    conducteur.SetGyroPID(0, 0, 0);
    conducteur.SetTrackerPID(1, 1, 1);

    conducteur.SetTurnSpeed(turnSpeed);
    conducteur.SetMinSpeed(minSpeed);
    conducteur.SetMaxSpeed(maxSpeed);
    conducteur.SetSpeed(speed);
    conducteur.SetState(CALIBRATE);
}

void loop() {
    currentTime = millis();

    conducteur.Update();
    // if(conducteur.GetState() == STOP) conducteur.SetState(FOLLOW);
}

