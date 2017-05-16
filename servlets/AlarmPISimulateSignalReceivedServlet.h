/*
 * AlarmPISimulateSignalReceivedServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef ALARMPISIMULATESIGNALRECEIVEDSERVLET_H_
#define ALARMPISIMULATESIGNALRECEIVEDSERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPISimulateSignalReceivedServlet: public AlarmPIServlet {
public:
	AlarmPISimulateSignalReceivedServlet(AlarmSystem *system);
	virtual ~AlarmPISimulateSignalReceivedServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest& request, HTTPResponse& response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);
};

} /* namespace alarmpi */

#endif /* ALARMPISIMULATESIGNALRECEIVEDSERVLET_H_ */
