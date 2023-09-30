#define SFML_HELPER_IMPLEMENTATION
#include <sfml-helper.hpp>

#define MAX_PACKET_SIZE 1024 * 5

void err() {
  std::cout << "ERROR!\n";
  exit(1);
}

int main(int argc, char *argv[]) {
  //

  unsigned short port{8888};
  sf::TcpSocket server;
  sf::SocketSelector selector;
  std::string name{""};

  // graphical display things
  std::string server_buf{};
  enum State { Name_writing, Chatting, Send_name_to_server };
  State state{State::Name_writing};

  sf::IpAddress ip{"127.0.0.1"};
  std::cout << "\nINFO: Connecting to server " << ip.toString() << ":" << port
            << "...\n";
  auto s = server.connect(ip, port, sf::seconds(10.f));
  if (s == sf::Socket::Status::Error) {
    err();
  }
  selector.add(server);

  std::string receive_packet{};
  std::string send_packet{};

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
      if (e.type == sf::Event::TextEntered) {
        if (state == State::Name_writing) {
          if (e.text.unicode == 8) { // backspace
            if (!name.empty())
              name.pop_back();
          } else if (32 <= e.text.unicode &&
                     e.text.unicode <= 127) { // only  ascii supported yet
            name.push_back((char)e.text.unicode);
          }
        }
      }
    }

    // clear
    d.clear();

    // update
    switch (state) {
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
      state = State::Chatting;
    } break;

    case State::Chatting: {
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

      send_packet.clear();
      if (d.k_pressed(Key::Space)) {
        send_packet = "Hi!!!";
      }
      if (!send_packet.empty()) {
        s = server.send(send_packet.c_str(), send_packet.size());
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
