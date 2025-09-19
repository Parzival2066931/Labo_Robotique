#pragma once
#include "MeConducteur.h"
#include "MeSonar.h"
#include "MeAnneau.h"



#pragma once

enum RobotState {NORMAL, RALENTI, DANGER, RONDE};

class Patrouille {
  private:
    int _dist;
    int _minDist;
    int _dangerDist;
    int _slowDist;

    int _backwardDelay;
    int _stopDelay;
    int _printDelay;
    int _rondeDelay;
    int _blinkDelay;
    int _rondeStateDelay;
    
    unsigned long _currentTime;
    unsigned long _lastStop;
    unsigned long _lastBackward;
    unsigned long _lastRonde;
    unsigned long _lastBlink;
    unsigned long _lastSuccess;

    bool _blinkState;
    bool _hasTurned;


    int _turnAngle;
    int _turnSpeed;
    int _normalSpeed;
    int _slowSpeed;
    int _maxSpeed;
    
    RobotState _state;
    ConducteurState _cState;
    Conducteur _conducteur;
    Anneau _anneau;
    Sonar _sonar;

    void _normalState();
    void _slowState();
    void _dangerState();
    void _rondeState();

    


  public:
    Patrouille();

    void SetBackwardDelay(int delay);
    void SetStopDelay(int delay);
    void SetPrintDelay(int delay);
    void SetTurnAngle(int angle);
    void SetNormalSpeed(int speed);
    void SetSlowSpeed(int speed);
    void PrintTask();
    void Update();
    void Setup();
};