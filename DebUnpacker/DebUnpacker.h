#pragma once
#include <string>
#include <vector>

class DebUnpacker
{
public:
  DebUnpacker();

  bool run(std::string input, std::string output);

private:
  unsigned int packageFileSize;
  unsigned int controlFileSize;
  unsigned int dataFileSize;

  std::pair<bool, unsigned int> checkCommonBytes(const std::vector<char>& section, const std::string& identifier);
  bool checkArchiveFileSignature(const std::vector<char>& section);
  bool checkPackageSection(const std::vector<char>& section);
  bool checkControlSection(const std::vector<char>& section);
  bool checkDataSection(const std::vector<char>& section);
};

