# loki-p2p-chat (C++17 & libjuice)

- Using C++ and libjuice, we implemented P2P processes and message transmission lightly.
- This project was not developed for service or commercialization.
- Signaling server design/implementation is required to develop based on this project. (It was created for MVP purposes.)
---

## Features
- Automatic ICE candidate gathering and exchange
- SDP Offer/Answer handling for P2P connection setup
- P2P message exchange over established connection
- **_Demo_** program showing full connection lifecycle:
    - ICE Gathering
    - SDP Offer/Answer exchange
    - Connection establishment
    - Message exchange

---

## Getting Started

### Prerequisites
- Linux (Ubuntu 20.04 or later recommended)
- Requires CMake 3.14 or later
- Requires C++17 or later compiler
- [libjuice](https://github.com/paullouisageneau/libjuice) (included as a submodule)
- [spdlog](https://github.com/gabime/spdlog) (included as a submodule)

---

### Build Instructions

```bash
# Update package lists
sudo apt update

# Clone the repository
git clone https://github.com/loki2001-dev/loki-p2p-chat.git
cd loki-p2p-chat

# Initialize submodules
git submodule update --init --recursive

# Build the project
. ./build_project.sh
```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.