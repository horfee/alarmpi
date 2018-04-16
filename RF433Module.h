/*
 * RF433Module.h
 *
 *  Created on: 10 mai 2017
 *      Author: horfee
 */

#ifndef SOURCES_RF433MODULE_H_
#define SOURCES_RF433MODULE_H_

#include <vector>
#include <chrono>
#include <map>
#include <atomic>
#ifndef WIRINGPI

#ifdef RPI
#include "_433D.h"
#endif



namespace alarmpi {

class RF433MessageListener {

public:
	virtual void onSignalReceived(int message) = 0;
	virtual ~RF433MessageListener() {} ;
};

class RF433Module {
public:
	RF433Module(int receivingPIN, int transmittingPIN);
	virtual ~RF433Module();

	int getReceivingPIN();
	int getTransmittingPIN();

	void setReceivingPIN(int pin);
	void setTransmittingPIN(int pin);

	void start();
	void stop();

	bool isStarted() const;

	void addListener(RF433MessageListener* listener);
	void removeListener(RF433MessageListener* listener);

	void sendMessage(unsigned long long int toSend);

private:
	int receivingPIN;
	int transmittingPIN;

	bool started;

	std::vector<RF433MessageListener*> listeners;

	std::atomic<bool> sending;

	int sendRepeat = 1;


#ifdef RPI
	int pi;

	void cbf(_433D_rx_data_t r);
   _433D_rx_t *rx;
   _433D_tx_t *tx;

   std::map<int, std::chrono::milliseconds> receivedTimes;
#endif
//		vector<int> messagesToSend;
};

} /* namespace alarmpi */

#endif
#endif /* SOURCES_RF433MODULE_H_ */
