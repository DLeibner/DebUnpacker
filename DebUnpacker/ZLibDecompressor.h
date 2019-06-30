#pragma once
#include <string>
#include <vector>
#include <optional>
#include "../zlib/include/zlib/zlib.h"

class ZLibDecompressor
{
public:
  ZLibDecompressor();
  ~ZLibDecompressor();

  std::optional<std::string> decompress(std::ifstream& input, int from, int to, std::ofstream& output);
  void extractWithoutInflate(std::ifstream& input, int from, int to, std::ofstream& output);

protected:
  static constexpr size_t chunk = 262144;

private:
  char in[chunk];
  char out[chunk];
  z_stream strm;
};

