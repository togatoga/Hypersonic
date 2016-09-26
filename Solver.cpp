#include <algorithm>
#include <assert.h>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sys/time.h>
#include <tuple>
#include <vector>

#include <string.h>

using namespace std;

/*
--------------------------Template--------------------------
*/
const int DX[] = {1, 0, -1, 0, 0}, DY[] = {0, -1, 0, 1, 0};

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

const int64_t BIT_POW[14] = {1,           8,           64,         512,
                             4096,        32768,       262144,     2097152,
                             16777216,    134217728,   1073741824, 8589934592,
                             68719476736, 549755813888};
// 7 decimal bit board
// 0 empty cell
// 1 box cell
// 2 item(bomb_range) cell
// 3 item_box(bomb_range) cell
// 4 item(bomb_cnt) cell
// 5 item_box(bomb_cnt) cell
// 6 wall cell
namespace CellType {
const int EMPTY_CELL = 0;
const int BOX_CELL = 1;
const int BOMB_CELL = 2;
const int ITEM_BOMB_RANGE_UP_CELL = 3;
const int BOX_ITEM_BOMB_RANGE_UP_CELL = 4;
const int ITEM_BOMB_CNT_UP_CELL = 5;
const int BOX_ITEM_BOMB_CNT_UP_CELL = 6;
const int WALL_CELL = 7;
}

class BitBoard {
public:
  BitBoard() { memset(array, 0, sizeof(array)); }
  int64_t get(int y, int x) const {
    assert(0 <= y and y < 11);
    assert(0 <= x and x < 13);
    int64_t bit = (array[y] / BIT_POW[12 - x]) % BIT_POW[1];
    return bit;
  }
  void set(int y, int x, int kind) {
    assert(0 <= y and y < 11);
    assert(0 <= x and x < 13);
    assert(0 <= kind and kind < 8);
    int64_t bit = (array[y] / BIT_POW[12 - x]) % BIT_POW[1];
    array[y] -= bit * (BIT_POW[12 - x]);
    int64_t kind_bit = kind * BIT_POW[12 - x];
    array[y] += kind_bit;
  }

  bool operator<(const BitBoard &right) const {
    for (int y = 0; y < 11; y++) {
      if (array[y] != right.array[y]) {
        return array[y] < right.array[y];
      }
    }
    return false;
  }
  // debug
  void debug() const {
    cerr << "-----------------------Bit Board Start---------------------------"
         << endl;
    for (int y = 0; y < 11; y++) {
      for (int x = 0; x < 13; x++) {
        int64_t bit = get(y, x);

        cerr << bit << " ";
      }
      cerr << endl;
    }
    cerr << "-----------------------Bit Board End---------------------------"
         << endl;
  }

private:
  int64_t array[11];
};
/*
---------------------------Game Code---------------------------
*/
const int BOARD_HEIGHT = 11;
const int BOARD_WIDTH = 13;

namespace EntityType {
const int PLAYER = 0;
const int BOMB = 1;
const int ITEM = 2;
}
const int ITEM_TYPE_NUM = 2;
const int ITEM_BOMB_RANGE_UP_TYPE = 0;
const int ITEM_BOMB_CNT_UP_TYPE = 1;

const int EMPTY_CELL = 0;
const int BOX_CELL = 1;
const int BOMB_CELL = 2;
const int ITEM_BOMB_RANGE_UP_CELL = 3;
const int ITEM_BOMB_CNT_UP_CELL = 4;

const int ACT_MOVE = 0;
const int ACT_BOMB = 1;

class Solver {

public:
  Solver() {}
  void solve() {
    int width, height, myid;
    cin >> width >> height >> myid;

    assert(width == BOARD_WIDTH and height == BOARD_HEIGHT);
    cin.ignore();
    my_id = myid;

    int turn = 0;
    while (true) {

      StateInfo input_info = input(true);
      if (my_id == -1)
        break;
      think(input_info);
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
      if (owner != right.owner) {
        return owner < right.owner;
      }
      if (y != right.y) {
        return y < right.y;
      }
      if (x != right.x) {
        return x < right.x;
      }
      if (explosion_turn != right.explosion_turn) {
        return explosion_turn < right.explosion_turn;
      }
      return explosion_range < right.explosion_range;
    }
  };

