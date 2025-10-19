// server.hpp
#pragma once
#include "radar_data.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
#include <filesystem>
#include <fstream>
#include <string.h>

namespace beast = boost::beast;
namespace http = beast::http;
namespace ws = beast::websocket;
namespace asio = boost::asio;
namespace fs = std::filesystem;
using tcp = asio::ip::tcp;

void handle_session(tcp::socket socket, SharedRadarData radar, const std::string& root_path) {
    beast::flat_buffer buffer;

    // Peek first few bytes to detect websocket handshake
    socket.async_wait(tcp::socket::wait_read, [](auto...) {});

    try {
        beast::error_code ec;
        char buf[4];
        size_t n = socket.read_some(asio::buffer(buf), ec);
        if (n < 3) return;

        if (buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T') {
            // HTTP
            beast::flat_buffer req_buffer;
            http::request<http::string_body> req;
            http::read(socket, req_buffer, req);
            std::string target(req.target()); // 直接用 string_view 构造 std::string
            if (target == "/") target = "/index.html";
            fs::path file_path = fs::path(root_path) / target.substr(1);

            std::string body = "404 Not Found";
            if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
                std::ifstream ifs(file_path, std::ios::binary);
                body.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            }

            http::response<http::string_body> res{ http::status::ok, req.version() };
            res.set(http::field::server, "C++ Server");
            res.set(http::field::content_type, "text/html");
            res.body() = body;
            res.prepare_payload();
            http::write(socket, res);
        }
        else {
            // WebSocket
            ws::stream<tcp::socket> ws_stream(std::move(socket));
            ws_stream.accept();

            while (true) {
                beast::flat_buffer ws_buf;
                ws_stream.read(ws_buf);
                std::string msg = beast::buffers_to_string(ws_buf.data());
                if (msg == "requestInfo") {
                    auto json = radar->to_json().dump();
                    ws_stream.text(true);
                    ws_stream.write(asio::buffer(json));
                }
            }
        }
    }
    catch (...) {
        return;
    }
}

void run_server(unsigned short port, const std::string& root_path, SharedRadarData radar) {
    asio::io_context ioc{ 1 };
    tcp::acceptor acceptor{ ioc, tcp::endpoint(tcp::v4(), port) };
    while (true) {
        tcp::socket socket{ ioc };
        acceptor.accept(socket);
        std::thread(handle_session, std::move(socket), radar, root_path).detach();
    }
}
