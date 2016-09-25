#include <algorithm>
#include <assert.h>
#include <iostream>
#include <set>
#include <sys/time.h>
#include <vector>
#include <tuple>
#include <queue>

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
const int BOMB_CELL = 2;


const int ACT_MOVE = 0;
const int ACT_BOMB = 1;

class Solver {

public:
  Solver() {}
  void solve(bool debug=false) {
    int width, height, myid;
    cin >> width >> height >> myid;

    assert(width == BOARD_WIDTH and height == BOARD_HEIGHT);
    cin.ignore();
    my_id = myid;
    // cerr << static_cast<int>(my_id) << endl;
    int turn = 0;
    while (true) {
      StateInfo input_info = input(true);
      think(input_info);
      if (debug){
	break;
      }
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
      if (y != right.y) {
        return y < right.y;
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
    //set<pair<int, int>> future_destroied_boxes;
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
      } else {
	if (x != right.x){
	  return x < right.x;
	}else{
	  return act_id < right.act_id;
	}
      }
    }
  };

  struct SearchState {
    StateInfo state;
    Act first_act;
    int my_destoried_box_cnt;
    int my_future_destoried_box_cnt;
    double score;
    bool operator<(const SearchState &right) const {
      return score < right.score;
    }
    SearchState() {
      my_destoried_box_cnt = 0;
      my_future_destoried_box_cnt = 0;
      score = 0;
    }
  };
  bool in_board(int y, int x){
    if (y < 0 or y >= BOARD_HEIGHT or x < 0 or x >= BOARD_WIDTH)return false;
    return true;
  }
   

  

