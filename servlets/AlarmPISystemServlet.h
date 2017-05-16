/*
 * AlarmPISystemServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef ALARMPISYSTEMSERVLET_H_
#define ALARMPISYSTEMSERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPISystemServlet: public AlarmPIServlet {
public:
	AlarmPISystemServlet(AlarmSystem *system, bool* stopAsked, bool* rebootAsked);
	virtual ~AlarmPISystemServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest& request, HTTPResponse& response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);

private:
	bool* stopAsked;
	bool* rebootAsked;
};

} /* namespace alarmpi */

#endif /* ALARMPISYSTEMSERVLET_H_ */
