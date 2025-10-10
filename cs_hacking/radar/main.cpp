// main.cpp
#include "radar_data.hpp"
#include "data_update.hpp"
#include "server.hpp"
#include <memory>
#include <iostream>

int main() {
    auto radar = std::make_shared<RadarData>();

    std::thread data_thread(simulate_game_data_loop, radar);
    std::thread server_thread(run_server, 8080, "./webradar", radar);

    std::cout << "Server running at http://localhost:8080\n";

    data_thread.join();
    server_thread.join();
}
