/*
 * RF433Module.h
 *
 *  Created on: 10 mai 2017
 *      Author: horfee
 */

#ifndef SOURCES_RF433MODULE_H_
#define SOURCES_RF433MODULE_H_

#include <vector>
#include "_433D.h"

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

	void cbf(_433D_rx_data_t r);

	int pi;
   _433D_rx_t *rx;
   _433D_tx_t *tx;
//		vector<int> messagesToSend;
};

} /* namespace alarmpi */

#endif /* SOURCES_RF433MODULE_H_ */
