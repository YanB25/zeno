#ifndef PARSER_H_
#define PARSER_H_
#include <inttypes.h>

#include "zeno/header.hpp"
namespace zeno
{
namespace net
{
inline ClientId ParseClientId(const char *data)
{
    auto *header = (PacketHeader *) data;
    return header->client_id;
}
inline PacketLength ParsePacketLength(const char *data)
{
    auto *header = (PacketHeader *) data;
    return header->packet_length;
}
inline PacketType ParsePacketType(const char *data)
{
    auto *header = (PacketHeader *) data;
    return header->packet_type;
}

inline char *ParsePacketBody(char *data)
{
    return data + sizeof(PacketHeader);
}
}  // namespace net
}  // namespace zeno

#endif