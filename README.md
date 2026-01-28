<div align="center">
  <h2>ft_irc – 42 project IRC Server</h2>
  <br>
</div>

![Language](https://img.shields.io/badge/language-C%2B%2B98-blue)
![Network](https://img.shields.io/badge/network-IRC-orange)
![42](https://img.shields.io/badge/school-42-black)
![Status](https://img.shields.io/badge/status-completed-brightgreen)

> **ft_irc** is a fully functional **IRC server** written in **C++98**, compliant with the IRC protocol specifications.

---

## 📖 Table of Contents
- [Description](#-description)
- [Features](#-features)
- [Installation](#-installation)
- [Usage](#-usage)
- [Supported Commands](#-supported-commands)
- [Project Structure](#-project-structure)
- [Roadmap](#-roadmap)
- [Credits](#-credits)
- [License](#-license)

---

## 📝 Description
The goal of this project is to implement a **multi-client IRC server** from scratch.  
It focuses on:

- TCP/IP networking
- Poll-based multiplexing
- Client authentication
- Channel and user management
- Strict protocol parsing

---

## ✨ Features
- ✅ Multiple clients simultaneously.
- ✅ Authentication with password.
- ✅ Nickname and username management.
- ✅ Channel creation and moderation.
- ✅ Private and channel messaging.
- ✅ Operator privileges.
- ✅ Robust error handling.

---

## ⚙️ Installation
Requirements:
- Linux or macOS
- `c++` compiler with C++98 support
- `make`

Clone and compile:
```bash
git clone https://github.com/LogUmi/ft_irc.git
cd ft_irc
make
```

---

## 🖥 Usage
Run the server:
```bash
./ircserv <port> <password>
```

Connect using an IRC client:
```bash
nc localhost <port>
```

---

## ⌨️ Supported Commands
- PASS
- NICK
- USER
- JOIN
- PART
- PRIVMSG
- NOTICE
- KICK
- INVITE
- TOPIC
- MODE
- QUIT

---

## 📂 Project Structure
```
.
├── inc/
│   ├── Server.hpp
│   ├── User.hpp
│   ├── Channel.hpp
│   └── Commands.hpp
├── src/
│   ├── Server.cpp
│   ├── User.cpp
│   ├── Channel.cpp
│   ├── Commands.cpp
│   └── main.cpp
├── Makefile
└── README.md
```

---

## 🚀 Roadmap
This project represents the final validated version for the 42 curriculum.

---

## 👤 Credits
Project developed by:
- **Loïc Gérard** – 📧 lgerard@student.42perpignan.fr  
  [École 42 Perpignan](https://42perpignan.fr)

---

## 📜 License
This project is released under the MIT License.

> ⚠️ **Note for students**  
> Writing your own IRC server is the only way to truly understand networking.
