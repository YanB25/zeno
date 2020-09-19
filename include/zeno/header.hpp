#ifndef HEADER_H_
#define HEADER_H_

#include <inttypes.h>

namespace zeno
{
namespace net
{
/**
 * 0-8:   PacketLength
 * 8-16:  ClientId
 * 16-24: PacketType
 */
using PacketLength = uint64_t;
using ClientId = uint64_t;
enum class PacketType : uint8_t
{
    Normal = 1,
};

struct PacketHeader
{
    PacketLength packet_length;
    ClientId client_id;
    PacketType packet_type;
} __attribute__((packed));

}  // namespace net
}  // namespace zeno

#endif