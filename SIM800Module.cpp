/*
 * SIM900Module.cpp
 *
 *  Created on: 8 août 2015
 *      Author: horfee
 */

#include "SIM800Module.h"
#include <iostream>
#include <sstream>
#include <ostream>
#include <algorithm>
#include <chrono>
#include <time.h>
#include <regex>
#include <string>
#include <locale>
#include <iomanip>
#include "Utils.h"

//#include <wiringPi.h>
//#include <wiringSerial.h>

#if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
#else
#ifdef RPI
	#ifdef WIRINGPI
		#include <wiringPi.h>
	#else
		#include <pigpio.h>
	#endif
#endif
    #include <stdint.h>
    #define CHANGE 1
#ifdef __cplusplus
extern "C"{
#endif
typedef uint8_t boolean;
typedef uint8_t byte;

#if !defined(NULL)
#endif
#ifdef __cplusplus
}
#endif
#endif
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string>
#include <atomic>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <termios.h>

#define UARTFILENAME	"/dev/ttyS0"

#define CREG_NOT_REGISTERED_NOT_SEARCHING	0x1000
#define CREG_REGISTERED_HOME				0x1001
#define CREG_NOT_REGISTERED_SEARCHING		0x1010
#define CREG_REGISTRATION_DENIED			0x1011
#define CREG_UNKNOWN						0x1100
#define CREG_REGISTERED_ROAMING				0x1101

#define CREG_RC_DISABLE		0x1000000
#define CREG_RC_ENABLE		0x1010000
#define CREG_RC_ENABLE_LOC	0x1110000

#define LEN(array) (sizeof((array))/sizeof((array[0])))

