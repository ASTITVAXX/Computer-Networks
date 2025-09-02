#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <errno.h>

#define PORT 12345
#define BACKLOG 10
#define BUFFER_SIZE 1024

// ----------------- Global Variables -----------------
std::mutex cout_mutex;  // For synchronized console output
std::vector<std::thread> client_threads;

std::mutex clients_mutex;
// client_socket -> username
std::unordered_map<int, std::string> clients;
// username -> client_socket
std::unordered_map<std::string, int> usernames;
// group_name -> set of client_sockets
std::unordered_map<std::string, std::unordered_set<int>> groups;
// username -> password from users.txt
std::unordered_map<std::string, std::string> users;

// ----------------- Utility Functions ----------------
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \n\r\t");
    size_t end = str.find_last_not_of(" \n\r\t");
    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }
    return str.substr(start, end - start + 1);
}

void load_users(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open " << filename << std::endl;
        exit(1);
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t delimiter = line.find(':');
        if (delimiter != std::string::npos) {
            std::string username = line.substr(0, delimiter);
            std::string password = line.substr(delimiter + 1);
            users[username] = password;
        }
    }
    file.close();
    std::cout << "Users loaded successfully!" << std::endl;
}

// ----------------- Authentication --------------------
bool authenticate_client(int client_socket, std::string &username_out) {
    char buffer[BUFFER_SIZE] = {0};

    // Prompt for username
    std::string prompt = "Connected to the server .\nEnter username : ";
    send(client_socket, prompt.c_str(), prompt.size(), 0);
    if (recv(client_socket, buffer, BUFFER_SIZE, 0) <= 0) {
        return false; // client disconnected or error
    }
    username_out = trim(buffer);

    // Prompt for password
    memset(buffer, 0, BUFFER_SIZE);
    prompt = "Enter password : ";
    send(client_socket, prompt.c_str(), prompt.size(), 0);
    if (recv(client_socket, buffer, BUFFER_SIZE, 0) <= 0) {
        return false;
    }
    std::string password = trim(buffer);

    // Validate
    if (users.find(username_out) != users.end() && users[username_out] == password) {
        // According to examples, we show "Welcome to the chat server !"
        std::string welcome = "Welcome to the chat server !\n";
        send(client_socket, welcome.c_str(), welcome.size(), 0);
        return true;
    } else {
        // Examples show "Authentication failed ." rather than "Disconnecting..."
        std::string failure = "Authentication failed .\n";
        send(client_socket, failure.c_str(), failure.size(), 0);
        return false;
    }
}

// ----------------- Announcements ---------------------
void announce_user_join(const std::string& username, int joining_socket) {
    // Example: "bob has joined the chat ."
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::string msg = username + " has joined the chat .\n";

    // Send this to all other connected clients
    for (auto& [sock, uname] : clients) {
        if (sock != joining_socket) {
            send(sock, msg.c_str(), msg.size(), 0);
        }
    }
}

void announce_user_leave(const std::string& username) {
    // e.g., "frank has left the chat ."
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::string msg = username + " has left the chat .\n";
    for (auto& [sock, uname] : clients) {
        send(sock, msg.c_str(), msg.size(), 0);
    }
}

// ----------------- Broadcast -------------------------
void broadcast_message(const std::string& message, int sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& [client_socket, user] : clients) {
        if (client_socket != sender_socket) {
            send(client_socket, message.c_str(), message.size(), 0);
        }
    }
}

// ----------------- Private Messaging -----------------
void private_message(const std::string& sender, const std::string& recipient, const std::string& message) {
    // Example target: "[ bob ]: alice great , I will join"
    // but from the perspective of the receiving user, it should be: "[ alice ]: <message>"
    std::lock_guard<std::mutex> lock(clients_mutex);

    if (usernames.find(recipient) != usernames.end()) {
        int recipient_socket = usernames[recipient];
        std::string full_message = "[ " + sender + " ]: " + message + "\n";
        send(recipient_socket, full_message.c_str(), full_message.size(), 0);
    } else {
        int sender_socket = usernames[sender];
        std::string error_msg = "User " + recipient + " is not connected.\n";
        send(sender_socket, error_msg.c_str(), error_msg.size(), 0);
    }
}

// ----------------- Group Management ------------------
void create_group(const std::string& raw_group, int client_socket, const std::string& user) {
    
	(void)user;
	// Example output: "Group CS425 created ."
    std::string group_name = trim(raw_group);
    std::lock_guard<std::mutex> lock(clients_mutex);

    if (groups.find(group_name) != groups.end()) {
        // If group exists
        std::string err = "Group " + group_name + " already exists.\n";
        send(client_socket, err.c_str(), err.size(), 0);
    } else {
        groups[group_name].insert(client_socket);
        std::string msg = "Group " + group_name + " created .\n";
        send(client_socket, msg.c_str(), msg.size(), 0);
    }
}

void join_group(const std::string& raw_group, int client_socket, const std::string& user) {
    
	(void)user;
	// Example output: "You joined the group CS425 ."
    std::string group_name = trim(raw_group);
    std::lock_guard<std::mutex> lock(clients_mutex);

    if (groups.find(group_name) == groups.end()) {
        std::string err = "Group " + group_name + " does not exist.\n";
        send(client_socket, err.c_str(), err.size(), 0);
    } else {
        if (groups[group_name].count(client_socket)) {
            // Already in group
            std::string info = "You joined the group " + group_name + " .\n";
            send(client_socket, info.c_str(), info.size(), 0);
        } else {
            groups[group_name].insert(client_socket);
            std::string msg = "You joined the group " + group_name + " .\n";
            send(client_socket, msg.c_str(), msg.size(), 0);
        }
    }
}

