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
