/*
 * AlarmPIPhonesServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef SERVLETS_ALARMPIPHONESSERVLET_H_
#define SERVLETS_ALARMPIPHONESSERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPIPhonesServlet: public AlarmPIServlet {
public:
	AlarmPIPhonesServlet(AlarmSystem *system);
	virtual ~AlarmPIPhonesServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest &request, HTTPResponse &response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);
};

} /* namespace alarmpi */

#endif /* SERVLETS_ALARMPIPHONESSERVLET_H_ */
