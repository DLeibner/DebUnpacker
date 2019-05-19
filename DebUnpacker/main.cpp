#include "pch.h"
#include <iostream>
#include "DebUnpacker.h"
#include "Environment.h"


class ConsoleEnvironment : public Environment
{
public:
  void Trace(TraceLevel severity, std::ostream& stream)
  {
    std::cout << traceLevels.at(severity) << stream.rdbuf() << std::endl;
  }
};


int main()
{
  std::cout << "Deb unpacker\n";

  //std::cout << "Insert full path to .deb file:\n";
  //std::string filePath;
  //std::cin >> filePath;

  //std::string filePath = "D:\\other\\simple\\DebFileFormat\\DebUnpacker\\DebFiles\\pinta_1.3-3ubuntu0.14.04.2_all.deb";
  std::string filePath = "D:\\other\\simple\\DebFileFormat\\DebUnpacker\\DebFiles\\firefox_63.0.3+build1-0ubuntu0.14.04.1_amd64.deb";

  //std::cout << "Insert full path to foler for exporting data from deb file:\n";
  //std::string exportPath;
  //std::cin >> exportPath;

  std::string exportPath = "D:\\other\\simple\\DebFileFormat\\DebUnpacker\\Exported";

  ConsoleEnvironment env;
  DebUnpacker unpacker(env);

  bool ok = unpacker.run(filePath, exportPath);

  const std::string report = ok ? "successful" : "failed";
  std::cout << "Export for selected deb file " << report << std::endl;
}
