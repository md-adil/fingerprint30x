#include<Biometric.h>
#include<SoftwareSerial.h>

SoftwareSerial fs(2,3) // rx, tx



Biometric biometric(&fs);

void onSearch(const char * err, Biometric::Finger finger) {
	
}

void setup() {
	biometric.init();

	biometric.onSearch(onSearch);
}

void loop() {
	biometric.loop();
}

