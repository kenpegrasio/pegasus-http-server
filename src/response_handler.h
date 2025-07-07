#include <sys/socket.h>

#include <map>
#include <string>

struct HttpResponseStartLine {
  std::string protocol;
  std::string status_code;
  std::string status_text;

  HttpResponseStartLine(int identifier, std::string new_protocol = "HTTP/1.1");

  HttpResponseStartLine(std::string new_protocol, std::string new_status_code,
                        std::string new_status_text);
};

void add_response_header(std::map<std::string, std::string> &headers,
                         const std::string &key, const std::string &val);

void send_response(int client_socket, HttpResponseStartLine start_line,
                   std::map<std::string, std::string> &headers,
                   const std::string &body_message);