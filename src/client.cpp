#define SFML_HELPER_IMPLEMENTATION
#include <sfml-helper.hpp>

// TODO: User friendly text input
// TODO: Make a chat_buf instead and pushing other user's messages to server_buf

#define MAX_PACKET_SIZE 1024 * 5

void err() {
  std::cout << "ERROR!\n";
  exit(1);
}

int main(int argc, char *argv[]) {
  //

  unsigned short port{8888};
  sf::IpAddress ip{"127.0.0.1"};
  sf::TcpSocket server;
  sf::SocketSelector selector;
  std::string name{""};

  // graphical display things
  std::string server_buf{};
  std::string current_buf{};
  enum State {
    Port_ip_writing,
    Connect_to_server,
    Name_writing,
    Lobby,
    Chatting,
    Send_name_to_server
  };
  State state{State::Port_ip_writing};

  sf::Socket::Status s{};
  std::string receive_packet{};
  std::string send_packet{};
  std::string port_ip_str{"127.0.0.1:8888"};

  // errors
  bool server_connect_err{false};

  Data d;
  d.init(1280, 720, 1, "Client");

  UI ui(d);

  // game loop
  while (d.win.isOpen()) {
    // calculate delta time
    float delta = d.calc_delta();

    // update window title
    d.update_title();

    // event loop
    sf::Event e;
    d.update_mouse();
    d.update_key();
    while (d.win.pollEvent(e)) {
      d.handle_close(e);
      d.update_mouse_event(e);
      d.update_key_event(e);
      if (state == State::Port_ip_writing) {
        d.handle_text(e, port_ip_str);
      } else if (state == State::Name_writing) {
        d.handle_text(e, name);
      } else if (state == State::Chatting) {
        d.handle_text(e, current_buf);
      }
    }

    // clear
    d.clear();

    // update
    switch (state) {
    case State::Port_ip_writing: {
      if (d.k_pressed(Key::Enter) && !port_ip_str.empty()) {
        size_t colon_p = port_ip_str.find_last_of(':') + 1;
        port =
            static_cast<unsigned short>(std::stoi(port_ip_str.substr(colon_p)));
        ip = port_ip_str.substr(0, colon_p - 1);
        state = State::Connect_to_server;
        server_connect_err = false;
      }
    } break;

    case State::Connect_to_server: {
      std::cout << "\nINFO: Connecting to server " << ip.toString() << ":"
                << port << "...\n";
      s = server.connect(ip, port, sf::seconds(5.f));
      if (s != sf::Socket::Status::Done) {
        state = State::Port_ip_writing;
        server_connect_err = true;
      } else {
        selector.add(server);
        state = State::Name_writing;
      }
    } break;

    case State::Name_writing: {
      if (d.k_pressed(Key::Enter) && !name.empty()) {
        state = State::Send_name_to_server;
      }
    } break;

    case State::Send_name_to_server: {
      send_packet = name;
      s = server.send(send_packet.c_str(), send_packet.size());

      if (s == sf::Socket::Status::Error) {
        err();
      } else if (s == sf::Socket::Status::NotReady) {
        std::cout << "INFO: Server busy...\n";
        return 0;
      }
      state = State::Lobby;
    } break;

    case State::Chatting: {
      if (d.k_pressed(Key::Enter)) {
        state = State::Lobby;
        send_packet = current_buf;
        current_buf.clear();
      }
    } break;

    case State::Lobby: {
      if (d.k_pressed(Key::Enter)) {
        state = State::Chatting;
      }
      if (selector.wait(sf::seconds(0.01f))) {
        if (selector.isReady(server)) {
          receive_packet.resize(MAX_PACKET_SIZE);
          size_t received{0};
          s = server.receive((void *)receive_packet.c_str(), MAX_PACKET_SIZE,
                             received);
          if (s == sf::Socket::Status::Error) {
            err();
          }
          receive_packet.resize(received);

          server_buf += fmt("SERVER: {}\n", receive_packet);
          std::cout << "SERVER: " << receive_packet << "\n";
        }
      }

      if (d.k_pressed(Key::Space)) {
        send_packet = "Hi!!!";
      }
      if (!send_packet.empty()) {
        s = server.send(send_packet.c_str(), send_packet.size());
        send_packet.clear();
        if (s == sf::Socket::Status::Error) {
          err();
        }
      }

    } break;

    default: {
      ASSERT(0);
    } break;
    }

    // draw
    switch (state) {
    case State::Port_ip_writing: {
      ui.begin(d.ss() / 2.f);

      if (server_connect_err) {
        ui.text("Couldn't connect to server...", TopCenter, DEFAULT_CHAR_SIZE,
                sf::Color::Red);
      }

      ui.text("Enter Server address:", TopCenter);
      ui.text(port_ip_str, TopCenter);

      ui.end();
    } break;

    case State::Connect_to_server: {
      d.draw_text(d.ss() / 2.f, "Connecting to server...", CenterCenter);
    } break;

    case State::Name_writing: {
      ui.begin(d.ss() / 2.f);

      ui.text("Enter Name:", TopCenter);
      ui.text(name, TopCenter);

      ui.end();
    } break;

    case State::Send_name_to_server: {
      d.draw_text(d.ss() / 2.f, "Sending name to server...", CenterCenter);
    } break;

    case State::Chatting: {
      ui.begin({d.width / 2.f, 0.f});

      ui.text("Chatting", TopCenter, 24);

      ui.spacing({0.f, DEFAULT_CHAR_SIZE * 4.f});
      ui.text(fmt("Send: {}", current_buf), TopCenter);

      ui.end();
    } break;

    case State::Lobby: {
      ui.begin({d.width / 2.f, 0.f});

      ui.text("Lobby", TopCenter, 24);

      ui.spacing({0.f, DEFAULT_CHAR_SIZE * 4.f});
      ui.text(server_buf, TopCenter);

      ui.end();
    } break;

    default: {
      ASSERT(0);
    } break;
    }

    // display
    d.display();
  }

  return 0;
}
