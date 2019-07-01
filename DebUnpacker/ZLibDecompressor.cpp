#include "pch.h"
#include "ZLibDecompressor.h"
#include <fstream>

ZLibDecompressor::ZLibDecompressor(): in{}, out{}, strm()
{
}

ZLibDecompressor::~ZLibDecompressor()
{
  inflateEnd(&strm);
}

std::optional<std::string> ZLibDecompressor::decompress(std::ifstream& input, int from, const int to, const std::string& outputPath)
{
  std::ofstream output(outputPath, std::ios::out | std::ios::binary | std::ios::trunc);

  if (!output)
  {
    return "Failed to open file on path " + outputPath;
  }

  /* allocate inflate state */
  strm.zalloc = nullptr;
  strm.zfree = nullptr;
  strm.opaque = nullptr;
  strm.avail_in = 0;
  strm.next_in = nullptr;
  auto ret = inflateInit2(&strm, 16 + MAX_WBITS);
  if (ret != Z_OK)
    return "Inflate initialization failed";

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

    auto sum = 0;

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
      sum += have;
      // length check for possible memory bomb
      if (sum - size > 1e+9)
      {
        inflateEnd(&strm);
        return "Decompressed file is a lot larger than compressed - Most likely memory bomb!";
      }
      output.write(out, have);
    } while (strm.avail_out == 0);
  } while (ret != Z_STREAM_END);

  inflateEnd(&strm);
  return std::nullopt;
}

std::optional<std::string> ZLibDecompressor::extractWithoutInflate(std::ifstream& input, int from, const int to, const std::string& outputPath)
{
  std::ofstream output(outputPath, std::ios::out | std::ios::binary | std::ios::trunc);

  if (!output)
  {
    return "Failed to open file on path " + outputPath;
  }

  while (from != to)
  {
    const auto size = to - from > chunk ? chunk : to - from;
    from += size;

    input.read(in, size);
    output.write(in, size);
  }

  return std::nullopt;
}
