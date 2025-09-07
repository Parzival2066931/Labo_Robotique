#include "Sonar.hpp"
#include "SSD1306.hpp"

#define TRIGGER_PIN 9
#define ECHO_PIN 10

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET -1


Sonar mySonar(TRIGGER_PIN, ECHO_PIN);
ScreenSSD myScreen(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS, OLED_RESET);

unsigned long currentTime;

void setup() {
  Serial.begin(115200);
  myScreen.setup();
  
}

void loop() {
  mySonar.update();
  myScreen.update(mySonar.getDist());
  


}

