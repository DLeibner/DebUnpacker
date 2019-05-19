#pragma once
#include <string>
#include <vector>
//#include "../zlib-1.2.11/zlib.h"
#include "../zlib/include/zlib/zlib.h"

class ZLibDecompressor
{
public:
  ZLibDecompressor();
  ~ZLibDecompressor();

  std::string decompress(const std::vector<char>& input);

protected:
  static const unsigned int chunk = 16384;

private:
  char in[chunk];
  char out[chunk];
  z_stream strm;
};

