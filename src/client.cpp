#define SFML_HELPER_IMPLEMENTATION
#include <sfml-helper.hpp>

#define MAX_PACKET_SIZE 1024 * 5

void err() {
  std::cout << "ERROR!\n";
  exit(1);
}

int main(int argc, char *argv[]) {
  //
  std::cout << std::unitbuf;

  unsigned short port{8888};
  sf::TcpSocket server;
  sf::SocketSelector selector;
  selector.add(server);
  std::string name{""};
  std::cout << "Enter your name: ";
  std::getline(std::cin, name);

  sf::IpAddress ip{"127.0.0.1"};
  std::cout << "\nINFO: Connecting to server " << ip.toString() << ":" << port
            << "...\n";
  auto s = server.connect(ip, port, sf::seconds(10.f));
  if (s == sf::Socket::Status::Error) {
    err();
  }

  // send name to server
  std::string send_packet{name};
  s = server.send(send_packet.c_str(), send_packet.size());

  if (s == sf::Socket::Status::Error) {
    err();
  } else if (s == sf::Socket::Status::NotReady) {
    std::cout << "INFO: Server busy...\n";
    return 0;
  }
  bool should_exit = false;

  while (!should_exit) {
    if (selector.wait(sf::seconds(0.1f))) {
      if (selector.isReady(server)) {
        std::string receive_packet{};
        receive_packet.resize(MAX_PACKET_SIZE);
        size_t received{0};
        s = server.receive((void *)receive_packet.c_str(), MAX_PACKET_SIZE,
                           received);
        if (s == sf::Socket::Status::Error) {
          err();
        }
        std::cout << "SERVER: " << receive_packet << "\n";
        receive_packet.resize(received);
      }
    }

    std::cout << "Enter msg to send to server: ";
    std::string send_packet{""};
    std::getline(std::cin, send_packet); // this blocks so i can receive shit
    // std::cout << "MSG: " << send_packet << "\n";
    if (!send_packet.empty()) {
      s = server.send(send_packet.c_str(), send_packet.size());
      if (s == sf::Socket::Status::Error) {
        err();
      }
    }
  }

  return 0;
}