namespace alarmpi {

void SIM800Module::sendCommand(std::string cmd, bool includeRF) {

	sendingMutex.lock();
	if ( simUARTFileStream != -1 ) {
		logMessage( LOG_DEBUG, "SIM800L - send] : %s", cmd.c_str());
		if ( includeRF ) {
			cmd.append("\r\n");
		}
		int count = write(simUARTFileStream, (void*)cmd.c_str(), cmd.length());
		if ( count != (int)cmd.length() ){
			logMessage( LOG_CRIT, "NB Sent : %d. Command lenghth : %d", count, cmd.length());
			throw SIM800TransmitException();
		}
	} else {
		logMessage( LOG_CRIT, "Unable to send %s", cmd.c_str());
	}
	sendingMutex.unlock();
	logMessage( LOG_DEBUG, "SIM800L - send] : done");
}

SIM800Command SIM800Module::readCommand(std::string awaitedCommand) {
	std::unique_lock<std::mutex> lock(mutex);
	condition.wait(lock, [&](){
		return readCommands.size() > 0 && readCommands[readCommands.size() - 1].find(awaitedCommand) != std::string::npos;}
	);

	std::string res = readCommands[readCommands.size() - 1];
	readCommands.pop_back();

	condition.notify_all();
	res.erase(res.find(awaitedCommand), awaitedCommand.size());
	int val = 0;
	if ( res.find("OK") != std::string::npos) {
		res.erase(res.find("OK"),2);
		val = 1;
	}
	if ( res.find("ERROR") != std::string::npos) {
		res.erase(res.find("ERROR"),5);
		val = -1;
	}

	res = trim(res);
	SIM800Command cmd;
	cmd.text = res;
	cmd.status = val;
	logMessage( LOG_DEBUG, "SIM800L - read] : command \"%s\" read", res.c_str());
	return cmd;
}

void SIM800Module::unattendCommandsCallBack() {
	std::vector<std::string> cmds;
	cmds.push_back("+CMTI:");
}


void SIM800Module::receivingThreadCallBack() {
	std::string buf;
	char rx_buffer[256];

	while(!stop) {
		if ( simUARTFileStream != -1 ) {
			logMessage( LOG_DEBUG, "Waiting for data");
			int rx_length = read(simUARTFileStream, (void*)rx_buffer, LEN(rx_buffer)-1);		//Filestream, buffer to store in, number of bytes to read (max)
			if (rx_length < 0) {
				logMessage( LOG_ERR, "An error occured : %d bytes read", rx_length);
			} else if (rx_length >= 0) {
				rx_buffer[rx_length] = '\0';
				buf.append(rx_buffer, rx_length);

				logMessage( LOG_DEBUG, "Data received (%d) : %s", rx_length, rx_buffer);
				std::string ringCommandStr("(\r\nRING\r\n)");
				if ( clipFlag ) {
					ringCommandStr.append("(\r\n\\+CLIP: )(\"[^\"]*\")(,)([0-9]+)((,)(\"[^\"]*\")(,)([0-9]+)(,)(\"[^\"]*\")(,)([0-9]+))?\r\n") ;
				}
				std::string incommingMessageRegexString("\r\n\\+CMTI: \".*\",[0-9]+\r\n");
				std::regex incommingMessageRegex{incommingMessageRegexString};
				std::regex ringCommand{ringCommandStr};

				std::smatch m;
				if (  std::regex_search(buf, m, ringCommand) ) {
						std::string callingNumber("");
						if ( clipFlag ) {
							callingNumber = m[3];
							callingNumber = callingNumber.substr(1, callingNumber.size() - 2);
						}

					logMessage( LOG_DEBUG, "SIM800L - receive] : incomming call from %s", callingNumber.c_str());
					for(auto l : this->listeners) {
						std::thread([&]{
							l->onIncomingCall(callingNumber);
						}).detach();
					}

					std::string::size_type i = buf.find(m[0].str());
					buf = buf.erase(i, i + m[0].str().size());
				} else if ( std::regex_match(buf, incommingMessageRegex) ) {
					int msgId = atoi(buf.c_str() + buf.find_last_of(",") + 1);
					logMessage( LOG_DEBUG, "SIM800L - receive] : incomming message, id :%d", msgId);
					for(auto l : this->listeners) {
						std::thread([&]{
							l->onMessageReceived(msgId);
						}).detach();
					}

					buf = std::regex_replace(buf, incommingMessageRegex, "$2");
				} else if ( buf.find("\r\nOK\r\n") != std::string::npos	) {
					std::unique_lock<std::mutex> lock(mutex);
					readCommands.push_back(buf);
					logMessage( LOG_DEBUG, "SIM800L - receive] : %s", buf.c_str());
					buf = "";
					condition.notify_all();

				} else if ( buf.find("\r\nERROR\r\n") != std::string::npos	) {
					std::unique_lock<std::mutex> lock(mutex);
					readCommands.push_back(buf);
					logMessage( LOG_DEBUG, "SIM800L - receive] : %s", buf.c_str());
					buf = "";
					condition.notify_all();
				}
			}
		} else {
			logMessage (LOG_ERR, "Unable to read data");
			stop = true;
		}
	}
}


bool SIM800Module::getClipStatus() {
	sendCommand("AT+CLIP?");
	SIM800Command res = readCommand("AT+CLIP?");
	res.text.erase(res.text.begin(), res.text.begin() + 7);
	int f = atoi(res.text.c_str());

	return f == 1;
}

void SIM800Module::toggleDisplayCalleeNumber(){
	bool f = !getClipStatus();
	sendCommand("AT+CLIP=" + std::to_string(f));
	readCommand("AT+CLIP=" + std::to_string(f));


}	//AT+CLIP=0|1

SIM800Module::SIM800Module(int resetPIN, std::string uartStream, int baudrate) {

#ifdef RPI
	resetPin = resetPIN;
#ifdef WIRINGPI
	pinMode(resetPin, OUTPUT);
	digitalWrite(resetPin, HIGH);
#else
	gpioSetMode(resetPin, PI_OUTPUT);
	gpioWrite(resetPin, PI_HIGH);
#endif
#endif

	simUARTFileStream = open(uartStream.c_str(), O_RDWR | O_NOCTTY );
	if ( simUARTFileStream == -1 ) {
		logMessage( LOG_ERR, "Unable to open the SIM800 uart stream on %s\n%s", uartStream.c_str(), strerror(errno));
		throw SIM800UARTException();
	}

	struct termios options;
	tcgetattr(simUARTFileStream, &options);
	cfsetspeed(&options, baudrate);

	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &=~PARENB;
	options.c_cflag &= ~CSTOPB;

	options.c_cflag |= CLOCAL;
	options.c_cflag |= CREAD;

//	options.c_cflag = baudrate | CS8 | CLOCAL | CREAD ;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 5;

	cfmakeraw(&options);
	tcsetattr(simUARTFileStream, TCSANOW, &options);

	//tcflush(simUARTFileStream, TCIFLUSH);
	//tcsetattr(simUARTFileStream, TCSANOW, &options);

	callingPhone = false;

	stop = false;
	deamon = new std::thread(&SIM800Module::receivingThreadCallBack, this);
	unattendedCommands = new std::thread(&SIM800Module::unattendCommandsCallBack, this);

	int wait = 0;
	bool r;
	do {
		logMessage( LOG_DEBUG, "Checking if module is ready");
		r = !isReady();
		if ( r ) {
			logMessage( LOG_DEBUG, "Module not ready. Waiting 10ms");
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	} while(r && wait++ < 500) ;
	logMessage( LOG_DEBUG, "We stopped waiting");

	clipFlag = getClipStatus();

	sendCommand("AT+CMGF=1");
	SIM800Command cmd = readCommand("AT+CMGF=1");
	if ( cmd.status != 1 ) {
		logMessage(LOG_CRIT, "Unable to enable CMGF=1");
		throw SIM800Exception();
	}


	logMessage( LOG_DEBUG, "SIM 800 module started");
}

SIM800Module::~SIM800Module() {
	stop = true;

	if ( simUARTFileStream != -1 ) {
		fcntl(simUARTFileStream, F_SETFL, O_NONBLOCK);
		int res = close(simUARTFileStream);
		if ( res == -1 ) {
			logMessage (LOG_ERR, "Unable to close uart : %d\n%s", errno, strerror(errno));
		}
	}

	deamon->join();
	unattendedCommands->join();
	delete deamon;
}

void SIM800Module::reset() {
#ifdef RPI
#ifdef WIRINGPI
	digitalWrite(this->resetPin, LOW);
	delay(500);
	digitalWrite(resetPin, HIGH);
#else
	gpioWrite(resetPin, PI_LOW);
	gpioDelay(5000);
	gpioWrite(resetPin, PI_HIGH);
#endif
#endif
}

//void SIM800Module::sleep() {
//
//}
//
//void SIM800Module::wakeUp() {
//
//}

void SIM800Module::addListener(SIM800ModuleListener* listener) {
	if ( listener ) listeners.push_back(listener);
}

void SIM800Module::removeListener(SIM800ModuleListener* listener) {
	if ( listener ) {
		listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
	}
}

bool SIM800Module::isCallingPhone() {
	return callingPhone;
}

bool SIM800Module::callPhone(std::string numberPhone) {
	// ATD<numberphone>;
	sendCommand("ATD"+numberPhone + ";");
	SIM800Command cmd = readCommand("ATD");
	callingPhone = cmd.text.empty();
	return callingPhone;
}

void SIM800Module::hangUp() {
	// ATH
//	if ( callingPhone ) {
	sendCommand("ATH");
	SIM800Command res = readCommand("ATH");
//	}
}

std::string SIM800Module::getIMEI() {
	sendCommand("AT+GSN");
	SIM800Command res = readCommand("AT+GSN");
	return res.text;
}

int SIM800Module::sendMessage(std::string numberPhone, std::string message) {
	SIM800Command res;

//	sendingMessageMutex.lock();
	sendCommand("AT+CSMP?");
	res = readCommand("AT+CSMP?");
	std::string t = res.text.substr(res.text.find(": ") + 2);
	std::vector<std::string>vals = split(t,{","});
	sendCommand("AT+CSMP=" + vals[0] + "," + vals[1] + "," + vals[2] + ",0");
	res = readCommand("AT+CSMP=" + vals[0] + "," + vals[1] + "," + vals[2] + ",0");

	sendCommand("AT+CMGS=\"" + numberPhone + "\"");// + message + char(26), false);
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	sendCommand(message + char(26),false);
	res = readCommand("AT+CMGS=\"" + numberPhone + "\"");
	if ( res.text.find("ERROR") != std::string::npos ) {
		return -1;
	}
	std::string::size_type indx = res.text.find("+CMGS: ") + 7;

	sendCommand("AT+CSMP=" + vals[0] + "," + vals[1] + "," + vals[2] + "," + vals[3]);
	readCommand("AT+CSMP=" + vals[0] + "," + vals[1] + "," + vals[2] + "," + vals[3]);

	return atoi(res.text.c_str() + indx);
}


std::wstring s2ws(const std::string& s) {
    std::string curLocale = setlocale(LC_ALL, "");
    const char* _Source = s.c_str();
    size_t _Dsize = mbstowcs(NULL, _Source, 0) + 1;
    wchar_t *_Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    mbstowcs(_Dest,_Source,_Dsize);
    std::wstring result = _Dest;
    delete []_Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}


std::string unicode(std::string text) {
	std::wstring res = s2ws(text);

	std::stringstream stream;
	for(auto it : res) {
		int v1,v2;
		v1 = (0xff00 & ((unsigned char)it)) >> 8;
		v2 = 0x00ff & ((unsigned char)it);

		stream << std::setfill ('0') << std::setw(sizeof(char)*2) << std::hex << v1 << std::hex << v2;
	}
	return stream.str();
}

int SIM800Module::sendUnicodeMessage(std::string numberPhone, std::string message) {
	SIM800Command res;

	sendCommand("AT+CSMP?");
	res = readCommand("AT+CSMP?");
	std::string t = res.text.substr(res.text.find(": ") + 2);
	std::vector<std::string>vals = split(t,{","});
	if ( vals[3] != "8" ) {
		sendCommand("AT+CSMP=" + vals[0] + "," + vals[1] + "," + vals[2] + ",8");
		res = readCommand("AT+CSMP=" + vals[0] + "," + vals[1] + "," + vals[2] + ",8");
	}

	sendCommand("AT+CSCS?");
	res = readCommand("AT+CSCS?");
	t = res.text.substr(res.text.find(": ") + 2);
	if ( t !="\"UCS2\"") {
		sendCommand("AT+CSCS=\"UCS2\"");
		res = readCommand("AT+CSCS=\"UCS2\"");
	}

	std::string unicodeNumberPhone = unicode(numberPhone);
	std::string unicodeMessage = unicode(message);

	logMessage(LOG_DEBUG, "Number  : %s", unicodeNumberPhone.c_str());
	logMessage(LOG_DEBUG, "Message : %s", unicodeMessage.c_str());
	sendCommand("AT+CMGS=\"" + unicodeNumberPhone + "\"");// + message + char(26), false);
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	sendCommand(unicodeMessage + char(26),false);
	res = readCommand("AT+CMGS=\"" + unicodeNumberPhone + "\"");

	std::string::size_type indx = res.text.find("+CMGS: ") + 7;

	if ( vals[3] != "8" ) {
		sendCommand("AT+CSMP=" + vals[0] + "," + vals[1] + "," + vals[2] + "," + vals[3]);
		readCommand("AT+CSMP=" + vals[0] + "," + vals[1] + "," + vals[2] + "," + vals[3]);
	}

	if ( t !="\"UCS2\"") {
		sendCommand("AT+CSCS=" + t);
		readCommand("AT+CSCS=" + t);
	}

	return atoi(res.text.c_str() + indx);
}

void SIM800Module::answerIncommingCall() {

	sendCommand("ATA");
	SIM800Command res(readCommand("ATA"));

	callingPhone = res.text.empty();
}

void SIM800Module::toggleBusy() {

	sendCommand("AT+GSMBUSY?");
	SIM800Command res = readCommand("AT+GSMBUSY?");
	//+GSMBUSY:
	int r = atoi(res.text.c_str() + 10);
	std::string cmd;
	if ( r == 0 ) {
		cmd = "AT+GSMBUSY=1";
	} else if ( r == 1) {
		cmd = "AT+GSMBUSY=2";
	} else {
		cmd = "AT+GSMBUSY=0";
	}
	sendCommand(cmd);
	readCommand(cmd);
}

SMS createSmsFromString(std::string headerLine, std::string message) {
	SMS s;
	s.text = message;

//		std::stringstream ss(headerLine);
//		std::string tmp;
	int msgId, year, month, day, hours, minutes, seconds;
//	char sender[50];
//	char status[50];
//	char unused[50];

	std::string sender, status, date;
	msgId = atoi(headerLine.c_str() + 7);

	std::string::size_type i, i2;
	i = headerLine.find('"');
	i2 = headerLine.find('"', i + 1);
	status = headerLine.substr(i + 1, i2 - i - 1);

	i = headerLine.find('"', i2 + 1);
	i2 = headerLine.find('"', i + 1);

	sender = headerLine.substr(i + 1, i2 - i - 1);

	i = headerLine.find_last_of('"');
	i2 = i - 1;
	while ( i2 > 0 && headerLine[i2] != '"') i2--;


	date = headerLine.substr(i2 + 1, i2 - i - 1);

//	strptime(date.c_str(), "%Y-%m-%d:%H:%M", &time);
	year = 2000 + atoi(date.c_str()) - 1900;
	date = date.substr(date.find("/") + 1);
	month = atoi(date.c_str());
	date = date.substr(date.find("/") + 1);
	day = atoi(date.c_str());
	date = date.substr(date.find(",") + 1);
	hours = atoi(date.c_str());
	date = date.substr(date.find(":") + 1);
	minutes = atoi(date.c_str());
	date = date.substr(date.find(":") + 1);
	seconds = atoi(date.c_str());
	date = date.substr(date.find_first_of("+-"));
	timezone = atoi(date.c_str());

//	sscanf(headerLine.c_str(), "+CMGL: %i,\"%s\",\"%s\",\"%s\",\"%i/%i/%i,%i:%i:%i+%i\"", &msgId, status, sender, unused, &year, &month, &day, &hours, &minutes, &seconds, &timezone);
	s.phoneNumber = std::string(sender);
	s.msgId = msgId;
	if ( strncmp(status.c_str(), "REC UNREAD", 10) == 0 ) {
		s.status = UNREAD;
	} else if ( strncmp(status.c_str(), "REC READ", 8) == 0 ) {
		s.status = READ;
	}

	struct tm time;
	time.tm_year = year;
	time.tm_mon = month - 1;
	time.tm_mday = day;
	time.tm_hour = hours;
	time.tm_min = minutes;
	time.tm_sec = seconds;
	s.timestamp = mktime(&time);
	return s;
}

void SIM800Module::deleteSMS(int msgId){
	std::string command("AT+CMGD=" + std::to_string(msgId));
	sendCommand(command);
	SIM800Command res = readCommand(command);
}

std::vector<SMS> SIM800Module::listSMS(SMSStatus status) {
	std::string cmd;
	if ( status == READ ) {
		cmd = "AT+CMGL=\"REC READ\"";
	} else if ( status == UNREAD ) {
		cmd = "AT+CMGL=\"REC UNREAD\"";
	} else if ( status == ALL ){
		cmd = "AT+CMGL=\"ALL\"";
	} else if ( status == SENT ) {
		cmd = "AT+CMGL=\"STO SENT\"";
	} else if ( status == UNSENT ) {
		cmd = "AT+CMGL=\"STO UNSENT\"";
	} else {
		return std::vector<SMS>();
	}

	sendCommand(cmd);
	SIM800Command res = readCommand(cmd);
	std::vector<std::string> lines = split(res.text,{"\r","\n"});
	std::vector<SMS> smss;


	for(unsigned int i = 0; i < lines.size(); i+=2) {
		smss.push_back(createSmsFromString(lines[i], lines[i+1]));

	}
	return smss;
}

SMS SIM800Module::getSms(int msgId) {
	std::string cmd = "AT+CMGR=" + std::to_string(msgId);
	sendCommand(cmd);
	SIM800Command res = readCommand(cmd);
	if ( res.status == 1 ) {
		std::vector<std::string> lines = split(trim(res.text),{"\r","\n"});
		if ( lines.size() < 2 ) throw SIM800Exception();
		return createSmsFromString(lines[0], lines[1]);
	}
	throw SIM800Exception();

}

void SIM800Module::deleteSMS(SMSStatus status){
	std::string cmd;
	if ( status == READ ) {
		cmd = "AT+CMGDA=\"DEL READ\"";
	} else if ( status == UNREAD ) {
		cmd = "AT+CMGDA=\"DEL UNREAD\"";
	} else if ( status == ALL ){
		cmd = "AT+CMGDA=\"DEL ALL\"";
	} else if ( status == SENT ) {
		cmd = "AT+CMGDA=\"DEL SENT\"";
	} else if ( status == UNSENT ) {
		cmd = "AT+CMGDA=\"DEL UNSENT\"";
	} else if ( status == INBOX ) {
		cmd = "AT+CMGDA=\"DEL INBOX\"";
	} else {
		return;
	}

	sendCommand(cmd);
	SIM800Command res = readCommand(cmd);


} // AT+CMGDA="DEL <TYPE>"

std::string SIM800Module::getModuleVersion() {
	sendCommand("ATI");
	SIM800Command res(readCommand("ATI"));
	std::vector<std::string> lines = split(res.text, {"\r","\n"});
	return lines[0];
}

bool SIM800Module::isReady() {
	sendCommand("AT");
	SIM800Command res = readCommand("AT");
	return res.status == 1;
}

int SIM800Module::getNetworkStatus() {
	sendCommand("AT+CREG?");
	SIM800Command res(readCommand("AT+CREG?"));
	std::vector<std::string> lines = split(res.text, {"\r","\n"});
	lines = split(split(lines[0],{":"})[1],{","});
	int p1 = 0;
	if ( lines[0] == "0") p1 = CREG_RC_DISABLE;
	else if ( lines[0] == "1" ) p1 = CREG_RC_ENABLE;
	else if ( lines[0] == "2" ) p1 = CREG_RC_ENABLE_LOC;
	if ( lines[1] == "0") p1 |= CREG_NOT_REGISTERED_NOT_SEARCHING;
	else if ( lines[1] == "1" ) p1 |= CREG_REGISTERED_HOME;
	else if ( lines[1] == "2" ) p1 |= CREG_NOT_REGISTERED_SEARCHING;
	else if ( lines[1] == "3" ) p1 |= CREG_REGISTRATION_DENIED;
	else if ( lines[1] == "4" ) p1 |= CREG_UNKNOWN;
	else if ( lines[1] == "5" ) p1 |= CREG_REGISTERED_ROAMING;

	return p1;
}


std::vector<std::string> SIM800Module::listOperators() {
	sendCommand("AT+COPN");
	SIM800Command res(readCommand("AT+COPN"));
	std::vector<std::string> operators = split(res.text, {"\r","\n"});
	std::vector<std::string> results;
	for(std::string op : operators) {
		std::string l(split(op, {","})[1]);
		results.push_back(l.substr(1, l.size() - 3));
	}

	return results;
}

std::string SIM800Module::getOperator() {
	sendCommand("AT+COPS?");
	SIM800Command res(readCommand("AT+COPS?"));
	std::vector<std::string> lines = split(res.text, {"\r","\n"});
	return lines[0].substr(lines[0].find_last_of(",") + 2, lines[0].find_last_of("\"") - (lines[0].find_last_of(",") + 2));
}

bool SIM800Module::needPinCode() {
	sendCommand("AT+CPIN?");
	SIM800Command res(readCommand("AT+CPIN?"));
	std::vector<std::string> lines = split(res.text, {"\r","\n"});
	return !stringContains(lines[0], "READY");

}

const int rssiValues[] = {
		-115, -111, -110, -108, -106,-104,-102,-100,
		-98,-96,-94,-92,-90,-88,-86,-84,-82,-80,-78,-76,
		-74,-72,-70,-68,-66,-64,-62,-60,-58,-56,-54,-52};

SignalQuality SIM800Module::getSignalQuality() {
	SignalQuality res{0,0};
	sendCommand("AT+CSQ");
	SIM800Command lgs(readCommand("AT+CSQ"));
	std::vector<std::string> lines = split(lgs.text, {"\r","\n"});

	lines = split(split(lines[0],{":"})[1],{","});
	res.rssi = atoi(lines[0].c_str());
	res.bitErrorRate = atoi(lines[1].c_str()) / 100.0f;

	res.rssi = rssiValues[res.rssi];

	return res;
}

void SIM800Module::setPinCode(std::string pinCode) {
	sendCommand("AT+CPIN="+pinCode);
	SIM800Command res = readCommand("AT+CPIN");
	std::vector<std::string> lines = split(res.text, {"\r","\n"});
}

} /* namespace alarmpi */


