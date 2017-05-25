//============================================================================
// Name        : AlarmPI.cpp
// Author      : Horfee
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <algorithm>
#include "AlarmSystemDAO.h"
#include "AlarmSystemDAOSQLite.h"
#include "DAOFactory.h"
#include "httpserver/HTTPServer.h"
#include "httpserver/HTTPFileServlet.h"
#include "json/json.h"
#include "Property.h"
#include "Properties.h"
#include "servlets/AlarmPIModesServlet.h"
#include "servlets/AlarmPIPropertiesServlet.h"
#include "servlets/AlarmPIPhonesServlet.h"
#include "servlets/AlarmPIAssociationsServlet.h"
#include "servlets/AlarmPIDevicesServlet.h"
#include "servlets/AlarmPIActionsServlet.h"
#include "servlets/AlarmPIDetectedDevicesServlet.h"
#include "servlets/AlarmPISystemServlet.h"
#include "servlets/AlarmPISimulateSignalReceivedServlet.h"
#include "servlets/AlarmPIPingServlet.h"
#include <syslog.h>

#ifdef RPI
#ifdef WIRINGPI
	#include <wiringPi.h>
#else
	#include <pigpio.h>
#endif

#endif

#include <unistd.h>

using namespace std;
using namespace alarmpi;

//bool stopAsked = false;

#include <regex>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <errno.h>
#include <evhttp.h>


#include <vector>
#include <sstream>

#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif


bool stopAsked = false;
bool rebootAsked = false;


void daemonize(const char* pidfile) {

	/* Daemonize - make ourselves a subprocess. */
#ifdef HAVE_DAEMON
	if ( daemon( 1, 1 ) < 0 ) {
	    syslog( LOG_CRIT, "daemon - %m" );
	    exit( 1 );
	}
#else /* HAVE_DAEMON */
	switch (fork()) {
	case 0:
		break;
	case -1:
		syslog( LOG_CRIT, "fork - %m");
		exit(1);
	default:
		exit(0);
	}
#endif /* HAVE_DAEMON */

	setsid();
	if (pidfile != NULL) {
		/* Write the PID file. */
		FILE* pidfp = fopen(pidfile, "w");
		if (pidfp == NULL) {
			syslog( LOG_CRIT, "%.80s - %m", pidfile);
			exit(1);
		}
		fprintf(pidfp, "%d\n", (int) getpid());
		fclose(pidfp);
	}

	char cwd[2048];
	getcwd( cwd, sizeof(cwd) - 1 );
	if ( cwd[strlen( cwd ) - 1] != '/' )
	(void) strcat( cwd, "/" );
	if (chroot(cwd) < 0) {
		syslog( LOG_CRIT, "chroot - %m");
		perror("chroot");
		exit(1);
	}
	if (chdir(cwd) < 0) {
		syslog( LOG_CRIT, "chroot chdir - %m");
		perror("chroot chdir");
		exit(1);
	}

}

int main(int argc, char* argv[]) {

	struct sigaction sigIntHandler;
	alarmpi::AlarmSystemDAO* dao;
	alarmpi::AlarmSystem* alarmSystem;
	httpserver::HTTPServer* server;

	daemonize("/var/run/alarmPI.pid");
	dao = new alarmpi::AlarmSystemDAOSQLite("/var/alarmpi/alarmsystem.db");
	alarmpi::DAOFactory::getInstance()->setDAO(dao);

	sigIntHandler.sa_handler = [](int s){
		stopAsked = true;
	};
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);
	sigaction(SIGABRT, &sigIntHandler, NULL);
	sigaction(SIGKILL, &sigIntHandler, NULL);

#ifdef RPI
#ifdef WIRINGPI
	if ( wiringPiSetup() == -1 ) {
		throw std::logic_error("unable to initialize wiring API");
	}
#else
	if ( gpioInitialise() == PI_INIT_FAILED ) {
		throw std::logic_error("unable to initialize pigpio API");
	}
