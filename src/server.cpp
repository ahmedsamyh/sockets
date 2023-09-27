#define SOCKET_IMPLEMENTATION
#include <socket.hpp>

#define SFML_HELPER_IMPLEMENTATION
#include <sfml-helper.hpp>

int main(int argc, char *argv[]) {
  unsigned short port{8888};
  sf::TcpListener listener;
  std::vector<sf::TcpSocket> clients;

  if (listener.listen(port) != sf::Socket::Done) {
    std::cerr << "ERROR: Could not listen to port: " << port << "\n";
    return 1;
  }

  server::accept_client(listener);

  if (send_message_to_socket(client, "Hello from server!!!") ==
      sf::Socket::Status::Error) {
    std::cerr << "ERROR: Could no send data\n";
    return 1;
  }

  while (true) {
    sf::Socket::Status s{};
    std::string data{};
    s = receive_message_from_socket(client, data);
    if (s == sf::Socket::Status::Error) {
      std::cerr << "ERROR: Could not receive data\n";
      return 1;
    }

    if (!data.empty()) {
      std::cout << fmt("{}: {}\n", client.getRemoteAddress().toString(), data);
      s = send_message_to_socket(client, "Success");

      if (s == sf::Socket::Status::Error) {
        std::cerr << "ERROR: Could not send data\n";
        return 1;
      }
    }

    if (s == sf::Socket::Status::Disconnected) {
      std::cout << "INFO: disconnected from client ["
                << client.getRemoteAddress().toString() << "]\n";
      break;
    }
  }
  return 0;
}
