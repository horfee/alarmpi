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
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include "AlarmSystemDAO.h"
#include "AlarmSystemDAOSQLite.h"
#include "DAOFactory.h"
#include "httpserver/HTTPServer.h"
#include "httpserver/HTTPFileServlet.h"
#include "json/json.h"
#include "Property.h"
#include "Properties.h"
#include <iomanip>
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
#include "servlets/AlarmPISimulateMessageServlet.h"
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
#include "Utils.h"

//#include <codecvt>
#include <cstdlib>
#include <string>

#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif


bool stopAsked = false;
bool rebootAsked = false;


void daemonize(const char* pidfile) {
	logMessage(LOG_DEBUG, "Daemonizing");
	/* Daemonize - make ourselves a subprocess. */
#ifdef HAVE_DAEMON
	if ( daemon( 1, 1 ) < 0 ) {
	    logMessage( LOG_CRIT, "daemon - %m" );
	    exit( EXIT_FAILURE );
	}
#else /* HAVE_DAEMON */
	switch (fork()) {
	case 0:
		break;
	case -1:
		logMessage( LOG_CRIT, "fork - %m");
		exit(EXIT_FAILURE);
	default:
		exit(EXIT_SUCCESS);
	}
#endif /* HAVE_DAEMON */

	int sid = setsid();
	if (sid < 0) {
		logMessage( LOG_CRIT, "Error on setsid %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

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

}


bool mustDaemonize = false;
int logLevel = LOG_NOTICE;
void parseArgs(int argc, char *argv[]) {
	for(int i = 1; i < argc; i++) {
		if ( std::string(argv[i]) == "-d" || std::string(argv[i]) == "-D") {
			mustDaemonize = true;
		} else if ( argv[i][0] == '-' && (argv[i][1] == 'v' || argv[i][1] == 'V') ) {
			logLevel = atoi(argv[i]+2);
		}
	}
}


int main(int argc, char* argv[]) {

	struct sigaction sigIntHandler;
	alarmpi::AlarmSystemDAO* dao;
	alarmpi::AlarmSystem* alarmSystem;
	httpserver::HTTPServer* server;

	parseArgs(argc, argv);

	setlogmask (LOG_UPTO (logLevel));
	openlog (argv[0], LOG_CONS | LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	sigIntHandler.sa_handler = [](int s){
		stopAsked = true;
	};
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGTERM, &sigIntHandler, NULL);
	sigaction(SIGKILL, &sigIntHandler, NULL);
	sigaction(SIGABRT, &sigIntHandler, NULL);
	sigaction(SIGINT, &sigIntHandler, NULL);
	sigaction(SIGCONT, &sigIntHandler, NULL);
	sigaction(SIGSEGV, &sigIntHandler, NULL);

	if ( mustDaemonize ) {
		daemonize("/var/run/alarmPI.pid");
	}

	umask(0);

	char cwd[2048] = "/etc/alarmpi/";
	if (chdir(cwd) < 0) {
		logMessage( LOG_CRIT, "chdir - %m %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	dao = new alarmpi::AlarmSystemDAOSQLite("alarmsystem.db");
	alarmpi::DAOFactory::getInstance()->setDAO(dao);

#ifdef RPI
#ifdef WIRINGPI
	if ( wiringPiSetup() == -1 ) {
		logMessage( LOG_CRIT, "unable to initialize wiring API");
	} else {
		logMessage( LOG_NOTICE, "WIRINGPI initialized successfully");
	}
#else
	int nbTry = 10;
	int res;
	while ( nbTry-- > 0 && (res = gpioInitialise()) == PI_INIT_FAILED ) {
		gpioTerminate();
		logMessage( LOG_CRIT, "unable to initialize pigpio API");
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	}
	if ( res == PI_INIT_FAILED ) {
		return 1;
	} else {
		logMessage( LOG_NOTICE, "PIGPIO initialized successfully");
	}
#endif
#endif

	alarmSystem = new alarmpi::AlarmSystem();
	std::string m = alarmSystem->activeMode();

	Property* accessPointPassword = alarmSystem->getProperty(PROPERTY_ACCESS_POINT_PASS);

	if ( accessPointPassword == NULL ) {
		accessPointPassword = new Property(PROPERTY_ACCESS_POINT_PASS, PROPERTY_ACCESS_POINT_PASS_DESCRIPTION, (std::string)DEFAULT_VALUE_ACCESS_POINT_PASS);
		alarmSystem->addProperty(accessPointPassword);
	}

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



	HTTPServlet* servlets[12];
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
	server->addServlet("/rest/message.*", 			servlets[11] = new AlarmPISimulateMessageServlet(alarmSystem));
	//server->addServlet("/services/.*", 				servlets[8] = new ShutdownServiceServlet());


	logMessage( LOG_INFO, "Starting Web Server on node %s:%d", listeningAddress->getStringValue().c_str(), listeningPort->getIntValue());
	server->start();


	class InnerNetworkListener : public NetworkListener {
	public:
		void onConnectionStateChanged(bool connected) {
			server->stop();
			server->start();
		}
		void onClientConnected(bool created) {

		}

		InnerNetworkListener(httpserver::HTTPServer *server) {
			this->server = server;
		}
		virtual ~InnerNetworkListener() {

		}
	private:
		httpserver::HTTPServer *server;

	};

	alarmSystem->addNetworkListener(new InnerNetworkListener(server));
	std::string sysAvailableOn("System available on : \n");
	int portNumber = alarmSystem->getProperty(PROPERTY_WEBSERVER_PORT)->getIntValue();
	for(std::string s : alarmSystem->ipAddresses()) {
		sysAvailableOn += s + ":" + std::to_string(portNumber) + "\n";
	}
	logMessage( LOG_INFO, sysAvailableOn);

	while ( !stopAsked ) {
		usleep(500);
	}

	logMessage( LOG_NOTICE, "Exit from main loop");
	delete server;
	for(auto it : servlets) {
		delete it;
	}
	logMessage( LOG_NOTICE, "Web server deleted");
	delete alarmSystem;
	logMessage( LOG_NOTICE, "Alarm system deleted");
	alarmpi::DAOFactory::getInstance()->setDAO(NULL);
	delete dao;
	logMessage( LOG_NOTICE, "DAO deleted");

#ifdef RPI
#ifndef WIRINGPI
	gpioTerminate();
#endif
#endif

	closelog ();

	if ( rebootAsked ) {
		return system("reboot now");
	}
	return 0;
}
