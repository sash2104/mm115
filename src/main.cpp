// C++11
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <unordered_map>
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
  Timer() : LIMIT(2.95) { reset(); }
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

struct XorShift {
  unsigned int x, y, z, w;
  double nL[65536];
  XorShift() { init(); }
  void init()
  {
    x = 314159265; y = 358979323; z = 846264338; w = 327950288;
    double n = 1 / (double)(2 * 65536);
    for (int i = 0; i < 65536; i++) {
      nL[i] = log(((double)i / 65536) + n);
    }
  }
  inline unsigned int next() { unsigned int t=x^x<<11; x=y; y=z; z=w; return w=w^w>>19^t^t>>8; }
  inline double nextLog() { return nL[next()&0xFFFF]; }
  inline int nextInt(int m) { return (int)(next()%m); } // [0, m)
  int nextInt(int min, int max) { return min+nextInt(max-min+1); } // [min, max]
  inline double nextDouble() { return (double)next()/((long long)1<<32); } // [0, 1]
};

XorShift rnd;

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
using graph_t = vector<edges_t>;

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
int dist[N_MAX][N_MAX];

struct State {
  int invalid = -1000000;
  int bid, bscore, br, bid2; // backup
  int score;
  vector<int> lv;
  vector<int> fixed_ids, free_ids, used_ids;
  int V, E;
  vector<bool> used, fixed;
  unordered_map<int, int> rid;
  State() {}
  State(const vector<int> &lv, const vector<int> &fixed_ids, const vector<int> &free_ids): 
    lv(lv), fixed_ids(fixed_ids), free_ids(free_ids), V(lv.size()), E(free_ids.size()), used(N_MAX*N_MAX,false), fixed(N_MAX*N_MAX,false) {
    for (int i = 0; i < V; ++i) { rid[lv[i]] = i; }
    score = calcScore();
  }

  int update() {
    int j = rnd.nextInt(10);
    if (j > 6 && used_ids.size() > 0 && free_ids.size() > 1) {
      // 辺の入れ替え
      br = 1;
      int i = rnd.nextInt(used_ids.size());
      int id = used_ids[i];
      int i2 = rnd.nextInt(E);
      int id2 = free_ids[i2];
      while (fixed[id2] || id2 == id || used[id2]) { 
        i2 = rnd.nextInt(E);
        id2 = free_ids[i2];
      }
      return update(id, id2);
    }
    else {
      br = 2;
      int i, id;
      if (j > 5 && used_ids.size() > 0) {
        i = rnd.nextInt(used_ids.size());
        id = used_ids[i];
      }
      else {
        i = rnd.nextInt(E);
        id = free_ids[i];
        while (fixed[id]) { 
          i = rnd.nextInt(E);
          id = free_ids[i];
        }
      }
      return update(id);
    }
  }

