#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_map>

using namespace std;

struct Router {
    int id, x, y, cost, coverage, capacity, selected = 0, load = 0;
};

struct User {
    int id, x, y;
};

struct Edge {
    int u, v;
    double weight;
    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
};

struct DSU {
    vector<int> parent;
    DSU(int n) { parent.resize(n); for (int i = 0; i < n; ++i) parent[i] = i; }
    int find(int x) { return x == parent[x] ? x : parent[x] = find(parent[x]); }
    void unite(int x, int y) { parent[find(x)] = find(y); }
};

vector<Router> readRouters(string filename) {
    vector<Router> routers;
    ifstream file(filename);
    string line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        stringstream ss(line);
        string val;
        Router r;
        getline(ss, val, ','); r.id = stoi(val);
        getline(ss, val, ','); r.x = stoi(val);
        getline(ss, val, ','); r.y = stoi(val);
        getline(ss, val, ','); r.cost = stoi(val);
        getline(ss, val, ','); r.coverage = stoi(val);
        getline(ss, val, ','); r.capacity = stoi(val);
        routers.push_back(r);
    }
    return routers;
}

vector<User> readUsers(string filename) {
    vector<User> users;
    ifstream file(filename);
    string line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        stringstream ss(line);
        string val;
        User u;
        getline(ss, val, ','); u.id = stoi(val);
        getline(ss, val, ','); u.x = stoi(val);
        getline(ss, val, ','); u.y = stoi(val);
        users.push_back(u);
    }
    return users;
}

int knapsackSelectRouters(vector<Router>& routers, int budget) {
    int n = routers.size();
    vector<vector<int>> dp(n+1, vector<int>(budget+1, 0));

    for (int i = 1; i <= n; i++) {
        for (int j = 0; j <= budget; j++) {
            if (routers[i-1].cost <= j)
                dp[i][j] = max(dp[i-1][j], dp[i-1][j - routers[i-1].cost] + routers[i-1].coverage);
            else
                dp[i][j] = dp[i-1][j];
        }
    }

    int j = budget;
    for (int i = n; i > 0; i--) {
        if (dp[i][j] != dp[i-1][j]) {
            routers[i-1].selected = 1;
            j -= routers[i-1].cost;
        }
    }
    return dp[n][budget];
}

double distance(int x1, int y1, int x2, int y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

vector<Edge> buildMST(vector<Router>& routers) {
    vector<Edge> edges, mst;
    vector<int> idx;
    for (int i = 0; i < routers.size(); i++)
        if (routers[i].selected) idx.push_back(i);

    for (int i = 0; i < idx.size(); i++) {
        for (int j = i + 1; j < idx.size(); j++) {
            Edge e = {idx[i], idx[j], distance(routers[idx[i]].x, routers[idx[i]].y, routers[idx[j]].x, routers[idx[j]].y)};
            edges.push_back(e);
        }
    }
    sort(edges.begin(), edges.end());
    DSU dsu(routers.size());
    for (Edge e : edges) {
        if (dsu.find(e.u) != dsu.find(e.v)) {
            dsu.unite(e.u, e.v);
            mst.push_back(e);
        }
    }
    return mst;
}

void assignUsers(vector<Router>& routers, vector<User>& users, vector<pair<int, int>>& assignments) {
    for (User& u : users) {
        int best = -1, minLoad = 1e9;
        for (Router& r : routers) {
            if (!r.selected) continue;
            if (distance(u.x, u.y, r.x, r.y) <= r.coverage && r.load < r.capacity) {
                if (r.load < minLoad) {
                    best = r.id;
                    minLoad = r.load;
                }
            }
        }
        if (best != -1) {
            routers[best].load++;
            assignments.push_back({u.id, best});
        }
    }
}

void writeCSV(const string& filename, const vector<Router>& routers) {
    ofstream file(filename);
    file << "id,x,y,cost,coverage,capacity,load\n";
    for (const Router& r : routers)
        if (r.selected)
            file << r.id << "," << r.x << "," << r.y << "," << r.cost << "," << r.coverage << "," << r.capacity << "," << r.load << "\n";
}

void writeMST(const string& filename, const vector<Edge>& mst, const vector<Router>& routers) {
    ofstream file(filename);
    file << "from,to,distance\n";
    for (auto& e : mst)
        file << routers[e.u].id << "," << routers[e.v].id << "," << e.weight << "\n";
}

void writeAssignments(const string& filename, const vector<pair<int, int>>& assignments) {
    ofstream file(filename);
    file << "user_id,router_id\n";
    for (auto& a : assignments)
        file << a.first << "," << a.second << "\n";
}

int main() {
    string routerFile = "routers.csv", userFile = "users.csv";
    vector<Router> routers = readRouters(routerFile);
    vector<User> users = readUsers(userFile);

    int budget = 30000;
    int totalCoverage = knapsackSelectRouters(routers, budget);
    vector<Edge> mst = buildMST(routers);
    vector<pair<int, int>> assignments;
    assignUsers(routers, users, assignments);

    writeCSV("selected_routers.csv", routers);
    writeMST("mst_edges.csv", mst, routers);
    writeAssignments("user_assignments.csv", assignments);

    ofstream summary("summary.txt");
    summary << "Total routers selected: " << count_if(routers.begin(), routers.end(), [](Router& r) { return r.selected; }) << "\n";
    summary << "Total users assigned: " << assignments.size() << "\n";
    double totalCable = 0;
    for (auto& e : mst) totalCable += e.weight;
    summary << "Total cable length (MST): " << totalCable << "\n";
    summary << "Total coverage value: " << totalCoverage << "\n";
    summary.close();

    cout << "Project run complete. Outputs written to CSV files." << endl;
    return 0;
}