  struct PlayerInfo {
    int x, y;
    int remain_bomb_cnt;
    int max_bomb_cnt;
    int explosion_range;
    // Todo
    int life_point;
    PlayerInfo() {
      max_bomb_cnt = 3;
      life_point = 1;
    }
    PlayerInfo(int y, int x, int remain_bomb_cnt, int explosion_range)
        : y(y), x(x), remain_bomb_cnt(remain_bomb_cnt),
          explosion_range(explosion_range) {
      max_bomb_cnt = 3;
      life_point = 1;
    }

    bool can_set_bomb() { return remain_bomb_cnt >= 1; }
    int get_remain_bomb_cnt() const { return remain_bomb_cnt; }

    bool operator<(const PlayerInfo &right) const {
      if (y != right.y) {
        return y < right.y;
      }
      if (x != right.x) {
        return x < right.x;
      }
      if (life_point != right.life_point) {
        return life_point < right.life_point;
      }
      if (remain_bomb_cnt != right.remain_bomb_cnt) {
        return remain_bomb_cnt < right.remain_bomb_cnt;
      }
      if (max_bomb_cnt != right.max_bomb_cnt) {
        return max_bomb_cnt < right.max_bomb_cnt;
      }
      return explosion_range < right.explosion_range;
    }
  };
  struct StateInfo {
    BitBoard board;
    vector<Bomb> bombs;
    set<pair<int, int>> future_destroied_boxes;
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
    bool operator<(const Act &right) const {
      if (y != right.y) {
        return y < right.y;
      }
      if (x != right.x) {
        return x < right.x;
      }
      return act_id < right.act_id;
    }
  };

  struct SearchState {
    StateInfo state;
    Act first_act;
    int my_destroied_box_cnt;
    int my_future_destroied_box_cnt;
    double score;
    bool operator<(const SearchState &right) const {
      return score < right.score;
    }
    SearchState() {
      my_destroied_box_cnt = 0;
      my_future_destroied_box_cnt = 0;
      score = 0;
    }
  };
  bool in_board(int y, int x) {
    if (y < 0 or y >= BOARD_HEIGHT or x < 0 or x >= BOARD_WIDTH)
      return false;
    return true;
  }