  int update(int id, int id2) {
    // cerr << "[upd2]" << id << " " << id2 << endl;
    // backup
    bid = id;
    bid2 = id2;
    bscore = score;
    {
      int rev = revid(id);
      if (used[id]) { // deletion
        used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), id), used_ids.end());
        used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), rev), used_ids.end());
      }
      else { // insertion
        used_ids.push_back(id);
        used_ids.push_back(rev);
      }
      // 辺の使用/不使用を反転させる
      used[id] = !used[id];
      used[rev] = !used[rev];
    }
    {
      int rev = revid(id2);
      if (used[id2]) { // deletion
        used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), id2), used_ids.end());
        used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), rev), used_ids.end());
      }
      else { // insertion
        used_ids.push_back(id2);
        used_ids.push_back(rev);
      }
      // 辺の使用/不使用を反転させる
      used[id2] = !used[id2];
      used[rev] = !used[rev];
    }
    int tmp = calcScore();
    if (tmp == invalid) return invalid;
    score = tmp;
    return score-bscore;
  }
  int update(int id) {
    // backup
    bid = id;
    bscore = score;

    int rev = revid(id);
    if (used[id]) { // deletion
      used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), id), used_ids.end());
      used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), rev), used_ids.end());
    }
    else { // insertion
      used_ids.push_back(id);
      used_ids.push_back(rev);
    }
    // 辺の使用/不使用を反転させる
    used[id] = !used[id];
    used[rev] = !used[rev];
    // cerr << "[upd]" << id << " " << used_ids.size() << endl;

    int tmp = calcScore();
    if (tmp == invalid) return invalid;
    score = tmp;
    return score-bscore;
  }

  int calcScore() {
    // graphの構築
    vector<vector<int>> g(V);
    for (int id: fixed_ids) {
      int v1 = rid[id2x(id)];
      int v2 = rid[id2y(id)];
      g[v1].push_back(v2);
      g[v2].push_back(v1);
    }
    for (int id: used_ids) {
      int v1 = rid[id2x(id)];
      int v2 = rid[id2y(id)];
      g[v1].push_back(v2);
      g[v2].push_back(v1);
    }
    // bfs
    for (int v = 0; v < V; ++v) {
      for (int v2 = 0; v2 < V; ++v2) {
        dist[v][v2] = INF;
      }
    }

    for (int v = 0; v < V; ++v) {
      dist[v][v] = 0;
      queue<int> q;
      q.push(v);
      vector<bool> visit(lv.size(), false);
      visit[v] = true;
      while (!q.empty()) { 
        int v1 = q.front(); q.pop();
        for (int v2 : g[v1]) {
          if (visit[v2]) continue;
          int id = xy2id(lv[v1], lv[v2]);
          visit[v2] = true;
          dist[v][v2] = dist[v][v1]+1;
          // if (dgold[id] != INF && dist[v][v2] < dgold[id]) return -1;
          q.push(v2);
        }
      }
    }

    // 実際に判明している距離との差分を計算
    int ret = 0;
    for (int v = 0; v < V; ++v) {
      for (int v2 = 0; v2 < V; ++v2) {
        if (v >= v2) continue;
        int id = xy2id(lv[v], lv[v2]);
        // if (dist[v][v2] == INF || dgold[id] == NG) {
        //   continue;
        // }
        if (dgold[id] == INF) {
          // ret -= V;
          // ret -= dist[v][v2];
          // if (dist[v][v2] < INF) {
          //   ret -= 1; // 未確定の部分を確定しない辺を優先したい
          // }
          // else { 
          //   ret += 1;
          // }
          continue;
        }
        // if (dist[v][v2] < dgold[id]) {
        //   ret -= 99;
        // }
        // if (dist[v][v2] == dgold[id]) {
        //   ret += 100;
        // }
        if (dist[v][v2] == INF) {
          ret -= dgold[id];
          continue;
        }
        ret -= abs(dgold[id]-dist[v][v2]);
        if (dist[v][v2] < dgold[id]) {
          ret -= abs(dgold[id]-dist[v][v2]);
        }
        // if (dist[v][v2] != dgold[id]) {
        //   ret -= V;
        //   // cerr << v << " " << v2 << " " << dist[v][v2] << " " << dgold[id] << endl;
        //   // return -10000;
        // }
      }
    }
    ret -= used_ids.size();

    ret *= 10;
    return ret;
  }

  void revert() { 
    score = bscore;
    {
      int rev = revid(bid);
      if (used[bid]) { // deletion
        used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), bid), used_ids.end());
        used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), rev), used_ids.end());
      }
      else { // insertion
        used_ids.push_back(bid);
        used_ids.push_back(rev);
      }
      used[bid] = !used[bid];
      used[rev] = !used[rev];
    }
    if (br == 1) {
      // cerr << "[rvt2]" << bid << " " << bid2 << endl;
      int rev = revid(bid2);
      if (used[bid2]) { // deletion
        used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), bid2), used_ids.end());
        used_ids.erase(std::remove(used_ids.begin(), used_ids.end(), rev), used_ids.end());
      }
      else { // insertion
        used_ids.push_back(bid2);
        used_ids.push_back(rev);
      }
      used[bid2] = !used[bid2];
      used[rev] = !used[rev];
    }
    // cerr << "[rvt]" << bid << " " << used_ids.size() << endl;
  }

  void write() {
  }
};

struct SASolver {
  double startTemp = 30;
  double endTemp = 10;
  Timer timer = Timer(4.5);
  State best;
  vector<int> best_ids;
  SASolver() { init(); }
  SASolver(double st, double et): startTemp(st), endTemp(et) { init(); }
  SASolver(double st, double et, double limit): startTemp(st), endTemp(et), timer(limit) { init(); }
  void init() {
    logger::json("tag", "param", "type", "sa", "start_temp", startTemp, "end_temp", endTemp, "limit", timer.LIMIT);
  } // 初期化処理をここに書く

