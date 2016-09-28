#include <algorithm>
#include <assert.h>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sys/time.h>
#include <tuple>
#include <vector>
#include <array>
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


const int SHIFT_WIDTH[13] = {0, 4, 8, 12, 16 , 20, 24, 28, 32, 36, 40, 44, 48};
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



class BitBoard {
public:
  BitBoard() { memset(array, 0, sizeof(array)); }
  inline int get(int y, int x) const {
    assert(0 <= y and y < 11);
    assert(0 <= x and x < 13);
    int bit = (array[y] >> SHIFT_WIDTH[x]) & WIDTH_BIT[0];
    return bit;
  }
  inline void set(int y, int x, int kind) {
    assert(0 <= y and y < 11);
    assert(0 <= x and x < 13);
    assert(0 <= kind and kind < 9);
    //clear bit
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
namespace GameRule{

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
  Solver() {
  }
  void solve() {
    int width, height, myid;
    cin >> width >> height >> myid;
    assert(width == BOARD_WIDTH and height == BOARD_HEIGHT);
    cin.ignore();
    //-----------------------------init------------------------------------------
    my_id = myid;
    game_turn = 0;
    external_player_info.fill(0);
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
    int max_bomb_cnt;
    
    int remain_bomb_cnt;
    int explosion_range;
    
    //determine winner player
    int sum_box_point;
    bool survival;
    PlayerInfo() {
      survival = false;
    }
    // PlayerInfo(int y, int x, bool survival, int sum_box_point, int remain_bomb_cnt, int explosion_range)
    //   : y(y), x(x), survival(survival), sum_box_point(sum_box_point), remain_bomb_cnt(remain_bomb_cnt),
    //       explosion_range(explosion_range) {
    // }
    bool is_survive() const {return survival;}
    bool is_dead() const {return not survival;}
    
    bool can_set_bomb() { return remain_bomb_cnt >= 1; }
    int get_remain_bomb_cnt() const { return remain_bomb_cnt; }

    bool operator<(const PlayerInfo &right) const {
      if (y != right.y) {
        return y < right.y;
      }
      if (x != right.x) {
        return x < right.x;
      }
      if (survival != right.survival){
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
  using Player = array<PlayerInfo, GameRule::MAX_PLAYER_NUM>;
  struct StateInfo {
    BitBoard board;
    vector<Bomb> bombs;
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
    SearchState() {
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
          res.players[owner].remain_bomb_cnt = param1;//remain
          res.players[owner].explosion_range = param2;//range
	  //survive
	  res.players[owner].survival = true;
	  res.players[owner].sum_box_point = external_player_info[owner];//later update
	  res.players[owner].max_bomb_cnt = param1;//later update

      } else if (entityType == EntityType::BOMB) { // Bomb
	res.players[owner].max_bomb_cnt++;
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
    //cerr << "my_id = " << my_id << endl;

    cerr
        << "----------------------------Input End------------------------------"
        << endl;
    return res;
  }

  inline bool on_bomb_cell(int cell_type){
    if (cell_type == CellType::BOMB_CELL){
      return true;
    }
    return false;
  }
  inline bool on_bomb_exploded_cell(int cell_type){
    if (cell_type == CellType::BOMB_EXPLODED_CELL){
      return true;
    }
    return false;
  }
  inline bool on_any_bomb_cell(int cell_type){
    if (on_bomb_cell(cell_type) or on_bomb_exploded_cell(cell_type)){
      return true;
    }
    return false;
  }
  inline bool on_box_cell(int cell_type){
    if (cell_type == CellType::BOX_CELL){
      return true;
    }
    return false;
  }
  inline bool on_box_item_cell(int cell_type){
    if (cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL){
      return true;
    }
    return false;
  }
  inline bool on_any_box_cell(int cell_type){
    if (on_box_cell(cell_type) or on_box_item_cell(cell_type)){
      return true;
    }
    return false;
  }
  inline bool on_any_item_cell(int cell_type){
    if (cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL or cell_type == CellType::ITEM_BOMB_CNT_UP_CELL){
      return true;
    }
    return false;
  }
  
  inline bool on_object_cell(int cell_type){
    if (cell_type != CellType::EMPTY_CELL){
      return true;
    }
    return false;
  }
  

  void simulate_bomb_inducing_explosion(const pair<int, int> exploded_key, const int range, bool occupied, const multimap<pair<int, int>, int> &multimp_explosion_range, BitBoard &board) {
    int px = exploded_key.second;;
    int py = exploded_key.first;
    int cell_type = board.get(py, px);
    
    assert(in_board(py, px));
    
    board.set(py, px, CellType::BOMB_EXPLODED_CELL);
    
    //d ==0
    if (occupied == false and multimp_explosion_range.count(make_pair(py, px)) > 1){//duplicated
      auto iter_equal_range = multimp_explosion_range.equal_range(exploded_key);
      for (auto it = iter_equal_range.first; it != iter_equal_range.second; it++){
	simulate_bomb_inducing_explosion(it->first, it->second, true, multimp_explosion_range, board);
      }
      return ;
    }
    

    //d > 1
    for (int k = 0; k < 4; k++) {
      for (int d = 1; d < range; d++) {
        int ny, nx;
        nx = px + d * DX[k];
        ny = py + d * DY[k];
        if (not in_board(ny, nx))
          break;
	int cell_type = board.get(ny , nx);
        if (on_bomb_cell(cell_type)){//inducing explosion
	  auto iter_equal_range = multimp_explosion_range.equal_range(make_pair(ny, nx));
	  pair<int, int> next_key = iter_equal_range.first->first;
	  int next_range = iter_equal_range.first->second;
	  simulate_bomb_inducing_explosion(next_key, next_range, true, multimp_explosion_range, board);
	  break;
	}else if(on_object_cell(cell_type)){//exsist object
	  break;
	}
      }
    }
    return ;
  }
  void simulate_bomb_explosion(StateInfo &state, bool do_update = false) {
    vector<Bomb> &bombs = state.bombs;
    multimap<pair<int, int>, int> multimp_explosion_range;
    for (int i = 0; i < bombs.size(); i++) {
      bombs[i].dec_turn();
      int x,y;
      x = bombs[i].x;
      y = bombs[i].y;
      int range = bombs[i].explosion_range;
      multimp_explosion_range.emplace(make_pair(y, x), range);
    }
    
    for (int i = 0; i < bombs.size(); i++) {
      int x, y;
      x = bombs[i].x;
      y = bombs[i].y;
      int cell_type = state.board.get(y, x);
      if (bombs[i].is_explode() and on_bomb_cell(cell_type)) {
        // cerr << y << " " << x << endl;
        simulate_bomb_inducing_explosion(make_pair(y, x),bombs[i].explosion_range, false,
                                         multimp_explosion_range,
                                         state.board);
      }
    }

    // destroy object
    // cerr << exploded_bombs.size() << endl;
    set<pair<int, int>> destroyed_objects;
    for (int i = 0; i < bombs.size(); i++) {
      const int px = bombs[i].x;
      const int py = bombs[i].y;
      const int current_cell_type = state.board.get(py, px);
      if (on_bomb_exploded_cell(current_cell_type)) {// bomb is exploded 
        int owner = bombs[i].owner;
        int range = bombs[i].explosion_range;
	state.players[owner].remain_bomb_cnt++;
        bool damged = false;
	//d == 0
	{
	  int player_y, player_x;
	  for (int player_id = 0; player_id < GameRule::MAX_PLAYER_NUM; player_id++){
	    if (state.players[player_id].is_dead())continue;
	    player_y = state.players[player_id].y;
	    player_x = state.players[player_id].x;
	    if ((py == player_y and px == player_x)) {
	      state.players[player_id].survival = false;
	    }
	  }

	}

	//d > 1
        for (int k = 0; k < 4; k++) {
          for (int d = 1; d < range; d++) {
            int ny, nx;
            nx = px + d * DX[k];
            ny = py + d * DY[k];
            if (not in_board(ny, nx))
              break;
            int cell_type = state.board.get(ny, nx);

	{
	  int player_y, player_x;
	  for (int player_id = 0; player_id < GameRule::MAX_PLAYER_NUM; player_id++){
	    if (state.players[player_id].is_dead())continue;
	    player_y = state.players[player_id].y;
	    player_x = state.players[player_id].x;
	    if ((ny == player_y and nx == player_x)) {
	      state.players[player_id].survival = false;
	    }
	  }

	}
	
            if (cell_type == CellType::BOMB_CELL or cell_type == CellType::BOMB_EXPLODED_CELL or
                cell_type == CellType::WALL_CELL) { // exsit bombs or wall
              break;
            }
            if (on_object_cell(cell_type)) {
              if (on_any_box_cell(cell_type)){
		//assert(state.state.players[owner].box_point >= 0);
		state.players[owner].sum_box_point += 1;
		//update
		if (do_update){//sum box point
		  external_player_info[owner] += 1;
		}
		destroyed_objects.emplace(make_pair(ny, nx));
              } else if (on_any_box_cell(cell_type)){
                destroyed_objects.emplace(make_pair(ny, nx));
              }
	      break;
            }
          }
        }
      }
    }
    
    for (const auto &val : destroyed_objects) {
      int y, x;
      x = val.second;
      y = val.first;
      int cell_type = state.board.get(y, x);
      if (cell_type == CellType::BOX_CELL or
          cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL or
          cell_type == CellType::ITEM_BOMB_CNT_UP_CELL) { // EMPTY CELL
        state.board.set(y, x, CellType::EMPTY_CELL);
      } else if (cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL) {//box item  range up
        state.board.set(y, x, CellType::ITEM_BOMB_RANGE_UP_CELL);
      } else if (cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL) {//box item cnt up
        state.board.set(y, x, CellType::ITEM_BOMB_CNT_UP_CELL);
      }
    }
    vector<Bomb> next_bombs;
    for (int i = 0; i < bombs.size(); i++) {
      int x = bombs[i].x;
      int y = bombs[i].y;
      int cell_type = state.board.get(y, x);
      if (on_bomb_cell(cell_type)) { // not exploded
        next_bombs.emplace_back(bombs[i]);
      } else { // exploed
        state.board.set(y, x, CellType::EMPTY_CELL);
      }
    }
    sort(next_bombs.begin(), next_bombs.end());
    // bombs = next_bombs;
    //bombs.swap(next_bombs);
    bombs = move(next_bombs);
    // cerr << bombs.size() << endl;
    return ;
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
    score += 3 * (min(13, search_state.state.players[id].explosion_range) - 3);
    score += 3 * (min(7, search_state.state.players[id].max_bomb_cnt) - 1);
    score += (search_state.state.players[id].remain_bomb_cnt);
    
    score *= 100;
    //score += 4 * (search_state.state.players[id].explosion_range - 3);
    //score += 4 * (search_state.state.players[id].max_bomb_cnt - 1);
    //score += (search_state.state.my_info.remain_bomb_cnt - 1);
    int sum_man_dist = 0;
    int min_dist = (BOARD_HEIGHT + BOARD_WIDTH + 1);
    int active_boxes_cnt = 0;
    
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      for (int x = 0; x < BOARD_WIDTH; x++) {
        int cell_type = search_state.state.board.get(y, x);
        if (cell_type == CellType::BOX_CELL or
            cell_type == CellType::BOX_ITEM_BOMB_RANGE_UP_CELL or
            cell_type == CellType::BOX_ITEM_BOMB_CNT_UP_CELL) {
          // if (search_state.state.future_destroied_boxes.count(make_pair(y, x)) >
          //     0)
          //   continue;
          int dist = abs(px - x) + abs(py - y);
          active_boxes_cnt++;
          min_dist = min(min_dist, dist);
          sum_man_dist += dist;
        }
      }
    }
    score += 12 * ((BOARD_HEIGHT + BOARD_WIDTH) - min_dist);
    score += (BOARD_HEIGHT + BOARD_WIDTH) * (active_boxes_cnt)-sum_man_dist;
    score *= 100;


    //0 0
    //penlaty corner
    //upper left 0 0
    int d0,d1,d2,d3;
    d0 = abs(px - 0) + abs(py - 0);
    //upper right BOARD_HEIGHH - 1, 0
    d1 = abs(px - (BOARD_WIDTH - 1)) + abs(py - 0);
    //lower left
    d2 = abs(px - 0) + abs(py - (BOARD_HEIGHT - 1));
    //lower right
    d3 = abs(px - (BOARD_WIDTH - 1)) + abs(py - (BOARD_HEIGHT - 1));
    score -= max({d0, d1, d2, d3});
    return score;
  }
  void simulate_next_move(int id, const SearchState &state,
                          priority_queue<SearchState> &search_states,
                          const int &turn) {
    if (state.state.players[id].is_dead())return ;
    const int px = state.state.players[id].x;
    const int py = state.state.players[id].y;

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
        if (cell_type == CellType::BOMB_CELL or cell_type == CellType::BOMB_EXPLODED_CELL) {
          continue;
        }
      }
      SearchState next_state = state;
      if (cell_type == CellType::ITEM_BOMB_RANGE_UP_CELL) {

        next_state.state.players[id].explosion_range++;
        next_state.state.board.set(ny, nx, CellType::EMPTY_CELL);
      } else if (cell_type == CellType::ITEM_BOMB_CNT_UP_CELL) {
        next_state.state.players[id].max_bomb_cnt += 1;
	//assert(next_state.state.players[id].max_bomb_cnt >= 0);
        next_state.state.players[id].remain_bomb_cnt += 1;

        next_state.state.board.set(ny, nx, CellType::EMPTY_CELL);
      }
      next_state.state.players[id].y = ny;
      next_state.state.players[id].x = nx;
      if (turn == 0) {
        next_state.first_act = Act(ny, nx, ACT_MOVE);
      }
      // simulate_bomb_explosion(next_state, turn);
      next_state.score = calc_score(id, state, next_state);
      search_states.emplace(next_state);
      // visited.emplace(make_tuple(next_state.state.my_info,
      // next_state.state.enemy_info, next_state.state.boxes,
      // next_state.state.bombs));
    }
  }
  void simulate_next_set_bomb(int id, const SearchState &state,
                              priority_queue<SearchState> &search_states,
                              const int &turn) {
    if (state.state.players[id].is_dead())return ;
    if (state.state.players[id].get_remain_bomb_cnt() <= 0)
      return;

    const int px = state.state.players[id].x;
    const int py = state.state.players[id].y;
    const int range = state.state.players[id].explosion_range;
    if (state.state.board.get(py, px) ==
        CellType::BOMB_CELL) { // already set bomb
      return;
    }

    SearchState next_state = state;

    next_state.state.board.set(py, px, CellType::BOMB_CELL);
    next_state.state.bombs.emplace_back(Bomb(py, px, id, 8, range));
    next_state.state.players[id].remain_bomb_cnt--;
    // next_state.state.board[py][px] = BOMB_CELL;
    if (turn == 0) {
      next_state.first_act = Act(py, px, ACT_BOMB);
    }
    // simulate_bomb_explosion(next_state, turn);
    next_state.score = calc_score(id, state, next_state);
    search_states.emplace(next_state);

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

    const int beam_width = 20;
    const int depth_limit = 20;
    priority_queue<SearchState> curr_search_states[depth_limit + 1];
    set<tuple<Player, BitBoard, vector<Bomb>>>
        visited[depth_limit + 1];

    SearchState init_search_state;
    init_search_state.state = init_info;
    //print players info
    
    debug_players_info(init_search_state.state.players);
    
    curr_search_states[0].emplace(init_search_state);
    Act tmp_best_act;
    pair<int, double> tmp_best(0, 0);
    int chokudai_iter = 0;
    int prune_cnt = 0;
    while (timer.get_mill_duration() <= 85) {
      chokudai_iter++;
      for (int turn = 0; turn < depth_limit; turn++) {
	//cerr << curr_search_states[turn].size() << endl;
        for (int iter = 0;
             iter < beam_width and (not curr_search_states[turn].empty());
             iter++) {
          if (timer.get_mill_duration() >= 85)
            goto END;
          SearchState curr_search_state = curr_search_states[turn].top();
          curr_search_states[turn].pop();
          auto key = make_tuple(curr_search_state.state.players,                                
                                curr_search_state.state.board,
                                curr_search_state.state.bombs);
          if (visited[turn].count(key) > 0) {
            iter--;
            //prune_cnt++;
            continue;
          }
	  
	  
	  auto tmp_score = make_pair(turn, curr_search_state.score);
	  if (tmp_score > tmp_best){
	    tmp_best = tmp_score;
	    tmp_best_act = curr_search_state.first_act;
	  }
          visited[turn].emplace(key);
          // simulate bomb
	  //if (turn != 0)
	  simulate_bomb_explosion(curr_search_state.state, false);
          // next state
          // move
          simulate_next_move(my_id,curr_search_state, curr_search_states[turn + 1],
                             turn);
          // set bomb
          simulate_next_set_bomb(my_id, curr_search_state,curr_search_states[turn + 1], turn);
        }
        // cerr << "prune = " << prune_cnt << endl;
      }
      //break;
    }
  END:;
    //cerr << prune_cnt << endl;
    // cerr << curr_search_states[depth_limit].size() << endl;
    cerr << chokudai_iter++ << endl;
    if (not curr_search_states[depth_limit].empty() and curr_search_states[depth_limit].top().score > 0){
      SearchState best = curr_search_states[depth_limit].top();
      //assert(best.state.players[my_id].max_bomb_cnt >= 0 and best.state.players[my_id].max_bomb_cnt < 13);
      cerr << best.state.players[my_id].survival << " " << best.state.players[my_id].sum_box_point << " " << best.state.players[my_id].max_bomb_cnt << " " << best.state.players[my_id].explosion_range << " " << best.score << endl;
      output_act(best.first_act);
    }else{
      cerr << "temp action" << endl;
      output_act(tmp_best_act);
    }
    if (game_turn > 0)
      update_state(init_search_state.state);
  }
  //----------------------------data----------------------------------------------
  int my_id;
  int game_turn;
  array<int, GameRule::MAX_PLAYER_NUM> external_player_info;//box point, max_bomb_cnt
  void debug_players_info(const Player &players){
    cerr << "----------------------Player Info Start----------------------------------------------" << endl;
    for (int i = 0; i < GameRule::MAX_PLAYER_NUM; i++){
      if (players[i].is_dead())continue;
      cerr << "id = "<< i << " sum_box_point = " << players[i].sum_box_point << " max_bomb_cnt = " << players[i].max_bomb_cnt << " explosion_range = " << players[i].explosion_range << " remain_bomb_cnt = " << players[i].remain_bomb_cnt << endl;
    }
    cerr << "----------------------Player Info End----------------------------------------------" << endl;
  }
  void update_state(StateInfo &curr_state){
    //update box point
    StateInfo tmp_state = curr_state;
    simulate_bomb_explosion(tmp_state, true);
    for (int i = 0; i < GameRule::MAX_PLAYER_NUM; i++){
      if (tmp_state.players[i].is_dead())continue;
      //current 
      int py = tmp_state.players[i].y;
      int px = tmp_state.players[i].x;
      //update state
      curr_state.players[i].sum_box_point = external_player_info[i];
    }
    return ;
  }
  
};

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
  Solver solver;
  solver.solve();
}
