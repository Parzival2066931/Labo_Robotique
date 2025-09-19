#include "Patrouilleur.h"

Patrouille patrouille;

void setup() {
  Serial.begin(115200);
  patrouille.Setup();
}

void loop() {
  patrouille.Update();
  patrouille.PrintTask();
}