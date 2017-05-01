/*
 * rpc_message.hpp
 *
 *  Created on: May 1, 2017
 *      Author: danny.havenith
 */

#ifndef RPC_MESSAGE_HPP_
#define RPC_MESSAGE_HPP_

#include "blob.hpp"
#include <tuple>

typedef std::tuple<std::string, Blob> RpcMessage;
typedef Blob RpcReply;

#endif /* RPC_MESSAGE_HPP_ */
