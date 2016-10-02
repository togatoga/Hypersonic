#pragma GCC optimize("O3")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")

#include <algorithm>
#include <array>
#include <assert.h>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string.h>
#include <sys/time.h>
#include <tuple>
#include <vector>

using namespace std;

/*
--------------------------Template--------------------------
*/
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
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

const int SHIFT_WIDTH[13] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48};
const int64_t WIDTH_BIT[13] = {
    0b1111,
    0b11110000,
    0b111100000000,
    0b1111000000000000,
    0b11110000000000000000,
    0b111100000000000000000000,
    0b1111000000000000000000000000,
    0b11110000000000000000000000000000,
    0b111100000000000000000000000000000000,
    0b1111000000000000000000000000000000000000,
    0b11110000000000000000000000000000000000000000,
    0b111100000000000000000000000000000000000000000000,
    0b1111000000000000000000000000000000000000000000000000};

// max 0000 ~ 1111 (0~15)
class BitBoard {
public:
  BitBoard() { memset(array, 0, sizeof(array)); }
  inline int get(int y, int x) const {
    assert(0 <= y and y < 11);
    assert(0 <= x and x < 13);
    return (array[y] >> SHIFT_WIDTH[x]) & WIDTH_BIT[0];
  }
  inline void set(int y, int x, int kind) {
    assert(0 <= y and y < 11);
    assert(0 <= x and x < 13);
    assert(0 <= kind and kind < 9);
    // clear bit
    array[y] = (array[y] & (~WIDTH_BIT[x])) | ((int64_t)kind << SHIFT_WIDTH[x]);
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
        int bit = get(y, x);
        cerr << bit << " ";
      }
      cerr << endl;
    }
    cerr << "-----------------------Bit Board End---------------------------"
         << endl;
  }
  void clear(){
    for (int y = 0; y < 11; y++){
      array[y] = 0;
    }
  }

  
private:
  int64_t array[11];
};


template<typename T, int limit>
class FastVector{
public:
  FastVector() {
    head = 0;
  }
  FastVector(const FastVector& right) {
    head = right.head;
    for (int i = 0; i < head; i++){
      data[i] = right.data[i];
    }
  }
  
  inline void clear(){
    head = 0;
  }
  inline bool empty() const{
    return head == 0;
  }
  inline void push_back(const T & value) {
    assert(0<= head and head < limit);
    data[head] = value;
    head++;
  }
  template<class... TyArgs> inline void emplace_back(TyArgs&&... args) {
    assert(0<= head and head < limit);
    ::new(&data[head])T(forward<TyArgs>(args)...);
    head++;
  }
  inline int size() const {
    return  head;
  }
  inline void resize(int n){
    assert(0 <= n and n <= limit);
    head = n;
  }
  inline T* begin(){
    return data.begin();
  }
  inline T* end(){
    return data.begin() + head;
  }
  
  inline const T *begin() const {
    return data.begin();
  }
  inline const T* end() const {
    return data.begin() + head;
  }
  inline  T &operator[](int i){
    assert(0 <= i and i < head);
    return data[i];
  }
  inline const T &operator[](int i) const {
    assert(0 <= i and i < head);
    return data[i];
  }
  
  inline bool operator == (const FastVector &right) const {
    if (head != right.head){
      return false;
    }
    for (int i = 0; i < head; i++){
      if (data[i] != right[i]){
	return false;
      }
    }
    return true;
  }
  
  inline bool operator < (const FastVector & right) const {
    if (head != right.head){
      return head < right.head;
    }
    for (int i = 0; i < head; i++){
      if (data[i] != right[i]){
	return data[i] < right[i];
      }
    }
    return false;
  };
  
private:
  int head;
  array<T, limit> data;
};





/*
---------------------------Game Code---------------------------
*/

// 7 decimal bit board
// 0 empty cell
// 1 box cell
// 2 item(bomb_range) cell
// 3 item_box(bomb_range) cell
// 4 item(bomb_cnt) cell
// 5 item_box(bomb_cnt) cell
// 6 wall cell
namespace CellType {
const int8_t EMPTY_CELL = 0;
const int8_t BOX_CELL = 1;
const int8_t BOMB_CELL = 2;
const int8_t BOMB_EXPLODED_CELL = 3;
const int8_t ITEM_BOMB_RANGE_UP_CELL = 4;
const int8_t BOX_ITEM_BOMB_RANGE_UP_CELL = 5;
const int8_t ITEM_BOMB_CNT_UP_CELL = 6;
const int8_t BOX_ITEM_BOMB_CNT_UP_CELL = 7;
const int8_t WALL_CELL = 8;
}
namespace GameRule {

const int MAX_PLAYER_NUM = 4;
const int BASE_BOM_CNT = 1;
const int BASE_BOM_RANGE = 3;
const int MAX_TURN = 200;
}

