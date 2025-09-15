#include "dogan.hh"
#include <yaml-cpp/yaml.h>
#include <iostream>

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

void Dogan::load_config(const std::string& filename) {
    YAML::Node config = YAML::LoadFile(filename);

    for (const auto& p : config["players"]) {
        Player player;
        player.name = p["name"].as<std::string>();
        player.type = color_player_type_map.at(p["color"].as<std::string>());
        players.push_back(player);
    }

    bl = balance_level_map.at(config["balance_level"].as<std::string>());

    // hexes = config["board"]["hexes"].as<int>();
    // ports = config["board"]["ports"].as<int>();
}

const std::vector<Player>& CatanConfig::getPlayers() const {
    return players;
}

int CatanConfig::getHexes() const {
    return hexes;
}

int CatanConfig::getPorts() const {
    return ports;
}
