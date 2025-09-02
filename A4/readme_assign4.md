# 🛰️ Routing Simulation: Distance Vector & Link State

## 👥 Team Members

| Name         | Roll Number |
|--------------|-------------|
| Aatman Jain  | 210018      |
| Astitva Roy  | 220248      |
| Ayush Patel  | 220269      |

---

## 📌 Overview

This project simulates two fundamental network routing algorithms:
1. **Distance Vector Routing (DVR)** – using Bellman-Ford style iterative updates.
2. **Link State Routing (LSR)** – using Dijkstra’s algorithm.

The input is a square cost matrix representing direct link costs between nodes. Output consists of individual routing tables computed using both algorithms.

---

## 🧰 Features

- ✅ Validates input matrix for squareness, non-negativity, and diagonal zeros.
- ✅ DVR: Uses distance and next hop tables; updates until convergence.
- ✅ LSR: Applies Dijkstra’s algorithm from each node to determine shortest paths.
- ✅ Output includes routing tables for each node with metrics and next hops.

---

## ❌ Not Implemented:

- ❌ Dynamic topology changes or real-time updates.
- ❌ Handling of negative edge weights.
- ❌ Graph visualization or GUI.
- ❌ Packet-level simulation or delay models.

---

## ⚙️ Design Decisions

- **Language**: C++ for speed and STL support.
- **Cost Representation**: `INF = 9999` used for unreachable nodes.
- **Matrix Format**: Square adjacency matrix with `0` on diagonals.
- **File I/O**: Uses `freopen()` to redirect standard input from file.
- **Exception Handling**: Input errors caught using `runtime_error`.

---

## 🔍 Key Functional Sections in `routing_sim.cpp`

- `validateMatrix(...)`: Ensures proper format of cost matrix.
- `printDVRTables(...)`: Simulates Distance Vector Routing and prints results.
- `printLSRTables(...)`: Simulates Link State Routing using Dijkstra’s algorithm.
- `main()`: Manages input file reading, validation, and function calls.

---

## 🔄 Code Flow (High-Level)

```mermaid
flowchart TD
    A[Start] --> B[Input filename]
    B --> C[freopen and read n x n matrix]
    C --> D[validateMatrix()]
    D --> E[printDVRTables()]
    E --> F[printLSRTables()]
    F --> G[Done]
🧪 Testing
✔️ Correctness Testing:
✅ Manually verified matrices from slides.

✅ Checked output consistency across different node configurations.

✅ Verified loopback/self-routes always show 0 and - for next hop.

🧪 Stress Testing:
✅ Tested up to 100 nodes with randomly generated matrices.

✅ Ensured convergence and stable output in DVR iterations.

🚧 Challenges Faced
Ensuring matrix validation was strict and caught all formatting errors.

Avoiding infinite loops in DVR due to poor convergence conditions.

Making Dijkstra’s path reconstruction accurate and readable.

Cleanly formatting output tables per node.

👤 Individual Contributions
Aatman Jain – Link State Routing implementation, testing.

Astitva Roy – Distance Vector Routing, matrix validation logic.

Ayush Patel – Input handling, exception management, output formatting.

📚 Sources Referenced
Computer Networking: A Top-Down Approach – Kurose & Ross

GeeksforGeeks: Dijkstra and Bellman-Ford articles

Lecture notes from [Your Course Name Here]

✍️ Declaration
We declare that the submitted code is our own work and has not been copied or derived from any unauthorized source. We have followed all academic integrity principles.

💬 Feedback
DVR convergence behavior was interesting to observe — often reaches stable state faster than expected.

Next Hop recovery in Dijkstra adds practical realism to LSR simulation.

Future improvement: Add visualization or support for dynamic updates.

🚀 Compilation & Execution
bash
Copy
Edit
g++ routing_sim.cpp -o routing_sim
./routing_sim
# Enter filename when prompted, e.g., matrix.txt

Input Format (file)
yaml
Copy
Edit
4
0 1 3 9999
1 0 1 4
3 1 0 2
9999 4 2 0
📤 Sample Output
sql
Copy
Edit
--- Distance Vector Routing Simulation ---
Node 0 Routing Table:
Dest    Metric  Next Hop
0       0       -
1       1       1
2       2       1
3       4       2

...

--- Link State Routing Simulation ---
Node 0 Routing Table:
Dest    Metric  Next Hop
1       1       1
2       2       1
3       4       2

...