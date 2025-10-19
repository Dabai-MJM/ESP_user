// data_update_loop.hpp
#pragma once
#include "radar_data.hpp"
#include <thread>
#include <chrono>
#include "../cs2_dumper/offsets.hpp"
#include "../cs2_dumper/client_dll.hpp"
#include "../hacking/esp.h"
#include <gui/gui.h>

inline std::string ReadString(uintptr_t addr, size_t max_len = 32)
{
    if (!addr) return "";
    char buffer[128] = { 0 };
    size_t len = max_len > sizeof(buffer) ? sizeof(buffer) : max_len;
    memcpy(buffer, reinterpret_cast<void*>(addr), len);
    buffer[len - 1] = '\0'; // 确保结尾
    return std::string(buffer);
}

uintptr_t get_C4plant()
{
    const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));
    if (!client) return 0;

    auto C4_1 = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwPlantedC4);
    if (!C4_1) return 0;

    auto C4_2 = *reinterpret_cast<uintptr_t*>(C4_1);
    if (!C4_2) return 0;

    return C4_2;
}

int local_team() {
    const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));
    if (!client) return -1;

    auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);
    if (!local_ctrl) return -1;

    auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
    if (local_hpawn == 0xFFFFFFFF) return -1;

    auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);
    if (!localpawn) return -1;

    return *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
}
bool IsC4(uintptr_t entity_ptr)
{
    if (!entity_ptr) return false;

    uintptr_t entity_identity_ptr = *reinterpret_cast<uintptr_t*>(entity_ptr + cs2_dumper::schemas::client_dll::CEntityInstance::m_pEntity);
    if (!entity_identity_ptr) return false;

    uintptr_t designer_name_ptr = *reinterpret_cast<uintptr_t*>(entity_identity_ptr + cs2_dumper::schemas::client_dll::CEntityIdentity::m_designerName);
    if (!designer_name_ptr) return false;

    char name_buff[3] = { 0 };
    memcpy(name_buff, reinterpret_cast<void*>(designer_name_ptr + 7), 2);

    return strcmp(name_buff, "c4") == 0;
}

void simulate_game_data_loop(SharedRadarData radar) {
    const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));
    if (!client) return;

    while (true) {
        RadarData tmp_radar; // 临时对象，最后一次性赋值
        tmp_radar.ingame = true;

        auto globals = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwGlobalVars);
        if (globals) {
            auto map_ptr = *reinterpret_cast<uintptr_t*>(globals + 0x180);
            tmp_radar.map_name = map_ptr ? ReadString(map_ptr, 32) : "unknown";
        }

        auto gamerules = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwGameRules);
        if (gamerules) {
            tmp_radar.bomb_planted = *reinterpret_cast<bool*>(gamerules + cs2_dumper::schemas::client_dll::C_CSGameRules::m_bBombPlanted);
        }

        auto bomb_entity = tmp_radar.bomb_planted ? get_C4plant() : 0;
        if (bomb_entity) {
            tmp_radar.bomb_timer = *reinterpret_cast<float*>(bomb_entity + cs2_dumper::schemas::client_dll::C_PlantedC4::m_flTimerLength);
            tmp_radar.bomb_defused = *reinterpret_cast<bool*>(bomb_entity + cs2_dumper::schemas::client_dll::C_PlantedC4::m_bBombDefused);
            tmp_radar.bomb_defuse_len = *reinterpret_cast<float*>(bomb_entity + cs2_dumper::schemas::client_dll::C_PlantedC4::m_flDefuseLength);
            tmp_radar.bomb_exploded = *reinterpret_cast<bool*>(bomb_entity + cs2_dumper::schemas::client_dll::C_PlantedC4::m_bHasExploded);
            tmp_radar.bomb_being_defused = *reinterpret_cast<bool*>(bomb_entity + cs2_dumper::schemas::client_dll::C_PlantedC4::m_bBeingDefused);
        }

        // 遍历玩家实体
        auto game_ent_sys = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwGameEntitySystem);
        if (game_ent_sys) {
            for (int i = 0; i < 64; ++i) {
                auto player_co = GetBaseEntity(i, client);
                if (!player_co) continue;

                auto player_hpawn = *reinterpret_cast<uint32_t*>(player_co + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
                if (player_hpawn == 0xFFFFFFFF) continue;

                auto player_pawn = GetBaseEntityFromHandle(player_hpawn, client);
                if (!player_pawn) continue;

                // 三维坐标
                auto pos = *reinterpret_cast<Vector3*>(player_pawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
                auto yaw = *reinterpret_cast<float*>(player_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_angEyeAngles + 4);
                auto player_team = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
                auto player_health = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
                if (player_health <= 0 || player_health > 100) continue;

                PlayerType player_type = PlayerType::Spectator;
                int lteam = local_team();
                if (player_team == lteam) player_type = PlayerType::Local;
                else if (player_team != lteam) player_type = PlayerType::Enemy;

                // 武器信息
                auto weaponServices = *reinterpret_cast<uintptr_t*>(player_pawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_pWeaponServices);
                bool has_bomb = false;
                bool has_awp = false;
                bool is_scoped = *reinterpret_cast<bool*>(player_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_bIsScoped);

                std::string weapon_name = "unknown";
                if (weaponServices) {
                    auto activeWeaponHandle = *reinterpret_cast<uint32_t*>(weaponServices + cs2_dumper::schemas::client_dll::CPlayer_WeaponServices::m_hActiveWeapon);
                    auto weaponBase = GetBaseEntityFromHandle(activeWeaponHandle, client);
                    if (weaponBase) {
                        auto player_weaponID = *reinterpret_cast<uint16_t*>(weaponBase + cs2_dumper::schemas::client_dll::C_EconEntity::m_AttributeManager
                            + cs2_dumper::schemas::client_dll::C_AttributeContainer::m_Item
                            + cs2_dumper::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex);
                        weapon_name = GetWeaponName(int(player_weaponID));
                        has_awp = (player_weaponID == 9);
                        has_bomb = (player_weaponID == 49);
                    }
                }

                PlayerData pd{ {pos.x, pos.y, 0}, yaw, player_type, has_bomb, has_awp, is_scoped, player_health, weapon_name };
                tmp_radar.entities.push_back(EntityData{ EntityData::Type::Player, pd, {} });

                // C4 实体
                if (IsC4(player_pawn)) {
                    BombData bd{ {pos.x, pos.y, 0}, tmp_radar.bomb_planted };
                    tmp_radar.entities.push_back(EntityData{ EntityData::Type::Bomb, {}, bd });
                }
            }
        }

        // 最后一次性赋值给共享 radar
        {
            std::lock_guard<std::mutex> lock(radar->mtx);
            radar->ingame = tmp_radar.ingame;
            radar->map_name = tmp_radar.map_name;
            radar->entities = tmp_radar.entities;
            radar->bomb_planted = tmp_radar.bomb_planted;
            radar->bomb_defused = tmp_radar.bomb_defused;
            radar->bomb_timer = tmp_radar.bomb_timer;
            radar->bomb_exploded = tmp_radar.bomb_exploded;
            radar->bomb_being_defused = tmp_radar.bomb_being_defused;
            radar->bomb_defuse_len = tmp_radar.bomb_defuse_len;
            radar->freq += 1;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
