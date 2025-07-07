#include <arpa/inet.h>

#include <fstream>
#include <thread>

#include "request_handler.h"
#include "response_handler.h"
#include "utils.h"

void process_client(int client_socket, std::string &directory) {
  while (true) {
    std::string res = read_request(client_socket);
    if (res == "") return;

    std::string method, path, version;
    std::map<std::string, std::string> request_headers;
    std::string request_body = "";
    parse_request(method, path, version, request_headers, request_body, res);

    // Construct Response
    HttpResponseStartLine response_start_line = HttpResponseStartLine(200);
    std::map<std::string, std::string> response_headers;
    std::string response_body = "";

    bool compress_body = false;
    if (request_headers.find("accept-encoding") != request_headers.end() &&
        request_headers["accept-encoding"].find("gzip") != std::string::npos) {
      std::vector<std::string> compression_headers =
          split_compression_header(request_headers["accept-encoding"], ',');
      for (auto compression_header : compression_headers) {
        if (compression_header == "gzip") {
          compress_body = true;
          add_response_header(response_headers, "Content-Encoding", "gzip");
          break;
        }
      }
    }

    if (method == "POST") {
      if (path.find("/files") != std::string::npos) {
        if (directory == "") {
          response_start_line = HttpResponseStartLine(404);
        } else {
          std::string filename = path.substr(7);
          std::ofstream outputFile(directory + filename);
          outputFile << request_body;
          response_start_line = HttpResponseStartLine(201);
        }
      }
    } else if (method == "GET") {  // GET request handler
      if (path == "/")
        ;
      else if (path.substr(0, 5) == "/echo") {
        add_response_header(response_headers, "Content-Type", "text/plain");
        response_body = path.substr(6, (int)path.length() - 6);
      } else if (path.find("/user-agent") != std::string::npos &&
                 request_headers.find("user-agent") != request_headers.end()) {
        add_response_header(response_headers, "Content-Type", "text/plain");
        response_body = request_headers["user-agent"];
      } else if (path.find("/files") != std::string::npos) {
        std::string filename = path.substr(7);
        std::ifstream inputFile(directory + filename);
        if (!inputFile.is_open()) {
          response_start_line = HttpResponseStartLine(404);
        } else {
          std::string inputLine;
          std::string res = "";
          while (std::getline(inputFile, inputLine)) {
            res += inputLine + '\n';
          }
          res.pop_back();  // remove the last \n
          add_response_header(response_headers, "Content-Type",
                              "application/octet-stream");
          response_body = res;
        }
      } else {
        response_start_line = HttpResponseStartLine(404);
      }
    }

    if (compress_body) {
      response_body = gzip_compress(response_body);
    }

    add_response_header(response_headers, "Content-Length",
                        std::to_string(response_body.size()));

    if (request_headers.find("connection") != request_headers.end() &&
        request_headers["connection"] == "close") {
      add_response_header(response_headers, "Connection", "close");
    }

    send_response(client_socket, response_start_line, response_headers,
                  response_body);

    if (request_headers.find("connection") != request_headers.end() &&
        request_headers["connection"] == "close") {
      break;
    }
  }

  close(client_socket);
}

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // Process argument
  std::string directory = "";
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--directory") {
      if (i + 1 < argc) {
        directory = argv[i + 1];
      }
    }
  }

  std::cout << "Logs from your program will appear here!\n";

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  while (true) {
    // Documentation related:
    // https://pubs.opengroup.org/onlinepubs/009604499/functions/accept.html
    int client_socket = accept(server_fd, (struct sockaddr *)&client_addr,
                               (socklen_t *)&client_addr_len);
    if (client_socket < 0) {
      std::cerr << "Failed to accept connection" << std::endl;
      return 1;
    }
    std::cout << "Client connected\n";
    std::thread t(process_client, client_socket, std::ref(directory));
    t.detach();
  }

  close(server_fd);

  return 0;
}
