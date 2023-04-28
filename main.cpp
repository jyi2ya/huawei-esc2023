// copyright (c) 2023 ligen131 <i@ligen131.com>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <bitset>
#include <vector>
#include <assert.h>
#include <ctime>

/************************************* I/O Begin *************************************/
template <typename T>
inline T read(T &a) {
  char c;
  bool neg = false;
  a = 0;
  for (c = getchar(); c < '0' || c > '9'; neg |= c == '-', c = getchar())
    ;
  for (; c >= '0' && c <= '9'; a = a * 10 - '0' + c, c = getchar())
    ;
  if (neg)
    a = -a;
  return a;
}
template <typename T, typename... Args>
inline void read(T &a, Args &...args) {
  read(a);
  read(args...);
}
inline long long read() {
  long long a;
  return read(a);
}
char WRITE_BUF[40];
template <typename T>
inline void write(T a) {
  if (!a)
    return putchar('0'), void();
  if (a < 0)
    putchar('-'), a = -a;
  int len = 0;
  while (a)
    WRITE_BUF[++len] = a % 10 + '0', a /= 10;
  for (int i = len; i; --i)
    putchar(WRITE_BUF[i]);
}
inline void write_() { return; }
template <typename T, typename... Args>
inline void write_(T a, Args... args) {
  write(a);
  putchar(' ');
  write_(args...);
}
inline void writeln() {
  putchar('\n');
  return;
}
template <typename T, typename... Args>
inline void writeln(T a, Args... args) {
  write(a);
  if (sizeof...(args))
    putchar(' ');
  writeln(args...);
}
/************************************* I/O End *************************************/

/************************************* Definition Begin *************************************/
#define MAX_NODE_NUM 5000
#define MAX_EDGE_NUM 5000
#define MAX_EXTRA_EDGE_NUM 20000
#define MAX_MISSION_NUM 10000
#define MAX_CHANNEL_NUM 80
#define MAX_DIS 1000
#define INF_INT (int)(1<<30)

int n, m;
int missionNum; // T
int channelNumEachEdge; // P
int maxDis; // D
int extraEdgeNum; // Y
int minDisBetweenNodes[MAX_NODE_NUM][MAX_NODE_NUM];

struct Edge {
  int x, y, dis;
  std::bitset<MAX_CHANNEL_NUM> channelNotUsed;
} edges[MAX_EDGE_NUM + MAX_EXTRA_EDGE_NUM];
int _y(Edge &e, int x) {
  return e.x == x ? e.y : e.x;
}

struct Mission {
  bool solved;
  int s, t;
  int ansChannel;
  std::vector<int> ansEdges;
  std::vector<int> ansAmplifier;
} missions[MAX_MISSION_NUM];
std::vector<int> edgeFromNode[MAX_NODE_NUM];
/************************************* Definition End *************************************/

std::bitset<MAX_CHANNEL_NUM> INIT_CHANNEL;

void addEdgeToNode(int edgeIndex) {
  int x = edges[edgeIndex].x, y = edges[edgeIndex].y;
  edgeFromNode[x].push_back(edgeIndex);
  edgeFromNode[y].push_back(edgeIndex);
}

void input() {
  read(n, m, missionNum, channelNumEachEdge, maxDis);
  int x, y, dis;
    for (int i = 0; i < channelNumEachEdge; i++) {
      INIT_CHANNEL[i] = 1;
    }
  for (int i = 0; i < m; i++) {
    read(x, y, dis);
    edges[i].x = x;
    edges[i].y = y;
    edges[i].dis = dis;
    if (!minDisBetweenNodes[x][y] || dis < minDisBetweenNodes[x][y]) {
      minDisBetweenNodes[x][y] = minDisBetweenNodes[y][x] = dis;
    }
    edges[i].channelNotUsed = INIT_CHANNEL;
    addEdgeToNode(i);
  }
  for (int i = 0; i < missionNum; i++) {
    read(missions[i].s, missions[i].t);
    missions[i].solved = false;
  }
}

