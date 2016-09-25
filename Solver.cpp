#include <algorithm>
#include <assert.h>
#include <iostream>
#include <set>
#include <sys/time.h>
#include <vector>

using namespace std;

/*
--------------------------Template--------------------------
*/
const int DX[] = {1, 0, -1, 0}, DY[] = {0, -1, 0, 1};

class Timer {
public:
  Timer() {}
  void start() { start_time = get_mill_sec(); }
  double get_mill_duration() { return get_mill_sec() - start_time; }

private:
  double start_time;
  double get_mill_sec() {
    struct timeval res;
    gettimeofday(&res, NULL);
    return res.tv_sec * 1000 + res.tv_usec / 1000;
  }
};

/*
---------------------------Game Code---------------------------
*/

const int BOARD_WIDTH = 13;
const int BOARD_HEIGHT = 11;
namespace EntityType {
const int PLAYER = 0;
const int BOMB = 1;
}

const int EMPTY_CELL = 0;
const int BOX_CELL = 1;

class Solver {

public:
  Solver() {}
  void solve() {
    int width, height, myid;
    cin >> width >> height >> myid;
    assert(width == BOARD_WIDTH and height == BOARD_HEIGHT);
    cin.ignore();
    my_id = myid;
    // cerr << static_cast<int>(my_id) << endl;
    int turn = 0;
    while (true) {
      StateInfo input_info = input();
      think(input_info);
      break;
    }
  }

private:
  struct Bomb {
    int x, y;
    int owner;
    int explosion_turn;
    int explosion_range;
    Bomb() {}
    Bomb(int y, int x, int owner, int explosion_turn, int explosion_range)
        : y(y), x(x), owner(owner), explosion_turn(explosion_turn),
          explosion_range(explosion_range) {}
    bool is_explode() const { return explosion_turn <= 0; }
    void dec_turn() {
      assert(explosion_range > 0);
      explosion_turn--;
    }
    bool operator<(const Bomb &right) const {
      if (y != right.x) {
        return y < right.x;
      } else {
        return x < right.x;
      }
    }
  };

  struct PlayerInfo {

    int x, y;
    int remain_bomb_cnt;
    int explosion_range;
    // Todo
    // int life;
    PlayerInfo() {}
    PlayerInfo(int y, int x, int remain_bomb_cnt, int explosion_range)
        : y(y), x(x), remain_bomb_cnt(remain_bomb_cnt),
          explosion_range(explosion_range) {}

    bool can_set_bomb() { return remain_bomb_cnt >= 1; }
    int get_remain_bomb_cnt() const { return remain_bomb_cnt; }
  };
  struct StateInfo {
    int board[BOARD_HEIGHT][BOARD_WIDTH];
    vector<Bomb> bombs;
    PlayerInfo my_info, enemy_info;
    StateInfo() {}
  };
  struct Act {
    int y, x;
    int act_id;
    Act() {
      y = -1;
      x = -1;
      act_id = -1;
    }
    Act(int y, int x, int act_id) : y(y), x(x), act_id(act_id) {}
  };

