#include<Biometric.h>
#include<SoftwareSerial.h>

SoftwareSerial fs(2,3); // rx, tx



Biometric biometric(&fs);

void onSearch(const char * err, Biometric::Finger finger) {
  Serial.println(finger.id);
}

void onConnect(char * err) {
  if(err) {
    Serial.println(err);
  } else {
    Serial.println("Connected");
  }
}


void setup() {
  Serial.begin(9600);
  biometric.onConnect(onConnect);
  biometric.init();

  biometric.onFound(onSearch);

  biometric.startScan();
}

void loop() {
  biometric.loop();
}

