#ifndef PARSER_H_
#define PARSER_H_
#include <inttypes.h>
namespace zeno
{
namespace parse
{
inline int ParseClientId(const char *data)
{
    int *offset = (int *) (data + sizeof(uint64_t));
    return *offset;
}
}  // namespace parse
}  // namespace zeno

#endif