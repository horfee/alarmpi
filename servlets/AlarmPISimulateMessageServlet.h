/*
 * AlarmPISimulateMessageServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef ALARMPISIMULATEMESSAGESERVLET_H_
#define ALARMPISIMULATEMESSAGESERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPISimulateMessageServlet: public AlarmPIServlet {
public:
	AlarmPISimulateMessageServlet(AlarmSystem *system);
	virtual ~AlarmPISimulateMessageServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest& request, HTTPResponse& response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);
};

} /* namespace alarmpi */

#endif /* ALARMPISIMULATEMESSAGESERVLET_H_ */
