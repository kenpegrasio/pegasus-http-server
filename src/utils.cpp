#include "utils.h"

std::string transform_to_lowercase(std::string &s) {
  std::string res = "";
  for (int i = 0; i < (int)s.length(); i++) {
    if (s[i] >= 'A' && s[i] <= 'Z') {
      res += s[i] - 'A' + 'a';
    } else {
      res += s[i];
    }
  }
  return res;
}

std::string gzip_compress(const std::string &input) {
  z_stream zs{};
  deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED,
               15 + 16,  // 15 window bits + 16 = gzip
               8, Z_DEFAULT_STRATEGY);

  zs.next_in = (Bytef *)input.data();
  zs.avail_in = input.size();

  int ret;
  char outbuffer[32768];
  std::string outstring;

  do {
    zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = deflate(&zs, Z_FINISH);
    outstring.append(outbuffer, sizeof(outbuffer) - zs.avail_out);
  } while (ret == Z_OK);

  deflateEnd(&zs);

  if (ret != Z_STREAM_END) throw std::runtime_error("gzip compression failed!");

  return outstring;
}

std::vector<std::string> split_compression_header(
    std::string compression_header, char delimiter) {
  std::vector<std::string> res;
  std::string cur = "";
  for (int i = 0; i < (int)compression_header.size(); i++) {
    if (compression_header[i] == ' ') continue;
    if (compression_header[i] == delimiter) {
      res.push_back(cur);
      cur.clear();
      continue;
    }
    cur += compression_header[i];
  }
  if (!cur.empty()) res.push_back(cur);
  return res;
}