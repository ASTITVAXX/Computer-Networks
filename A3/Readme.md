🔁 Raw TCP Three-Way Handshake (Client Side) – C++ Implementation

A deep-dive into low-level networking with a manual TCP client that performs the TCP three-way handshake using raw sockets and custom TCP/IP headers, all built from scratch in C++.

👥 Team Members

Name

Roll Number

Aatman Jain

210018

Astitva Roy

220248

Ayush Patel

220269

📌 Overview

This project manually replicates the TCP three-way handshake using raw sockets, bypassing the OS's TCP stack. It offers a ground-up understanding of how connection-oriented protocols like TCP work, allowing students and enthusiasts to visualize and learn the step-by-step process of establishing a reliable connection in networking.

This is not a typical socket program using connect() or send() — instead, it manually constructs packets, calculates checksums, sets flags, and interacts at the network and transport layers of the OSI model.

🧰 Features

✅ Assignment Features Implemented:

Raw socket creation and packet crafting.

Construction of TCP and IP headers manually.

Checksum calculation for TCP integrity.

Three-way handshake: SYN → SYN-ACK → ACK.

Logging of all handshake steps.

Timeout handling for SYN-ACK reception.

❌ Not Implemented:

Persistent data transfer post-handshake.

Multi-threaded or process-based architecture.

⚙️ Design Decisions

No Threads/Processes: Since the assignment is a raw socket TCP handshake client, multithreading is not required. It interacts with a single server, so a single-threaded, sequential logic is sufficient.

@32 Design Decision: Not applicable directly in this context as no @32-like architecture was needed.

No Synchronization Needed: As it’s a single-threaded design with no shared data among multiple threads or processes, synchronization constructs were not used.



🔍 Key Functional Sections in .cpp

Section

Description

calculate_checksum()

Computes the TCP checksum for reliability

build_ip_header()

Creates and fills a custom IP header

build_tcp_header()

Creates a TCP header with specified flags

send_syn()

Sends a SYN packet with a raw socket

receive_syn_ack()

Waits for and parses a SYN-ACK

send_ack()

Sends the final ACK to complete handshake

main()

Entry point calling the above in sequence

🔄 Code Flow (High-Level + Visual)

Main Flow Diagram:

┌──────────────┐
│ Start        │
└─────┬────────┘
      ↓
┌──────────────┐
│ send_syn()   │
└─────┬────────┘
      ↓
┌──────────────┐
│ receive_syn_ack() │
└─────┬────────┘
      ↓
┌──────────────┐
│ send_ack()   │
└─────┬────────┘
      ↓
┌──────────────┐
│  Handshake Done │
└──────────────┘

🧪 Testing

✔️ Correctness Testing:

Verified three-way handshake on 127.0.0.1:12345 using a custom or netcat server.

Printed TCP flags and sequence numbers.

🧪 Stress Testing:

Tried repeated connections in loop.

Simulated server not responding (to test timeout and retry handling).

🚧 Challenges Faced

🔍 Checksum Bugs: Initially faced errors due to incorrect pseudo-header alignment. Solved using bitwise debugging.

📚 Understanding TCP/IP Headers: Required deep dives into RFCs to understand byte offsets and flag manipulation.

🧪 Testing Limitations: Testing required raw privileges and server-side configuration.

🛠 Low-Level Debugging: Required Wireshark to verify actual packet-level behavior.

📏 Server-Side Restrictions (Hypothetical if extended to server):

Limitation

Value

Max Clients

1 (single handshake implementation)

Max Groups

N/A

Max Group Members

N/A

Max Message Size

N/A (not part of handshake)

👤 Individual Contributions

Name

Contribution

Aatman Jain

Design logic, checksum, header construction

Astitva Roy

Implementation, socket configuration, ACK handling

Ayush Patel

Testing, debugging, documentation, README preparation

All members collaborated via GitHub and peer-review of pull requests.

📚 Sources Referenced

Beej's Guide to Network Programming

[TCP/IP Illustrated - W. Richard Stevens]

RFC 793 - TCP Protocol Specification

Linux Man Pages

Stack Overflow threads on raw sockets

✍️ Declaration

We declare that we have not indulged in plagiarism. The entire implementation and documentation are original, with references clearly mentioned. Any external code has been properly credited.

💬 Feedback

This assignment gave us a powerful low-level understanding of TCP operations.

Testing was slightly tough due to raw socket permissions and system-level access.

We would love more assignments involving packet inspection or raw UDP communication as a follow-up!

🚀 Compilation & Execution

🔧 Compile

g++ tcp_handshake_raw.cpp -o tcp_handshake_raw

🏃 Run the Client

sudo ./tcp_handshake_raw

⚠️ Note: You must run this with sudo due to raw socket usage.

📤 Sample Output

If handshake is successful:

[+] Sent SYN (seq=200)
[+] TCP Flags:  SYN: 1 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 400
[+] Received SYN-ACK from server 127.0.0.1 (seq=400, ack=201)
[+] Sent ACK (seq=600, ack=401) -> Handshake complete.

If server doesn't respond:

Timeout: No SYN-ACK received. Handshake failed
[-] Failed to receive valid SYN-ACK from server.