const int BOARD_HEIGHT = 11;
const int BOARD_WIDTH = 13;

namespace EntityType {
const int PLAYER = 0;
const int BOMB = 1;
const int ITEM = 2;
}


const int ACT_MOVE = 0;
const int ACT_BOMB = 1;




namespace MCTS{
  const int MAX_NODE = 0;
  const int MAX_PLAY_OUT_CNT = 20;
  const int MAX_DEPTH_LIMIT = 2;
}






class Solver {

public:
  Solver() {}
  void solve() {

    int width, height, myid;
    cin >> width >> height >> myid;
    assert(width == BOARD_WIDTH and height == BOARD_HEIGHT);
    cin.ignore();
    //-----------------------------init------------------------------------------
    my_id = myid;
    game_timer.start();
    game_turn = 0;
    external_player_info.fill(0);
    next_pos = make_pair(-1, -1);
    game_player_num = -1;
    curr_player_num = 0;
    //-----------------------------init------------------------------------------
    while (true) {
      StateInfo input_info = input(true);
      if (my_id == -1)
        break;
      think(input_info);
      game_turn++;
    }
  }

private:
  struct Bomb {
    int8_t x, y;
    int8_t owner;
    int8_t explosion_turn;
    int8_t explosion_range;
    Bomb() {}
    Bomb(int y, int x, int owner, int explosion_turn, int explosion_range)
        : y(y), x(x), owner(owner), explosion_turn(explosion_turn),
          explosion_range(explosion_range) {}
    bool is_explode() const { return explosion_turn <= 0; }
    void dec_turn() {
      assert(explosion_range > 0);
      explosion_turn--;
    }
    inline bool operator != (const Bomb &right) const {
      if (explosion_turn != right.explosion_turn) {
        return true;
      }
      if (owner != right.owner) {
        return true;
      }
      if (y != right.y) {
        return true;
      }
      if (x != right.x) {
        return true;
      }
      if (explosion_range != right.explosion_range){
	return true;
      }
      return false;
    }
    inline bool operator == (const Bomb &right) const {
      if (explosion_turn != right.explosion_turn) {
        return false;
      }
      if (owner != right.owner) {
        return false;
      }
      if (y != right.y) {
        return false;
      }
      if (x != right.x) {
        return false;
      }
      if (explosion_range != right.explosion_range) {
        return false;
      }
      return true;
    }
    inline bool operator<(const Bomb &right) const {
      if (explosion_turn != right.explosion_turn) {
        return explosion_turn < right.explosion_turn;
      }
      if (owner != right.owner) {
        return owner < right.owner;
      }
      if (y != right.y) {
        return y < right.y;
      }
      if (x != right.x) {
        return x < right.x;
      }
      return explosion_range < right.explosion_range;
    }
  };

  struct PlayerInfo {
    int8_t x, y;
    int8_t max_bomb_cnt;

    int8_t remain_bomb_cnt;
    int8_t explosion_range;

    // determine winner player
    int8_t sum_box_point;
    bool survival;
    PlayerInfo() { survival = false; }
    // PlayerInfo(int y, int x, bool survival, int sum_box_point, int
    // remain_bomb_cnt, int explosion_range)
    //   : y(y), x(x), survival(survival), sum_box_point(sum_box_point),
    //   remain_bomb_cnt(remain_bomb_cnt),
    //       explosion_range(explosion_range) {
    // }
    bool is_survive() const { return survival; }
    bool is_dead() const { return not survival; }

    bool can_set_bomb() { return remain_bomb_cnt >= 1; }
    int8_t get_remain_bomb_cnt() const { return remain_bomb_cnt; }

