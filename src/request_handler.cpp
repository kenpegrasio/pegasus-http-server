#include "request_handler.h"

#include "utils.h"

std::string read_request(int client_socket) {
  std::string res = "";
  while (true) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read == 0) {
      std::cerr << "Empty request" << std::endl;
      close(client_socket);
      return "";
    }
    if (bytes_read <= 0) {
      std::cerr << "Failed to receive data from client" << std::endl;
      close(client_socket);
      return "";
    }
    buffer[bytes_read] = '\0';
    res += buffer;
    if (res.find("\r\n\r\n") != std::string::npos) break;
  }
  return res;
}

void parse_request(std::string &method, std::string &path, std::string &version,
                   std::map<std::string, std::string> &request_headers,
                   std::string &request_body, std::string &input) {
  std::istringstream iss(input);
  std::string line;

  // Parse Request Start Line
  std::getline(iss, line);
  std::istringstream request_stream(line);
  request_stream >> method >> path >> version;

  // Parse Request Headers
  while (std::getline(iss, line) && line != "\r") {
    int colon_pos = line.find(':');
    if (colon_pos != std::string::npos) {
      std::string key = line.substr(0, colon_pos);
      std::string val = line.substr(colon_pos + 2);
      val.pop_back();  // need to pop the \r
      request_headers[transform_to_lowercase(key)] = val;
    }
  }

  // Parse Request Body
  while (std::getline(iss, line)) {
    request_body += line + '\n';
  }
  request_body.pop_back();  // remove the last \n
}