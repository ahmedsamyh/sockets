#define SFML_HELPER_IMPLEMENTATION
#include <sfml-helper.hpp>

#include <memory>

#define MAX_PACKET_SIZE 1024 * 5

typedef std::shared_ptr<sf::TcpSocket> socket_ptr;

void err() {
  std::cout << "ERROR!\n";
  exit(1);
}

struct Client {
  std::string name;
  socket_ptr socket;
};

int main(int argc, char *argv[]) {
  //
  std::cout << std::unitbuf;

  unsigned short port{8888};
  sf::TcpListener listener;
  sf::SocketSelector l_selector;
  sf::SocketSelector selector;
  std::vector<Client> clients;

  std::cout << "INFO: Listening to incoming connections...\n";
  auto s = listener.listen(port);
  if (s == sf::Socket::Status::Error) {
    err();
  }
  l_selector.add(listener);

  while (true) {
  accept:
    if (l_selector.wait(sf::seconds(0.1f))) {
      clients.push_back({"NO_NAME", std::make_shared<sf::TcpSocket>()});
      Client &client = clients.back();
      s = listener.accept(*client.socket);
      if (s == sf::Socket::Status::Error) {
        err();
      }

      selector.add(*client.socket);
    }

    if (selector.wait(sf::seconds(0.1f))) {
      for (int i = 0; i < clients.size(); ++i) {
        auto &c = clients[i];
        if (selector.isReady(*c.socket)) {

          std::string receive_packet;
          receive_packet.resize(MAX_PACKET_SIZE);
          size_t received{0};

          s = c.socket->receive((void *)receive_packet.c_str(), MAX_PACKET_SIZE,
                                received);
          if (s == sf::Socket::Status::Error) {
            err();
          } else if (s == sf::Socket::Status::Disconnected) {
            std::cout << "INFO: Client " << c.name << " disconnected...\n";
            selector.remove(*c.socket);
            c.socket->disconnect();
            clients.erase(clients.begin() + i);
            goto accept;
          }
          receive_packet.resize(received);

          if (c.name == "NO_NAME") {
            c.name = receive_packet;
            std::cout << "INFO: " << c.name << " connected...\n";
            std::string send_packet{"Successfully connected to server"};
            s = c.socket->send(send_packet.c_str(), send_packet.size());
            if (s == sf::Socket::Status::Error) {
              err();
            }
          } else {
            // send message to other clients
            for (auto &other : clients) {
              if (other.name != c.name) {
                std::string send_packet = receive_packet;
                s = other.socket->send(send_packet.c_str(), send_packet.size());
                if (s == sf::Socket::Status::Error) {
                  err();
                }
              }
            }

            std::cout << c.name << ": " << receive_packet << "\n";
          }
        }
      }
    }
  }

  return 0;
}
