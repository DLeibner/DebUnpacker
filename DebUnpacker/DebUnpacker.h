#pragma once
#include <string>
#include <vector>
#include "Environment.h"

class DebUnpacker
{
public:
  DebUnpacker(Environment& env);

  bool run(std::string input, std::string output);

private:
  Environment& env;
  unsigned int packageFileSize;
  unsigned int controlFileSize;
  unsigned int dataFileSize;

  std::pair<bool, unsigned int> checkCommonBytes(const std::vector<char>& section, const std::string& identifier);
  bool checkArchiveFileSignature(const std::vector<char>& section);
  bool checkPackageSection(const std::vector<char>& section);
  bool checkControlSection(const std::vector<char>& section);
  bool checkDataSection(const std::vector<char>& section);

  void logStatus(bool ok, std::stringstream& ss);
};