  void solve(State &state) {
    double t;
    double score = state.calcScore();
    best = state;
    double bestScore = score;
    int counter = 0;
    while ((t = timer.get()) < timer.LIMIT) // 焼きなまし終了時刻までループ
    {
      for (int i = 0; i < 1000; ++i) { // 時間計算を間引く
        int diff = state.update();
        if (diff == -1000000) { // 絶対に更新しない場合
          state.revert();
          continue;
        }

        // 最初t=0のときは、スコアが良くなろうが悪くなろうが、常にnextを使用
        // 最後t=timer.LIMITのときは、スコアが改善したときのみ、nextを使用
        double T = startTemp + (endTemp - startTemp) * t / timer.LIMIT;
        // スコアが良くなった or 悪くなっても強制遷移
        if (diff >= T*rnd.nextLog())
        {
          score += diff;
          if (bestScore < score) {
            bestScore = score;
            best_ids = state.used_ids;
            // best = state;
            // logger::json("tag", "!", "time", t, "counter", counter, "score", score, "e", best.used_ids.size());
          }
        }
        else { state.revert(); }
        ++counter;
      }
    }
    logger::json("tag", "result", "counter", counter, "score", bestScore);
  }
};

struct Solver {
  int N;
  double C;
  int K, E;
  int n_given; // はじめに与えられた辺のうち繋がっている辺の数
  int n_sa; // 焼きなましの必要なかず
  edges_t edges;
  graph_t graph;
  vector<vector<int>> vertices; // 繋がっている点同士の集合
  Timer timer = Timer(5);
  void read() {
    cerr << timer.get() << endl;
    n_given = 0;
    cin >> N >> C >> K >> E;
    graph.resize(N);
    logger::json("N",N,"C",C,"K",K,"E",E);
    for (int i=0; i<E; i++)
    {
      int from, to, distance;
      cin >> from >> to >> distance;
      edges.emplace_back(from, to, distance);
      graph[from].emplace_back(from, to, distance);
      graph[to].emplace_back(to, from, distance);
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
      if (e.cost > 0) ++n_given;
      int id1 = xy2id(e.from,e.to);
      int id2 = xy2id(e.to,e.from);
      dgold[id1] = e.cost;
      dgold[id2] = e.cost;
    }
    n_given *= 2;
    cerr << timer.get() << endl;
  }