  StateInfo input(bool verbose = false) {
    StateInfo res;
    cerr << "----------------------------Input "
            "Start------------------------------"
         << endl;
    for (int i = 0; i < BOARD_HEIGHT; i++) {
      string row;
      if (!(cin >> row)) {
        my_id = -1;
        return StateInfo();
      }
      if (verbose) {
        cerr << row << endl;
      }
      for (int j = 0; j < BOARD_WIDTH; j++) {
        if (row[j] == '.') { // empty
          res.board.set(i, j, CellType::EMPTY_CELL);
        } else if (row[j] == '0') { // box
          res.board.set(i, j, CellType::BOX_CELL);
        } else if (row[j] == '1') { // item BOMB_RANGE_UP
          res.board.set(i, j,
                        CellType::BOX_ITEM_BOMB_RANGE_UP_CELL); // temporary set
        } else if (row[j] == '2') { // item BOMB_CNT_UP
          res.board.set(i, j,
                        CellType::BOX_ITEM_BOMB_CNT_UP_CELL); // temporary set
        } else if (row[j] == 'X') {
          res.board.set(i, j, CellType::WALL_CELL); // temporary set
        }
      }
    }

    int entities;
    cin >> entities;
    if (verbose) {
      cerr << entities << endl;
    }
    cin.ignore();
    // cerr << "my_id = "  << my_id << endl;
    // cerr << entities << endl;
    for (int i = 0; i < entities; i++) {
      int entityType;
      int owner;
      int x, y;
      int param1, param2;
      cin >> entityType >> owner >> x >> y >> param1 >> param2;
      if (verbose) {
        cerr << entityType << " " << owner << " " << x << " " << y << " "
             << param1 << " " << param2 << endl;
      }
      cin.ignore();
      // cerr << entityType << endl;
      if (entityType == EntityType::PLAYER) { // Player
        if (owner == my_id) {                 // Me
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
      } else if (entityType == EntityType::BOMB) { // Bomb
        res.bombs.emplace_back(Bomb(y, x, owner, param1, param2));
        res.board.set(y, x, CellType::BOMB_CELL);
      } else if (entityType == EntityType::ITEM) { // Item
                                                   // pass
        if (param1 == 1) {
          res.board.set(y, x, CellType::ITEM_BOMB_RANGE_UP_CELL);
        } else if (param1 == 2) {
          res.board.set(y, x, CellType::ITEM_BOMB_CNT_UP_CELL);
        }
      }
    }
    if (verbose) {
      res.board.debug();
    }
    sort(res.bombs.begin(), res.bombs.end());
    // cerr << res.my_info.y << " " << res.my_info.x << endl;
    cerr
        << "----------------------------Input End------------------------------"
        << endl;
    return res;
  }
  int my_id;

  void simulate_bomb_inducing_explosion(
      pair<int, int> exploded_key, set<pair<int, int>> &exploded_bombs,
      map<pair<int, int>, int> &map_explosion_range, const BitBoard &board) {
    if (exploded_bombs.count(exploded_key) > 0)
      return;

    int px = exploded_key.second;
    int py = exploded_key.first;
    // cerr << py << " " << px << endl;
    assert(in_board(py, px));
    exploded_bombs.emplace(make_pair(py, px));

    int range = map_explosion_range[make_pair(py, px)];
    // cerr << range << endl;
    for (int k = 0; k < 4; k++) {
      for (int d = 0; d < range; d++) {
        int ny, nx;
        nx = px + d * DX[k];
        ny = py + d * DY[k];
        if (not in_board(ny, nx))
          break;
        if (map_explosion_range.count(make_pair(ny, nx)) > 0 and
            exploded_bombs.count(make_pair(ny, nx)) ==
                0) { // inducing explosion
          simulate_bomb_inducing_explosion(make_pair(ny, nx), exploded_bombs,
                                           map_explosion_range, board);
          break;
        }
        int cell_type = board.get(ny, nx);
        if (d != 0 and cell_type != CellType::EMPTY_CELL) { // exsit object
          // cerr << d << " " << k << endl;
          break;
        }
      }
    }
  }
  void simulate_bomb_explosion(SearchState &state, int turn) {
    vector<Bomb> &bombs = state.state.bombs;
    set<pair<int, int>> exploded_bombs;
    map<pair<int, int>, int> map_explosion_range;
    for (int i = 0; i < bombs.size(); i++) {
      pair<int, int> key = make_pair(bombs[i].y, bombs[i].x);
      int explosion_range = bombs[i].explosion_range;
      map_explosion_range[key] = explosion_range;
    }
    for (int i = 0; i < bombs.size(); i++) {
      bombs[i].dec_turn();
      int x, y;
      x = bombs[i].x;
      y = bombs[i].y;
      if (bombs[i].is_explode()) {
        // cerr << y << " " << x << endl;
        simulate_bomb_inducing_explosion(make_pair(y, x), exploded_bombs,
                                         map_explosion_range,
                                         state.state.board);
      }
    }

    // destroy object
    // cerr << exploded_bombs.size() << endl;
    set<pair<int, int>> destroyed_objects;
    for (int i = 0; i < bombs.size(); i++) {
      const int px = bombs[i].x;
      const int py = bombs[i].y;
      if (exploded_bombs.count(make_pair(py, px)) > 0) { // exploded

        int owner = bombs[i].owner;
        int range = bombs[i].explosion_range;
        // assert(owner == 0 or owner == 1 or owner == 2);
        // assert(range == 3);
        // state.state.board.set(py, px, CellType::EMPTY_CELL);
        if (owner == my_id) { // me
          state.state.my_info.remain_bomb_cnt++;
        } else { // enemy
          state.state.enemy_info.remain_bomb_cnt++;
        }
        // right  up left down
        bool damged = false;
        for (int k = 0; k < 4; k++) {
          for (int d = 0; d < range; d++) {
            int ny, nx;
            nx = px + d * DX[k];
            ny = py + d * DY[k];

            if (not in_board(ny, nx))
              break;
            int cell_type = state.state.board.get(ny, nx);

            if ((d != 0 and cell_type == CellType::BOMB_CELL) or
                cell_type == CellType::WALL_CELL) { // exsit bombs
              break;
            }

            {
              int player_y, player_x;
              player_y = state.state.my_info.y;
              player_x = state.state.my_info.x;
              // cerr << d << " " << k << endl;
              if (owner == my_id) {
                // cerr << py << " " << px << " " << ny << " " << nx << endl;
                // cerr << player_y << " " << player_x << endl;
              }

              if (not damged and (ny == player_y and nx == player_x)) {
                // cerr << py << " " << px << " " << ny << " " << nx << " " << d
                // << " " << k << endl;
                state.state.my_info.life_point--;
                // cerr << state.state.my_info.life_point << endl;
                // assert(state.state.my_info.life_point == 0);
                // assert(false);
                // assert(false);
                damged = true;
              }

              // player_y = state.state.enemy_info.y;
              // player_x = state.state.enemy_info.x;
              // if (ny == player_y and nx == player_x){
              //   state.state.enemy_info.life_point--;
              // }
            }

            if (cell_type != CellType::EMPTY_CELL) {
              if (cell_type == CellType::BOX_CELL or
                  cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
                  cell_type ==
                      CellType::BOX_ITEM_BOMB_CNT_UP_CELL) { // exsit
                                                             // box(contain item
                                                             // box)
                // destory\

                if (owner == my_id) { // me
                  // cerr << "turn = " << turn << endl;
                  // cerr << py << " " << px << " " << " " << ny << " " << nx <<
                  // endl;
                  // cerr << d << " " << k << endl;

                  state.my_destroied_box_cnt += 1;
                  destroyed_objects.emplace(make_pair(ny, nx));

                } else { // enemy
                  // Todo
                }
              } else if (cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL or
                         cell_type ==
                             CellType::ITEM_BOMB_CNT_UP_CELL) { // item box
                destroyed_objects.emplace(make_pair(ny, nx));
              }

              // cerr << py << " " << px << " " << d << " " << k << endl;
              if (d != 0) {
                break;
              }
            }
          }
        }
      }
    }
    for (const auto &val : destroyed_objects) {
      int y, x;
      x = val.second;
      y = val.first;
      int cell_type = state.state.board.get(y, x);
      if (cell_type == CellType::BOX_CELL or
          cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL or
          cell_type == CellType::ITEM_BOMB_CNT_UP_CELL) { // EMPTY CELL
        state.state.board.set(y, x, CellType::EMPTY_CELL);
      } else if (cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL) {
        state.state.board.set(y, x, CellType::ITEM_BOMB_RANGE_UP_CELL);
      } else if (cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL) {
        state.state.board.set(y, x, CellType::ITEM_BOMB_CNT_UP_CELL);
      }
    }
    vector<Bomb> next_bombs;
    for (int i = 0; i < bombs.size(); i++) {
      int x = bombs[i].x;
      int y = bombs[i].y;
      if (exploded_bombs.count(make_pair(y, x)) == 0) { // not exploded
        next_bombs.emplace_back(bombs[i]);
      } else { // exploed
        state.state.board.set(y, x, CellType::EMPTY_CELL);
      }
    }
    sort(next_bombs.begin(), next_bombs.end());
    // bombs = next_bombs;
    bombs.swap(next_bombs);
    // cerr << bombs.size() << endl;
  }

  double calc_score(const SearchState &pre_state,
                    const SearchState &search_state) {
    double score = 0;
    const int px = search_state.state.my_info.x;
    const int py = search_state.state.my_info.y;
    const int range = search_state.state.my_info.explosion_range;
    // score += 6 * (search_state.my_destroied_box_cnt -
    // pre_state.my_destroied_box_cnt);

    // death penalty
    // cerr << "unko" << endl;
    if (search_state.state.my_info.life_point <= 0) {

      score -= 1e30;
    }
    score *= 100;

    score += 20 * (search_state.my_destroied_box_cnt);
    score += 10 * (search_state.my_future_destroied_box_cnt);

    score += 10 * search_state.state.my_info.max_bomb_cnt;
    score += 5 * search_state.state.my_info.explosion_range;

    // score += 4 * (search_state.my_future_destroied_box_cnt -
    // pre_state.my_future_destroied_box_cnt);
    // score += (search_state.state.my_info.remain_bomb_cnt -
    // pre_state.state.my_info.remain_bomb_cnt);
    // score -= search_state.state.my_info.get_remain_bomb_cnt();
    // int near_box_cnt = 0;
    // for (int k = 0; k < 4; k++) {
    //   for (int d = 0; d < range; d++) {
    // 	int ny, nx;
    // 	nx = px + d * DX[k];
    // 	ny = py + d * DY[k];
    // 	if (not in_board(ny,nx))break;
    // 	if (search_state.state.board[ny][nx] == BOX_CELL){
    // 	  if (search_state.state.future_destroied_boxes.count(make_pair(ny, nx))
    // > 0)continue;
    // 	  near_box_cnt++;
    // 	  break;
    // 	  // destory
    // 	  //cerr << py << " " << px << " " << ny << " " << nx << endl;
    // 	  //next_state.my_future_destroied_box_cnt += 1;
    // 	  //next_state.state.future_destroied_boxes.emplace(make_pair(ny, nx));
    // 	  //break;
    // 	}
    //   }
    // }
    // score += near_box_cnt;

    score *= 100;
    score += search_state.state.my_info.explosion_range;
    score += 4 * search_state.state.my_info.max_bomb_cnt;

    int sum_man_dist = 0;
    int min_dist = (BOARD_HEIGHT + BOARD_WIDTH + 1);
    int active_boxes_cnt = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      for (int x = 0; x < BOARD_WIDTH; x++) {
        int cell_type = search_state.state.board.get(y, x);
        if (cell_type == CellType::BOX_CELL or
            cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
            cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL) {
          if (search_state.state.future_destroied_boxes.count(make_pair(y, x)) >
              0)
            continue;
          int dist = abs(px - x) + abs(py - y);
          active_boxes_cnt++;
          min_dist = min(min_dist, dist);
          sum_man_dist += dist;
        }
      }
    }
    score += 12 * ((BOARD_HEIGHT + BOARD_WIDTH) - min_dist);
    score += (BOARD_HEIGHT + BOARD_WIDTH) * (active_boxes_cnt)-sum_man_dist;

    return score;
  }
  void simulate_next_move(const SearchState &state,
                          priority_queue<SearchState> &search_states,
                          const int &turn) {
    const int px = state.state.my_info.x;
    const int py = state.state.my_info.y;

    for (int k = 0; k < 5; k++) {
      int nx = px + DX[k];
      int ny = py + DY[k];
      if (not in_board(ny, nx))
        continue;
      int cell_type = state.state.board.get(ny, nx);
      if (cell_type == CellType::BOX_CELL or
          cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
          cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL or
          cell_type == CellType::WALL_CELL)
        continue;
      if (k != 4) {
        if (cell_type == CellType::BOMB_CELL) {
          continue;
        }
      }
      SearchState next_state = state;
      if (cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL) {
        // cerr << "unko" << endl;
        next_state.state.my_info.explosion_range++;
        next_state.state.board.set(ny, nx, CellType::EMPTY_CELL);
      } else if (cell_type == CellType::ITEM_BOMB_CNT_UP_CELL) {
        next_state.state.my_info.max_bomb_cnt += 1;
        next_state.state.my_info.remain_bomb_cnt += 1;
        next_state.state.board.set(ny, nx, CellType::EMPTY_CELL);
      }
      next_state.state.my_info.y = ny;
      next_state.state.my_info.x = nx;
      if (turn == 0) {
        next_state.first_act = Act(ny, nx, ACT_MOVE);
      }
      // simulate_bomb_explosion(next_state, turn);
      next_state.score = calc_score(state, next_state);
      search_states.emplace(next_state);
      // visited.emplace(make_tuple(next_state.state.my_info,
      // next_state.state.enemy_info, next_state.state.boxes,
      // next_state.state.bombs));
    }
  }
  void simulate_next_set_bomb(const SearchState &state,
                              priority_queue<SearchState> &search_states,
                              const int &turn) {
    if (state.state.my_info.get_remain_bomb_cnt() <= 0)
      return;

    const int px = state.state.my_info.x;
    const int py = state.state.my_info.y;
    const int range = state.state.my_info.explosion_range;
    if (state.state.board.get(py, px) ==
        CellType::BOMB_CELL) { // already set bomb
      return;
    }

    SearchState next_state = state;
    for (int k = 0; k < 4; k++) {
      for (int d = 0; d < range; d++) {
        int ny, nx;
        nx = px + d * DX[k];
        ny = py + d * DY[k];
        if (not in_board(ny, nx))
          break;
        int cell_type = next_state.state.board.get(ny, nx);
        if (cell_type == CellType::BOX_CELL or
            cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
            cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL) {
          if (next_state.state.future_destroied_boxes.count(make_pair(ny, nx)) >
              0)
            continue;
          // destory
          // cerr << py << " " << px << " " << ny << " " << nx << endl;
          next_state.my_future_destroied_box_cnt += 1;
          next_state.state.future_destroied_boxes.emplace(make_pair(ny, nx));
          break;
        }
      }
    }
    next_state.state.board.set(py, px, CellType::BOMB_CELL);
    next_state.state.bombs.emplace_back(Bomb(py, px, my_id, 8, range));
    next_state.state.my_info.remain_bomb_cnt--;
    // next_state.state.board[py][px] = BOMB_CELL;
    if (turn == 0) {
      next_state.first_act = Act(py, px, ACT_BOMB);
    }
    // simulate_bomb_explosion(next_state, turn);
    next_state.score = calc_score(state, next_state);
    search_states.emplace(next_state);
    // visited.emplace(make_tuple(next_state.state.my_info,
    // next_state.state.enemy_info, next_state.state.boxes,
    // next_state.state.bombs));
    // visited.emplace(next_state.state);
  }
  void output_act(const Act &act) {
    int y, x;
    y = act.y;
    x = act.x;
    if (act.act_id == ACT_MOVE) {
      cout << "MOVE"
           << " " << x << " " << y << endl;
    } else {
      cout << "BOMB"
           << " " << x << " " << y << endl;
    }
  }

