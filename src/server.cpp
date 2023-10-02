#define SFML_HELPER_IMPLEMENTATION
#include <sfml-helper.hpp>

using namespace sh;

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
  // Verify step:
  // Client connects to server
  // Client responds "Verify 1,{NAME}"
  // Server sends "Verify 1 Success"
  // Client responds "Verify 2,{NAME}"
  // Server sends "Verify 2 Success"
  // Client is connected
  bool verified[2]{false, false};
  bool is_verified() const { return verified[0] && verified[1]; }
};

int main(int argc, char *argv[]) {
  //
  std::cout << std::unitbuf;

  unsigned short port{6969};
  sf::TcpListener listener;
  sf::SocketSelector l_selector;
  sf::SocketSelector selector;
  std::vector<Client> clients;

  auto s = listener.listen(port);
  if (s == sf::Socket::Status::Error) {
    err();
  }
  l_selector.add(listener);

  print("INFO: Server started on port {}...\n", port);
  while (true) {
  accept:
    // accept clients
    if (l_selector.wait(sf::seconds(0.1f))) {
      clients.push_back({"", std::make_shared<sf::TcpSocket>()});
      Client &client = clients.back();
      s = listener.accept(*client.socket);
      if (s == sf::Socket::Status::Error) {
        err();
      }

      selector.add(*client.socket);
      print("INFO: Client is trying to connect...\n");
    }

    // receive/send to connected clients
    if (selector.wait(sf::seconds(0.01f))) {
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

          if (!c.verified[0]) {
            if (receive_packet.find(',') ==
                std::string_view::npos) { // Verify check fail
              std::cerr << FMT("INFO: Verify check 1 failed...\n");
              selector.remove(*c.socket);
              c.socket->disconnect();
              clients.erase(clients.begin() + i);
              goto accept;
            }
            std::string verify =
                receive_packet.substr(0, receive_packet.find(','));
            std::string name =
                receive_packet.substr(receive_packet.find(',') + 1);

            c.name = name;

            std::string send_packet = FMT("Verify 1 Success");
            s = c.socket->send(send_packet.c_str(), send_packet.size());
            if (s == sf::Socket::Status::Error) {
              err();
            }
            print("INFO: Verify 1 send...\n");
            c.verified[0] = true;
          } else if (!c.verified[1]) {
            if (receive_packet.find(',') ==
                std::string_view::npos) { // Verify check fail
              std::cerr << FMT("INFO: Verify check 2 failed...\n");
              selector.remove(*c.socket);
              c.socket->disconnect();
              clients.erase(clients.begin() + i);
              goto accept;
            }
            std::string verify =
                receive_packet.substr(0, receive_packet.find(','));
            std::string name =
                receive_packet.substr(receive_packet.find(',') + 1);

            if (c.name != name) {
              c.name = name;
            }

            std::string send_packet = FMT("Verify 2 Success");
            s = c.socket->send(send_packet.c_str(), send_packet.size());
            if (s == sf::Socket::Status::Error) {
              err();
            }
            c.verified[1] = true;
            print("INFO: Client {} connected successfully!\n", c.name);
          }

          if (c.is_verified()) {
            // send chat log to all clients
            for (auto &other : clients) {
              std::string send_packet = FMT("{}: {}", c.name, receive_packet);
              s = other.socket->send(send_packet.c_str(), send_packet.size());
              if (s == sf::Socket::Status::Error) {
                err();
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
