#include <Arduino.h>
#include "LyneTracker.h"

Tracker::Tracker() {}

void Tracker::Setup() {
  _calibrationDelay = 10;
  
  if (!ss.begin()) {
      Serial.println("Erreur de connexion au LyneTracker");
      while (1);
  }
  Serial.println("Connexion réussie au LyneTracker!");
}

void Tracker::Update() {
  for (int i = 0; i < NB_IR; i++) {
    _capteur[i].val = ss.analogRead(i);
  }
  _normalizeValues();
}



int Tracker::GetMinVal(int index) const { return _capteur[index].min; }

int Tracker::GetMaxVal(int index) const { return _capteur[index].max; }

double Tracker::GetLinePosition() {
  double numerator = 0;
  double denominator = 0;

  for(int i = 0; i < NB_IR; i++) {
    numerator += _capteur[i].normal_val * (i - 2);
    denominator += _capteur[i].normal_val;
  }

  return (denominator == 0) ? 0 : (numerator / denominator * 1000.0);
}

void Tracker::_SetMinVal(int value, int index) { _capteur[index].min = value; }

void Tracker::_SetMaxVal(int value, int index) { _capteur[index].max = value; }

void Tracker::Calibrate_IR() {
  _currentTime = millis();

  if(_currentTime - _lastCalibration < _calibrationDelay) return;
  _lastCalibration = _currentTime;

  //Calibration
  for(int i = 0; i < NB_IR; i++) {
    _capteur[i].val = ss.analogRead(i);
    if(_capteur[i].min > _capteur[i].val) _SetMinVal(_capteur[i].val, i);
    if(_capteur[i].max < _capteur[i].val) _SetMaxVal(_capteur[i].val, i);
    _capteur[i].seuil = _capteur[i].min + 300;
  }
}

int Tracker::_capteurLectureNormalisee(int index) {
  int range = _capteur[index].max - _capteur[index].min;

  return (range == 0) ? 0 : ((_capteur[index].val - _capteur[index].min) * 1000.0 / range);
}

void Tracker::_normalizeValues() {
  for(int i = 0; i < NB_IR; i++) {
    _capteur[i].normal_val = _capteurLectureNormalisee(i);
    _capteur[i].onLine = (_capteur[i].normal_val < _capteur[i].seuil);
  }
}

void Tracker::DebugPrint() {
  Serial.print("IR : ");
  for (int i = 0; i < NB_IR; i++) {
    
    Serial.print("Pin "); Serial.print(i);
    Serial.print(" = "); Serial.print(ss.analogRead(i));
    Serial.print("\t");
  }
  Serial.print(" | Pos = ");
  Serial.println(GetLinePosition());
}

double Tracker::GetTargetVal(int index) const { return _capteur[index].normal_val; }

bool Tracker::IsNoLine() {
  for (int i = 0; i < NB_IR; i++)
      if (_capteur[i].onLine) return false;
  return true;
}

bool Tracker::IsCentered() const {
    return (
        !_capteur[0].onLine &&
        !_capteur[4].onLine &&
        (_capteur[1].onLine || _capteur[2].onLine || _capteur[3].onLine)
    );
}

bool Tracker::IsRightAngleLeft() const {
    return _capteur[0].onLine &&
           _capteur[1].onLine &&
           _capteur[2].onLine &&
          !_capteur[3].onLine &&
          !_capteur[4].onLine;
}

bool Tracker::IsRightAngleRight() const {
    return !_capteur[0].onLine &&
           !_capteur[1].onLine &&
            _capteur[2].onLine &&
            _capteur[3].onLine &&
            _capteur[4].onLine;
}

bool Tracker::IsIntersectionT() const {
    for (int i = 0; i < NB_IR; i++)
        if (!_capteur[i].onLine) return false;
    return true;
}