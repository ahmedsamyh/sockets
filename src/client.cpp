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

  sf::IpAddress ip{"127.0.0.1"};
  std::cout << "INFO: Connecting to server " << ip.toString() << ":" << port
            << "...\n";
  auto s = server.connect(ip, port, sf::seconds(10.f));
  if (s == sf::Socket::Status::Error) {
    err();
  }

  std::cout << "INFO: Sending packet to server...\n";
  std::string send_packet{"ACK REQ\n"};
  s = server.send(send_packet.c_str(), send_packet.size());
  std::string s_str{"Done"};
  switch (s) {
  case sf::Socket::Status::Done:
    s_str = "Done";
    break;

  case sf::Socket::Status::NotReady:
    s_str = "NotReady";
    break;

  case sf::Socket::Status::Partial:
    s_str = "Partial";
    break;

  case sf::Socket::Status::Disconnected:
    s_str = "Disconnected";
    break;

  case sf::Socket::Status::Error:
    s_str = "Error";
    break;
  default:
    std::cerr << "unreachable\n";
    exit(1);
    break;
  }
  std::cout << "STATUS:  " << s_str << "\n";

  if (s == sf::Socket::Status::Error) {
    err();
  } else if (s == sf::Socket::Status::NotReady) {
    std::cout << "INFO: Server busy...\n";
    return 0;
  }
  bool should_exit = false;

  while (!should_exit) {
    if (selector.wait()) {
      if (selector.isReady(server)) {
        std::cout << "INFO: Waiting for packet to receive from server...\n";
        std::string receive_packet{};
        receive_packet.resize(MAX_PACKET_SIZE);
        size_t received{0};
        s = server.receive((void *)receive_packet.c_str(), MAX_PACKET_SIZE,
                           received);
        if (s == sf::Socket::Status::Error) {
          err();
        }
        std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
                     "<<<<<<<\n";
        std::cout << "SERVER: " << receive_packet << "\n";
        std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
                     "<<<<<<<\n";
        receive_packet.resize(received);
      }
    }
    // if (receive_packet == "ACK CONFIRM") {
    //   std::string ack_ack_packet{"ACK ACK"};
    //   s = server.send(ack_ack_packet.c_str(), ack_ack_packet.size());
    //   if (s == sf::Socket::Status::Error) {
    //     err();
    //   }
    // }

    std::cout << "Enter msg to send to server: ";
    std::string send_packet{""};
    std::getline(std::cin, send_packet);
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    std::cout << "MSG: " << send_packet << "\n";
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    if (!send_packet.empty()) {
      std::cout << "\nINFO: Sending package to server...\n";
      s = server.send(send_packet.c_str(), send_packet.size());
      if (s == sf::Socket::Status::Error) {
        err();
      }
    }
  }

  return 0;
}
