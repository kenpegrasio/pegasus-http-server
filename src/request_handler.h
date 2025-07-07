#include <sys/socket.h>

#include <iostream>
#include <map>
#include <sstream>
#include <string>

const int BUFFER_SIZE = 4096;

std::string read_request(int);
void parse_request(std::string &, std::string &, std::string &,
                   std::map<std::string, std::string> &, std::string &,
                   std::string &);