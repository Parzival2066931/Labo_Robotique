#pragma once
#include <Adafruit_seesaw.h>

#define NB_IR 5

struct Capteur_IR {
  int min = 1023;
  int max = 0;
  double normal_val = 0;
  int val = 0;
  bool onLine = false;
  int seuil = 0;
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
  double GetTargetVal(int index) const; 

  bool IsOnLine() const;
  bool IsNoLine() const;
  bool IsCentered() const;
  bool IsRightAngleLeft() const;
  bool IsRightAngleRight() const;
  bool IsIntersection() const;

  void DebugPrint();
  void ResetValues();
};
