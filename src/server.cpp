#define SFML_HELPER_IMPLEMENTATION
#include <sfml-helper.hpp>

#include <memory>

#define MAX_PACKET_SIZE 1024 * 5

typedef std::shared_ptr<sf::TcpSocket> socket_ptr;

void err() {
  std::cout << "ERROR!\n";
  exit(1);
}

int main(int argc, char *argv[]) {
  //
  std::cout << std::unitbuf;

  unsigned short port{8888};
  sf::TcpListener listener;
  sf::SocketSelector l_selector;
  l_selector.add(listener);
  sf::SocketSelector selector;
  std::vector<socket_ptr> clients;

  // std::cout << "INFO: Listening to incoming connections...\n";
  auto s = listener.listen(port);
  if (s == sf::Socket::Status::Error) {
    err();
  }
  while (true) {
  accept:
    if (l_selector.wait()) {
      std::cout << "INFO: Accepting client connection...\n";
      clients.push_back(std::make_shared<sf::TcpSocket>());
      socket_ptr client = clients.back();
      s = listener.accept(*client);
      if (s == sf::Socket::Status::Error) {
        err();
      }

      std::cout << "INFO: Client " << client->getRemoteAddress().toString()
                << ":" << client->getRemotePort() << " connected...\n";

      // std::cout << "INFO: Added Client: " << clients.size() << "\n";
      selector.add(*client);
    } else {
      std::cout << "INFO: Waiting for client connection...\r";
    }

    if (selector.wait()) {
      for (int i = 0; i < clients.size(); ++i) {
        auto &c = clients[i];
        if (selector.isReady(*c)) {
          // std::cout << "INFO: Waiting to receive packet from Client "
          //           << client->getRemoteAddress().toString() << ":"
          //           << client->getRemotePort() << "...\n";

          std::string receive_packet;
          receive_packet.resize(MAX_PACKET_SIZE);
          size_t received{0};

          s = c->receive((void *)receive_packet.c_str(), MAX_PACKET_SIZE,
                         received);
          if (s == sf::Socket::Status::Error) {
            err();
          } else if (s == sf::Socket::Status::Disconnected) {
            std::cout << "INFO: Client " << c->getRemoteAddress().toString()
                      << ":" << c->getRemotePort() << " disconnected...\n";
            c->disconnect();
            clients.erase(clients.begin() + i);
            selector.remove(*c);
            selector.clear();
            goto accept;
          }
          receive_packet.resize(received);
          std::cout
              << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
                 "<<<<<\n";
          std::cout << c->getRemoteAddress().toString() << ":"
                    << c->getRemotePort() << ": " << receive_packet << "\n";
          std::cout
              << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
                 "<<<<<\n";
        }
      }
    }
    // std::string send_packet{};
    // if (receive_packet == "ACK REQ") {
    //   send_packet = "ACK CONFIRM";
    // } else if (receive_packet == "ACK ACK") {
    // }

    // if (!send_packet.empty()) {
    //   std::cout << "INFO: Sending packet to client "
    //             << client->getRemoteAddress().toString() << ":"
    //             << client->getRemotePort() << "...\n";
    //   s = client->send(send_packet.c_str(), send_packet.size());
    //   if (s == sf::Socket::Status::Error) {
    //     err();
    //   }
    // }
  }

  return 0;
}