  void count_ACT_BOMB(priority_queue<SearchState> curr_search_states,
                      int turn) {
    int cnt = 0;
    while (not curr_search_states.empty()) {
      SearchState state = curr_search_states.top();
      curr_search_states.pop();
      if (state.first_act.act_id == ACT_BOMB) {
        cnt++;
      }
    }
    // cerr << "turn = "<< turn << " " << "ACT_BOMB = " << cnt << endl;
  }
  void
  count_duplicated_first_ACT(priority_queue<SearchState> curr_search_states,
                             int turn) {
    map<Act, int> count;
    while (not curr_search_states.empty()) {
      SearchState state = curr_search_states.top();
      curr_search_states.pop();
      count[state.first_act]++;
    }
    cerr << "---turn = " << turn << endl;
    for (const auto &val : count) {
      cerr << val.first.y << " " << val.first.x << " " << val.first.act_id
           << " " << val.second << endl;
    }
    cerr << "--------------------------------" << endl;
  }
  void think(const StateInfo &init_info) {
    // cerr << "--think--" << endl;
    // cerr << init_info.board[0][0] << endl;

    Timer timer;
    timer.start();

    const int beam_width = 30;
    const int depth_limit = 10;
    priority_queue<SearchState> curr_search_states[depth_limit + 1];
    set<tuple<PlayerInfo, PlayerInfo, BitBoard, vector<Bomb>>>
        visited[depth_limit + 1];

    SearchState init_search_state;
    init_search_state.state = init_info;
    // cerr << init_info.my_info.y << " " << init_info.my_info.x << endl;
    // cerr << init_search_state.state.my_info.y << " " <<
    // init_search_state.state.my_info.x << endl;
    curr_search_states[0].emplace(init_search_state);
    Act best_act;
    double best_score = 0.0;
    int chokudai_iter = 0;
    while (timer.get_mill_duration() <= 85) {
      chokudai_iter++;
      for (int turn = 0; turn < depth_limit; turn++) {
        // count_duplicated_first_ACT(curr_search_states[turn], turn);
        // count_ACT_BOMB(curr_search_states[turn], turn);
        // cerr << curr_search_states[turn].size() << endl;
        // int prune_cnt = 0;
        for (int iter = 0;
             iter < beam_width and (not curr_search_states[turn].empty());
             iter++) {
          if (timer.get_mill_duration() >= 85)
            goto END;
          SearchState curr_search_state = curr_search_states[turn].top();
          curr_search_states[turn].pop();
          auto key = make_tuple(curr_search_state.state.my_info,
                                curr_search_state.state.enemy_info,
                                curr_search_state.state.board,
                                curr_search_state.state.bombs);
          if (visited[turn].count(key) > 0) {
            iter--;
            // prune_cnt++;
            continue;
          }
          visited[turn].emplace(key);
          // simulate bomb
          simulate_bomb_explosion(curr_search_state, turn);
          // next state
          // move
          simulate_next_move(curr_search_state, curr_search_states[turn + 1],
                             turn);
          // set bomb
          simulate_next_set_bomb(curr_search_state,
                                 curr_search_states[turn + 1], turn);
        }
        // cerr << "prune = " << prune_cnt << endl;
      }
      break;
    }
  END:;
    // cerr << curr_search_states[depth_limit].size() << endl;
    cerr << chokudai_iter++ << endl;
    SearchState best = curr_search_states[depth_limit].top();
    cerr << best.my_destroied_box_cnt << " " << best.my_future_destroied_box_cnt
         << " " << best.state.my_info.max_bomb_cnt << " "
         << best.state.my_info.explosion_range << " " << best.score << endl;
    output_act(best.first_act);
  }
};

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
  Solver solver;
  solver.solve();
}
