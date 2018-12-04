#include "pch.h"
#include "ZLibDecompressor.h"
#include <algorithm>

ZLibDecompressor::ZLibDecompressor()
{
  /* allocate inflate state */
  strm.zalloc = nullptr;
  strm.zfree = nullptr;
  strm.opaque = nullptr;
  strm.avail_in = 0;
  strm.next_in = nullptr;
  const int ret = inflateInit(&strm);
  if (ret != Z_OK)
    throw std::bad_alloc();
}

ZLibDecompressor::~ZLibDecompressor()
{
  inflateEnd(&strm);
}

std::string ZLibDecompressor::decompress(const std::vector<char>& input)
{
  std::string res;

  for (int i = 0; i < input.size(); i += chunk)
  {
    int left = input.size() - i;
    int size = left > chunk ? chunk : left;
    std::copy(input.begin() + i, input.begin() + size, in);

    strm.avail_in = size;
    strm.next_in = reinterpret_cast<unsigned char*>(in);

    do
    {
      strm.avail_out = chunk;
      strm.next_out = reinterpret_cast<unsigned char*>(out);
      int ret = inflate(&strm, Z_NO_FLUSH);
      if (ret == Z_STREAM_ERROR)
        throw std::bad_alloc();
      int have = chunk - strm.avail_out;
      res += std::string(out, have);
    } while (strm.avail_out == 0);
  }

  return res;
}