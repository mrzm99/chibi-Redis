/*-------------------------------------------------------------------*/
/*!
 *      @file       chibi-redis.hpp
 *      @date       2026.xx.xx
 *      @author     mrzm99
 *      @brief      data structure for chibi-redis.hpp
 *      @note
 */
/*-------------------------------------------------------------------*/
#ifndef __CHIBI_REDIS_HPP__
#define __CHIBI_REDIS_HPP__

#include <string>
#include <vector>
#include <string_view>

/*-------------------------------------------------------------------*/
/*! @brief      parse state
 */
enum class parse_state_t {
    WAIT_ARRAY_LEN,             //!< wait array length (e.g. *3)
    WAIT_BULK_LEN,              //!< wait bulk lenth (e.g. $3)
    WAIT_BULK_DATA              //!< wait data (e.g. mykey, 142)
};

/*-------------------------------------------------------------------*/
/*! @brief  client mange structure
 */
class client_manage {
public:
    int fd = -1;                            //!< socket discriptor
    std::string read_buff;                  //!< read buffer
    std::string write_buff;                 //!< write buffer

    parse_state_t current_state = parse_state_t::WAIT_ARRAY_LEN;    //!< parase state
    int expected_args = 0;                                          //!< recv args
    int current_bulk_len = 0;                                       //!< each data length
    std::vector<std::string_view> parsed_args;                      //!< parased strings (e.g. ['SET' 'mykey' '123'])

    client_manage() = default;

    client_manage(int fd)
    : fd(fd)
    {}

    void reset_parse_state()
    {
        current_state = parse_state_t::WAIT_ARRAY_LEN;
        expected_args = 0;
        current_bulk_len = 0;
        parsed_args.clear();
    }
};

#endif
