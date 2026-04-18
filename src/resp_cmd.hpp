/*-------------------------------------------------------------*/
/*!
 *      @file   resp_cmd.hpp
 *      @date   2026.xx.xx
 *      @author mrzm99
 *      @brief  execute command and make return RESP string
 *      @note
 *
 */
/*-------------------------------------------------------------*/
#ifndef __RESP_CMD_HPP__
#define __RESP_CMD_HPP__

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

/*-------------------------------------------------------------*/
/*! @brief  Command Handerl Type Definition
 */
using command_handler_t = std::function<std::string(const std::vector<std::string_view>&)>;

/*-------------------------------------------------------------*/
/*! @brief  Command Class
 */
class resp_command  {
public :
    // constractor
    resp_command();

    // execute command method
    std::string execute_command(const std::string& cmd_name, const std::vector<std::string_view>& args);

private:
    // command table
    std::unordered_map<std::string, command_handler_t> cmd_table;

    // register command
    void register_commands();
};

#endif

