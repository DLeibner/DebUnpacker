#include "pch.h"
#include "ZLibDecompressor.h"
#include <fstream>

ZLibDecompressor::ZLibDecompressor(): in{}, out{}
{
  /* allocate inflate state */
  strm.zalloc = nullptr;
  strm.zfree = nullptr;
  strm.opaque = nullptr;
  strm.avail_in = 0;
  strm.next_in = nullptr;
  const auto ret = inflateInit2(&strm, 16 + MAX_WBITS);
  if (ret != Z_OK)
    throw std::bad_alloc();
}

ZLibDecompressor::~ZLibDecompressor()
{
  inflateEnd(&strm);
}

std::optional<std::string> ZLibDecompressor::decompress(std::ifstream& input, int from, const int to, std::ofstream& output)
{
  int ret;

  do
  {
    if (from == to)
    {
      inflateEnd(&strm);
      return "File size 0";
    }

    const auto size = to - from > chunk ? chunk : to - from;
    from += size;

    input.read(in, size);

    strm.avail_in = size;
    strm.next_in = reinterpret_cast<unsigned char*>(in);

    do
    {
      strm.avail_out = chunk;
      strm.next_out = reinterpret_cast<unsigned char*>(out);

      ret = inflate(&strm, Z_NO_FLUSH);
      if (ret == Z_STREAM_ERROR)
      {
        inflateEnd(&strm);
        return "Z_STREAM_ERROR (-2)";
      }
      switch (ret)
      {
      case Z_NEED_DICT:
        inflateEnd(&strm);
        return "Z_NEED_DICT (2)";
      case Z_DATA_ERROR:
        inflateEnd(&strm);
        return "Z_DATA_ERROR (-3)";
      case Z_MEM_ERROR:
        inflateEnd(&strm);
        return "Z_MEM_ERROR (-4)";
      default: ;
      }

      const auto have = chunk - strm.avail_out;
      output.write(out, have);
    } while (strm.avail_out == 0);
  } while (ret != Z_STREAM_END);

  inflateEnd(&strm);
  return std::nullopt;
}

void ZLibDecompressor::extractWithoutInflate(std::ifstream& input, int from, int to, std::ofstream& output)
{
  while (from != to)
  {
    const auto size = to - from > chunk ? chunk : to - from;
    from += size;

    input.read(in, size);
    output.write(in, size);
  }
}
