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
  packageFileSize(0), controlFileSize(0), dataFileSize(0)
{
}

bool DebUnpacker::run(std::string inputFilePath, std::string outputFilePath)
{
  std::ifstream input(inputFilePath, std::ifstream::binary);
  std::ofstream output(outputFilePath, std::ofstream::binary);

  bool ok;
  std::stringstream ss;

  const short archiveSignatureLength = 8;
  std::vector<char> archiveSignature(archiveSignatureLength, 0);
  input.read(&archiveSignature[0], archiveSignatureLength);

  ss << "Archive file Signature check -> ";
  ok = checkArchiveFileSignature(archiveSignature);
  logStatus(ok, ss);
  if (!ok) return ok;

  const short packageSectionLength = 60;
  std::vector<char> packageSection(packageSectionLength, 0);
  input.read(&packageSection[0], packageSectionLength);

  ss << "Check package section -> ";
  ok = checkPackageSection(packageSection);
  logStatus(ok, ss);
  if (!ok) return ok;

  std::vector<char> packageFile(packageFileSize, 0);
  input.read(&packageFile[0], packageFileSize);

  // todo check version from package file

  const short controlSectionLength = 60;
  std::vector<char> controlSection(controlSectionLength, 0);
  input.read(&controlSection[0], controlSectionLength);

  ss << "Check control section -> ";
  ok = checkControlSection(controlSection);
  logStatus(ok, ss);
  if (!ok) return ok;

  int from = input.tellg();
  int to = from + controlFileSize;

  // extract control file using zlib
  if (!controlFileInflate)
  {
    zlibDecompress.extractWithoutInflate(input, from, to, output);
  }
  else if (auto ret = zlibDecompress.decompress(input, from, to, output))
  {
    ss << "Error in decompressing Control File: " << *ret;
    env.Trace(Environment::TraceLevel::Error, ss);
    return false;
  }

  const short dataSectionLength = 60;
  std::vector<char> dataSection(dataSectionLength, 0);
  input.read(&dataSection[0], dataSectionLength);

  ss << "Check data section -> ";
  ok = checkDataSection(dataSection);
  logStatus(ok, ss);
  if (!ok) return ok;

  from = input.tellg();
  to = from + dataFileSize;

  if (!dataFileInflate)
  {
    zlibDecompress.extractWithoutInflate(input, from, to, output);
  }
  else if(auto ret = zlibDecompress.decompress(input, from, to, output))
  {
    ss << "Error in decompressing Data File: " << *ret;
    env.Trace(Environment::TraceLevel::Error, ss);
    return false;
  }

  return ok;
}

std::tuple<bool, unsigned int, bool> DebUnpacker::checkCommonBytes(
  const std::vector<char>& section, const std::vector<std::string>& identifier)
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

  auto isDigitOrEmpty = [](unsigned char x) { return std::isdigit(x) || x == ' '; };

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
  std::string end = "`\n";
  if (!std::equal(section.cbegin() + 58, section.cbegin() + 60, std::cbegin(end)))
    return std::make_tuple(false, size, inflate);

  return std::make_tuple(true, size, inflate);
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
  std::vector<std::string> identifier{ "debian-binary   " };
  std::tie(ok, packageFileSize, std::ignore) = checkCommonBytes(section, identifier);

  return ok;
}

bool DebUnpacker::checkControlSection(const std::vector<char>& section)
{
  bool ok;
  std::vector<std::string> identifier{ "control.tar.gz  ", "control.tgz     ", "control.tar.xz  " };
  std::tie(ok, controlFileSize, controlFileInflate) = checkCommonBytes(section, identifier);

  return ok;
}

bool DebUnpacker::checkDataSection(const std::vector<char>& section)
{
  bool ok;
  std::vector<std::string> identifier{ "data.tar.gz     ", "data.tgz        ", "data.tar.bz2    ", "data.tar.7z     ", "data.tar.xz     " };
  std::tie(ok, dataFileSize, dataFileInflate) = checkCommonBytes(section, identifier);

  return ok;
}

void DebUnpacker::logStatus(bool ok, std::stringstream& ss)
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
