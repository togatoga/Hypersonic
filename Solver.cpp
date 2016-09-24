#include <iostream>
#include <assert.h>
#include <sys/time.h>
#include <set>
#include <vector>

using namespace std;

/*
--------------------------Template--------------------------
*/
const int8_t DX[]={1,0,-1,0},DY[]={0,-1,0,1};

class Timer{
public:
  Timer(){}
  void start(){
    start_time = get_mill_sec();
  }
  double get_mill_duration(){
    return get_mill_sec() - start_time;
  }
  
private:
  double start_time;
  double get_mill_sec(){
    struct timeval res;
    gettimeofday(&res, NULL);
    return res.tv_sec * 1000 + res.tv_usec / 1000;
  }
};

/*
---------------------------Game Code---------------------------
*/

const int8_t BOARD_WIDTH = 13;
const int8_t BOARD_HEIGHT = 11;
namespace EntityType{
  const int8_t PLAYER = 0;
  const int8_t BOMB = 1;
}
class Solver{
  
public:
  Solver(){}
  void solve(){
    int8_t width,height,myid;
    cin >> width >> height >> myid;
    assert(width == BOARD_WIDTH and height == BOARD_HEIGHT);
    cin.ignore();
    my_id = myid;
    int turn = 0;
    while (true){
      cerr << "turn = " << turn << endl;
      StateInfo input_info = input();
      think(input_info);
    }
  }
private:
  
  struct Bomb{
    int x,y;
    int owner;
    int explosion_turn;
    int explosion_range;
    Bomb(){}
    Bomb(int y, int x, int owner, int explosion_turn, int explosion_range):y(y),x(x),owner(owner),explosion_turn(explosion_turn),explosion_range(explosion_range){}
    bool is_explode()const {
      return explosion_turn <= 0;
    }
    void dec_turn(){
      assert(explosion_range > 0);
      explosion_turn--;
    }
  };

  struct PlayerInfo{

    int x,y;
    int remain_bomb_cnt;
    int explosion_range;
    PlayerInfo(){}
    PlayerInfo(int y, int x, int remain_bomb_cnt, int explosion_range):y(y),x(x),remain_bomb_cnt(remain_bomb_cnt),explosion_range(explosion_range){}
    
    bool can_set_bomb(){
      return remain_bomb_cnt >= 1;
    }
    int get_remain_bomb_cnt() const {
      return remain_bomb_cnt;
    }
  };
  struct StateInfo{
    int board[BOARD_HEIGHT][BOARD_WIDTH];
    vector<Bomb> Bombs;
    PlayerInfo my_info,enemy_info;
    StateInfo(){}
  };

  
  StateInfo input(){
    StateInfo res;
    for (int i = 0; i < BOARD_HEIGHT; i++){
      string row;
      for (int j = 0; j < BOARD_WIDTH; j++){
	if (row[i] == '.'){//empty
	  res.board[i][j] = 0;
	}else if(row[i] == '0'){//box
	  res.board[i][j] = 1;
	}
      }
    }
    int entities;
    cin >> entities;
    cin.ignore();
    for (int i = 0; i < entities; i++){
      int entityType;
      int owner;
      int x,y;
      int param1,param2;
      cin >> entityType >> owner >> x >> y >> param1 >> param2;
      if (entityType == EntityType::PLAYER){//Player
	if (owner == my_id){//Me
	  res.my_info.x = x;
	  res.my_info.y = y;
	  res.my_info.remain_bomb_cnt = param1;
	  res.my_info.explosion_range = param2;
	}else{//enemy
	  res.enemy_info.x = x;
	  res.enemy_info.y = y;
	  res.enemy_info.remain_bomb_cnt = param1;
	  res.enemy_info.explosion_range = param2;
	}
      }else{//Bomb
	res.Bombs.emplace_back(Bomb(y, x, owner, param1, param2));
      }
    }
    return res;
  }

  int8_t my_id;
  void think(const StateInfo& StateInfo){
    
  }
  
};



int main(){
  cin.tie(0);
  ios::sync_with_stdio(false);
  Solver solver;
  solver.solve();
}
