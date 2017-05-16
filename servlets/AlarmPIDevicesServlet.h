/*
 * AlarmPIDevicesServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef ALARMPIDEVICESSERVLET_H_
#define ALARMPIDEVICESSERVLET_H_

#include "AlarmPIServlet.h"


namespace alarmpi {

class AlarmPIDevicesServlet: public AlarmPIServlet {
public:
	AlarmPIDevicesServlet(AlarmSystem *system);
	virtual ~AlarmPIDevicesServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest &request, HTTPResponse &response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);
};

} /* namespace alarmpi */

#endif /* ALARMPIDEVICESSERVLET_H_ */