void addExtraEdge(int x, int y) {
  extraEdgeNum++;
  if (extraEdgeNum > 5) exit(0);
  // std::cout<<"Extra Edge Added ";writeln(extraEdgeNum, x, y, minDisBetweenNodes[x][y]);
  edges[m + extraEdgeNum - 1] = (Edge){
    x, y, minDisBetweenNodes[x][y], INIT_CHANNEL
  };
  addEdgeToNode(m + extraEdgeNum - 1);
}

/** Using Dijkstra **/
struct Node {
  int x, dis, edgeNum, from;
  std::bitset<MAX_CHANNEL_NUM> channel;
  int invalidEdgeUsed;
  std::vector<int>ansEdges;
};
int minDis[MAX_NODE_NUM], minEdgeNum[MAX_NODE_NUM];
bool operator< (const Node &a, const Node &b) {
  return a.invalidEdgeUsed == b.invalidEdgeUsed ? (a.dis == b.dis ? a.edgeNum > b.edgeNum : a.dis > b.dis) : a.invalidEdgeUsed > b.invalidEdgeUsed;
}
std::priority_queue<Node> q;
std::priority_queue<Node> qAns;
int edgesUsed[MAX_NODE_NUM][MAX_NODE_NUM], invalidEdgeUsed[MAX_NODE_NUM];
int from[MAX_NODE_NUM];
bool visited[MAX_NODE_NUM];
bool solveOneMission(int missionIndex, bool isAllowIgnoreChannelLimit) {
  int s = missions[missionIndex].s, t = missions[missionIndex].t;
  // std::cout<<"solve one mission "<<s<<' '<<t<<' '<<isAllowIgnoreChannelLimit<<std::endl;
  while (!q.empty()) q.pop();
  while (!qAns.empty()) qAns.pop();
  for (int i = 0; i < n; i++) {
    minDis[i] = INF_INT;
    minEdgeNum[i] = INF_INT;
    visited[i] = false;
    // from[i] = -1;
    invalidEdgeUsed[i] = INF_INT;
  }
  minDis[s] = 0;
  minEdgeNum[s] = 0;
  invalidEdgeUsed[s] = 0;
  std::vector<int>tmp;
  q.push((Node){s, 0, 0, 0, INIT_CHANNEL, 0, tmp});
  int x;
  Node node;
  int ansChannelIndex = 0;
  while (!q.empty()) {
    node = q.top();
    q.pop();
    x = node.x;
    // std::cout<<"Dist ";write_(s, x, node.dis, node.edgeNum, node.invalidEdgeUsed, minDis[x], minEdgeNum[x]);std::cout<<node.channel<<std::endl;
    if (visited[x] && (node.dis != minDis[x] || node.edgeNum != minEdgeNum[x])) continue;
    // std::cout<<node.x<<' '<<node.dis<<' '<<node.edgeNum<<' '<<node.invalidEdgeUsed<<' '<<node.channel<<std::endl;
    visited[x] = true;
    // from[x] = node.from;
    if (x == t) {
      ansChannelIndex = node.channel._Find_first();
      qAns.push(node);
    }
    for (auto i: edgeFromNode[x]) {
      int isInvalid = !(edges[i].channelNotUsed & node.channel).any();
      if (!isAllowIgnoreChannelLimit && isInvalid) {
        continue;
      }
      int y = _y(edges[i], x);
      if ((invalidEdgeUsed[y] > invalidEdgeUsed[x] + isInvalid) ||
          (invalidEdgeUsed[y] == invalidEdgeUsed[x] + isInvalid && minDis[y] > minDis[x] + edges[i].dis) ||
          (invalidEdgeUsed[y] == invalidEdgeUsed[x] + isInvalid && minDis[y] == minDis[x] + edges[i].dis && minEdgeNum[y] > minEdgeNum[x] + 1)) {
            minDis[y] = minDis[x] + edges[i].dis;
            minEdgeNum[y] = minEdgeNum[x] + 1;
            invalidEdgeUsed[y] = invalidEdgeUsed[x] + isInvalid;
            tmp = node.ansEdges;
            tmp.push_back(i);
            q.push((Node){y, minDis[y], minEdgeNum[y], i, isInvalid && false ? node.channel : edges[i].channelNotUsed & node.channel, invalidEdgeUsed[y], tmp});
          }
    }
  }
  if (!visited[t]) return false;
  // std::cout<<"OK"<<std::endl;
  std::vector<int> ansEdgeIndex;
  int y;
  // x = t;
  // while (from[x] != -1) {
  //   ansEdgeIndex.push_back(from[x]);
  //   x = _y(edges[from[x]], x);
  // }
  tmp = qAns.top().ansEdges;
  std::bitset<MAX_CHANNEL_NUM> bs = INIT_CHANNEL;
  int nowDis = maxDis;
  x = s;
  for (auto it = tmp.begin(); it != tmp.end(); it++) {
    y = _y(edges[*it], x);
    // writeln(x, y, minDis[y]);
    if (!isAllowIgnoreChannelLimit) {
      missions[missionIndex].ansEdges.push_back(*it);
      edges[*it].channelNotUsed[ansChannelIndex] = 0;
      if (nowDis < edges[*it].dis) {
        missions[missionIndex].ansAmplifier.push_back(x);
        nowDis = maxDis - edges[*it].dis;
      } else {
        nowDis -= edges[*it].dis;
      }
      // std::cout<<"Count amp ";writeln(s, t, *it, nowDis, edges[*it].dis);
    }
    if (!(bs & edges[*it].channelNotUsed).any()) {
      edgesUsed[x][y]++;
      edgesUsed[y][x]++;
    } else {
      bs &= edges[*it].channelNotUsed;
    }
    x = y;
  }
  if (!isAllowIgnoreChannelLimit) {
    missions[missionIndex].solved = true;
    missions[missionIndex].ansChannel = qAns.top().channel._Find_first();
  }
  return true;
}

