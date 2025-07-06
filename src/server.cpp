#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>

#define BUFFER_SIZE 4096

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
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
  
  // Documentation related: https://pubs.opengroup.org/onlinepubs/009604499/functions/accept.html
  int client_socket = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  if (client_socket < 0) {
    std::cerr << "Failed to accept connection" << std::endl;
    return 1;
  }
  std::cout << "Client connected\n";

  char buffer[BUFFER_SIZE] = {0};
  int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
  if (bytes_read <= 0) {
    std::cerr << "Failed to receive data from client" << std::endl;
    close(client_socket);
    return 1;
  }
  buffer[bytes_read] = '\0';

  std::string request(buffer);
  size_t line_end = request.find("\r\n");
  std::string request_line = request.substr(0, line_end);
  
  std::istringstream iss(request_line);
  std::string method, path, version;
  iss >> method >> path >> version;
  
  if (path == "/") {
    send(client_socket, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  } else if (path.substr(0, 5) == "/echo") {
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string((int) path.length() - 6) + "\r\n\r\n" + path.substr(6, (int) path.length() - 6);
    send(client_socket, response.c_str(), response.length(), 0);
  } else {
    send(client_socket, "HTTP/1.1 404 Not Found\r\n\r\n", 26, 0);
  }
  
  close(server_fd);

  return 0;
}
