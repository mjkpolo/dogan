#include "enums.hh"
#include <string>

class Player {
public:
    Player() = default;
    Player(unsigned int id, const std::string& name, PlayerType type)
        : id(id), name(name), type(type) {}

    unsigned int get_id() const { return id; }
    const std::string& get_name() const { return name; }
    PlayerType get_type() const { return type; }

    void set_id(unsigned int new_id) { id = new_id; }
    void set_name(const std::string& new_name) { name = new_name; }
    void set_type(PlayerType new_type) { type = new_type; }

private:
    unsigned int id{};
    std::string name{};
    PlayerType type{};
};
