#include "pch.h"
#include "DebUnpacker.h"
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <cctype>
#include <numeric>

DebUnpacker::DebUnpacker() :
  packageFileSize(0), controlFileSize(0), dataFileSize(0)
{
}

bool DebUnpacker::run(std::string input, std::string output)
{
  std::ifstream f(input, std::ifstream::binary);

  // not good solution...
  //std::ifstream f(input);
  //std::istream_iterator<std::string> start(f), end;
  //std::vector<std::string> data;
  //std::copy(start, end, std::back_inserter(data));

  // would be better if possible to iterate till desired byte in file
  //std::istream_iterator<std::string> endPackageSection(start);
  //std::advance(endPackageSection, 9); // TODO find how to stream part of the file?
  //std::copy(start, endPackageSection, std::back_inserter(data));

  //bool ok = checkFileFormat(data);
  bool ok = true;

  // read exact amount of data
  // not needed
  //f.seekg(0, f.end);
  //int length = f.tellg();
  //f.seekg(0, f.beg);

  const short archiveSignatureLength = 8;
  std::vector<char> archiveSignature(archiveSignatureLength, 0);
  f.read(&archiveSignature[0], archiveSignatureLength);

  ok = checkArchiveFileSignature(archiveSignature);
  if (!ok)  return ok;

  const short packageSectionLength = 60;

  // avoid char* better to use vector of chars
  //char packageSection2[packageSectionLength];
  //std::fill(std::begin(packageSection2), std::end(packageSection2), 0);
  //f.read(packageSection2, packageSectionLength);

  std::vector<char> packageSection(packageSectionLength, 0);
  f.read(&packageSection[0], packageSectionLength);

  ok = checkPackageSection(packageSection);
  if (!ok)  return ok;

  // read package file
  std::vector<char> packageFile(packageFileSize, 0);
  f.read(&packageFile[0], packageFileSize);

  // todo check version from package file

  const short controlSectionLength = 60;

  // avoid char* better to use vector of chars but just to see content
  //char controlSection2[cotrolSectionLength];
  //std::fill(std::begin(controlSection2), std::end(controlSection2), 0);
  //f.read(controlSection2, cotrolSectionLength);

  // read control section
  std::vector<char> controlSection(controlSectionLength, 0);
  f.read(&controlSection[0], controlSectionLength);

  ok = checkControlSection(controlSection);
  if (!ok)  return ok;

  // todo extract control file using zlib
  std::vector<char> controlFile(controlFileSize, 0);
  f.read(&controlFile[0], controlFileSize);

  // read data section
  const short dataSectionLength = 58;
  std::vector<char> dataSection(dataSectionLength, 0);
  f.read(&dataSection[0], dataSectionLength);

  ok = checkDataSection(dataSection);
  if (!ok)  return ok;

  // todo extract data file using zlib
  std::vector<char> dataFile(dataFileSize, 0);
  f.read(&dataFile[0], dataFileSize);

  return ok;
}

std::pair<bool, unsigned int> DebUnpacker::checkCommonBytes(
  const std::vector<char>& section, const std::string& identifier)
{
  unsigned int size = 0;

  // file identifider
  if (!std::equal(section.cbegin(), section.cbegin() + 16, std::cbegin(identifier)))
    return std::make_pair(false, size);

  auto isDigitOrEmpty = [](unsigned char x) { return std::isdigit(x) || x == ' '; };

  // file modification timestamp
  if (!std::all_of(section.cbegin() + 16, section.cbegin() + 28, isDigitOrEmpty))
    return std::make_pair(false, size);

  // owner ID
  if (!std::all_of(section.cbegin() + 28, section.cbegin() + 34, isDigitOrEmpty))
    return std::make_pair(false, size);

  // group ID
  if (!std::all_of(section.cbegin() + 34, section.cbegin() + 40, isDigitOrEmpty))
    return std::make_pair(false, size);

  // file mode
  if (!std::all_of(section.cbegin() + 40, section.cbegin() + 48, isDigitOrEmpty))
    return std::make_pair(false, size);

  // file size in bytes (decimal)
  size = std::accumulate(section.cbegin() + 48, section.cbegin() + 58, 0,
    [](int a, unsigned char b) { return std::isdigit(b) ? a * 10 + b - 48 : a; });

  // end char
  std::string end = "`\n";
  if (!std::equal(section.cbegin() + 58, section.cbegin() + 60, std::cbegin(end)))
    return std::make_pair(false, size);

  return std::make_pair(true, size);
}

bool DebUnpacker::checkArchiveFileSignature(const std::vector<char>& section)
{
  // archive file signature
  std::string archive = "!<arch>\n";
  if (!std::equal(section.cbegin(), section.cbegin() + 8, std::cbegin(archive)))
    return false;

  return true;
}

bool DebUnpacker::checkPackageSection(const std::vector<char>& section)
{
  bool ok;
  std::string identifier = "debian-binary   ";
  std::tie(ok, packageFileSize) = checkCommonBytes(section, identifier);

  return ok;
}

bool DebUnpacker::checkControlSection(const std::vector<char>& section)
{
  bool ok;
  std::string identifier = "control.tar.gz  ";

  std::tie(ok, controlFileSize) = checkCommonBytes(section, identifier);

  return ok;
}

bool DebUnpacker::checkDataSection(const std::vector<char>& section)
{
  bool ok;
  std::string identifier = "data.tar.gz     ";

  std::tie(ok, dataFileSize) = checkCommonBytes(section, identifier);

  return ok;
}
