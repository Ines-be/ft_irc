# IRC (C++98) — RFC-Compliant IRC Server & Client

## 💬 Overview
IRC++ is a fully **RFC-compliant Internet Relay Chat (IRC)** server and client implemented in **C++98**.  
It uses `select()` to handle multiple clients concurrently in a **non-blocking** manner over **TCP/IP**.  
Users can chat privately, join channels, and manage them through standard IRC commands and modes.  
Channel operators can set options such as invite-only mode, channel key, topic restrictions, operator privileges, and user limits.  
The project includes a built-in **moderation bot** present in every channel that automatically kicks users using forbidden words.  
It can be tested with standard IRC clients like **irssi**, or simply with **netcat**.  

## 🚀 Features
- Full compliance with **IRC Protocol** (RFC-based implementation)
- Non-blocking multi-client handling using **select()**
- TCP/IP socket communication
- Core IRC commands implemented: `PASS`, `NICK`, `USER`, `JOIN`, `PART`, `QUIT`, `PRIVMSG`, `NOTICE`, `KICK`, `INVITE`, `TOPIC`, `MODE`
- Channel Management:  
    - Join/leave channels, private messaging, and topic setting
    - Channel operators with permissions to manage users and modes
- Supported channel modes:
    - +i (invite-only), +t (topic protected), +k (key), +o (operator), +l (user limit)
- File transfer between users
- Built-in bot that kicks users sending forbidden words in channels
- Compatible with:
    - **netcat** (manual testing)
    - **irssi** (fully functional IRC client)
- Makefile for building
- Launch with `./irc <port> <password>`

## 📖 Command Reference
| Command                                       | Description                           |
| --------------------------------------------- | ------------------------------------- |
| `PASS <password>`                             | Authenticate with the server password |
| `NICK <nickname>`                             | Set or change nickname                |
| `USER <username> <mode> <unused> :<realname>` | Register username and real name       |
| `JOIN <channel> [key]`                        | Join a channel (use key if required)  |
| `PART <channel> [:reason]`                    | Leave a channel                       |
| `PRIVMSG <target> :<message>`                 | Send a private or channel message     |
| `NOTICE <target> :<message>`                  | Send a notice message                 |
| `TOPIC <channel> [:topic]`                    | Set or view channel topic             |
| `KICK <channel> <user> [:reason]`             | Kick a user from a channel            |
| `INVITE <nick> <channel>`                     | Invite a user to a channel            |
| `QUIT [:message]`                             | Disconnect from the server            |
| `MODE <channel> <modes> [params]`             | Change or view channel modes          |

## 🧑‍💻 Channel Modes & Operator Actions
| Mode         | Description                                           |
| ------------ | ----------------------------------------------------- |
| `+i`         | Invite-only — users must be invited to join           |
| `+t`         | Topic protected — only operators can change the topic |
| `+k <key>`   | Channel key required to join                          |
| `+o <nick>`  | Give operator status to a user                        |
| `-o <nick>`  | Remove operator status                                |
| `+l <limit>` | Limit the number of users in the channel              |

**Operator Commands:**
- *Promote/demote operator:* MODE #chan +o nick / MODE #chan -o nick
- *Set channel key:* MODE #chan +k secretkey
- *Remove channel key:* MODE #chan -k
- *Set topic:* TOPIC #chan :New topic
- *Remove topic:* TOPIC #chan :
- *Kick user:* KICK #chan nick :reason
- *Invite user:* INVITE nick #chan
All mode and operator changes are broadcast to the channel and follow standard IRC reply formatting.

## ⚙️ Requirements
- C++98 compatible compiler (c++)
- POSIX sockets (Linux, macOS, BSD)
- Make
- Optional: netcat, irssi for testing

## 🛠️ Setup
```bash

# Build the server
make

# Start the server
./irc <port> <password>

# Connect using netcat
nc localhost <port>

# Connect using irssi
irssi --connect localhost -p <port>