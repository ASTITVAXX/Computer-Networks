
# Chat Server

## Overview

This is a multi-client chat server that enables real-time communication between connected users. It supports user authentication, message broadcasting, and private messaging. The server efficiently handles concurrent client connections using socket programming.

## Features

- **Multi-client support** – Multiple clients can connect and communicate simultaneously.
- **User authentication** – Clients must log in with a valid username and password.
- **Message broadcasting** – Users can send messages to all connected clients.
- **Private messaging** – Clients can send direct messages to specific users.
- **Client connection handling** – The server manages multiple client connections using threads.
- **Graceful disconnection** – If a client disconnects, the server handles it appropriately.

---

## Code Structure

### 1. `server_grp.cpp` (Chat Server)

The server is responsible for:

- Creating and managing a listening socket.
- Accepting new client connections.
- Authenticating users before allowing them to chat.
- Handling incoming messages and distributing them.
- Managing a list of active clients and tracking usernames.

**Key components of the server:**

- **Socket creation:** The server initializes a TCP socket and binds it to a specified port.
- **Listening and accepting clients:** It listens for incoming connections and accepts new clients.
- **Thread management:** Each client connection is handled in a separate thread.
- **Message handling:** Processes incoming messages and sends them to the appropriate recipients.

**How the server code works:**

1. **Initialization:** The server sets up a TCP socket and binds it to a specific port, making it ready to accept incoming connections.

2. **Listening for Connections:** It enters a listening state, waiting for clients to connect.

3. **Accepting Clients:** Upon a client's connection request, the server accepts the connection and spawns a new thread dedicated to handling that client's communication.

4. **User Authentication:** In the dedicated thread, the server prompts the client for a username and password. It verifies these credentials against a predefined list or database.

5. **Message Handling:** Once authenticated, the server listens for messages from the client. Depending on the message type (broadcast or private), it forwards the message to all clients or a specific client.

6. **Graceful Disconnection:** If a client disconnects or encounters an error, the server ensures that resources are freed, and the client's thread is properly terminated.

### 2. `client_grp.cpp` (Chat Client)

Each client connects to the server and can:

- Log in using a username and password.
- Send messages to all connected users.
- Send private messages to specific users.
- Receive messages from other clients in real-time.

**Key components of the client:**

- **Socket connection:** The client connects to the server using a TCP socket.
- **User authentication:** The client must enter valid credentials before accessing the chat.
- **Message sending & receiving:** The client continuously sends and listens for messages from the server.

**How the client code works:**

1. **Initialization:** The client sets up a TCP socket and attempts to connect to the server's specified IP address and port.

2. **User Authentication:** Upon connection, the client is prompted to enter a username and password, which are sent to the server for verification.

3. **Message Handling:** After successful authentication, the client enters a loop where it can send messages to the server. Simultaneously, it listens for incoming messages from the server, displaying them to the user in real-time.

4. **Private Messaging:** The client can specify a recipient for a private message using a designated command format (e.g., `/msg <username> <message>`). The client code parses this command and sends the appropriate request to the server.

5. **Graceful Disconnection:** If the user decides to exit (e.g., by typing `/exit`), the client notifies the server of the disconnection and closes the socket gracefully.

---

## Prerequisites

Ensure you have the following dependencies installed on your Ubuntu system:

```sh
sudo apt update
sudo apt install build-essential
```

---

## Compilation and Execution (Ubuntu)

Follow these steps to compile and run the chat server and client:

1. **Clone the repository:** Fork and clone the repository from the GitHub link provided in the assignment PDF.

2. **Place source files:** Copy the `server_grp.cpp` and `client_grp.cpp` files into the cloned directory.

3. **Compile the project:** Open a terminal in the project directory and run:

   ```sh
   make
   ```

   This will generate two executable files:

   - `server_grp` → Chat server
   - `client_grp` → Chat client

4. **Start the server:** Run the following command in the terminal:

   ```sh
   ./server_grp
   ```

   The server will start listening for incoming client connections.

5. **Start a client:** Open a separate terminal instance and execute:

   ```sh
   ./client_grp
   ```

   The client program will prompt for a username and password. Enter valid credentials to connect.

6. **Multiple clients:** Repeat Step 5 in separate terminals to connect multiple clients.

7. **Using the chat functionalities:**

   - Type a message and press Enter to send it to all clients.
   - Use a command (e.g., `/msg <username> <message>`) to send a private message.
   - Type `/exit` to disconnect from the server.

---

## Troubleshooting

- **Make command fails:** Ensure you have `g++` installed:

  ```sh
  sudo apt install g++
  ```

- **Port already in use:** Change the port number in `server_grp.cpp` and `client_grp.cpp` and recompile.

- **Permission errors:** Grant execution permissions:

  ```sh
  chmod +x server_grp client_grp
  ```

