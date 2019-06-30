#pragma once
#include <string>
#include <optional>
#include "../zlib/include/zlib/zlib.h"

class ZLibDecompressor
{
public:
  ZLibDecompressor();
  ~ZLibDecompressor();

  std::optional<std::string> decompress(std::ifstream& input, int from, int to, const std::string& outputPath);
  std::optional<std::string> extractWithoutInflate(std::ifstream& input, int from, int to, const std::string& outputPath);

protected:
  static constexpr size_t chunk = 262144;

private:
  char in[chunk];
  char out[chunk];
  z_stream strm;
};

