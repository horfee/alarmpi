/*
 * AlarmPISystemServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef ALARMPIPINGSERVLET_H_
#define ALARMPIPINGSERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPIPingServlet: public AlarmPIServlet {
public:
	AlarmPIPingServlet(AlarmSystem *system);
	virtual ~AlarmPIPingServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest& request, HTTPResponse& response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);

};

} /* namespace alarmpi */

#endif /* ALARMPIPINGSERVLET_H_ */
