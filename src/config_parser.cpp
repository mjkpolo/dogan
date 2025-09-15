#include "config.hh"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <string>

const std::unordered_map<std::string, PlayerType> color_player_type_map = {
  {"orange", PLAYER_ORANGE},
  {"red", PLAYER_RED},
  {"blue", PLAYER_BLUE},
  {"white", PLAYER_WHITE},
};

const std::unordered_map<std::string, BALANCE_LEVEL> balance_level_map = {
  {"none", BALANCE_LEVEL::None},
  {"low", BALANCE_LEVEL::Low},
  {"medium", BALANCE_LEVEL::Medium},
  {"high", BALANCE_LEVEL::High},
  {"extreme", BALANCE_LEVEL::Extreme},
};

void GameConfig::load_config(const std::string& filename) {
    YAML::Node config = YAML::LoadFile(filename);
    unsigned int player_id = 0;
    for (const auto& p : config["players"]) {
        std::string pname = p["name"].as<std::string>();
        PlayerType ptype = color_player_type_map.at(p["color"].as<std::string>());
        Player player(player_id++, pname, ptype);
        players.push_back(player);
    }

    bl = balance_level_map.at(config["balance_level"].as<std::string>());

}

static std::string player_type_to_string(PlayerType pt) {
    switch (pt) {
        case PLAYER_ORANGE: return "orange";
        case PLAYER_RED:    return "red";
        case PLAYER_BLUE:   return "blue";
        case PLAYER_WHITE:  return "white";
        default:            return "unknown";
    }
}

static std::string balance_level_to_string(BALANCE_LEVEL bl) {
    switch (bl) {
        case BALANCE_LEVEL::None:    return "none";
        case BALANCE_LEVEL::Low:     return "low";
        case BALANCE_LEVEL::Medium:  return "medium";
        case BALANCE_LEVEL::High:    return "high";
        case BALANCE_LEVEL::Extreme: return "extreme";
        default:                     return "unknown";
    }
}

std::string GameConfig::summary() {
    std::ostringstream oss;

    oss << "Players:\n";
    for (const auto& p : players) {
        oss << "  id=" << p.get_id()
            << " name=" << p.get_name()
            << " type=" << player_type_to_string(p.get_type())
            << "\n";
    }

    oss << "Balance level: " << balance_level_to_string(bl) << "\n";

    return oss.str();
}

