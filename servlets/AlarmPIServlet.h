/*
 * AlarmPIServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef ALARMPISERVLET_H_
#define ALARMPISERVLET_H_

#include "../httpserver/HTTPServlet.h"
#include "../AlarmSystem.h"

using namespace httpserver;

namespace alarmpi {

class AlarmPIServlet: public HTTPServlet {
public:
	AlarmPIServlet(AlarmSystem *system);
	virtual ~AlarmPIServlet();
protected:
	AlarmSystem* system;
};

} /* namespace alarmpi */

#endif /* ALARMPISERVLET_H_ */
