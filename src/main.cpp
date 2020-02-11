// C++11
#include <cassert>
#include <cstdlib>
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <vector>

using namespace std;

const int N_MAX = 128;

struct Timer {
  static constexpr int64_t CYCLES_PER_SEC = 2800000000;
  const double LIMIT; // FIXME: 時間制限(s)
  int64_t start;
  Timer() : LIMIT(0.95) { reset(); }
  Timer(double limit) : LIMIT(limit) { reset(); }
  void reset() { start = getCycle(); }
  void plus(double a) { start -= (a * CYCLES_PER_SEC); }
  inline double get() { return (double)(getCycle() - start) / CYCLES_PER_SEC; }
  inline int64_t getCycle() {
    uint32_t low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((int64_t)low) | ((int64_t)high << 32);
  }
};

namespace logger {
inline void json_() {}
template<typename Key, typename Value, typename... Rest>
void json_(const Key& key, const Value& value, const Rest&... rest)
{
  std::cerr << "\"" << key << "\":\"" << value << "\"";
  if (sizeof...(Rest) > 0) std::cerr << ",";
  json_(rest...);
}

// example : json("key1", "foo", "key2", 3, "key", 4.0);
// {"key1":"foo","key2":"3","key":"4"}
template<typename... Args>
void json(const Args&... args)
{
#ifdef DEBUG
  std::cerr << "{"; json_(args...); std::cerr << "}\n";
#endif
}
} // namespace logger

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


const int INF = 999; // 二つの点の情報がない
const int NG = -1; // 二つの点が繋がっていない
inline int xy2id(int x, int y) { return (y<<7)+x; }
inline int revid(int id) { return xy2id(id>>7,id&127); }
inline int id2x(int id) { return id&127; }
inline int id2y(int id) { return id>>7; }
int matrix[N_MAX*N_MAX]; // (x,y)が繋がっている -> matrix[xy2id(x,y)] = 1, そうでなければ0
int dgold[N_MAX*N_MAX]; // matrix[xy2id(x,y)] = (x,y)間の与えられた距離. 情報なしはINF 
int dpred[N_MAX*N_MAX]; // matrix[xy2id(x,y)] = (x,y)間の推測した距離. 未確定はINF
bool done[N_MAX*N_MAX]; // done[xy2id(x,y)] = (x,y)間のdpredが確定したならtrue

struct Solver {
  int N;
  double C;
  int K, E;
  edges_t edges;
  vector<vector<int>> vertices; // 繋がっている点同士の集合
  Timer timer = Timer(5);
  void read() {
    cin >> N >> C >> K >> E;
    logger::json("N",N,"C",C,"K",K,"E",E);
    for (int i=0; i<E; i++)
    {
      int from, to, distance;
      cin >> from >> to >> distance;
      edges.emplace_back(from, to, distance);
    }

    // 初期化処理
    for (int y=0; y<N; y++) {
      for (int x=0; x<N; x++) {
        int id = xy2id(x,y);
        dgold[id] = INF;
        dpred[id] = INF;
      }
    }
    for (auto &e: edges) { 
      int id1 = xy2id(e.from,e.to);
      int id2 = xy2id(e.to,e.from);
      dgold[id1] = e.cost;
      dgold[id2] = e.cost;
    }
  }

  void solve() { 
    UnionFind uf(N);
    for (auto &e: edges) { 
      int id1 = xy2id(e.from,e.to);
      int id2 = xy2id(e.to,e.from);
      if (e.cost == 1) { 
        // cost = 1はつながっているとして確定させる
        dpred[id1] = 1;
        dpred[id2] = 1;
        matrix[id1] = 1;
        matrix[id2] = 1;
        done[id1] = true;
        done[id2] = true;
      }
      else { 
        // それ以外のcostはつながっていないとして確定させる
        dpred[id1] = NG;
        dpred[id2] = NG;
        matrix[id1] = 0;
        matrix[id2] = 0;
        done[id1] = true;
        done[id2] = true;
      }
      // 繋がっている点をまとめる
      if (e.cost > 0) { 
        uf.unite(e.from, e.to);
      }
    }
    // 与えられた辺情報で繋がっていない点同士は以後も繋げない
    for (int y=0; y<N; y++) {
      for (int x=0; x<N; x++) {
        if (!uf.same(x,y)) {
          int id1 = xy2id(x,y);
          int id2 = xy2id(y,x);
          dpred[id1] = NG;
          dpred[id2] = NG;
          matrix[id1] = 0;
          matrix[id2] = 0;
          done[id1] = true;
          done[id2] = true;
        }
      }
    }

    // 繋がっている点集合に分け、点集合が小さかったら総当たりで辺をつないでみる
    map<int, vector<int>> tmp_vmap; // 繋がっている点同士の集合
    for (int i = 0; i < N; ++i) {
      tmp_vmap[uf.find(i)].push_back(i);
      // logger::json("id",i,"s",uf.find(i));
    }
    for (auto it: tmp_vmap) {
      if (it.second.size() <= 2) continue;
      vertices.emplace_back(it.second);
    }
    for (auto &lv : vertices) {
      bruteforce(lv);
    }

    // 貪欲に辺を拡張する
    // expand_edges();
  }

