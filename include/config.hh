// #include "dogan.hh"
#include <vector>
#include <string>
#include "player.hh"

// using namespace std;

class GameConfig{
  
public:
  GameConfig():
    players(std::vector<Player>()),
    bl(BALANCE_LEVEL::None){}

  std::vector<Player> players;
  BALANCE_LEVEL bl = BALANCE_LEVEL::None;

  void load_config(const std::string& filename);
  std::string summary();
  void set_players();
  
  const std::vector<Player>& get_players() const{
    return players;
  }

  int get_bl() const{
    return bl;
  }

private:

};
