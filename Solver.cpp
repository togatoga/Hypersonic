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
  const int EMPTY_CELL = 0;
  const int BOX_CELL = 1;
  const int BOMB_CELL = 2;
  const int BOMB_EXPLODED_CELL = 3;
  const int ITEM_BOMB_RANGE_UP_CELL = 4;
  const int BOX_ITEM_BOMB_RANGE_UP_CELL = 5;
  const int ITEM_BOMB_CNT_UP_CELL = 6;
  const int BOX_ITEM_BOMB_CNT_UP_CELL = 7;
  const int WALL_CELL = 8;
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
    //-----------------------------init------------------------------------------
    my_id = myid;
    game_timer.start();
    game_turn = 0;
    external_player_info.fill(0);
    next_pos = make_pair(-1, -1);
    game_player_num = -1;
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
      if (owner != right.owner) {
        return true;
      }
      if (y != right.y) {
        return true;
      }
      if (x != right.x) {
        return true;
      }
      if (explosion_turn != right.explosion_turn) {
        return true;
      }
      return false;
    }
    inline bool operator == (const Bomb &right) const {
      if (owner != right.owner) {
        return false;
      }
      if (y != right.y) {
        return false;
      }
      if (x != right.x) {
        return false;
      }
      if (explosion_turn != right.explosion_turn) {
        return false;
      }
      return true;
    }
    inline bool operator<(const Bomb &right) const {
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
    int8_t x, y;
    int8_t max_bomb_cnt;

    int8_t remain_bomb_cnt;
    int8_t explosion_range;

    // determine winner player
    int8_t sum_box_point;
    int8_t dead_turn;
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
      if (dead_turn != right.dead_turn){
	return dead_turn < right.dead_turn;
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
    Bombs bombs;
    Player players;
    int8_t curr_game_player_num;
    int16_t curr_game_turn;
    StateInfo() {
      curr_game_player_num = 0;
      curr_game_turn = 0;
    }
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

    int curr_game_player_num = 0;
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
	curr_game_player_num++;
      } else if (entityType == EntityType::BOMB) { // Bomb
        res.players[owner].max_bomb_cnt++;
        res.bombs.emplace_back(move(Bomb(y, x, owner, param1, param2)));
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
    res.curr_game_player_num = curr_game_player_num;
    if (game_player_num == -1){
      game_player_num = curr_game_player_num;
    }
    for (int i = 0; i < game_player_num; i++){
      if (res.players[i].is_survive()){
	res.players[i].dead_turn = -1;
      }else{
	res.players[i].dead_turn = player_dead_turns[i];
      }
    }

      
    if (verbose) {
      res.board.debug();
    }
    
    cerr
        << "----------------------------Input End------------------------------"
        << endl;
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
  simulate_bomb_inducing_explosion(const pair<int, int> exploded_key,
                                   map<pair<int, int>, int> &mp_explosion_range,
                                   BitBoard &board) {
    int px = exploded_key.second;
    ;
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
    map<pair<int, int>, int> mp_explosion_range;
    for (int i = 0; i < bombs.size(); i++) {
      bombs[i].dec_turn();
      int x, y;
      x = bombs[i].x;
      y = bombs[i].y;
      int range = bombs[i].explosion_range;
      mp_explosion_range[make_pair(y, x)] =
          max(mp_explosion_range[make_pair(y, x)], range);
    }

    for (int i = 0; i < bombs.size(); i++) {
      int x, y;
      x = bombs[i].x;
      y = bombs[i].y;
      int cell_type = state.board.get(y, x);
      if (bombs[i].is_explode() and on_bomb_cell(cell_type)) {
        // cerr << y << " " << x << endl;
        simulate_bomb_inducing_explosion(make_pair(y, x), mp_explosion_range,
                                         state.board);
      }
    }

    BitBoard next_board = state.board;
    int player_num = state.curr_game_turn;
    for (int i = 0; i < bombs.size(); i++) {
      const int px = bombs[i].x;
      const int py = bombs[i].y;
      const int cell_type = next_board.get(py, px);
      if (on_bomb_exploded_cell(cell_type)) { // bomb is exploded
        int owner = bombs[i].owner;
        int range = bombs[i].explosion_range;
        state.players[owner].remain_bomb_cnt++;
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
	      state.players[player_id].dead_turn = state.curr_game_turn;
		
		
	      if (do_update) { // dead turn
		player_dead_turns[player_id] = state.curr_game_turn;
	      }
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
            int old_cell_type = state.board.get(ny, nx);
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
		  state.players[player_id].dead_turn = state.curr_game_turn;

		  if (do_update) { // dead tur
		    player_dead_turns[player_id] = state.curr_game_turn;
		  }
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

  bool is_game_end(const SearchState &state){
    if (state.state.curr_game_turn >= GameRule::MAX_TURN)return true;//turn over
    if (state.state.players[my_id].is_dead())return true;//killed
    if (state.state.curr_game_player_num == 1 and state.state.players[my_id].is_survive())return true;//win
    if (state.state.curr_game_player_num <= 0)return true;// all killed
    return false;
  }
  double calc_playout_score(const SearchState &state){
    assert(is_game_end(state));
    int score = game_player_num;//
    if (state.state.players[my_id].is_dead()){//dead
      int my_dead_turn = state.state.players[my_id].dead_turn;
      int my_sum_box_point = state.state.players[my_id].sum_box_point;
      for (int i = 0; i < game_player_num; i++){
	if (i == my_id)continue;
	if (state.state.players[i].is_survive()){
	  score--;
	}else{
	  if (my_dead_turn < state.state.players[i].dead_turn){
	    score--;
	  }else if(score == state.state.players[i].dead_turn){
	    if (my_sum_box_point < state.state.players[i].sum_box_point){
	      score--;
	    }
	  }
	}
      }
    }else{//alive
      if (state.state.curr_game_player_num == 1){
	
      }else if (state.state.curr_game_turn < GameRule::MAX_TURN){//turn over
	int8_t my_sum_box_point = state.state.players[my_id].sum_box_point;
	for (int i = 0; i < game_player_num; i++){
	  if (i == my_id)continue;
	  if (state.state.players[i].is_dead())continue;
	  if (my_sum_box_point < state.state.players[i].sum_box_point){
	    score--;
	  }
	}
      }
    }
    return score;
  }



  vector<pair<int, Act>> simulate_lazy_enemy_move_and_set_bomb(const SearchState &pre_state, SearchState &next_state){


    vector<pair<int, Act>> enemy_actions;
    for (int player_id = 0; player_id < GameRule::MAX_PLAYER_NUM; player_id++){//
      if (player_id == my_id)continue;
      if (pre_state.state.players[player_id].is_dead())continue;
      const int px = pre_state.state.players[player_id].x;
      const int py = pre_state.state.players[player_id].y;
      const int range = pre_state.state.players[player_id].explosion_range;
      bool place_bomb = xor128() % 2;
      int next_dir = xor128() % 5;
      const int cell_type = pre_state.state.board.get(py, px);
      if (pre_state.state.players[player_id].get_remain_bomb_cnt() <= 0) {
	place_bomb = false;
      } else if (cell_type == 
		 CellType::BOMB_CELL) { // already set bomb
	place_bomb = false;
      }
      int nx = px + DX[next_dir];
      int ny = py + DY[next_dir];
      if (not in_board(ny, nx)){//
	nx = px;
	ny = py;
      }

      int move_cell_type = pre_state.state.board.get(ny, nx);
      if (move_cell_type == CellType::BOX_CELL or
          move_cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
          move_cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL or
          move_cell_type == CellType::WALL_CELL){
	nx = px;
	ny = py;
	move_cell_type = cell_type;
      }else if (next_dir != 4) { // can not move
        if (move_cell_type == CellType::BOMB_CELL or
            move_cell_type == CellType::BOMB_EXPLODED_CELL) {
	  nx = px;
	  ny = py;
	  move_cell_type = cell_type;
        }
      }
      //item get
      if (move_cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL) {
        next_state.state.players[player_id].explosion_range++;
        next_state.state.board.set(ny, nx, CellType::EMPTY_CELL);
      } else if (move_cell_type == CellType::ITEM_BOMB_CNT_UP_CELL) {
        next_state.state.players[player_id].max_bomb_cnt += 1;
        next_state.state.players[player_id].remain_bomb_cnt += 1;
        next_state.state.board.set(ny, nx, CellType::EMPTY_CELL);
      }
      next_state.state.players[player_id].y = ny;
      next_state.state.players[player_id].x = nx;
      if (place_bomb){
	next_state.state.board.set(py, px, CellType::BOMB_CELL);
	next_state.state.bombs.emplace_back(Bomb(py, px, player_id, 8, range));
	//sort(next_state.state.bombs.begin(), next_state.state.bombs.end());
	next_state.state.players[player_id].remain_bomb_cnt--;
      }
      enemy_actions.emplace_back(make_pair(player_id, Act(ny, nx, place_bomb ? ACT_BOMB : ACT_MOVE)));
    }
    return enemy_actions;
  }

  void simulate_MC_play(SearchState &state, BitBoard &next_board){
    for (int player_id = 0; player_id < GameRule::MAX_PLAYER_NUM; player_id++){
      if (state.state.players[player_id].is_dead())continue;
      const int px = state.state.players[player_id].x;
      const int py = state.state.players[player_id].y;
      const int range = state.state.players[player_id].explosion_range;
      bool place_bomb = false;
      int next_dir = xor128() % 5;
      const int cell_type = state.state.board.get(py, px);
      if (state.state.players[player_id].get_remain_bomb_cnt() <= 0) {
	place_bomb = false;
      } else if (cell_type == 
		 CellType::BOMB_CELL) { // already set bomb
	place_bomb = false;
      }
      int nx = px + DX[next_dir];
      int ny = py + DY[next_dir];
      if (not in_board(ny, nx)){//
	nx = px;
	ny = py;
      }
      int move_cell_type = state.state.board.get(ny, nx);
      if (move_cell_type == CellType::BOX_CELL or
          move_cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
          move_cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL or
          move_cell_type == CellType::WALL_CELL){
	nx = px;
	ny = py;
	move_cell_type = cell_type;
      }else if (next_dir != 4) { // can not move
        if (move_cell_type == CellType::BOMB_CELL or
            move_cell_type == CellType::BOMB_EXPLODED_CELL) {
	  nx = px;
	  ny = py;
	  move_cell_type = cell_type;
        }
      }
      //item get
      if (move_cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL) {
        state.state.players[player_id].explosion_range++;
        next_board.set(ny, nx, CellType::EMPTY_CELL);
      } else if (move_cell_type == CellType::ITEM_BOMB_CNT_UP_CELL) {
        state.state.players[player_id].max_bomb_cnt += 1;
        state.state.players[player_id].remain_bomb_cnt += 1;
        next_board.set(ny, nx, CellType::EMPTY_CELL);
      }
      state.state.players[player_id].y = ny;
      state.state.players[player_id].x = nx;
      if (place_bomb){
	next_board.set(py, px, CellType::BOMB_CELL);
	state.state.bombs.emplace_back(Bomb(py, px, player_id, 8, range));
	//sort(state.state.bombs.begin(), state.state.bombs.end());
	state.state.players[player_id].remain_bomb_cnt--;
      }
    }
    state.state.board = next_board;

    
    return ;
  }

  
  pair<vector<pair<int, Act>>, double> playout(const SearchState &pre_state, const SearchState &next_state, int turn){
    //int curr_game_turn = turn + curr_game_turn;
    BitBoard next_board;
    SearchState playout_state = next_state;
    //lazy enemy move
    vector<pair<int, Act>> enemy_actions = simulate_lazy_enemy_move_and_set_bomb(pre_state, playout_state);
    playout_state.state.curr_game_turn++;
    while (true){
      if (is_game_end(playout_state))break;
      next_board = simulate_bomb_explosion(playout_state.state, false);
      simulate_MC_play(playout_state, next_board);
      playout_state.state.curr_game_turn++;
    }
    return make_pair(enemy_actions, calc_playout_score(playout_state));
  }

  void simulate_enemy_worst_actions(vector<pair<int, Act>> &actions, const SearchState &pre_state, SearchState &next_state){
    for (int i = 0; i < actions.size(); i++){
      int id = actions[i].first;
      Act act = actions[i].second;
      assert(id != my_id);
      const int nx = act.x;
      const int ny = act.y;
      const int act_id = act.act_id;
      const int range = next_state.state.players[id].explosion_range;
      int cell_type = pre_state.state.board.get(ny, nx);
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
      if (act_id == ACT_BOMB){
	next_state.state.board.set(ny, nx, CellType::BOMB_CELL);
        next_state.state.bombs.emplace_back(Bomb(ny, nx, id, 8, range));
        next_state.state.players[id].remain_bomb_cnt--;
      }
    }
  }
  double calc_score(int id, const SearchState &pre_state,
                    SearchState &search_state, int turn) {
    double score = 0;
    const int px = search_state.state.players[id].x;
    const int py = search_state.state.players[id].y;
    const int range = search_state.state.players[id].explosion_range;
    //cerr << "unko" << endl;
    if (search_state.state.players[id].is_dead()) {
      score -= 1e30;
    }
    score *= 100;

    //playout
    double playout_score = 0;
    double worst_score = 1e30;
    vector<pair<int, Act>> worst_enemy_actions;
    for (int iter = 0; iter < 5; iter++){
      pair<vector<pair<int, Act>>, double> res;
      res = playout(pre_state, search_state, turn);
      playout_score += res.second;
      if (res.second < worst_score){
	worst_score = res.second;
	worst_enemy_actions = move(res.first);
      }
    }
    simulate_enemy_worst_actions(worst_enemy_actions, pre_state, search_state);
    score += playout_score * 0.1;
    
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

    // for (int y = 0; y < BOARD_HEIGHT; y++) {
    //   for (int x = 0; x < BOARD_WIDTH; x++) {
    //     int cell_type = search_state.state.board.get(y, x);
    //     if (cell_type == CellType::BOX_CELL or
    //         cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
    //         cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL) {
    //       // if (search_state.state.future_destroied_boxes.count(make_pair(y,
    //       // x)) >
    //       //     0)
    //       //   continue;
    //       int dist = abs(px - x) + abs(py - y);
    //       active_boxes_cnt++;
    //       min_dist = min(min_dist, dist);
    //       sum_man_dist += dist;
    //     }
    //   }
    // }
    
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
      SearchState next_move_state = next_state;
      next_move_state.score = calc_score(id, state, next_move_state, turn);
      search_states.emplace(next_move_state);
      
      if (place_bomb) {
	SearchState next_bomb_state = move(next_state);
        next_state.state.board.set(py, px, CellType::BOMB_CELL);
        next_state.state.bombs.emplace_back(Bomb(py, px, id, 8, range));
        next_state.state.players[id].remain_bomb_cnt--;
        if (turn == 0) {
          next_state.first_act = Act(ny, nx, ACT_BOMB);
        }
        next_state.score = calc_score(id, state, next_state, turn);
	
        search_states.emplace(next_state);
      }
    }
  }

  void output_act(const Act &act, int iter) {
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
  void think(const StateInfo &init_info) {
    // cerr << "--think--" << endl;
    // cerr << init_info.board[0][0] << endl;
    Timer timer;
    timer.start();
    const int beam_width = 10;
    const int depth_limit = 13;
    priority_queue<SearchState> curr_search_states[depth_limit + 1];
    set<tuple<Player, BitBoard, Bombs>> visited[depth_limit + 1];

    SearchState init_search_state;
    init_search_state.state = init_info;
    // print players info

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
	if (turn + game_turn >= GameRule::MAX_TURN){//game end
	  break;
	}
        for (int iter = 0;
             iter < beam_width and (not curr_search_states[turn].empty());
             iter++) {
          if (timer.get_mill_duration() >= 85)
            goto END;
          SearchState curr_search_state = curr_search_states[turn].top();
          curr_search_states[turn].pop();
	  sort(curr_search_state.state.bombs.begin(), curr_search_state.state.bombs.end());
          auto key = make_tuple(curr_search_state.state.players,
                                curr_search_state.state.board,
                                curr_search_state.state.bombs);
	  
          if (visited[turn].count(key) > 0) {
            iter--;
            // prune_cnt++;
            continue;
          }
          auto tmp_score = make_pair(turn, curr_search_state.score);
          if (tmp_score > tmp_best) {
            tmp_best = tmp_score;
            tmp_best_act = curr_search_state.first_act;
          }
          visited[turn].emplace(key);
          // simulate bomb
          // if (turn != 0)
          BitBoard next_board =
              simulate_bomb_explosion(curr_search_state.state, false);
          // next state
          // move
          simulate_next_move_and_set_bomb(my_id, curr_search_state, next_board,
                                          curr_search_states[turn + 1], turn);
	  output_depth = MAX(turn + 1, output_depth);
        }

      }
      // break;
    }
  END:;

    cerr << chokudai_iter++ << " " << output_depth << endl;
    if (not curr_search_states[output_depth].empty() and
        curr_search_states[output_depth].top().score > 0) {
      SearchState best = curr_search_states[output_depth].top();

      cerr << (int)best.state.players[my_id].survival << " "
           << (int)best.state.players[my_id].sum_box_point << " "
           << (int)best.state.players[my_id].max_bomb_cnt << " "
           << (int)best.state.players[my_id].explosion_range << " " << best.score
           << endl;
      output_act(best.first_act, chokudai_iter);
      next_pos = make_pair(best.first_act.y, best.first_act.x);
    } else {
      cerr << "temp action" << endl;
      output_act(tmp_best_act, chokudai_iter);
      next_pos = make_pair(tmp_best_act.y, tmp_best_act.x);
    }
    if (game_turn > 0)
      update_state(init_search_state.state);
  }
  // dubug
  pair<int, int> next_pos;
  //----------------------------data----------------------------------------------
  int my_id;
  int game_turn;
  Timer game_timer;
  int game_player_num;
  array<int, GameRule::MAX_PLAYER_NUM>
      external_player_info; // box point, max_bomb_cnt
  array<int, GameRule::MAX_PLAYER_NUM> player_dead_turns;
  

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
  unsigned long xor128(){
    static unsigned long x=123456789,y=362436069,z=521288629,w=88675123;
    unsigned long t;
    t=(x^(x<<11));x=y;y=z;z=w; return( w=(w^(w>>19))^(t^(t>>8)) );
  } 

  
};

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
 
  Solver solver;
  solver.solve();
}
