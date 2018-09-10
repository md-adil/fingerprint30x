#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
class Biometric
{

public:
	struct Finger
	{
		uint16_t id;
		uint16_t confidence;
	};

	typedef void (*SearchClosure) ( char *, Finger );
	typedef void (*EnrollClosure) ( char * );
	typedef void (*DownloadTemplateClosure) ( char *, uint8_t * );
	typedef void (*UploadTemplateClosure) ( char *, uint8_t * );
	typedef void (*ConnectClosure) ( char * );

	Biometric();
	Biometric(SoftwareSerial *);
	void init();
	void init(uint32_t);
	void loop();
	void onFound(SearchClosure);
	void enroll(uint32_t, EnrollClosure);
	void cancelEnroll();
	void changePassword();
	void verifyPassword();
	void uploadTemplate();
	uint8_t * downloadTemplate(uint32_t);
	void onConnect(ConnectClosure);
	void startScan();
	void stopScan();
private:
	enum Action {
		ENROLL, SCAN
	};

	Adafruit_Fingerprint * driver;
	SoftwareSerial * serial;
	bool isScanning = false;
	bool isEnrolling = false;
	bool isFingerTuched = false;
	bool isTakeImage = true;
	bool isActionCompleted = false;

	bool takeImage();
	bool convertImage(Action, uint8_t slot);
	void startSearch();
	void startEnroll();
	void actionCompleted(Action, char * err);
	uint32_t enrollId;
	uint8_t currentSlot = 1;
	SearchClosure searchClosure;
	EnrollClosure enrollClosure;
	ConnectClosure connectClosure;
	// Timer section;
	void storeModel();
	unsigned long startTime = 0;
	const unsigned long _delay = 300;
};