    bool operator<(const PlayerInfo &right) const {
      if (y != right.y) {
        return y < right.y;
      }
      if (x != right.x) {
        return x < right.x;
      }
      if (survival != right.survival) {
        return survival < right.survival;
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
  using Bombs = vector<Bomb>;
  using Player = array<PlayerInfo, GameRule::MAX_PLAYER_NUM>;
  struct StateInfo {
    BitBoard board;
    BitBoard explosion_turn_board;
    Bombs bombs;
    Player players;
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
    double score;
    bool operator<(const SearchState &right) const {
      return score < right.score;
    }
    SearchState() {}
  };
  bool in_board(int y, int x) {
    if (y < 0 or y >= BOARD_HEIGHT or x < 0 or x >= BOARD_WIDTH)
      return false;
    return true;
  }

  StateInfo input(bool verbose = false) {
    StateInfo res;
    for (int i = 0; i < BOARD_HEIGHT; i++){
      res.board.clear();
      res.explosion_turn_board.clear();
    }
    cerr << "----------------------------Input "
            "Start------------------------------"
         << endl;
    cerr << BOARD_WIDTH << " " << BOARD_HEIGHT << " " << my_id << endl;
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
      if (entityType == EntityType::PLAYER) { // Player
        res.players[owner].x = x;
        res.players[owner].y = y;
        res.players[owner].remain_bomb_cnt = param1; // remain
        res.players[owner].explosion_range = param2; // range
        // survive
        res.players[owner].survival = true;
        res.players[owner].sum_box_point =
            external_player_info[owner];          // later update
        res.players[owner].max_bomb_cnt = param1; // later update
        if (owner == my_id) {
          assert(next_pos.first == -1 or
                 (next_pos.first == y and next_pos.second == x));
        }
	curr_player_num++;
      } else if (entityType == EntityType::BOMB) { // Bomb
	//param1 remain turn
	//param2 range
        res.players[owner].max_bomb_cnt++;
        res.bombs.emplace_back(move(Bomb(y, x, owner, param1, param2)));
	const int pre_explosion_turn = res.explosion_turn_board.get(y, x);
	if (pre_explosion_turn != 0){
	  res.explosion_turn_board.set(y, x, MIN(pre_explosion_turn, param1));
	}else{
	  res.explosion_turn_board.set(y, x, param1);
	}
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
      res.explosion_turn_board.debug();
    }

    cerr
        << "----------------------------Input End------------------------------"
        << endl;
    if (game_player_num == -1){
      game_player_num = curr_player_num;
    }
    return res;
  }

  inline bool on_bomb_cell(int cell_type) {
    if (cell_type == CellType::BOMB_CELL) {
      return true;
    }
    return false;
  }
  inline bool on_bomb_exploded_cell(int cell_type) {
    if (cell_type == CellType::BOMB_EXPLODED_CELL) {
      return true;
    }
    return false;
  }
  inline bool on_any_bomb_cell(int cell_type) {
    if (on_bomb_cell(cell_type) or on_bomb_exploded_cell(cell_type)) {
      return true;
    }
    return false;
  }
  inline bool on_box_cell(int cell_type) {
    if (cell_type == CellType::BOX_CELL) {
      return true;
    }
    return false;
  }
  inline bool on_box_item_cell(int cell_type) {
    if (cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
        cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL) {
      return true;
    }
    return false;
  }
  inline bool on_any_box_cell(int cell_type) {
    if (on_box_cell(cell_type) or on_box_item_cell(cell_type)) {
      return true;
    }
    return false;
  }
  inline bool on_any_item_cell(int cell_type) {
    if (cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL or
        cell_type == CellType::ITEM_BOMB_CNT_UP_CELL) {
      return true;
    }
    return false;
  }

  inline bool on_object_cell(int cell_type) {
    if (cell_type != CellType::EMPTY_CELL) {
      return true;
    }
    return false;
  }

  void
  simulate_bomb_inducing_explosion(const pair<int8_t, int8_t> exploded_key,
                                   map<pair<int8_t, int8_t>, int8_t> &mp_explosion_range,
                                   BitBoard &board) {
    int px = exploded_key.second;
    int py = exploded_key.first;
    int cell_type = board.get(py, px);

    assert(in_board(py, px));
    board.set(py, px, CellType::BOMB_EXPLODED_CELL);

    int range = mp_explosion_range[make_pair(py, px)];
    // d > 1
    for (int k = 0; k < 4; k++) {
      for (int d = 1; d < range; d++) {
        int ny, nx;
        nx = px + d * DX[k];
        ny = py + d * DY[k];
        if (not in_board(ny, nx))
          break;
        int cell_type = board.get(ny, nx);
        if (on_bomb_cell(cell_type)) { // inducing explosion
          simulate_bomb_inducing_explosion(make_pair(ny, nx),
                                           mp_explosion_range, board);
          break;
        } else if (on_object_cell(cell_type)) { // exsist object
          break;
        }
      }
    }
    return;
  }
  BitBoard simulate_bomb_explosion(StateInfo &state, bool do_update = false) {
    Bombs &bombs = state.bombs;
    map<pair<int8_t, int8_t>, int8_t> mp_explosion_range;
    for (int i = 0; i < bombs.size(); i++) {
      bombs[i].dec_turn();
      int8_t x, y;
      x = bombs[i].x;
      y = bombs[i].y;
      int range = bombs[i].explosion_range;
      mp_explosion_range[make_pair(y, x)] =
          MAX(mp_explosion_range[make_pair(y, x)], range);
    }

    for (int i = 0; i < bombs.size(); i++) {
      int8_t x, y;
      x = bombs[i].x;
      y = bombs[i].y;
      int cell_type = state.board.get(y, x);
      if (bombs[i].is_explode() and on_bomb_cell(cell_type)) {
	simulate_bomb_inducing_explosion(make_pair(y, x), mp_explosion_range,
                                         state.board);
      }
    }

    BitBoard next_board = state.board;
    for (int i = 0; i < bombs.size(); i++) {
      const int px = bombs[i].x;
      const int py = bombs[i].y;
      const int cell_type = next_board.get(py, px);
      if (on_bomb_exploded_cell(cell_type)) { // bomb is exploded
        int owner = bombs[i].owner;
        int range = bombs[i].explosion_range;
        state.players[owner].remain_bomb_cnt++;
	state.explosion_turn_board.set(py, px, 0);
        // d == 0
        {
          int player_y, player_x;
          for (int player_id = 0; player_id < GameRule::MAX_PLAYER_NUM;
               player_id++) {
            if (state.players[player_id].is_dead())
              continue;
            player_y = state.players[player_id].y;
            player_x = state.players[player_id].x;
            if ((py == player_y and px == player_x)) {
              state.players[player_id].survival = false;

            }
          }
        }

        // d > 1
        for (int k = 0; k < 4; k++) {
          for (int d = 1; d < range; d++) {
            int ny, nx;
            nx = px + d * DX[k];
            ny = py + d * DY[k];
            if (not in_board(ny, nx))
              break;
            int old_cell_type = next_board.get(ny, nx);
	    state.explosion_turn_board.set(ny, nx, 0);
            // Attack Player
            {
              int player_y, player_x;
              for (int player_id = 0; player_id < GameRule::MAX_PLAYER_NUM;
                   player_id++) {
                if (state.players[player_id].is_dead())
                  continue;
                player_y = state.players[player_id].y;
                player_x = state.players[player_id].x;
                if ((ny == player_y and nx == player_x)) {
                  state.players[player_id].survival = false;
		  //cerr << "death " << py << " " << px << endl;
                }
              }
            }

            if (old_cell_type == CellType::BOMB_CELL or
                old_cell_type == CellType::BOMB_EXPLODED_CELL or
                old_cell_type == CellType::WALL_CELL) { // exsit bombs or wall
              break;
            }
            // Destroy object
            if (old_cell_type == CellType::BOX_CELL or
                old_cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL or
                old_cell_type ==
                    CellType::ITEM_BOMB_CNT_UP_CELL) { // EMPTY CELL
              next_board.set(ny, nx, CellType::EMPTY_CELL);
            } else if (old_cell_type ==
                       CellType::BOX_ITEM_BOMB_RANGE_UP_CELL) { // box item
                                                                // range up
              next_board.set(ny, nx, CellType::ITEM_BOMB_RANGE_UP_CELL);
            } else if (old_cell_type ==
                       CellType::BOX_ITEM_BOMB_CNT_UP_CELL) { // box item cnt up
              next_board.set(ny, nx, CellType::ITEM_BOMB_CNT_UP_CELL);
            }

            if (on_object_cell(old_cell_type)) {
              if (on_any_box_cell(old_cell_type)) {
                state.players[owner].sum_box_point += 1;
                // update
                if (do_update) { // sum box point
                  external_player_info[owner] += 1;
                }
              }
              break;
            }
          }
        }
      }
    }
    Bombs next_bombs;
    for (int i = 0; i < bombs.size(); i++) {
      int x = bombs[i].x;
      int y = bombs[i].y;
      int cell_type = state.board.get(y, x);
      if (on_bomb_cell(cell_type)) { // not exploded
        next_bombs.emplace_back(bombs[i]);
      } else { // exploed
        state.board.set(y, x, CellType::EMPTY_CELL);
        next_board.set(y, x, CellType::EMPTY_CELL);
      }
    }
    //sort(next_bombs.begin(), next_bombs.end());
    bombs = move(next_bombs);
    return next_board;
  }
  bool is_surrouned_bombs(int id, const SearchState &search_state){
    const int8_t x = search_state.state.players[id].x;
    const int8_t y = search_state.state.players[id].y;
    int8_t surround_cnt = 0;
    if (y % 2 == 0){//
      if (x % 2 == 0){
	//right up left down
	for (int i = 0; i < 4; i++){
	  const int8_t ny = y + DY[i];
	  const int8_t nx = x + DX[i];
	  if (not in_board(ny, nx)){
	    surround_cnt++;
	    continue;
	  }
	  const int8_t cell_type = search_state.state.board.get(ny, nx);

	  if (cell_type == CellType::BOMB_CELL or cell_type == CellType::WALL_CELL){
	    surround_cnt++;
	  }else{
	    break;
	  }
	}
	return surround_cnt >= 4;
      }else{
	//right
	const int8_t right_x = x + DX[0];
	const int8_t right_y = y + DY[0];
	if (not in_board(right_y, right_x)){
	  surround_cnt++;
	}else{
	  int8_t cell_type = search_state.state.board.get(right_y, right_x);
	  if (cell_type == CellType::BOMB_CELL or cell_type == CellType::WALL_CELL){
	    surround_cnt++;
	  }
	}

	//left
	const int8_t left_x = x + DX[2];
	const int8_t left_y = y + DY[2];
	if (not in_board(right_y, right_x)){
	  surround_cnt++;
	}else{
	  int8_t cell_type = search_state.state.board.get(left_y, left_x);
	  if (cell_type == CellType::BOMB_CELL or cell_type == CellType::WALL_CELL){
	    surround_cnt++;
	  }
	}
	return surround_cnt >= 2;
      }
    }else{
      if (x % 2 == 0){
	//upper lower
	const int8_t upper_x = x + DX[0];
	const int8_t upper_y = y + DY[0];
	if (not in_board(upper_y, upper_x)){
	  surround_cnt++;
	}else{
	  int8_t cell_type = search_state.state.board.get(upper_y, upper_x);
	  if (cell_type == CellType::BOMB_CELL or cell_type == CellType::WALL_CELL){
	    surround_cnt++;
	  }
	}

	//lower
	const int8_t lower_x = x + DX[3];
	const int8_t lower_y = y + DY[3];
	if (not in_board(lower_y, lower_x)){
	  surround_cnt++;
	}else{
	  int8_t cell_type = search_state.state.board.get(lower_y, lower_x);
	  if (cell_type == CellType::BOMB_CELL or cell_type == CellType::WALL_CELL){
	    surround_cnt++;
	  }
	}
	return surround_cnt >= 2;
      }

    }

    return false;
  }
  double calc_score(int id, const SearchState &pre_state,
                    const SearchState &search_state) {
    double score = 0;
    const int px = search_state.state.players[id].x;
    const int py = search_state.state.players[id].y;
    const int range = search_state.state.players[id].explosion_range;
    // score += 6 * (search_state.my_box_point -
    // pre_state.my_box_point);

    // death penalty
    // cerr << "unko" << endl;
    if (search_state.state.players[id].is_dead()) {
      score -= 1e30;
    }
    score *= 100;
    
    score += 20 * (search_state.state.players[id].sum_box_point);
    score += 3 * (MIN(13, (int)search_state.state.players[id].explosion_range) - 3);
    score += 3 * (MIN(7, (int)search_state.state.players[id].max_bomb_cnt) - 1);
    score += (MIN(7, (int)search_state.state.players[id].remain_bomb_cnt));
    
    score *= 100;
    // score += 4 * (search_state.state.players[id].explosion_range - 3);
    // score += 4 * (search_state.state.players[id].max_bomb_cnt - 1);
    // score += (search_state.state.my_info.remain_bomb_cnt - 1);
    int sum_man_dist = 0;
    int min_dist = (BOARD_HEIGHT + BOARD_WIDTH + 1);
    int active_boxes_cnt = 0;

    for (int y = 0; y < BOARD_HEIGHT; y++) {
      for (int x = 0; x < BOARD_WIDTH; x++) {
        int cell_type = search_state.state.board.get(y, x);
        if (cell_type == CellType::BOX_CELL or
            cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
            cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL) {
          int dist = abs(px - x) + abs(py - y);
          active_boxes_cnt++;
          min_dist = MIN(min_dist, dist);
          sum_man_dist += dist;
        }
      }
    }
    score += 12 * ((BOARD_HEIGHT + BOARD_WIDTH) - min_dist);
    score += (BOARD_HEIGHT + BOARD_WIDTH) * (active_boxes_cnt)-sum_man_dist;
    score *= 100;

    // 0 0
    // penlaty corner
    // upper left 0 0
    int d0, d1, d2, d3;
    int res = 0;
    d0 = abs(px - 0) + abs(py - 0);
    res = MAX(res, d0);
    // upper right BOARD_HEIGHH - 1, 0
    d1 = abs(px - (BOARD_WIDTH - 1)) + abs(py - 0);
    res = MAX(res, d1);
    // lower left
    d2 = abs(px - 0) + abs(py - (BOARD_HEIGHT - 1));
    res = MAX(res, d2);
    // lower right
    d3 = abs(px - (BOARD_WIDTH - 1)) + abs(py - (BOARD_HEIGHT - 1));
    res = MAX(res, d3);
    score -= res;
    //score -= max({d0, d1, d2, d3});
    return score;
  }
  void simulate_next_move_and_set_bomb(
      int id, const SearchState &state, const BitBoard &next_board,
      priority_queue<SearchState> &search_states, const int &turn) {
    if (state.state.players[id].is_dead())
      return;
    const int px = state.state.players[id].x;
    const int py = state.state.players[id].y;
    const int range = state.state.players[id].explosion_range;
    bool place_bomb = true;
    if (state.state.players[id].get_remain_bomb_cnt() <= 0) {
      place_bomb = false;
    } else if (state.state.board.get(py, px) ==
               CellType::BOMB_CELL) { // already set bomb
      place_bomb = false;
    }

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
      if (k != 4) { // can not move
        if (cell_type == CellType::BOMB_CELL or
            cell_type == CellType::BOMB_EXPLODED_CELL) {
          continue;
        }
      }
      SearchState next_state = state;
      next_state.state.board = next_board;
      //next_state.state.explosion_turn_board.clear();
      //for (int i = 0; i < )
      if (cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL) {
        next_state.state.players[id].explosion_range++;
        next_state.state.board.set(ny, nx, CellType::EMPTY_CELL);
      } else if (cell_type == CellType::ITEM_BOMB_CNT_UP_CELL) {
        next_state.state.players[id].max_bomb_cnt += 1;
        next_state.state.players[id].remain_bomb_cnt += 1;
        next_state.state.board.set(ny, nx, CellType::EMPTY_CELL);
      }
      next_state.state.players[id].y = ny;
      next_state.state.players[id].x = nx;

      // not set bombs
      if (turn == 0) {
        next_state.first_act = Act(ny, nx, ACT_MOVE);
      }
      next_state.score = calc_score(id, state, next_state);
      search_states.emplace(next_state);
      
      if (place_bomb) {
        next_state.state.board.set(py, px, CellType::BOMB_CELL);
	if (is_surrouned_bombs(id, next_state)){
	  continue;
	}
	//const int cell_explosion_turn = next_state.state.explosion_turn_board.get(py, px);
	
	// if (cell_explosion_turn != 0){
	//   //cerr << cell_explosion_turn << endl;
	//   next_state.state.explosion_turn_board.set(py, px, cell_explosion_turn - 1);
	//   next_state.state.bombs.emplace_back(Bomb(py, px, id, cell_explosion_turn - 1, range));
	// }else{
	//   next_state.state.explosion_turn_board.set(py, px, 8);
	//   next_state.state.bombs.emplace_back(Bomb(py, px, id, 8, range));
	// }
	next_state.state.explosion_turn_board.set(py, px, 8);
	next_state.state.bombs.emplace_back(Bomb(py, px, id, 8, range));
        next_state.state.players[id].remain_bomb_cnt--;

        if (turn == 0) {
	  //cerr << "set " << ny << " " << nx << " " << next_state.state.bombs.size() << endl;
	  
          next_state.first_act = Act(ny, nx, ACT_BOMB);
	  
        }
        next_state.score = calc_score(id, state, next_state);
        search_states.emplace(next_state);
      }
    }
  }

  void output_act_by_cho_search(const Act &act, int iter) {
    int y, x;
    y = act.y;
    x = act.x;

    if (act.act_id == ACT_MOVE) {
      cout << "MOVE"
           << " " << x << " " << y << " " << iter << endl;
    } else {
      cout << "BOMB"
           << " " << x << " " << y << " " << iter << endl;
    }
  }


  void simulate_propagated_remain_turn(Bombs& bombs, BitBoard &explosion_turn_board, const BitBoard &board){
    if (bombs.empty())return ;

    explosion_turn_board.clear();
    for (int i = 0; i < bombs.size(); i++){
      const int x = bombs[i].x;
      const int y = bombs[i].y;
      const int range = bombs[i].explosion_range;
      const int explosion_turn = bombs[i].explosion_turn;
      assert(explosion_turn > 0);
      explosion_turn_board.set(y, x, explosion_turn);
      
    }

    
    for (int i = 0; i < bombs.size(); i++){
      const int x = bombs[i].x;
      const int y = bombs[i].y;
      const int range = bombs[i].explosion_range;
      const int cell_explosion_turn = explosion_turn_board.get(y, x);
      assert(cell_explosion_turn > 0);
      bombs[i].explosion_turn = cell_explosion_turn;
      //bombs[i].explosion_turn -= 1;
      for (int dir = 0; dir < 4; dir++){
	for (int d = 1; d < range; d++){
	  const int nx = x + d * DX[dir];
	  const int ny = y + d * DY[dir];
	  if (not in_board(ny, nx))break;
	  const int cell_type = board.get(ny, nx);
	  if (cell_type == CellType::WALL_CELL)break;
	  if (on_any_box_cell(cell_type) or on_any_item_cell(cell_type))break;
	  const int cell_curr_explosion_turn = explosion_turn_board.get(ny, nx);
	  const int explosion_turn = bombs[i].explosion_turn;
	  if (cell_curr_explosion_turn != 0){
	    explosion_turn_board.set(ny, nx, MIN(cell_curr_explosion_turn, explosion_turn));
	  }else{
	    explosion_turn_board.set(ny, nx, explosion_turn);
	  }
	}
      }
    }

    //update bombs
    for (int i = 0; i < bombs.size(); i++){
      const int x = bombs[i].x;
      const int y = bombs[i].y;
      const int range = bombs[i].explosion_range;
      const int cell_explosion_turn = explosion_turn_board.get(y, x);
      assert(cell_explosion_turn > 0);
      //update
      bombs[i].explosion_turn = cell_explosion_turn;
      explosion_turn_board.set(y, x, cell_explosion_turn);
    }    
    // cerr << "--after propagated--"<< endl;
    // explosion_turn_board.debug();
    
    return ;
  }
  
  void think(const StateInfo &init_info) {
    Timer timer;
    timer.start();
    if (game_turn > 0){
      SearchState update_search_state;
      update_search_state.state = init_info;
      update_state(update_search_state.state);
    }

    const int beam_width = 20;
    const int depth_limit = 20;
    priority_queue<SearchState> curr_search_states[depth_limit + 1];
    set<tuple<Player, BitBoard, Bombs>> visited[depth_limit + 1];

    SearchState init_search_state;
    init_search_state.state = init_info;
    for (int i = 0; i < GameRule::MAX_PLAYER_NUM; i++){
      if (init_search_state.state.players[i].is_dead())continue;
      init_search_state.state.players[i].sum_box_point = external_player_info[i];
    }
    debug_players_info(init_search_state.state.players);
    curr_search_states[0].emplace(init_search_state);
    Act tmp_best_act;
    pair<int, double> tmp_best(0, 0);
    int chokudai_iter = 0;
    int prune_cnt = 0;
    int output_depth = 0;
    while (timer.get_mill_duration() <= 85) {
      chokudai_iter++;
      for (int turn = 0; turn < depth_limit; turn++) {
	if (turn + game_turn >= GameRule::MAX_TURN)break;
        for (int iter = 0;
             iter < beam_width and (not curr_search_states[turn].empty());
             iter++) {
          if (timer.get_mill_duration() >= 85){
            goto END;
	  }
          SearchState curr_search_state = curr_search_states[turn].top();
          curr_search_states[turn].pop();
	  sort(curr_search_state.state.bombs.begin(), curr_search_state.state.bombs.end());

	  //estimate propagate decrease

	  
          auto key = make_tuple(curr_search_state.state.players,
                                curr_search_state.state.board,
                                curr_search_state.state.bombs);
          if (visited[turn].count(key) > 0) {
            iter--;
            // prune_cnt++;
            continue;
          }
          visited[turn].emplace(key);
	  
          auto tmp_score = make_pair(turn, curr_search_state.score);
          if (tmp_score > tmp_best) {
            tmp_best = tmp_score;
            tmp_best_act = curr_search_state.first_act;
          }

	  simulate_propagated_remain_turn(curr_search_state.state.bombs, curr_search_state.state.explosion_turn_board, curr_search_state.state.board);
          BitBoard next_board =
              simulate_bomb_explosion(curr_search_state.state, false);
          // next state
          // move
          simulate_next_move_and_set_bomb(my_id, curr_search_state, next_board,
                                          curr_search_states[turn + 1], turn);

        }
	output_depth = MAX(output_depth, turn + 1);
      }
      //break;
    }
  END:;
    // cerr << prune_cnt << endl;
    // cerr << curr_search_states[depth_limit].size() << endl;
    cerr << chokudai_iter++ << " " << (int)output_depth << endl;
    if (not curr_search_states[output_depth].empty() and
        curr_search_states[output_depth].top().score > 0) {
      SearchState best = curr_search_states[output_depth].top();
      //best.state.board.debug();
      //best.state.explosion_turn_board.debug();
      cerr << (int)best.state.players[my_id].survival << " "
           << (int)best.state.players[my_id].sum_box_point << " "
           << (int)best.state.players[my_id].max_bomb_cnt << " "
           << (int)best.state.players[my_id].explosion_range << " " << best.score
           << endl;
      output_act_by_cho_search(best.first_act, chokudai_iter);
      next_pos = make_pair(best.first_act.y, best.first_act.x);
    } else {
      cerr << "temp action" << endl;
      output_act_by_cho_search(tmp_best_act, chokudai_iter);
      next_pos = make_pair(tmp_best_act.y, tmp_best_act.x);
    }

  }
  
  array<int, GameRule::MAX_PLAYER_NUM>
      external_player_info; // box point, max_bomb_cnt
  void debug_players_info(const Player &players) {
    cerr << "----------------------Player Info "
            "Start----------------------------------------------"
         << endl;
    for (int i = 0; i < GameRule::MAX_PLAYER_NUM; i++) {
      if (players[i].is_dead())
        continue;
      cerr << "id = " << i << " sum_box_point = " << (int)players[i].sum_box_point
           << " max_bomb_cnt = " << (int)players[i].max_bomb_cnt
           << " explosion_range = " << (int)players[i].explosion_range
           << " remain_bomb_cnt = " << (int)players[i].remain_bomb_cnt << endl;
    }
    cerr << "----------------------Player Info "
            "End----------------------------------------------"
         << endl;
  }
  void update_state(StateInfo &curr_state) {
    // update box point
    StateInfo tmp_state = curr_state;
    simulate_bomb_explosion(tmp_state, true);
    for (int i = 0; i < GameRule::MAX_PLAYER_NUM; i++) {
      if (tmp_state.players[i].is_dead())
        continue;
      // current
      int py = tmp_state.players[i].y;
      int px = tmp_state.players[i].x;
      // update state
      curr_state.players[i].sum_box_point = external_player_info[i];
    }
    return;
  }
  //----------------------------data----------------------------------------------
  // dubug
  pair<int, int> next_pos;
  int my_id;
  int game_player_num;
  int curr_player_num;
  int game_turn;
  Timer game_timer;
  //----------------------------data----------------------------------------------

};

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
 
  Solver solver;
  solver.solve();
}

