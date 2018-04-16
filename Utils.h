/*
 * Utils.h
 *
 *  Created on: 6 avr. 2017
 *      Author: horfee
 */

#ifndef SOURCES_UTILS_H_
#define SOURCES_UTILS_H_
#include <string>
#include <vector>
#include <syslog.h>

std::string trim(const std::string &s);

bool stringContains(std::string text, std::string search) ;

std::vector<std::string> split(std::string text, std::initializer_list<std::string> separators );

template<typename... Args>
void logMsg2(int level, std::string file, int line, std::string message, Args...args) {

	syslog(level, message.c_str(), args...);
}


template<typename... Args>
void logMessage(int level, std::string message, Args...args) {
	syslog(level, message.c_str(), args...);
}

std::string urlDecode(std::string str);
#endif /* SOURCES_UTILS_H_ */
