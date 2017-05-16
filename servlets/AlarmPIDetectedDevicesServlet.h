/*
 * AlarmPIDevicesServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef ALARMPIDETECTEDDEVICESSERVLET_H_
#define ALARMPIDETECTEDDEVICESSERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPIDetectedDevicesServlet: public AlarmPIServlet {
public:
	AlarmPIDetectedDevicesServlet(AlarmSystem *system);
	virtual ~AlarmPIDetectedDevicesServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest &request, HTTPResponse &response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);
};

} /* namespace alarmpi */

#endif /* ALARMPIDETECTEDDEVICESSERVLET_H_ */