#endif
#endif

	alarmSystem = new alarmpi::AlarmSystem();
	std::string m = alarmSystem->activeMode();


	Property* listeningAddress = alarmSystem->getProperty(PROPERTY_WEBSERVER_ADDR);
	Property* listeningPort = alarmSystem->getProperty(PROPERTY_WEBSERVER_PORT);
	Property* webserverRoot = alarmSystem->getProperty(PROPERTY_WEBSERVER_ROOT);



	if ( listeningAddress == NULL || listeningPort == NULL || webserverRoot == NULL ) {
		alarmSystem->setConfigMode(DEFAULT_VALUE_PASSWORD);
		if ( listeningAddress == NULL ) {
			listeningAddress = new Property(PROPERTY_WEBSERVER_ADDR, PROPERTY_WEBSERVER_ADDR_DESCRIPTION, (std::string)DEFAULT_VALUE_WEBSERVER_ADDR);
			alarmSystem->addProperty(listeningAddress);
		}

		if ( listeningPort == NULL ) {
			listeningPort = new Property(PROPERTY_WEBSERVER_PORT, PROPERTY_WEBSERVER_PORT_DESCRIPTION, (int)DEFAULT_VALUE_WEBSERVER_PORT);
			alarmSystem->addProperty(listeningPort);
		}

		if ( webserverRoot == NULL ) {
			webserverRoot = new Property(PROPERTY_WEBSERVER_ROOT, PROPERTY_WEBSERVER_ROOT_DESCRIPTION, (std::string)DEFAULT_VALUE_WEBSERVER_ROOT);
			alarmSystem->addProperty(webserverRoot);
		}

		alarmSystem->activateMode(m, DEFAULT_VALUE_PASSWORD);
	}

	server = new HTTPServer(listeningAddress->getStringValue(), listeningPort->getIntValue());
	server->setAllowedMethods(EVHTTP_REQ_OPTIONS | EVHTTP_REQ_GET | EVHTTP_REQ_POST | EVHTTP_REQ_PUT | EVHTTP_REQ_DELETE);



	HTTPServlet* servlets[11];
	server->setDefaultServlet( 						servlets[0] = new HTTPFileServlet(webserverRoot->getStringValue()));
	server->addServlet("/rest/modes.*", 			servlets[1] = new AlarmPIModesServlet(alarmSystem));
	server->addServlet("/rest/devices.*", 			servlets[2] = new AlarmPIDevicesServlet(alarmSystem));
	server->addServlet("/rest/detecteddevices.*", 	servlets[3] = new AlarmPIDetectedDevicesServlet(alarmSystem));
	server->addServlet("/rest/properties.*", 		servlets[4] = new AlarmPIPropertiesServlet(alarmSystem));
	server->addServlet("/rest/actions.*", 			servlets[5] = new AlarmPIActionsServlet(alarmSystem));
	server->addServlet("/rest/phones.*", 			servlets[6] = new AlarmPIPhonesServlet(alarmSystem));
	server->addServlet("/rest/associations.*", 		servlets[7] = new AlarmPIAssociationsServlet(alarmSystem));
	server->addServlet("/services.*", 				servlets[8] = new AlarmPISystemServlet(alarmSystem, &stopAsked, &rebootAsked));
	server->addServlet("/ping.*", 					servlets[9] = new AlarmPIPingServlet(alarmSystem));
	server->addServlet("/rest/signal.*", 			servlets[10] = new AlarmPISimulateSignalReceivedServlet(alarmSystem));
	//server->addServlet("/services/.*", 				servlets[8] = new ShutdownServiceServlet());


	cout << "Starting Web Server on node " << listeningAddress->getStringValue() << ":" << listeningPort->getIntValue() << endl;
	server->start();

	std::cout << "System available on : " << std::endl;
	int portNumber = alarmSystem->getProperty(PROPERTY_WEBSERVER_PORT)->getIntValue();
	for(std::string s : alarmSystem->ipAddresses()) {
		std::cout << s << ":" << portNumber << std::endl;
	}

	while ( !stopAsked ) {
		usleep(500);
	}

	cout << "Exited from main loop" << endl;
	delete server;
	delete alarmSystem;
	alarmpi::DAOFactory::getInstance()->setDAO(NULL);
	delete dao;
	for(auto it : servlets) {
		delete it;
	}

#ifdef RPI
#ifndef WIRINGPI
	gpioTerminate();
#endif
#endif

	if ( rebootAsked ) {
		return system("sudo reboot now");
	}
	return 0;
}


