// Function for parsing OneDep filenames

#include "ParseOneDep.h"
#include <GenString.h>
#include <iostream>

// Split a string into a vector
static std::vector<std::string> split(const std::string& s, char seperator)
{
  std::vector<std::string> output;

  std::string::size_type prev_pos = 0, pos = 0;

  while((pos = s.find(seperator, pos)) != std::string::npos) {
    std::string substring( s.substr(prev_pos, pos-prev_pos) );
    output.push_back(substring);
    prev_pos = ++pos;
  }

  output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

  return output;
}


void parse_onedep(const std::string &s, std::vector<std::string>& out) {

  out.clear();

  std::vector<std::string> fFields = split(s, '.');
  if (fFields.size() < 1) {
    return;
  }

  //std::string baseName = trim(fFields[0]);
  std::string baseName = fFields[0];
  String::StripLeadingWs(baseName);
  String::StripTrailingWs(baseName);
  
  std::string fmt = fFields[1];
  String::StripLeadingWs(fmt);
  String::StripTrailingWs(fmt);

  std::string vno = "";
  
  if (fFields.size() > 2) {
    vno = fFields[2].substr(1);
  }
  
  // Split the first part into components

  std::vector<std::string> nFields = split(baseName, '_');

  if (nFields.size() != 4) {
    return;
  }

  std::string depid = nFields[0] + "_" + nFields[1];
  std::string ct = nFields[2];
  std::string partno = nFields[3].substr(1);

  out.push_back(depid);
  out.push_back(ct);
  out.push_back(partno);
  out.push_back(fmt);  
}