  void expand_edges() {
    // (a,b)=d1、(b,c)=null、(a,c)=d1+1のときに(b,c)をつなぐ
    for (int i = 0; i < 5; ++i) {
      for (auto &e : edges) {
        if (e.cost <= 1) continue;
        bool need_expand = true;
        for (int v = 0; v < N; ++v) { 
          if (v == e.from || v == e.to) continue;
          int id1 = xy2id(e.from, v);
          int id2 = xy2id(v, e.to);
          if (dpred[id1] == NG || dpred[id2] == NG) continue;
          if (dgold[id1]+dgold[id2] == e.cost) {
            need_expand = false;
            break;
          }
        }
        if (!need_expand) continue;
        for (int v = 0; v < N; ++v) { 
          if (v == e.from || v == e.to) continue;
          int id1 = xy2id(e.from, v);
          int id2 = xy2id(v, e.to);
          if (done[id1] && done[id2]) continue;
          if (dpred[id1] == NG || dpred[id2] == NG) continue;
          if (dgold[id1] == e.cost-1 && dgold[id2] == INF) {
            dgold[id2] = 1;
            dgold[revid(id2)] = 1;
            matrix[id2] = 1;
            matrix[revid(id2)] = 1;
            done[id2] = true;
            done[revid(id2)] = true;
            break;
          }
          if (dgold[id2] == e.cost-1 && dgold[id1] == INF) {
            dgold[id1] = 1;
            dgold[revid(id1)] = 1;
            matrix[id1] = 1;
            matrix[revid(id1)] = 1;
            done[id1] = true;
            done[revid(id1)] = true;
            break;
          }
        }
      }
    }
  }

  int bfs(const vector<int>& lv, const vector<int>& connected, const vector<int>& choosed) { 
    set<int> connect;
    for (int id: connected) connect.insert(id);
    for (int id: choosed) connect.insert(id);
    vector<vector<int>> dist(lv.size(), vector<int>(lv.size(), INF));
    for (int v = 0; v < lv.size(); ++v) {
      dist[v][v] = 0;
      queue<int> q;
      q.push(v);
      vector<bool> visit(lv.size(), false);
      visit[v] = true;
      while (!q.empty()) { 
        int v1 = q.front(); q.pop();
        for (int v2 = 0; v2 < lv.size(); ++v2) {
          if (visit[v2]) continue;
          int id = xy2id(lv[v1], lv[v2]);
          if (connect.find(id) == connect.end()) continue;
          visit[v2] = true;
          dist[v][v2] = dist[v][v1]+1;
          q.push(v2);
        }
      }
    }

    // 実際に判明している距離との差分を計算
    int ret = 0;
    for (int v = 0; v < lv.size(); ++v) {
      for (int v2 = 0; v2 < lv.size(); ++v2) {
        if (v == v2) continue;
        int id = xy2id(lv[v], lv[v2]);
        if (dgold[id] == INF) {
          continue;
        }
        if (dist[v][v2] < dgold[id]) {
          // cerr << v << " " << v2 << " " << dist[v][v2] << " " << dgold[id] << endl;
          return -1;
        }
        if (dist[v][v2] == dgold[id]) {
          ret += 1;
        }
      }
    }
    return ret;
  }

  void bruteforce(const vector<int>& lv) {
    // 繋がっている点同士の集合の全ての辺の組み合わせを試し、実際の距離との差がもっとも小さい候補を選ぶ
    vector<int> free_ids;
    vector<int> connected_ids;
    for (int v1 : lv) {
      for (int v2 : lv) {
        int id = xy2id(v1,v2);
        if (dpred[id] == 1) connected_ids.push_back(id);
        if (v1 >= v2) continue;
        if (done[id]) continue;
        free_ids.push_back(id);
      }
    }
    int n = free_ids.size();
    // int best_dist = N_MAX*N_MAX; // 繋げた時の正解との距離の差
    int best_dist = bfs(lv, connected_ids, {}); // 繋げた時の正解との距離の差
    vector<int> best;
    if (lv.size() <= 7) {
      for (int mask = 0; mask < (1<<n); ++mask) { 
        if (best_dist == 0) {
          int cnt = __builtin_popcount(mask);
          if (cnt > best.size()) continue;
        }
        vector<int> ids;
        for (int i = 0; i < n; ++i) { 
          if (mask & (1<<i)) {
            ids.push_back(free_ids[i]);
            ids.push_back(revid(free_ids[i]));
          }
        }
        assert(2*__builtin_popcount(mask) == (int)ids.size()); // 辺の数が立ってるフラグの数と一致する
        int ret = bfs(lv, connected_ids, ids);
        if (best_dist < ret || (best_dist == ret && best.size() > ids.size())) {
          best_dist = ret;
          best = ids;
          // cerr << mask << " " << best_dist << " " << best.size() << endl;
        }
      }
      // cerr << best_dist << " " << best.size() << endl;
    }
    else { 
      // lv.size() >= 8
      if (K < 3) return;
      vector<bool> used(n, false);
      vector<int> best2;
      int cnt = 0;
      while (true) {
        double t = timer.get();
        if (t > timer.LIMIT) break;
        int best_i = -1;
        int tmp_best = best_dist;
        vector<int> local_best;
        for (int i = 0; i < n; ++i) { 
          if (used[i]) continue;
          vector<int> ids;
          ids.push_back(free_ids[i]);
          ids.push_back(revid(free_ids[i]));
          int ret = bfs(lv, connected_ids, ids);
          if (best_dist < ret) {
            best_dist = ret;
            local_best = ids;
            best_i = i;
            // cerr << mask << " " << best_dist << " " << best.size() << endl;
          }
        }

        if (tmp_best == best_dist) break;
        if (local_best.size() > 0) {
          used[best_i] = true;
          for (int id: local_best) {
            connected_ids.push_back(id);
            best2.push_back(id);
            // best.push_back(id);
          }
        }
      }
      for (int id : best2) {
        best.push_back(id);
      }
    }
    for (int id : best) { 
      matrix[id] = 1;
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