  StateInfo input(bool verbose=false) {
    StateInfo res;
    cerr << "----------------------------Input Start------------------------------" << endl;
    for (int i = 0; i < BOARD_HEIGHT; i++) {
      string row;
      cin >> row;
      if (verbose){
	cerr << row << endl;
      }
      
      for (int j = 0; j < BOARD_WIDTH; j++) {
        if (row[j] == '.') { // empty
          res.board[i][j] = EMPTY_CELL;
        } else if (row[j] == '0') { // box
          res.board[i][j] = BOX_CELL;
        }
      }
    }
    int entities;
    cin >> entities;
    if (verbose){
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
      if (verbose){
	cerr << entityType << " " << owner << " "<<  x << " "<<  y << " " << param1 << " "<<  param2 << endl;
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
      } else { // Bomb
        res.bombs.emplace_back(Bomb(y, x, owner, param1, param2));
	res.board[y][x] = BOMB_CELL;
      }
    }
    // cerr << res.my_info.y << " " << res.my_info.x << endl;
    cerr << "----------------------------Input End------------------------------" << endl;
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
    set<pair<int, int>> destroyed_boxes;
    for (int i = 0; i < bombs.size(); i++) {
      if (bombs[i].is_explode()) {
        int px = bombs[i].x;
        int py = bombs[i].y;
        int owner = bombs[i].owner;
        int range = bombs[i].explosion_range;
	assert(owner == 0 or owner == 1);
	assert(range == 3);
	state.state.board[py][px] = EMPTY_CELL;
	if (owner == my_id){//me

	  state.state.my_info.remain_bomb_cnt++;
	}else{//enemy
	  state.state.enemy_info.remain_bomb_cnt++;
	}
        // right
        // up
        // left
        // down
        for (int k = 0; k < 4; k++) {
          for (int d = 0; d < range; d++) {
            int ny, nx;
	    nx = px + d * DX[k];
            ny = py + d * DY[k];
	    if (not in_board(ny,nx))break;
            if (is_bombs.count(make_pair(ny, nx)) > 0) { // exsit bombs
              break;
            }
            if (state.state.board[ny][nx] == BOX_CELL) { // exsit box
              // destory
              if (owner == my_id) {
		// cerr << py << " " << px << " " << owner << " " << ny << " " << nx << endl;
		// cerr << d << " " << k << endl;
                state.my_destoried_box_cnt += 1;
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
      x = val.second;
      y = val.first;
      state.state.board[y][x] = EMPTY_CELL;
    }
    vector<Bomb> next_bombs;
    for (int i = 0; i < bombs.size(); i++) {
      if (not bombs[i].is_explode()) {
        next_bombs.emplace_back(bombs[i]);
      }
    }
    bombs = next_bombs;
    //cerr << bombs.size() << endl;
  }

  double calc_score(const SearchState& pre_state, const SearchState &search_state) {
    double score = pre_state.score;
    score += 20 * (search_state.my_destoried_box_cnt - pre_state.my_destoried_box_cnt);
    score += 8 * (search_state.my_future_destoried_box_cnt - pre_state.my_future_destoried_box_cnt);
    score += (search_state.state.my_info.remain_bomb_cnt - pre_state.state.my_info.remain_bomb_cnt);
    //score -= search_state.state.my_info.get_remain_bomb_cnt();
    score *= 50;
    // int sum_man_dist = 0;
    // int px = search_state.state.my_info.x;
    // int py = search_state.state.my_info.y;
    // for (int y = 0; y < BOARD_HEIGHT; y++){
    //   for (int x = 0; x < BOARD_WIDTH; x++){
    // 	if (search_state.state.board[y][x] == BOX_CELL){
    // 	  sum_man_dist += abs(px - x) + abs(py - y);
    // 	}
    //   }
    // }
    // score -= sum_man_dist;
    
    return score;
  }
  void simulate_next_move(const SearchState &state, priority_queue<SearchState> &search_states, const int &turn){
    const int px = state.state.my_info.x;
    const int py = state.state.my_info.y;

    for (int k = 0; k < 5; k++){
      int ny = px + DY[k];
      int nx = py + DX[k];
      if (not in_board(ny, nx))continue;
      if (state.state.board[ny][nx] == BOX_CELL)continue;
      SearchState next_state = state;
      next_state.state.my_info.y = ny;
      next_state.state.my_info.x = nx;
      if (turn == 0){
	next_state.first_act = Act(ny, nx, ACT_MOVE);
      }
      next_state.score = calc_score(state, next_state);
      
      search_states.emplace(next_state);
    }
  }
  void simulate_next_set_bomb(const SearchState &state, priority_queue<SearchState> &search_states, const int &turn){
    if (state.state.my_info.get_remain_bomb_cnt() <= 0)return ;
    
    const int px = state.state.my_info.x;
    const int py = state.state.my_info.y;
    const int range = state.state.my_info.explosion_range;
    SearchState next_state = state;
    for (int k = 0; k < 4; k++) {
      for (int d = 0; d < range; d++) {
	int ny, nx;
	nx = px + d * DX[k];
	ny = py + d * DY[k];
	if (not in_board(ny,nx))break;
	if (next_state.state.board[ny][nx] == BOX_CELL){
	  // destory
	  //cerr << py << " " << px << " " << ny << " " << nx << endl;
	  next_state.my_future_destoried_box_cnt += 1;
	  break;
	}
      }
    }
    next_state.state.bombs.emplace_back(Bomb(py, px, my_id, 8, range));
    next_state.state.my_info.remain_bomb_cnt--;
    next_state.state.board[py][px] = BOMB_CELL;
    if (turn == 0){
      next_state.first_act = Act(py, px, ACT_BOMB);
    }
    next_state.score = calc_score(state, next_state);
    search_states.emplace(next_state);
  }
  void output_act(const Act& act){
    int y,x;
    y = act.y;
    x = act.x;
    if (act.act_id == ACT_MOVE){
      cout << "MOVE" << " " << x << " " << y << endl;
    }else{
      cout << "BOMB" << " " << x << " " << y << endl;
    }
  }


  void count_ACT_BOMB(priority_queue<SearchState> curr_search_states, int turn){
    int cnt = 0;
    while (not curr_search_states.empty()){
      SearchState state = curr_search_states.top();
      curr_search_states.pop();
      if (state.first_act.act_id == ACT_BOMB){
	cnt++;
      }
    }
    //cerr << "turn = "<< turn << " " << "ACT_BOMB = " << cnt << endl;
  }

  void think(const StateInfo &init_info) {
    // cerr << "--think--" << endl;
    //cerr << init_info.board[0][0] << endl;
      
    Timer timer;
    timer.start();
    const int beam_width = 50;
    const int depth_limit = 16;
    priority_queue<SearchState> curr_search_states[depth_limit + 1];

    SearchState init_search_state;
    init_search_state.state = init_info;
    // cerr << init_info.my_info.y << " " << init_info.my_info.x << endl;
    // cerr << init_search_state.state.my_info.y << " " <<
    // init_search_state.state.my_info.x << endl;
    curr_search_states[0].emplace(init_search_state);
    Act best_act;
    double best_score = 0.0;
    int chokudi_iter = 0;
    while (timer.get_mill_duration() <= 90){
      chokudi_iter++;
      for (int turn = 0; turn < depth_limit; turn++) {
	//count_ACT_BOMB(curr_search_states[turn], turn);
	//cerr << curr_search_states[turn].size() << endl;
	//check_ACT_BOMB(curr_search_states[turn], turn);
	for (int iter = 0; iter < beam_width and (not curr_search_states[turn].empty()); iter++) {
	  if (timer.get_mill_duration() >= 90)goto END;
	  SearchState curr_search_state = curr_search_states[turn].top();
	  curr_search_states[turn].pop();
	  //simulate bomb
	  simulate_bomb_explosion(curr_search_state);
	
	  //next state
	  // move
	  simulate_next_move(curr_search_state, curr_search_states[turn + 1], turn);
	  // set bomb
	  simulate_next_set_bomb(curr_search_state, curr_search_states[turn + 1], turn);
	}
      }
      //break;
    }
  END:;
    //cerr << curr_search_states[depth_limit].size() << endl;
    cerr << chokudi_iter++ << endl;
    SearchState best = curr_search_states[depth_limit].top();
    cerr << best.my_destoried_box_cnt << " " << best.my_future_destoried_box_cnt << " " << best.score << endl;
    output_act(best.first_act);
  }

};

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);
  Solver solver;
  solver.solve(true);
}
