/*-------------------------------------------------------------------*/
/*!
 *      @file       resp_parse.cpp
 *      @date       2026.xx.xx
 *      @author     mrzm99
 *      @brief      parse RESP data
 *      @note
 */
/*-------------------------------------------------------------------*/
#include "../include/chibi-redis.hpp"

#include <iostream>
#include <string_view>
#include <stdint.h>

/*-------------------------------------------------------------------*/
/*! @brief  parse
 */
bool parse_resp(client_manage& client_info, uint32_t& p_pos)
{
    // position in buffer
    size_t pos = 0;

    while (pos < client_info.read_buff.length()) {

        // recv data
        if (client_info.current_state == parse_state_t::WAIT_BULK_DATA) {
            size_t required_len = client_info.current_bulk_len + 2;

            if ((client_info.read_buff.length()) - pos < required_len) {
                // not enough data
                client_info.reset_parse_state();
                return false;
            }

            // get
            std::string_view arg_view(client_info.read_buff.data() + pos, client_info.current_bulk_len);
            client_info.parsed_args.push_back(arg_view);

            pos += required_len;
            client_info.expected_args--;

            if (client_info.expected_args == 0) {
                std::cout << "[GET]: " << arg_view << std::endl;
                client_info.current_state = parse_state_t::WAIT_ARRAY_LEN;
                p_pos = pos;
                return true;
            } else {
                std::cout << "[GET]: " << arg_view << std::endl;
                client_info.current_state = parse_state_t::WAIT_BULK_LEN;
                continue;
            }
        }

        // get CRLF position
        size_t delmiter_pos = client_info.read_buff.find("\r\n", pos);

        std::cout << "pos = " << pos << std::endl;

        // not recv CRLF yet
        if (delmiter_pos == std::string::npos) {
            return false;
        }

        // get chunck
        std::string line = client_info.read_buff.substr(pos, delmiter_pos - pos);

        // inclement pos /r/n
        pos = delmiter_pos + 2;

        // ignore space only data
        if (line.empty()) {
            continue;
        }

        // wait array len (e.g. *3)
        if (client_info.current_state == parse_state_t::WAIT_ARRAY_LEN) {
            if (line[0] != '*') {
                std::cerr << "Failed to parse data. It does not begin '*'" << std::endl;
                client_info.read_buff.clear();
                return false;
            }
            client_info.expected_args = std::stoi(line.substr(1));
            if (client_info.expected_args <= 0) {
                client_info.read_buff.erase(0, pos);
                return false;
            }
            client_info.current_state = parse_state_t::WAIT_BULK_LEN;

            std::cout << "array len = " << client_info.expected_args << std::endl;

        // wait bulk len (e.g. $3)
        } else if (client_info.current_state == parse_state_t::WAIT_BULK_LEN) {
            if (line[0] != '$') {
                std::cerr << "Failed to parse RESP. It does not begin '$'" << std::endl;
                client_info.read_buff.erase(0, pos);
                return false;
            }
            client_info.current_bulk_len = std::stoi(line.substr(1));
            client_info.current_state = parse_state_t::WAIT_BULK_DATA;

            std::cout << "bulk len = " << client_info.current_bulk_len << std::endl;

        } else {
            // nope
        }

    }

    return false;
}