void output() {
  writeln(extraEdgeNum);
  for (int i = 0; i < extraEdgeNum; i++) {
    writeln(edges[m + i].x, edges[m + i].y);
  }
  for (int i = 0; i < missionNum; i++) {
    write_(missions[i].ansChannel, missions[i].ansEdges.size(), missions[i].ansAmplifier.size());
    for (auto j: missions[i].ansEdges) {
      write_(j);
    }
    for (auto j: missions[i].ansAmplifier) {
      write_(j);
    }
    writeln();
  }
}

void solve() {
  int solvedMissionNum = 0;
  while (solvedMissionNum != missionNum) {
    // writeln(solvedMissionNum);
    for (int i = 0; i < missionNum; i++) if (!missions[i].solved) {
      if (solveOneMission(i, false)) {
        solvedMissionNum++;
        // writeln(solvedMissionNum);
      }
    }
    // output();
    if (solvedMissionNum == missionNum) break;
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        edgesUsed[i][j] = 0;
      }
    }
    for (int i = 0; i < missionNum; i++) if (!missions[i].solved) {
      solveOneMission(i, true);
    }
    int maxEdgesUsedX = 0, maxEdgesUsedY = 0;
    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        if (edgesUsed[i][j] > edgesUsed[maxEdgesUsedX][maxEdgesUsedY]) {
          maxEdgesUsedX = i;
          maxEdgesUsedY = j;
        }
      }
    }
    addExtraEdge(maxEdgesUsedX, maxEdgesUsedY);
  }
}

int main() {
#ifdef LOCAL_LIGEN
  freopen("0.in", "r", stdin);
  // freopen("0.out","w",stdout);
  const double PROGRAM_BEGIN_TIME = clock();
#endif
  input();
  solve();
  output();
#ifdef LOCAL_LIGEN
  printf("Time: %.0lfms\n", (clock() - PROGRAM_BEGIN_TIME) * 1e6 / CLOCKS_PER_SEC);
  fclose(stdin);
  fclose(stdout);
#endif
}
