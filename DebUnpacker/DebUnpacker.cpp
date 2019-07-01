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

bool DebUnpacker::run(const std::string& inputFilePath, const std::string& outputFolderPath)
{
  std::ifstream input(inputFilePath, std::ios::in | std::ios::binary);

  if (!checkArchiveFileSignature(input))
    return false;

  const std::vector<std::string> packageIdentifier{ "debian-binary   " };
  if (!checkSection(input, packageFileSize, packageFileInflate, packageIdentifier, "Package"))
    return false;

  if (!extractFile(input, packageFileSize, packageFileInflate, outputFolderPath + "\\PackageFile"))
    return false;

  const std::vector<std::string> controlIdentifier{ "control.tar.gz  ", "control.tgz     ", "control.tar.xz  " };
  if (!checkSection(input, controlFileSize, controlFileInflate, controlIdentifier, "Control"))
    return false;

  if (!extractFile(input, controlFileSize, controlFileInflate, outputFolderPath + "\\ControlFile"))
    return false;

  const std::vector<std::string> dataIdentifier{
    "data.tar.gz     ", "data.tgz        ", "data.tar.bz2    ", "data.tar.7z     ", "data.tar.xz     "
  };
  if (!checkSection(input, dataFileSize, dataFileInflate, dataIdentifier, "Data"))
    return false;

  if (!extractFile(input, dataFileSize, dataFileInflate, outputFolderPath + "\\DataFile"))
    return false;

  return true;
}

bool DebUnpacker::checkSection(std::ifstream& input, unsigned int& fileSize, bool& inflate,
  const std::vector<std::string>& identifier, const std::string& sectionName) const
{
  std::vector<char> section(sectionLength, 0);
  input.read(&section[0], sectionLength);

  std::stringstream ss;
  ss << "Check " << sectionName << " section -> ";

  std::string error;
  std::tie(error, fileSize, inflate) = checkCommonBytes(section, identifier);

  const auto ok = error.empty();
  logStatus(ok, ss, error);

  return ok;
}

bool DebUnpacker::extractFile(std::ifstream& input, unsigned int size, bool inflate, const std::string& outputPath)
{
  const auto from = static_cast<int>(input.tellg());
  const int to = from + size;

  if (!inflate)
  {
    zlibDecompress.extractWithoutInflate(input, from, to, outputPath);
  }
  else if (auto ret = zlibDecompress.decompress(input, from, to, outputPath))
  {
    std::stringstream ss;
    ss << "Error in decompressing: " << *ret;
    env.Trace(Environment::TraceLevel::Error, ss);
    return false;
  }

  return true;
}

std::tuple<std::string, unsigned int, bool> DebUnpacker::checkCommonBytes(
  const std::vector<char>& section, const std::vector<std::string>& identifier) const
{
  unsigned int size = 0;
  auto inflate = false;

  // file identifier
  auto identOk = false;
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
    return std::make_tuple("Inconsistency in file identifier", size, inflate);

  const auto isDigitOrEmpty = [](unsigned char x) { return std::isdigit(x) || x == ' '; };

  // file modification timestamp
  if (!std::all_of(section.cbegin() + 16, section.cbegin() + 28, isDigitOrEmpty))
    return std::make_tuple("Inconsistency in file modification timestamp", size, inflate);

  // owner ID
  if (!std::all_of(section.cbegin() + 28, section.cbegin() + 34, isDigitOrEmpty))
    return std::make_tuple("Inconsistency in owner ID", size, inflate);

  // group ID
  if (!std::all_of(section.cbegin() + 34, section.cbegin() + 40, isDigitOrEmpty))
    return std::make_tuple("Inconsistency in group ID", size, inflate);

  // file mode
  if (!std::all_of(section.cbegin() + 40, section.cbegin() + 48, isDigitOrEmpty))
    return std::make_tuple("Inconsistency in file mode", size, inflate);

  // file size in bytes (decimal)
  size = std::accumulate(section.cbegin() + 48, section.cbegin() + 58, 0,
    [](int a, unsigned char b) { return std::isdigit(b) ? a * 10 + b - 48 : a; });

  // end char
  const std::string end = "`\n";
  if (!std::equal(section.cbegin() + 58, section.cbegin() + 60, std::cbegin(end)))
    return std::make_tuple("Inconsistency in end char", size, inflate);

  return std::make_tuple("", size, inflate);
}

bool DebUnpacker::checkArchiveFileSignature(std::ifstream& input) const
{
  auto ok = true;
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

void DebUnpacker::logStatus(bool ok, std::stringstream& ss, const std::string& error) const
{
  if (ok)
  {
    ss << "OK";
    env.Trace(Environment::TraceLevel::Info, ss);
  }
  else
  {
    ss << "FAILED! " << error;
    env.Trace(Environment::TraceLevel::Error, ss);
  }
}
