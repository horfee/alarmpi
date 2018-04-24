/*
 * TestRC.cpp
 *
 *  Created on: 29 juin 2017
 *      Author: horfee
 */

#include "RCReceiverTransmitter.h"
#include "Utils.h"
#include <syslog.h>
#include <thread>

class RCListener: public RCMessageListener {
public:
	virtual void onSignalReceived(int message) {
		logMessage(LOG_INFO, "Message received : %d", message);
	}
};
int main1(int argc, char* argv[]) {
	setlogmask (LOG_UPTO (LOG_DEBUG));
	openlog (argv[0], LOG_CONS | LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_LOCAL1);


	if ( wiringPiSetup() == -1 ) {
		logMessage(LOG_ERR, "Unable to initialize wiringpi");
		exit(1);
	}

	RCReceiverTransmitter rcModule(4, 5);
	rcModule.addListener(new RCListener());
	rcModule.start();
	std::this_thread::sleep_for(std::chrono::minutes(5));
	return 0;
}
