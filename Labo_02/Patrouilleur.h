#include "MeConduire.h"
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
    unsigned long _currentTime;

    int _turnAngle;
    
    RobotState _state;
    Conduire _conduit;
    Anneau _anneau;
    Sonar _sonar;

    void _normalState();
    void _slowState();
    void _dangerState();
    void _rondeState();

    void _printTask();


  public:
    Patrouille();

    void setBackwardDelay(int delay);
    void setStopDelay(int delay);
    void setPrintDelay(int delay);
    void setTurnAngle(int angle);
    
    void Anneau();
    void Vitesse();
    void Update();
    void Setup();
};