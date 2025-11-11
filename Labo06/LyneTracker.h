#pragma once
#include <Adafruit_seesaw.h>

#define NB_IR 5

struct Capteur_IR {
  int min = 1023;
  int max = 0;
  double normal_val = 0;
  int val = 0;
};

class Tracker {
private:
  Capteur_IR _capteur[NB_IR];
  Adafruit_seesaw ss;

  unsigned long _currentTime = 0;
  unsigned long _lastCalibration = 0;
  int _calibrationDelay = 10;

  void _SetMinVal(int value, int index);
  void _SetMaxVal(int value, int index);

  // Retourne une valeur normalisée entre 0 et 1000 (protégée contre division par zéro)
  int _capteurLectureNormalisee(int index);
  void _normalizeValues();

public:
  Tracker();

  void Setup();
  void Update();
  void Calibrate_IR();
  double GetLinePosition();

  int GetMinVal(int index) const;
  int GetMaxVal(int index) const;
  double GetTargetVal(int index) const; // 🔹 utile pour debug

  void DebugPrint();        // 🔹 affichage série des valeurs
};
