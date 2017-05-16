/*
 * AlarmPIPropertiesServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef SERVLETS_ALARMPIPROPERTIESSERVLET_H_
#define SERVLETS_ALARMPIPROPERTIESSERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPIPropertiesServlet: public AlarmPIServlet {
public:
	AlarmPIPropertiesServlet(AlarmSystem *system);
	virtual ~AlarmPIPropertiesServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest &request, HTTPResponse &response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);
};

} /* namespace alarmpi */

#endif /* SERVLETS_ALARMPIPROPERTIESSERVLET_H_ */
