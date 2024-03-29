#include "pch.h"
#include <iostream>
#include "DebUnpacker.h"
#include "Environment.h"


class ConsoleEnvironment final : public Environment
{
public:
  void Trace(TraceLevel severity, std::ostream& stream) override
  {
    std::cout << traceLevels.at(severity) << stream.rdbuf() << std::endl;
  }
};


int main()
{
  std::cout << "-----Deb UnPacker-----\n";

  std::cout << "Insert full path to .deb file:\n";
  std::string filePath;
  std::cin >> filePath;

  std::cout << "Insert full path to folder for exporting data from deb file:\n";
  std::string exportPath;
  std::cin >> exportPath;

  ConsoleEnvironment env;
  DebUnpacker unpacker(env);

  const auto ok = unpacker.run(filePath, exportPath);

  const std::string report = ok ? "successful" : "failed";
  std::cout << "Export for selected deb file " << filePath << " to location " << exportPath << " -> " << report << std::endl;
}
