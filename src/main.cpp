/*-------------------------------------------------------------------*/
/*!
 *      @file       main.cpp
 *      @date       2026.xx.xx
 *      @author     mrzm99
 *      @brief      main function (Event Loop)
 *      @note
 */
/*-------------------------------------------------------------------*/
#include "../include/chibi-redis.hpp"
#include "resp_parse.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

/*-------------------------------------------------------------------*/
/*! @brief  MACRO
 */
constexpr int PORT_NO = 6333;
constexpr int MAX_EVENT = 10;

/*-------------------------------------------------------------------*/
/*! @brief  set non-blocking socket
 */
static void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/*-------------------------------------------------------------------*/
/*! main
 */
int main()
{
    // create socket for server
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to scoket() for server_fd" << std::endl;
        return -1;
    }

    // set SO_REUSERADDR
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // set non-blocking
    set_nonblocking(server_fd);

    // bind port
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT_NO);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind()" << std::endl;
        return -1;
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        std::cerr << "Failed to listen()" << std::endl;
        return -1;
    }

    // init epoll
    int epoll_fd = epoll_create1(0);
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENT];

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    // hash map for client manage
    std::unordered_map<int, client_manage> clients_map;

    // start server
    std::cout << "chbi-redsis echo server start" << std::endl;

    // main event loop
    while (true) {
        // wait event
        int num_ready = epoll_wait(epoll_fd, events, MAX_EVENT, -1);

        // process event
        for (int i = 0; i < num_ready; i++) {
            // get ready fd
            int read_fd = events[i].data.fd;

            // get event info
            uint32_t triggerd_event = events[i].events;

            // [EVENT] new client connection
            if (read_fd == server_fd) {
                struct sockaddr_in new_client_addr;
                socklen_t new_client_len = sizeof(new_client_addr);
                int new_client_fd = accept(server_fd, (struct sockaddr*)&new_client_addr, &new_client_len);

                if (new_client_fd < 0) {
                    std::cout << "Failed to accept() for new client" << std::endl;
                } else {
                    // set non-blocking
                    set_nonblocking(new_client_fd);

                    // add new client info to hash map
                    clients_map[new_client_fd] = ::client_manage(new_client_fd);

                    // add epoll list
                    struct epoll_event new_client_ev;
                    new_client_ev.events = EPOLLIN;
                    new_client_ev.data.fd = new_client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_client_fd, &new_client_ev);

                    // log message
                    std::cout << "new client connected" << std::endl;
                }

            // [EVENT] recv data from client
            } else if (triggerd_event & EPOLLIN) {
                char buffer[1024];
                uint32_t parsed_pos;
                ssize_t read_num = read(read_fd, buffer, sizeof(buffer));

                // read faile
                if (read_num <= 0) {
                    std::cout << "Failed to read(). Close connet." << std::endl;
                    close(read_fd);
                    clients_map.erase(read_fd);

                // read success
                } else {
                    client_manage& client = clients_map[read_fd];
                    client.read_buff.append(buffer, read_num);

                    while (parse_resp(client, parsed_pos)) {
                        //debug
                        client.read_buff.erase(0, parsed_pos);
                        std::cout << "parse done" << std::endl;
                    }
                }

            // [EVENT] write buffer empty
            } else if (triggerd_event & EPOLLOUT) {
                client_manage client = clients_map[read_fd];
                ssize_t write_num = write(read_fd, client.write_buff.c_str(), client.write_buff.length());

                // erase send data
                if (write_num > 0) {
                    client.write_buff.erase(0, write_num);
                }

                // if send all data in write_buffer, unset EPOLLOUT flag
                if (client.write_buff.empty()) {
                    struct epoll_event mod_ev;
                    mod_ev.events = EPOLLIN;
                    mod_ev.data.fd = read_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, read_fd, &mod_ev);
                }

            // [EVENT] nothing
            } else {
                // nope
            }
        }
    }

    close(server_fd);

    return 0;
}
