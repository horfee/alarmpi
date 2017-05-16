/*
 * AlarmPIServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPIServlet.h"

namespace alarmpi {

AlarmPIServlet::AlarmPIServlet(AlarmSystem *alarmSystem):HTTPServlet(), system(alarmSystem) {

}

AlarmPIServlet::~AlarmPIServlet() {
	system = NULL;
}

} /* namespace alarmpi */
