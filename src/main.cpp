// C++11
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <set>

using namespace std;

const int N_MAX = 128;

template< typename T >
struct Edge {
  int from, to;
  T cost;
  Edge() {}
  Edge(int f, int t) : from(f), to(t), cost(1) {}
  Edge(int f, int t, T c) : from(f), to(t), cost(c) {}
  friend bool operator < (const Edge& lhs, const Edge& rhs) { return lhs.cost < rhs.cost; };
  friend bool operator > (const Edge& lhs, const Edge& rhs) { return rhs < lhs; };
  friend bool operator <= (const Edge& lhs, const Edge& rhs) { return !(lhs > rhs); };
  friend bool operator >= (const Edge& lhs, const Edge& rhs) { return !(lhs < rhs); };
};

template< typename T >
using Edges = std::vector< Edge< T > >;
template< typename T >
using Graph = std::vector< Edges< T > >;
using edges_t = Edges<int>;

class UnionFind {
public:
  std::vector<int> data; // sizeとparを同時に管理する
  UnionFind(int size) : data(size, -1) {}

  int find(int x) {
    return data[x] < 0 ? x : data[x] = find(data[x]);
  }

  void unite(int x, int y) {
    int px = find(x);
    int py = find(y);
    if (px != py) {
      if (data[py] < data[px]) std::swap(px, py);
      data[px] += data[py]; data[py] = px;
    }
  }

  bool same(int x, int y) {
    return find(x) == find(y);
  }

  int size(int x) {
    return -data[find(x)];
  }
};


inline int xy2id(int x, int y) { return (y<<7)+x; }
int matrix[N_MAX*N_MAX]; // (x,y)が繋がっている -> matrix[xy2id(x,y)] = 1, そうでなければ0

struct Solver {
  int N;
  double C;
  int K, E;
  edges_t edges;
  void read() {
    cin >> N >> C >> K >> E;
    for (int i=0; i<E; i++)
    {
      int from, to, distance;
      cin >> from >> to >> distance;
      edges.emplace_back(from, to, distance);
    }
    for (int y=0; y<N; y++) {
      for (int x=0; x<N; x++) {
      }
    }
  }
  void solve() { 
    // UnionFind uf(N);
    for (auto &e: edges) { 
      if (e.cost == 1) { 
        matrix[xy2id(e.from,e.to)] = 1;
        matrix[xy2id(e.to,e.from)] = 1;
      }
    }
  }
  void write() {
    cout << N << endl;
    for (int y=0; y<N; y++) {
      for (int x=0; x<N; x++) {
        cout << matrix[xy2id(x,y)];
      }
      cout << endl;
    }
    cout.flush();
  }
};

int main() {
  Solver solver;
  solver.read();
  solver.solve();
  solver.write();
}