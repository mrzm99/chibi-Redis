/*-------------------------------------------------------------*/
/*!
 *      @file       resp_cmd.cpp
 *      @date       2026.xx.xx
 *      @author     mrzm99
 *      @brief      implementation of RESP command
 *      @note
 */
/*-------------------------------------------------------------*/

#include "resp_cmd.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

/*-------------------------------------------------------------*/
/*! @brief  constractor
 */
resp_command::resp_command()
{
    // init command table
    register_commands();
}

/*-------------------------------------------------------------*/
/*! @brief  register command
 */
void resp_command::register_commands()
{
    // PINGコcommand
    cmd_table["PING"] = [](const std::vector<std::string_view>& args)
    {
        (void)args;
        return "+PONG\r\n";
    };

    // ECHO command
    cmd_table["ECHO"] = [](const std::vector<std::string_view>& args)
    {
        if (args.empty()) {
            return static_cast<std::string>("-ERR wrong number of arguments for 'ECHO' command\r\n");
        }

        std::string msg = std::string(args[0]);
        return "$" + std::to_string(msg.length()) + "\r\n" + msg + "\r\n";
    };

    // SET command
    cmd_table["SET"] = [this](const std::vector<std::string_view>& args)
    {
        // check args
        if (args.size() != 2) {
            std::cout << "[Debug][SET] args.size() = "<< args.size() << std::endl;
            return "-ERR wrong number of arguments for SET command\r\n";
        }

        // get key-value
        std::string key(args[0]);
        std::string value(args[1]);

        // save value
        this->kv_store[key] = value;

        return "+OK\r\n";
    };

    // GET command
    cmd_table["GET"] = [this](const std::vector<std::string_view>& args)
    {
        // args check
        if (args.size() > 1) {
            std::cout << "[Debug][GET] args.size() = " << args.size() << std::endl;
            return static_cast<std::string>("-ERR wrong number of arguments for GET command\r\n");
        }

        // get key
        std::string key(args[0]);

        // serarch key
        auto it = this->kv_store.find(key);

        // find failed
        if (it == kv_store.end()) {
            return static_cast<std::string>("$-1\r\n");

        // find success
        } else {
            std::string value = it->second;
            return "$" + std::to_string(value.length()) + "\r\n" + value + "\r\n";
        }
    };

    // EXISTS command
    cmd_table["EXISTS"] = [this](const std::vector<std::string_view>& args)
    {
        // args check
        if (args.empty()) {
            return static_cast<std::string>("-ERR wrong number of arguments for EXISTS command\r\n");
        }

        // count exists item number
        int count = 0;
        for (auto arg : args) {
            // convert arg to key
            std::string sv_key(arg);

            // check item exists
            if (this->kv_store.find(sv_key) != this->kv_store.end()) {
                count++;
            }
        }

        return ":" + std::to_string(count) + "\r\n";
    };

    // DEL command
    cmd_table["DEL"] = [this](const std::vector<std::string_view>& args)
    {
        // check args
        if (args.empty()) {
            return static_cast<std::string>("-ERR wrong number of arguments for DEL command\r\n");
        }

        // count delete item number
        int count = 0;
        for (auto arg : args) {
            // convert arg to key
            std::string sv_key(arg);

            // erase method returns 1 if delete success
            count += this->kv_store.erase(sv_key);
        }

        return ":" + std::to_string(count) + "\r\n";
    };
}

/*-------------------------------------------------------------*/
/*! @brief  execute command
 */
std::string resp_command::execute_command(const std::string& cmd_name, const std::vector<std::string_view>& args)
{
    std::string upper_cmd = cmd_name;

    // conver command name to upper case
    std::transform(cmd_name.begin(), cmd_name.end(), upper_cmd.begin(),
                   [](unsigned char c){ return std::toupper(c); });

    // find command
    auto it = cmd_table.find(upper_cmd);

    // command exist
    if (it != cmd_table.end()) {
        return it->second(args);
    // command does not exist
    } else {
        return "-ERR unknown command '" + cmd_name + "'\r\n";
    }
}