  void solve() { 
    // cerr << C*N << " " << C << " " << N << endl;
    // return;
    UnionFind uf(N);
    for (auto &e: edges) { 
      int id1 = xy2id(e.from,e.to);
      int id2 = xy2id(e.to,e.from);
      dpred[id1] = e.cost;
      dpred[id2] = e.cost;
      if (e.cost == 1) { 
        // cost = 1はつながっているとして確定させる
        matrix[id1] = 1;
        matrix[id2] = 1;
        done[id1] = true;
        done[id2] = true;
      }
      else {
        // それ以外のcostはつながっていないとして確定させる
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
    n_sa = 0;
    for (auto &lv : vertices) {
      if (lv.size() > 7) ++n_sa;
    }
    cerr << timer.get() << endl;
    for (auto &lv : vertices) {
      bruteforce(lv);
    }
    cerr << timer.get() << endl;
    // 貪欲に辺を拡張する
    // expand_edges();
  }

  void expand_edges() {
    // (a,b)=d1、(b,c)=null、(a,c)=d1+1のときに(b,c)をつなぐ
    for (int i = 0; i < 10; ++i) {
      if (timer.get() > 8) break;
      bool expanded = false;
      for (auto &e : edges) {
        if (expanded) break;
        if (e.cost <= 1) continue;
        // cerr << e.from << " " << e.to << " " << e.cost << endl;
        bool need_expand = true;
        for (int v = 0; v < N; ++v) { 
          if (v == e.from || v == e.to) continue;
          int id1 = xy2id(e.from, v);
          int id2 = xy2id(v, e.to);
          if (dpred[id1] == NG || dpred[id2] == NG) continue;
          if (dpred[id1]+dpred[id2] == e.cost) {
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
          if (dpred[id1] == INF) swap(id1, id2);
          if (dpred[id1] == e.cost-1 && dpred[id2] == INF) {
            int v1 = id2x(id2);
            int v2 = id2y(id2);
            bool can_expand = true;
            for (auto &e : graph[v1]) { 
              if (e.to == v2) continue;
              int id3 = xy2id(v2, e.to);
              if (dpred[id3] == NG || dpred[id3] == INF) continue;
              if (dpred[id3] > e.cost+1) { 
                can_expand = false;
                break;
              }
            }
            for (auto &e : graph[v2]) { 
              if (e.to == v1) continue;
              int id3 = xy2id(v1, e.to);
              if (dpred[id3] == NG || dpred[id3] == INF) continue;
              if (dpred[id3] > e.cost+1) { 
                can_expand = false;
                break;
              }
            }
            if (!can_expand) continue;
            // cerr << e.from << " " << v << " " << e.to << " " << e.cost << " " << dpred[id1] << " " << dpred[id2] << endl;
            dpred[id2] = 1;
            dpred[revid(id2)] = 1;
            matrix[id2] = 1;
            matrix[revid(id2)] = 1;
            done[id2] = true;
            done[revid(id2)] = true;
            expanded = true;
            break;
          }
        }
      }
    }
  }

  int bfs(const vector<int>& lv, const vector<int>& connected, const vector<int>& choosed) { 
    vector<vector<int>> g(lv.size());
    unordered_map<int, int> rid;
    for (int i = 0; i < lv.size(); ++i) {
      rid[lv[i]] = i;
    }
    for (int id: connected) {
      int v1 = rid[id2x(id)];
      int v2 = rid[id2y(id)];
      g[v1].push_back(v2);
      g[v2].push_back(v1);
    }
    for (int id: choosed) {
      int v1 = rid[id2x(id)];
      int v2 = rid[id2y(id)];
      g[v1].push_back(v2);
      g[v2].push_back(v1);
    }
    vector<vector<int>> dist(lv.size(), vector<int>(lv.size(), INF));
    for (int v = 0; v < lv.size(); ++v) {
      dist[v][v] = 0;
      queue<int> q;
      q.push(v);
      vector<bool> visit(lv.size(), false);
      visit[v] = true;
      while (!q.empty()) { 
        int v1 = q.front(); q.pop();
        for (int v2 : g[v1]) {
          if (visit[v2]) continue;
          int id = xy2id(lv[v1], lv[v2]);
          visit[v2] = true;
          dist[v][v2] = dist[v][v1]+1;
          // if (dgold[id] != INF && dist[v][v2] < dgold[id]) return -1;
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
        if (dist[v][v2] == INF || dgold[id] == NG) {
          continue;
        }
        if (dgold[id] == INF) {
          if (dist[v][v2] < INF) --ret; // 未確定の部分を確定しない辺を優先したい
          continue;
        }
        if (dist[v][v2] < dgold[id]) {
          // cerr << v << " " << v2 << " " << dist[v][v2] << " " << dgold[id] << endl;
          return -10000;
        }
        if (dist[v][v2] == dgold[id]) {
          ret += 100;
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
        // assert(best_dist <= n_given);
        // if (best_dist == n_given) {
        //   int cnt = __builtin_popcount(mask);
        //   if (cnt > best.size()) continue;
        // }
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
      int t = 9/n_sa;
      SASolver sa(30, 10, t);
      State s(lv, connected_ids, free_ids);
      sa.solve(s);
      for (int id : sa.best_ids) {
        best.push_back(id);
      }
      // lv.size() >= 8
      // if (K < 3 && lv.size() < 30) return;
      // vector<bool> used(n, false);
      // vector<int> best2;
      // int cnt = 0;
      // while (true) {
      //   double t = timer.get();
      //   // cerr << t << endl;
      //   if (t > timer.LIMIT) break;
      //   int best_i = -1;
      //   int tmp_best = best_dist;
      //   vector<int> local_best;
      //   for (int i = 0; i < n; ++i) { 
      //     if (used[i]) continue;
      //     vector<int> ids;
      //     ids.push_back(free_ids[i]);
      //     ids.push_back(revid(free_ids[i]));
      //     int ret = bfs(lv, connected_ids, ids);
      //     if (best_dist < ret) {
      //       best_dist = ret;
      //       local_best = ids;
      //       best_i = i;
      //       // cerr << best_dist << endl;
      //     }
      //   }

      //   if (tmp_best == best_dist) break;
      //   if (local_best.size() > 0) {
      //     used[best_i] = true;
      //     for (int id: local_best) {
      //       connected_ids.push_back(id);
      //       best2.push_back(id);
      //       // best.push_back(id);
      //     }
      //   }
      // }
      // for (int id : best2) {
      //   best.push_back(id);
      // }
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