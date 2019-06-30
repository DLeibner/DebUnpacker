#include "pch.h"
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cctype>
#include <numeric>
#include <sstream>
#include "DebUnpacker.h"
#include "ZLibDecompressor.h"

DebUnpacker::DebUnpacker(Environment& env) : env(env),
  packageFileSize(0), controlFileSize(0), dataFileSize(0),
  packageFileInflate(false), controlFileInflate(false), dataFileInflate(false)
{
}

bool DebUnpacker::run(const std::string& inputFilePath, const std::string& outputFilePath)
{
  std::ifstream input(inputFilePath, std::ifstream::binary);
  std::ofstream output(outputFilePath, std::ofstream::binary);

  if (!checkArchiveFileSignature(input))
    return false;

  std::vector<std::string> packageIdentifier{ "debian-binary   " };
  if (!checkSection(input, packageFileSize, packageFileInflate, packageIdentifier, "Package"))
    return false;

  if (!extractFile(input, packageFileSize, packageFileInflate, output))
    return false;

  std::vector<std::string> controlIdentifier{ "control.tar.gz  ", "control.tgz     ", "control.tar.xz  " };
  if (!checkSection(input, controlFileSize, controlFileInflate, controlIdentifier, "Control"))
    return false;

  if (!extractFile(input, controlFileSize, controlFileInflate, output))
    return false;

  std::vector<std::string> dataIdentifier{ "data.tar.gz     ", "data.tgz        ", "data.tar.bz2    ", "data.tar.7z     ", "data.tar.xz     " };
  if (!checkSection(input, dataFileSize, dataFileInflate, dataIdentifier, "Data"))
    return false;

  if (!extractFile(input, dataFileSize, dataFileInflate, output))
    return false;

  return true;
}

bool DebUnpacker::checkSection(std::ifstream& input, unsigned int& fileSize, bool& inflate, const std::vector<std::string>& identifier, const std::string& sectionName) const
{
  std::vector<char> section(sectionLength, 0);
  input.read(&section[0], sectionLength);

  std::stringstream ss;
  ss << "Check " << sectionName << " section -> ";

  bool ok;
  std::tie(ok, fileSize, inflate) = checkCommonBytes(section, identifier);
  logStatus(ok, ss);

  return ok;
}

bool DebUnpacker::extractFile(std::ifstream& input, unsigned int size, bool inflate, std::ofstream& output)
{
  const int from = static_cast<int>(input.tellg());
  const int to = from + size;

  if (!inflate)
  {
    zlibDecompress.extractWithoutInflate(input, from, to, output);
  }
  else if (auto ret = zlibDecompress.decompress(input, from, to, output))
  {
    std::stringstream ss;
    ss << "Error in decompressing: " << *ret;
    env.Trace(Environment::TraceLevel::Error, ss);
    return false;
  }

  return true;
}

std::tuple<bool, unsigned int, bool> DebUnpacker::checkCommonBytes(
  const std::vector<char>& section, const std::vector<std::string>& identifier) const
{
  unsigned int size = 0;
  bool inflate = false;

  // file identifier
  bool identOk = false;
  for (auto&& i: identifier)
  {
    if (std::equal(section.cbegin(), section.cbegin() + 16, std::cbegin(i)))
    {
      identOk = true;
      if (i.find(".tar.gz") != std::string::npos || i.find(".tgz") != std::string::npos)
      {
        inflate = true;
      }
      break;
    }
  }

  if (!identOk)
    return std::make_tuple(false, size, inflate);

  const auto isDigitOrEmpty = [](unsigned char x) { return std::isdigit(x) || x == ' '; };

  // file modification timestamp
  if (!std::all_of(section.cbegin() + 16, section.cbegin() + 28, isDigitOrEmpty))
    return std::make_tuple(false, size, inflate);

  // owner ID
  if (!std::all_of(section.cbegin() + 28, section.cbegin() + 34, isDigitOrEmpty))
    return std::make_tuple(false, size, inflate);

  // group ID
  if (!std::all_of(section.cbegin() + 34, section.cbegin() + 40, isDigitOrEmpty))
    return std::make_tuple(false, size, inflate);

  // file mode
  if (!std::all_of(section.cbegin() + 40, section.cbegin() + 48, isDigitOrEmpty))
    return std::make_tuple(false, size, inflate);

  // file size in bytes (decimal)
  size = std::accumulate(section.cbegin() + 48, section.cbegin() + 58, 0,
    [](int a, unsigned char b) { return std::isdigit(b) ? a * 10 + b - 48 : a; });

  // end char
  const std::string end = "`\n";
  if (!std::equal(section.cbegin() + 58, section.cbegin() + 60, std::cbegin(end)))
    return std::make_tuple(false, size, inflate);

  return std::make_tuple(true, size, inflate);
}

bool DebUnpacker::checkArchiveFileSignature(std::ifstream& input) const
{
  bool ok = true;
  std::stringstream ss;
  const short archiveSignatureLength = 8;
  std::vector<char> archiveSignature(archiveSignatureLength, 0);
  input.read(&archiveSignature[0], archiveSignatureLength);

  ss << "Archive file Signature check -> ";
  const std::string archive = "!<arch>\n";
  if (!std::equal(archiveSignature.cbegin(), archiveSignature.cbegin() + 8, std::cbegin(archive)))
    ok = false;

  logStatus(ok, ss);
  return ok;
}

void DebUnpacker::logStatus(bool ok, std::stringstream& ss) const
{
  if (ok)
  {
    ss << "OK";
    env.Trace(Environment::TraceLevel::Info, ss);
  }
  else
  {
    ss << "FAILED!";
    env.Trace(Environment::TraceLevel::Error, ss);
  }
}
