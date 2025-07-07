#include "response_handler.h"

HttpResponseStartLine::HttpResponseStartLine(
    int identifier, std::string new_protocol) {
  protocol = new_protocol;
  status_code = std::to_string(identifier);
  if (identifier == 404)
    status_text = "Not Found";
  else if (identifier == 200)
    status_text = "OK";
  else if (identifier == 201)
    status_text = "Created";
}

HttpResponseStartLine::HttpResponseStartLine(std::string new_protocol,
                                             std::string new_status_code,
                                             std::string new_status_text) {
  protocol = new_protocol;
  status_code = new_status_code;
  status_text = new_status_text;
}

void add_response_header(std::map<std::string, std::string> &headers,
                         const std::string &key, const std::string &val) {
  headers[key] = val;
}

void send_response(int client_socket, HttpResponseStartLine start_line,
                   std::map<std::string, std::string> &headers,
                   const std::string &body_message) {
  std::string res = "";
  res += start_line.protocol + " " + start_line.status_code + " " +
         start_line.status_text + "\r\n";
  for (auto [key, val] : headers) {
    res += key + ": " + val + "\r\n";
  }
  res += "\r\n";
  res += body_message;
  send(client_socket, res.c_str(), res.length(), 0);
}