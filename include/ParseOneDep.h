
// Nothing at the moment

/*!
** \file ParseOneDep.h
**
** \brief Header file for functions to help parse OneDep filenames.
*/

#ifndef PARSE_ONE_DEP_H
#define PARSE_ONE_DEP_H

#include <string>
#include <vector>

// Parses a OneDep filename and returns a vector of strings
// DepId, Content Type, Part number, format
void parse_onedep(const std::string &s, std::vector<std::string> &parsed);

#endif