void leave_group(const std::string& raw_group, int client_socket, const std::string& user) {
    
	(void)user;
	// "You left the group CS425 ."
    std::string group_name = trim(raw_group);
    std::lock_guard<std::mutex> lock(clients_mutex);

    if (groups.find(group_name) == groups.end() ||
        groups[group_name].count(client_socket) == 0) {
        std::string err = "You are not a member of the group " + group_name + ".\n";
        send(client_socket, err.c_str(), err.size(), 0);
    } else {
        groups[group_name].erase(client_socket);
        std::string msg = "You left the group " + group_name + " .\n";
        send(client_socket, msg.c_str(), msg.size(), 0);
    }
}

void group_message(const std::string& raw_group, const std::string& raw_message,
                   int sender_socket, const std::string& sender_name)
{
	(void)sender_name;
    // Example output: "[ Group CS425 ]: Hi , Welcome to CS425"
    std::string group_name = trim(raw_group);
    std::string message = trim(raw_message);

    std::lock_guard<std::mutex> lock(clients_mutex);
    if (groups.find(group_name) == groups.end() ||
        groups[group_name].find(sender_socket) == groups[group_name].end()) {
        std::string err = "You are not a member of the group " + group_name + ".\n";
        send(sender_socket, err.c_str(), err.size(), 0);
        return;
    }
    std::string formatted_msg = "[ Group " + group_name + " ]: " + message + "\n";
    for (int member_socket : groups[group_name]) {
        if (member_socket != sender_socket) {
            send(member_socket, formatted_msg.c_str(), formatted_msg.size(), 0);
        }
    }
}

// ----------------- Client Handler ---------------------
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};

    // 1) Authenticate
    std::string username;
    if (!authenticate_client(client_socket, username)) {
        close(client_socket);
        return; // invalid or disconnected
    }

    // 2) Add to global maps
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients[client_socket] = username;
        usernames[username] = client_socket;
    }

    // 3) Announce to others that <username> has joined the chat
    announce_user_join(username, client_socket);

    // 4) Communication loop
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) {
            // user disconnected
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                usernames.erase(clients[client_socket]);
                clients.erase(client_socket);
            }
            close(client_socket);

            // announce user left
            announce_user_leave(username);
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Client disconnected: " << username << std::endl;
            }
            break;
        }

        std::string msg = trim(buffer);

        // ------------------- Broadcast ------------------
        if (msg.rfind("/broadcast ", 0) == 0) {
            std::string broadcast_msg = trim(msg.substr(11));
            // e.g. "Broadcast: Hello, everyone!"
            // But the example doesn't strictly show broadcast usage, so we'll keep it:
            std::string ack_msg = "You broadcasted: " + broadcast_msg + "\n";
            send(client_socket, ack_msg.c_str(), ack_msg.size(), 0);

            std::string formatted = "Broadcast: " + broadcast_msg + "\n";
            broadcast_message(formatted, client_socket);
            continue;
        }

        // ------------------- Private Msg ----------------
        if (msg.rfind("/msg ", 0) == 0) {
            size_t spacePos = msg.find(' ', 5);
            if (spacePos != std::string::npos) {
                std::string recipient = trim(msg.substr(5, spacePos - 5));
                std::string pm = trim(msg.substr(spacePos + 1));
                // e.g. "[ bob ]: hey!"
                private_message(username, recipient, pm);
            } else {
                std::string err = "Invalid format. Use /msg <username> <message>\n";
                send(client_socket, err.c_str(), err.size(), 0);
            }
            continue;
        }

        // ------------------- Create Group ---------------
        if (msg.rfind("/create_group ", 0) == 0) {
            std::string g = msg.substr(14);
            create_group(g, client_socket, username);
            continue;
        }

        // ------------------- Join Group -----------------
        if (msg.rfind("/join_group ", 0) == 0) {
            std::string g = msg.substr(12);
            join_group(g, client_socket, username);
            continue;
        }

        // ------------------- Leave Group ----------------
        if (msg.rfind("/leave_group ", 0) == 0) {
            std::string g = msg.substr(13);
            leave_group(g, client_socket, username);
            continue;
        }

        // ------------------- Group Msg ------------------
        if (msg.rfind("/group_msg ", 0) == 0) {
            size_t spacePos = msg.find(' ', 11);
            if (spacePos != std::string::npos) {
                std::string group_name = msg.substr(11, spacePos - 11);
                std::string group_msg = msg.substr(spacePos + 1);
                group_message(group_name, group_msg, client_socket, username);
            } else {
                std::string err = "Invalid format. Use /group_msg <group_name> <message>\n";
                send(client_socket, err.c_str(), err.size(), 0);
            }
            continue;
        }

        // ------------------- Unknown Command ------------
        std::string error_msg = "Unknown command.\n";
        send(client_socket, error_msg.c_str(), error_msg.size(), 0);
    }
}

// ----------------- Main -------------------------------
int main() {
    // 1) Load users from file
    load_users("users.txt");

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed.\n";
        return -1;
    }
    std::cout << "Socket created successfully.\n";

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 2) Bind
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }
    std::cout << "Bind successful on port " << PORT << ".\n";

    // 3) Listen
    if (listen(server_fd, BACKLOG) < 0) {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }
    std::cout << "Server is listening for connections...\n";

    // 4) Accept in a loop
    while (true) {
        socklen_t addrlen = sizeof(address);
        int new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            std::cerr << "Failed to accept connection.\n";
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Client connected successfully!\n";
        }

        client_threads.emplace_back(std::thread(handle_client, new_socket));
    }

    // Optional: if you do a graceful shutdown approach
    for (auto &th : client_threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    close(server_fd);
    return 0;
}
