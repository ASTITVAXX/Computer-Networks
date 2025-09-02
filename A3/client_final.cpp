#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <ctime>
#include <errno.h>  // Added for errno, EAGAIN, EWOULDBLOCK

#define DEST_PORT 12345
#define SRC_PORT 54321

// Used to construct pseudo-header for TCP checksum calculation
struct PseudoHeader {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint8_t zero;
    uint8_t proto;
    uint16_t tcp_len;
};

// Function to compute checksum for pseudo-header + TCP header
unsigned short compute_checksum(unsigned short* data, int len) {
    unsigned long sum = 0;
    while (len > 1) {
        sum += *data++;
        len -= 2;
    }
    if (len == 1) {
        sum += *(uint8_t*)data;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return static_cast<unsigned short>(~sum);
}

// Constructs and dispatches the SYN segment
void dispatch_syn(int socket_fd, struct sockaddr_in& server) {
    char frame[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(frame, 0, sizeof(frame));

    struct iphdr* ip_header = reinterpret_cast<struct iphdr*>(frame);
    struct tcphdr* tcp_header = reinterpret_cast<struct tcphdr*>(frame + sizeof(struct iphdr));

    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = htons(sizeof(frame));
    ip_header->id = htons(45678);
    ip_header->frag_off = 0;
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_TCP;
    ip_header->saddr = inet_addr("127.0.0.1");
    ip_header->daddr = server.sin_addr.s_addr;

    tcp_header->source = htons(SRC_PORT);
    tcp_header->dest = server.sin_port;
    tcp_header->seq = htonl(200);
    tcp_header->ack_seq = 0;
    tcp_header->doff = 5;
    tcp_header->syn = 1;
    tcp_header->ack = 0;
    tcp_header->window = htons(8192);
    tcp_header->check = 0;

    // Calculate and set TCP checksum
    char temp[sizeof(PseudoHeader) + sizeof(struct tcphdr)];
    PseudoHeader pseudo;
    pseudo.src_ip = ip_header->saddr;
    pseudo.dst_ip = ip_header->daddr;
    pseudo.zero = 0;
    pseudo.proto = IPPROTO_TCP;
    pseudo.tcp_len = htons(sizeof(struct tcphdr));

    memcpy(temp, &pseudo, sizeof(pseudo));
    memcpy(temp + sizeof(pseudo), tcp_header, sizeof(struct tcphdr));
    tcp_header->check = compute_checksum(reinterpret_cast<unsigned short*>(temp), sizeof(temp));

    if (sendto(socket_fd, frame, sizeof(frame), 0, reinterpret_cast<struct sockaddr*>(&server), sizeof(server)) < 0) {
        perror("SYN send error");
        exit(EXIT_FAILURE);
    }
    std::cout << "[+] SYN segment dispatched (seq=200)" << std::endl;
}

// Handles incoming packet and verifies SYN-ACK
bool await_syn_ack(int socket_fd) {
    char incoming_data[65536];
    struct sockaddr_in origin;
    socklen_t origin_len = sizeof(origin);

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (true) {
        int bytes_received = recvfrom(socket_fd, incoming_data, sizeof(incoming_data), 0,
                                      reinterpret_cast<struct sockaddr*>(&origin), &origin_len);
        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "[-] Timeout: No SYN-ACK received from server." << std::endl;
                return false;
            }
            perror("recvfrom() failed");
            return false;
        }

        struct iphdr* ip_hdr = reinterpret_cast<struct iphdr*>(incoming_data);
        int ip_len = ip_hdr->ihl * 4;
        struct tcphdr* tcp_hdr = reinterpret_cast<struct tcphdr*>(incoming_data + ip_len);

        if (ntohs(tcp_hdr->dest) != SRC_PORT) {
            std::cout << "[-] Packet dropped: wrong destination port" << std::endl;
            continue;
        }
        if (ntohs(tcp_hdr->source) != DEST_PORT) {
            std::cout << "[-] Packet dropped: invalid source port " << ntohs(tcp_hdr->source) << std::endl;
            continue;
        }
        if (origin.sin_addr.s_addr != inet_addr("127.0.0.1")) {
            std::cout << "[-] Packet dropped: unexpected source IP" << std::endl;
            continue;
        }

        std::cout << "[+] TCP Flags -> SYN: " << tcp_hdr->syn
                  << ", ACK: " << tcp_hdr->ack
                  << ", SEQ: " << ntohl(tcp_hdr->seq)
                  << ", ACK_SEQ: " << ntohl(tcp_hdr->ack_seq) << std::endl;

        if (tcp_hdr->syn == 1 && tcp_hdr->ack == 1 &&
            ntohl(tcp_hdr->seq) == 400 && ntohl(tcp_hdr->ack_seq) == 201) {
            std::cout << "[+] Valid SYN-ACK received (seq=400, ack=201)" << std::endl;
            return true;
        } else {
            std::cout << "[-] Invalid SYN-ACK content or sequence mismatch" << std::endl;
        }
    }

    return false;
}

