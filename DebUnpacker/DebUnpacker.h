#pragma once
#include <string>
#include <vector>
#include <tuple>
#include "Environment.h"
#include "ZLibDecompressor.h"

class DebUnpacker
{
public:
  DebUnpacker(Environment& env);

  bool run(const std::string& inputFilePath, const std::string& outputFilePath);

private:
  Environment& env;
  ZLibDecompressor zlibDecompress;
  unsigned int packageFileSize;
  unsigned int controlFileSize;
  unsigned int dataFileSize;
  bool packageFileInflate;
  bool controlFileInflate;
  bool dataFileInflate;
  const short sectionLength = 60;

  std::tuple<bool, unsigned int, bool> checkCommonBytes(const std::vector<char>& section, const std::vector<std::string>& identifier) const;
  bool checkArchiveFileSignature(std::ifstream& input) const;
  bool checkSection(std::ifstream& input, unsigned int& fileSize, bool& inflate, const std::vector<std::string>& identifier, const std::string& sectionName) const;
  bool extractFile(std::ifstream& input, unsigned int size, bool inflate, std::ofstream& output);

  void logStatus(bool ok, std::stringstream& ss) const;
};

