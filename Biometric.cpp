#ifndef ADIL_BIOMETRIC
#define ADIL_BIOMETRIC

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "Biometric.h"

Biometric::Biometric() {}

Biometric::Biometric(SoftwareSerial * _serial) {
	serial = _serial;
	driver = new Adafruit_Fingerprint(_serial);
}

void Biometric::init(uint32_t boudRate) {
	driver->begin(boudRate);
	if (driver->verifyPassword()) {
		connectClosure(NULL);
		isTakeImage = true;
	} else {
		connectClosure("Did not find fingerprint sensor");
	}
}

void Biometric::init() {
	init(57600);
}

void Biometric::loop() {

	if(isActionCompleted && isFingerTuched > 5) {
		isScanning = true;
	}

	if(!isTakeImage) {
		return;
	}

	if(!takeImage()) {
		return;
	}

	if(isScanning) {
		startSearch();
	}

	if(isEnrolling) {
		startEnroll();
	}
}

void Biometric::startSearch() {
	Action action = SCAN;
	isScanning = false;
	isActionCompleted = false;
	Finger f = {0, 0};
	if(!convertImage(action, 0)) {
		isScanning = true;
		return false;
	}
	switch (driver->fingerFastSearch()) {
		case FINGERPRINT_OK:
			// do something
			actionCompleted(action, NULL);
			break;
		case FINGERPRINT_PACKETRECIEVEERR:
			actionCompleted(action, "Finger print package receivce error");
		// do something
			break;
		case FINGERPRINT_NOTFOUND:
			if(searchClosure) {
				searchClosure(NULL, f);
				isActionCompleted = true;
			}
			// do something
			break;
		default:
			actionCompleted(action, "Something wrong while searching for fingers");
			break;
	       // do something
	}
}

void Biometric::startEnroll() {
	isEnrolling = false;
	Action a = ENROLL;

	if(!convertImage(a, currentSlot++)) {
		return false;
	}

	if(currentSlot >= 2) {
		// storeModel();
	}

	isEnrolling = true;
}

// void storeModel() {
// 	driver->createModel();
// 	currentSlot = 1;
// }


bool Biometric::takeImage() {
	unsigned long t = millis();
	if(t - startTime < _delay) {
		return 0;
	}
	startTime = t;
	Action a;
	if( isEnrolling ) {
		a = ENROLL;
	}
	if( isScanning ) {
		a = SCAN;
	}

	switch(driver->getImage()) {
		case FINGERPRINT_OK:
			isFingerTuched = 0;
			return true;

		case FINGERPRINT_NOFINGER:
			isFingerTuched++;
			break;

		case FINGERPRINT_PACKETRECIEVEERR:
			actionCompleted(a, "Communication Error");
			break;

		case FINGERPRINT_IMAGEFAIL:
			actionCompleted(a, "Imaging error");
		default:
			actionCompleted(a, "Something went wrong while getting finger image");
			break;
	}
	return false;
}

bool Biometric::convertImage(Action action, uint8_t slot) {
	switch(slot == 0 ? driver->image2Tz() : driver->image2Tz(slot)) {
		case FINGERPRINT_OK:
			// Image converted
	    	return true;
	    case FINGERPRINT_IMAGEMESS:
	    	// Image too messy
	    	actionCompleted(action, "Image too messy");
	    	break;
	    case FINGERPRINT_PACKETRECIEVEERR:
	    	// Communication error
	    	actionCompleted(action, "Communication error");
	    	break;
	    case FINGERPRINT_FEATUREFAIL:
	    	actionCompleted(action, "Could not find fingerprint features");
	    	break;
	    case FINGERPRINT_INVALIDIMAGE:
	    	// Could not find fingerprint features
	    	actionCompleted(action, "Could not find fingerprint features");
	    	break;
	    default:
	    	actionCompleted(action, "Something went wrong while converting image");
	    	break;
	}

	return false;
}

void Biometric::verifyPassword() {

}

void Biometric::changePassword() {

}

void Biometric::onFound(SearchClosure callback) {
	searchClosure = callback;
	isScanning = true;
}

void Biometric::enroll(uint32_t id, EnrollClosure callback) {
	enrollId = id;
	enrollClosure = callback;
}

uint8_t * Biometric::downloadTemplate(uint32_t id) {
	uint8_t p = driver->loadModel(id);
	switch (p) {
	    case FINGERPRINT_OK:
	    	// Template $id loaded
		    break;
	    case FINGERPRINT_PACKETRECIEVEERR:
	    	// Communication error
	    	return p;
	    default:
	    	// Unknown error
	    	return p;
	}

	switch (driver->getModel()) {
		case FINGERPRINT_OK:
			// Template $id transferring
			break;
		default:
			// Unknown Error p
			return p;
	}

	// one data packet is 267 bytes. in one data packet, 11 bytes are 'usesless' :D
	uint8_t bytesReceived[534]; // 2 data packets
	memset(bytesReceived, 0xff, 534);

	uint32_t starttime = millis();
	int i = 0;
	while (i < 534 && (millis() - starttime) < 20000) {
		if (serial->available()) {
		  bytesReceived[i++] = serial->read();
		}
	}
	// i bytes read.

	// Decoding packet...
	uint8_t fingerTemplate[512]; // the real template
	memset(fingerTemplate, 0xff, 512);
	  // filtering only the data packets
	int uindx = 9, index = 0;
	while (index < 534) {
	    while (index < uindx) ++index;
	    uindx += 256;
	    while (index < uindx) {
	        fingerTemplate[index++] = bytesReceived[index];
	    }
	    uindx += 2;
	    while (index < uindx) ++index;
	    uindx = index + 9;
	}
	return fingerTemplate;
}

void Biometric::uploadTemplate(uint8_t * tpl) {
	
}

// Enable or disable scanning

void Biometric::stopScan() {
	isScanning = false;
}

void Biometric::startScan() {
	isScanning = true;
}

void Biometric::actionCompleted(Action action, char * err) {
	Finger finger;
	switch(action) {
		case SCAN:
			finger = { driver->fingerID, driver->confidence };
			if(searchClosure) {
				searchClosure(err, finger);
			}
			if(isFingerTuched > 5) {
				isScanning = true;
			}
			break;
		case ENROLL:
			// enrollClosure(err);
			break;
	}
	isActionCompleted = true;
}

void Biometric::onConnect(ConnectClosure callback) {
	connectClosure = callback;
}

void Biometric::cancelEnroll() {
	isEnrolling = false;
}


#endif