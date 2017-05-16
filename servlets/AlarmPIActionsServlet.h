/*
 * AlarmPIActionsServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef ALARMPIACTIONSSERVLET_H_
#define ALARMPIACTIONSSERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPIActionsServlet: public AlarmPIServlet {
public:
	AlarmPIActionsServlet(AlarmSystem *system);
	virtual ~AlarmPIActionsServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest& request, HTTPResponse& response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);
};

} /* namespace alarmpi */

#endif /* ALARMPIACTIONSSERVLET_H_ */
