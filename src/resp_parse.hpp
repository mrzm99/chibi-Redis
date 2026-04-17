/*-------------------------------------------------------------------*/
/*!
 *      @file       resp_parse.hpp
 *      @date       2026.xx.xx
 *      @author     mrzm99
 *      @brief
 *      @note
 */
/*-------------------------------------------------------------------*/
#ifndef __RESP_PARSE_HPP__
#define __RESP_PARSE_HPP__

#include "../include/chibi-redis.hpp"
#include <stdint.h>

/*-------------------------------------------------------------------*/
/*! @brief  prototype
 */
bool parse_resp(client_manage& client_info, uint32_t& p_pos);

#endif