// Sends the last ACK packet to complete the handshake
void send_final_ack(int socket_fd, struct sockaddr_in& server) {
    char ack_packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(ack_packet, 0, sizeof(ack_packet));

    struct iphdr* ip_hdr = reinterpret_cast<struct iphdr*>(ack_packet);
    struct tcphdr* tcp_hdr = reinterpret_cast<struct tcphdr*>(ack_packet + sizeof(struct iphdr));

    ip_hdr->ihl = 5;
    ip_hdr->version = 4;
    ip_hdr->tos = 0;
    ip_hdr->tot_len = htons(sizeof(ack_packet));
    ip_hdr->id = htons(45679);
    ip_hdr->frag_off = 0;
    ip_hdr->ttl = 64;
    ip_hdr->protocol = IPPROTO_TCP;
    ip_hdr->saddr = inet_addr("127.0.0.1");
    ip_hdr->daddr = server.sin_addr.s_addr;

    tcp_hdr->source = htons(SRC_PORT);
    tcp_hdr->dest = server.sin_port;
    tcp_hdr->seq = htonl(600);
    tcp_hdr->ack_seq = htonl(401);
    tcp_hdr->doff = 5;
    tcp_hdr->ack = 1;
    tcp_hdr->window = htons(8192);
    tcp_hdr->check = 0;

    char temp[sizeof(PseudoHeader) + sizeof(struct tcphdr)];
    PseudoHeader pseudo;
    pseudo.src_ip = ip_hdr->saddr;
    pseudo.dst_ip = ip_hdr->daddr;
    pseudo.zero = 0;
    pseudo.proto = IPPROTO_TCP;
    pseudo.tcp_len = htons(sizeof(struct tcphdr));
    memcpy(temp, &pseudo, sizeof(pseudo));
    memcpy(temp + sizeof(pseudo), tcp_hdr, sizeof(struct tcphdr));
    tcp_hdr->check = compute_checksum(reinterpret_cast<unsigned short*>(temp), sizeof(temp));

    if (sendto(socket_fd, ack_packet, sizeof(ack_packet), 0,
               reinterpret_cast<struct sockaddr*>(&server), sizeof(server)) < 0) {
        perror("ACK send error");
        exit(EXIT_FAILURE);
    }
    std::cout << "[+] Final ACK sent (seq=600, ack=401). TCP handshake done." << std::endl;
}

int main() {
    if (geteuid() != 0) {
        std::cerr << "[-] Please run with sudo (root privileges required)." << std::endl;
        return EXIT_FAILURE;
    }

    int raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (raw_socket < 0) {
        perror("Raw socket creation failed");
        exit(EXIT_FAILURE);
    }

    int flag = 1;
    if (setsockopt(raw_socket, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag)) < 0) {
        perror("IP_HDRINCL option error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_info;
    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(DEST_PORT);
    server_info.sin_addr.s_addr = inet_addr("127.0.0.1");

    dispatch_syn(raw_socket, server_info);

    if (await_syn_ack(raw_socket)) {
        send_final_ack(raw_socket, server_info);
    } else {
        std::cerr << "[-] Valid SYN-ACK not received. Handshake failed." << std::endl;
    }

    close(raw_socket);
    return 0;
}

