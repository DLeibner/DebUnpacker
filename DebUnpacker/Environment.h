#pragma once
#include <map>
class Environment
{
public:
  virtual ~Environment() = default;

  /// trace levels
  enum class TraceLevel
  {
    Error = 0,
    Warning = 1,
    Info = 2,
    Debug = 3
  };

  std::map<TraceLevel, std::string> traceLevels = {
    {TraceLevel::Error, "Error: "},
    {TraceLevel::Warning, "Warning: "},
    {TraceLevel::Info, "Info: "},
    {TraceLevel::Debug, "Debug: "}
  };

  /// ability to trace
  virtual void Trace(
    TraceLevel severity,
    std::ostream& stream
  ) = 0;
};