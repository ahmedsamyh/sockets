#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sfml-helper.hpp>

sf::Socket::Status
receive_message_from_socket(sf::TcpSocket &socket, std::string &msg,
                            std::size_t max_msg_size = 1024 * 2);

sf::Socket::Status send_message_to_socket(sf::TcpSocket &socket,
                                          const std::string &msg);

namespace server {
void accept_client(sf::TcpListener &listener,
                   std::vector<sf::TcpSocket> &clients);
}; // namespace server

#endif /* _SOCKET_H_ */

////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SOCKET_IMPLEMENTATION
sf::Socket::Status receive_message_from_socket(sf::TcpSocket &socket,
                                               std::string &msg,
                                               std::size_t max_msg_size) {
  msg.resize(max_msg_size);

  size_t received{0};

  sf::Socket::Status s{sf::Socket::Done};
  do {
    s = socket.receive((void *)msg.c_str(), max_msg_size, received);

    switch (s) {
    case sf::Socket::Status::Error: {
      std::cerr << "ERROR: Could not receive data from server!\n";
      return s;
    } break;
    case sf::Socket::Status::Partial: {
    } break;
    case sf::Socket::Status::Disconnected: {
      std::cout << "INFO: Disconnected from ["
                << socket.getRemoteAddress().toString() << "]\n";
      return s;
    } break;
    case sf::Socket::Status::Done: {
      break;
    } break;
    }

  } while (s == sf::Socket::Partial);
  msg.resize(received);
  return s;
}

sf::Socket::Status send_message_to_socket(sf::TcpSocket &socket,
                                          const std::string &msg) {

  sf::Socket::Status s{sf::Socket::Done};
  do {
    s = socket.send(msg.c_str(), msg.size());

    switch (s) {
    case sf::Socket::Status::Error: {
      std::cerr << "ERROR: Could not send data from server!\n";
      return s;
    } break;
    case sf::Socket::Status::Partial: {
    } break;
    case sf::Socket::Status::Disconnected: {
      std::cout << "INFO: Disconnected from ["
                << socket.getRemoteAddress().toString() << "]\n";
      return s;
    } break;
    case sf::Socket::Status::Done: {
      break;
    } break;
    }

  } while (s == sf::Socket::Partial);
  return s;
}

namespace server {
void accept_client(sf::TcpListener &listener,
                   std::vector<sf::TcpSocket> &clients) {
  sf::TcpSocket client;

  sf::Socket::Status accept_s{};
  accept_s = listener.accept(client);
  if (accept_s == sf::Socket::Error) {
    std::cerr << "ERROR: Could not accept client\n";
  }

  std::cout << "INFO: "
            << fmt("Client [{}] connected successfully!",
                   client.getRemoteAddress().toString())
            << "\n";
  clients.push_back(client);
}
}; // namespace server
#endif
