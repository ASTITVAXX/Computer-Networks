#include <iostream>
#include <vector>
#include <climits>
#include <iomanip>
#include <queue>
#include <stdexcept>

using namespace std;

const int INF = 9999;

void validateMatrix(const vector<vector<int>>& mat) {
    int n = mat.size();
    for (int i = 0; i < n; ++i) {
        if (mat[i].size() != n) throw runtime_error("Matrix is not square.");
        for (int j = 0; j < n; ++j) {
            if (i == j && mat[i][j] != 0)
                throw runtime_error("Diagonal must be 0.");
            if (mat[i][j] < 0)
                throw runtime_error("Negative weights are not allowed.");
        }
    }
}

void printDVRTables(const vector<vector<int>>& costMatrix) {
    int n = costMatrix.size();
    vector<vector<int>> dist = costMatrix;
    vector<vector<int>> nextHop(n, vector<int>(n));

    // Initialize next hop table
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            nextHop[i][j] = (costMatrix[i][j] != INF && i != j) ? j : -1;

    bool updated;
    int iteration = 0;

    cout << "--- Distance Vector Routing Simulation ---\n";

    do {
        updated = false;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                for (int k = 0; k < n; ++k) {
                    if (dist[i][k] != INF && dist[k][j] != INF &&
                        dist[i][j] > dist[i][k] + dist[k][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        nextHop[i][j] = nextHop[i][k];
                        updated = true;
                    }
                }
            }
        }

        iteration++;

    } while (updated);

    for (int i = 0; i < n; ++i) {
        cout << "Node " << i << " Routing Table:\n";
        cout << "Dest\tMetric\tNext Hop\n";
        for (int j = 0; j < n; ++j) {
            cout << j << "\t";
            if (dist[i][j] >= INF)
                cout << "INF\t-\n";
            else if (i == j)
                cout << "0\t-\n";
            else
                cout << dist[i][j] << "\t" << nextHop[i][j] << "\n";
        }
        cout << endl;
    }
}

void printLSRTables(const vector<vector<int>>& costMatrix) {
    int n = costMatrix.size();
    cout << "--- Link State Routing Simulation ---\n";

    for (int src = 0; src < n; ++src) {
        vector<int> dist(n, INF), prev(n, -1);
        vector<bool> visited(n, false);
        dist[src] = 0;

        for (int i = 0; i < n; ++i) {
            int u = -1;
            for (int j = 0; j < n; ++j)
                if (!visited[j] && (u == -1 || dist[j] < dist[u]))
                    u = j;

            if (u == -1 || dist[u] == INF)
                break;

            visited[u] = true;

            for (int v = 0; v < n; ++v) {
                if (costMatrix[u][v] != INF && dist[v] > dist[u] + costMatrix[u][v]) {
                    dist[v] = dist[u] + costMatrix[u][v];
                    prev[v] = u;
                }
            }
        }

        cout << "Node " << src << " Routing Table:\n";
        cout << "Dest\tMetric\tNext Hop\n";

        for (int dest = 0; dest < n; ++dest) {
            if (src == dest) continue;

            if (dist[dest] == INF) {
                cout << dest << "\tINF\t-\n";
                continue;
            }

            int next = dest;
            while (prev[next] != -1 && prev[next] != src)
                next = prev[next];

            cout << dest << "\t" << dist[dest] << "\t" << (prev[dest] == -1 ? "-" : to_string(next)) << "\n";
        }

        cout << endl;
    }
}

int main() {
    string filename;
    cout << "Enter input filename: ";
    cin >> filename;

    freopen(filename.c_str(), "r", stdin);
    int n;
    cin >> n;

    if (n <= 0) {
        cerr << "Invalid number of nodes.\n";
        return 1;
    }

    vector<vector<int>> costMatrix(n, vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            cin >> costMatrix[i][j];

    try {
        validateMatrix(costMatrix);
        printDVRTables(costMatrix);
        printLSRTables(costMatrix);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
