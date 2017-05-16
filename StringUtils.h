/*
 * StringUtils.h
 *
 *  Created on: 6 avr. 2017
 *      Author: horfee
 */

#ifndef SOURCES_STRINGUTILS_H_
#define SOURCES_STRINGUTILS_H_
#include <string>
#include <vector>

std::string trim(const std::string &s);

bool stringContains(std::string text, std::string search) ;

std::vector<std::string> split(std::string text, std::initializer_list<std::string> separators );

#endif /* SOURCES_STRINGUTILS_H_ */
