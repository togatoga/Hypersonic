#include <iostream>
#include <sys/time.h>
using namespace std;
/*
--------------------------Template--------------------------
*/
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

class Solver{
  
public:
  Solver(){}
  void input(){
    int entities;
    cin >> entities;
    cin.ignore();
    for (int i = 0; i < BOARD_HEIGHT; i++){
      
    }
    for (int i = 0; i < entities; i++){
      int entityType;
      int owner;
      int x,y;
      int param1,param2;
      cin >> entityType >> owner >> x >> y >> param1 >> param2;
      cin.ignore();
    }
  }
private:
  
  
};



int main(){
  cin.tie(0);
  ios::sync_with_stdio(false);

}
