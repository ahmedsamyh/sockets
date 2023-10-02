#define SFML_HELPER_IMPLEMENTATION
#include <sfml-helper.hpp>

using namespace sh;

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
  bool verified[2]{false, false};
  bool send_verified[2]{false, false};

  // graphical display things
  std::string chat_log{};
  std::string current_buf{};
  enum State {
    Port_ip_writing,
    Connect_to_server,
    Name_writing,
    Verify_1,
    Lobby,
  };
  State state{State::Port_ip_writing};

  sf::Socket::Status s{};
  std::string receive_packet{};
  std::string send_packet{};
  std::string port_ip_str{"127.0.0.1:8888"};

  // errors
  std::string server_connect_err{};

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
        d.handle_ipv4(e, port_ip_str);
      } else if (state == State::Name_writing) {
        d.handle_text(e, name);
      } else if (state == State::Lobby) {
        d.handle_text(e, current_buf);
      }
    }

    // clear
    d.clear();

    // update
    switch (state) {
    case State::Port_ip_writing: {
      if (d.k_held(Key::LControl) && d.k_pressed(Key::V)) {
        std::string clip = static_cast<std::string>(sf::Clipboard::getString());
        bool safe = true;
        for (size_t i = 0; i < clip.size(); ++i) {
          if (!std::isdigit(clip[i]) && !(clip[i] == ':') &&
              !(clip[i] == '.')) {
            safe = false;
          }
        }
        if (safe)
          port_ip_str += clip;
      }
      if (d.k_pressed(Key::Enter) && !port_ip_str.empty()) {
        size_t colon_p = port_ip_str.find_last_of(':') + 1;
        port =
            static_cast<unsigned short>(std::stoi(port_ip_str.substr(colon_p)));
        ip = port_ip_str.substr(0, colon_p - 1);
        state = State::Connect_to_server;
        server_connect_err.clear();
      }
    } break;

    case State::Connect_to_server: {
      std::cout << "\nINFO: Connecting to server " << ip.toString() << ":"
                << port << "...\n";
      s = server.connect(ip, port, sf::seconds(5.f));
      if (s != sf::Socket::Status::Done) {
        state = State::Port_ip_writing;
        server_connect_err = "Failed to connect to server...";
      } else {
        selector.add(server);
        state = State::Name_writing;
      }
    } break;

    case State::Name_writing: {
      if (d.k_pressed(Key::Enter) && !name.empty()) {
        state = State::Verify_1;
      }
    } break;

    case State::Verify_1: {
      if (!verified[0]) {
        // send "Verify 1 {NAME}" to server if not already send
        if (!send_verified[0]) {
          send_packet = FMT("Verify 1,{}", name);
          s = server.send(send_packet.c_str(), send_packet.size());
          if (s == sf::Socket::Status::Error) {
            err();
          }
          send_verified[0] = true;
          print("INFO: Verify 1 send to server...\n");
        }
        // if it was already send, wait until server responds with "Verify 1
        // Success"
        else {
          if (selector.wait(sf::seconds(0.01f))) {
            if (selector.isReady(server)) {
              receive_packet.resize(MAX_PACKET_SIZE);
              size_t received{0};
              s = server.receive((void *)receive_packet.c_str(),
                                 MAX_PACKET_SIZE, received);
              if (s == sf::Socket::Status::Error) {
                err();
              }
              receive_packet.resize(received);

              if (receive_packet == "Verify 1 Success") {
                print("INFO: Verify 1 Success!\n");
                verified[0] = true;
              } else { // fail
                server_connect_err = "Verify 1 failed...";
                state = State::Port_ip_writing;
              }
            }
          }
        }
      } else if (!verified[1]) {
        // send "Verify 2 {NAME}" to server if not already send
        if (!send_verified[1]) {
          send_packet = FMT("Verify 2,{}", name);
          s = server.send(send_packet.c_str(), send_packet.size());
          if (s == sf::Socket::Status::Error) {
            err();
          }
          send_verified[1] = true;
          print("INFO: Verify 2 send to server...\n");
        }
        // if it was already send, wait until server responds with "Verify 1
        // Success"
        else {
          print("INFO: Receiving until server responds...\n");
          if (selector.wait(sf::seconds(0.01f))) {
            if (selector.isReady(server)) {
              receive_packet.resize(MAX_PACKET_SIZE);
              size_t received{0};
              s = server.receive((void *)receive_packet.c_str(),
                                 MAX_PACKET_SIZE, received);
              if (s == sf::Socket::Status::Error) {
                err();
              }
              receive_packet.resize(received);

              if (receive_packet == "Verify 2 Success") {
                print("INFO: Verify 2 Success!\n");
                verified[1] = true;
              } else { // fail
                server_connect_err = "Verify 2 failed...";
                state = State::Port_ip_writing;
              }
            }
          }
        }
      }
      if (verified[0] && verified[1]) {
        state = State::Lobby;
      }
    } break;

    case State::Lobby: {
      if (d.k_pressed(Key::Enter)) {
        send_packet = current_buf;
        current_buf.clear();
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

          if (receive_packet != FMT("{}: Verify 1,{}", name, name) &&
              receive_packet != FMT("{}: Verify 2,{}", name, name)) {
            chat_log += receive_packet + '\n';
          }
        }
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

      if (!server_connect_err.empty()) {
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

    case State::Verify_1: {
      d.draw_text(d.ss() / 2.f, "Verifying...", CenterCenter);
    } break;

    case State::Lobby: {
      ui.begin({d.width / 2.f, 0.f});

      ui.text("Lobby", TopCenter, 24);

      ui.spacing({0.f, DEFAULT_CHAR_SIZE * 4.f});
      ui.text(chat_log, TopCenter);

      ui.end();
      // ----------
      ui.begin({d.width / 2.f, d.height - DEFAULT_CHAR_SIZE * 2.f});

      ui.text(FMT("{}: {}", name, current_buf), TopCenter);

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
