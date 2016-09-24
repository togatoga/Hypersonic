#include <iostream>

using namespace std;

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
