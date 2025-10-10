// radar_data.hpp
#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include <nlohmann/json.hpp>
#include <hacking/Helper.h>


struct Vec3 {
    float x, y, z;
};

enum class PlayerType {Spectator, Local, Enemy};

struct PlayerData {
    Vec3 pos;
    float yaw;
    PlayerType player_type;
    bool has_bomb;
    bool has_awp;
    bool is_scoped;
    int health;            // 血量
    std::string weapon_name; // 当前武器名称

    nlohmann::json to_json() const {
        return {
            {"pos", {pos.x, pos.y, pos.z}},
            {"yaw", yaw},
            {"playerType", static_cast<int>(player_type)},
            {"hasBomb", has_bomb},
            {"hasAwp", has_awp},
            {"isScoped", is_scoped},
            {"health", health},
			{"weaponName", weapon_name}

        };
    }
};

struct BombData {
    Vec3 pos;
    bool is_planted;

    nlohmann::json to_json() const {
        return {
            {"pos", {pos.x, pos.y, pos.z}},
            {"isPlanted", is_planted}
        };
    }
};

struct EntityData {
    enum class Type { Player, Bomb } type;
    PlayerData player;
    BombData bomb;

    nlohmann::json to_json() const {
        if (type == Type::Player) return player.to_json();
        return bomb.to_json();
    }
};

struct RadarData {
    bool ingame = false;
    std::string map_name;
    std::vector<EntityData> entities;
    size_t freq = 0;
    bool bomb_planted = false;
    bool bomb_defused = false;
    float bomb_timer = 0.0f;
    bool bomb_exploded = false;
    bool bomb_being_defused = false;
    float bomb_defuse_len = 0.0f;

    std::mutex mtx;

    nlohmann::json to_json() {
        std::lock_guard<std::mutex> lock(mtx);
        nlohmann::json j;
        j["ingame"] = ingame;
        j["mapName"] = map_name;
        j["freq"] = freq;
        j["bombPlanted"] = bomb_planted;
        j["bomb_defused"] = bomb_defused;
        j["bomb_timer"] = bomb_timer;
        j["bomb_exploded"] = bomb_exploded;
        j["bomb_being_defused"] = bomb_being_defused;
        j["bomb_defuse_len"] = bomb_defuse_len;

        std::vector<nlohmann::json> entity_json;
        for (auto& e : entities) entity_json.push_back(e.to_json());
        j["entityData"] = entity_json;
        return j;
    }
};

using SharedRadarData = std::shared_ptr<RadarData>;
