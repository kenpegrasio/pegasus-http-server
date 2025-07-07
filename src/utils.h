#include <zlib.h>

#include <iostream>
#include <string>
#include <vector>

std::string transform_to_lowercase(std::string &);
std::string gzip_compress(const std::string &);
std::vector<std::string> split_compression_header(std::string, char);