  struct SearchState {
    StateInfo state;
    Act first_act;
    int my_destory_box_cnt;
    double score;
    bool operator<(const SearchState &right) const {
      return score < right.score;
    }
    SearchState() {
      my_destory_box_cnt = 0;
      score = 0;
    }
  };
  void calc_score(SearchState &search_state) {}
  StateInfo input() {
    StateInfo res;
    for (int i = 0; i < BOARD_HEIGHT; i++) {
      string row;
      cin >> row;
      for (int j = 0; j < BOARD_WIDTH; j++) {
        if (row[i] == '.') { // empty
          res.board[i][j] = 0;
        } else if (row[i] == '0') { // box
          res.board[i][j] = 1;
        }
      }
    }
    int entities;
    cin >> entities;
    cin.ignore();
    // cerr << "my_id = "  << my_id << endl;
    // cerr << entities << endl;
    for (int i = 0; i < entities; i++) {
      int entityType;
      int owner;
      int x, y;
      int param1, param2;
      cin >> entityType >> owner >> x >> y >> param1 >> param2;
      cin.ignore();
      // cerr << entityType << endl;
      if (entityType == EntityType::PLAYER) { // Player
        if (owner == my_id) {                 // Me
          // cerr << y << " " << x << endl;
          res.my_info.x = x;
          res.my_info.y = y;
          res.my_info.remain_bomb_cnt = param1;
          res.my_info.explosion_range = param2;
        } else { // enemy
          res.enemy_info.x = x;
          res.enemy_info.y = y;
          res.enemy_info.remain_bomb_cnt = param1;
          res.enemy_info.explosion_range = param2;
        }
      } else { // Bomb
        res.bombs.emplace_back(Bomb(y, x, owner, param1, param2));
      }
    }
    // cerr << res.my_info.y << " " << res.my_info.x << endl;
    return res;
  }
  int my_id;
  void simulate_bomb_explosion(SearchState &state) {
    vector<Bomb> &bombs = state.state.bombs;
    set<pair<int, int>> is_bombs;
    for (int i = 0; i < bombs.size(); i++) {
      bombs[i].dec_turn();
      if (not bombs[i].is_explode()) {
        is_bombs.emplace(make_pair(bombs[i].y, bombs[i].x));
      }
    }
    //
    set<pair<int, int>> destroyed_boxes;
    for (int i = 0; i < bombs.size(); i++) {
      if (bombs[i].is_explode()) {
        int px = bombs[i].x;
        int py = bombs[i].y;
        int owner = bombs[i].owner;
        int range = bombs[i].explosion_range;
        // right
        // up
        // left
        // down
        for (int k = 0; k < 4; k++) {
          for (int d = 0; d < range; d++) {
            int ny, nx;
            ny = py + d * DY[k];
            nx = px + d * DX[k];
            if (ny < 0 or ny > BOARD_HEIGHT or nx < 0 or nx >= BOARD_HEIGHT) {
              break;
            }
            if (is_bombs.count(make_pair(ny, nx)) > 0) { // exsit bombs
              break;
            }
            if (state.state.board[ny][nx] == BOX_CELL) { // exsit box
              // destory
              if (owner == my_id) {
                state.my_destory_box_cnt += 1;
                destroyed_boxes.emplace(make_pair(ny, nx));
              }
              break;
            }
          }
        }
      }
    }

    for (const auto &val : destroyed_boxes) {
      int y, x;
      y = val.first;
      x = val.second;
      state.state.board[y][x] = 0;
    }
    vector<Bomb> next_bombs;
    for (int i = 0; i < bombs.size(); i++) {
      if (not bombs[i].is_explode()) {
        next_bombs.emplace_back(bombs[i]);
      }
    }
    bombs = move(next_bombs);
  }

  void think(const StateInfo &init_info) {
    // cerr << "--think--" << endl;
    const int beam_width = 10;
    const int depth_limit = 5;
    vector<SearchState> curr_search_states[depth_limit];
    SearchState init_search_state;
    init_search_state.state = init_info;
    // cerr << init_info.my_info.y << " " << init_info.my_info.x << endl;
    // cerr << init_search_state.state.my_info.y << " " <<
    // init_search_state.state.my_info.x << endl;
    curr_search_states[0].emplace_back(init_search_state);
    Act best_act;
    double best_score = 0.0;
    for (int turn = 0; turn < depth_limit; turn++) {
      for (int i = 0; i < curr_search_states[turn].size(); i++) {
        if (curr_search_states[turn].size() > beam_width) {
          sort(curr_search_states[turn].rbegin(),
               curr_search_states[turn].rend());
          curr_search_states[turn].resize(beam_width);
        }
        SearchState &curr_search_state = curr_search_states[turn][i];
        simulate_bomb_explosion(curr_search_state);

        // move

        // set bomb

        // simulate action
      }
    }
  }
};

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
  Solver solver;
  solver.solve();
}
