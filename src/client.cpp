#define SOCKET_IMPLEMENTATION
#include <socket.hpp>

#define SFML_HELPER_IMPLEMENTATION
#include <sfml-helper.hpp>

int main(int argc, char *argv[]) {
  //
  sf::TcpSocket server;

  // try to connect until it gets connected
  sf::Socket::Status status{};
  do {
    sf::Socket::Status status = server.connect("127.0.0.1", 8888);
  } while (status != sf::Socket::Status::Done);

  if (status != sf::Socket::Status::Done) {
    std::cerr << "ERROR: Could not connect socket!\n";
    return 1;
  }

  while (true) {
    std::string data{};
    sf::Socket::Status s{};
    s = receive_message_from_socket(server, data);

    if (s == sf::Socket::Status::Error) {
      std::cerr << "ERROR: Could not receive data\n";
      return 1;
    }

    if (!data.empty() && data != "Success")
      std::cout << fmt("SERVER: {}\n", data);

    std::string data_to_send{};
    std::cin >> data_to_send;
    if (!data_to_send.empty()) {
      s = send_message_to_socket(server, data_to_send);
      if (s == sf::Socket::Status::Error) {
        std::cerr << "ERROR: Could not send data\n";
        return 1;
      }
    }

    if (s == sf::Socket::Status::Disconnected) {
      std::cout << "INFO: disconnected from server!\n";
      break;
    }
  }
}
