#pragma once
#include <map>
class Environment
{
public:
  /// trace levels
  enum class TraceLevel
  {
    Error = 0,
    Warning = 1,
    Info = 2,
    Debug = 3
  };

  std::map<TraceLevel, std::string> traceLevels = {
    {Environment::TraceLevel::Error, "Error: "},
    {Environment::TraceLevel::Warning, "Warning: "},
    {Environment::TraceLevel::Info, "Info: "},
    {Environment::TraceLevel::Debug, "Debug: "}
  };

  /// ability to trace
  virtual void Trace(
    TraceLevel severity,
    std::ostream& stream
  ) = 0;
};