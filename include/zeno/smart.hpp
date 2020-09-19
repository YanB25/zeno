#ifndef SMART_H
#define SMART_H
#include <string>

#include "zeno/define.hpp"
namespace zeno
{
namespace smart
{
inline std::string toSize(double size)
{
    if (size < 1 * define::KiB)
    {
        return std::to_string(size) + " B";
    }
    else if (size < 1 * define::MiB)
    {
        return std::to_string(size / define::KiB) + " KiB";
    }
    else if (size < 1 * define::GiB)
    {
        return std::to_string(size / define::MiB) + " MiB";
    }
    else if (size < 1 * define::TiB)
    {
        return std::to_string(size / define::GiB) + " GiB";
    }
    else
    {
        return std::to_string(size / define::TiB) + " TiB";
    }
}
inline std::string toNum(double ops)
{
    if (ops < 1 * define::K)
    {
        return std::to_string(ops) + " ";
    }
    else if (ops < 1 * define::M)
    {
        return std::to_string(ops / define::K) + " K";
    }
    else if (ops < 1 * define::G)
    {
        return std::to_string(ops / define::M) + " M";
    }
    else if (ops < 1 * define::T)
    {
        return std::to_string(ops / define::G) + " G";
    }
    else
    {
        return std::to_string(ops / define::T) + " T";
    }
}
inline std::string toOps(double ops)
{
    return toNum(ops) + "ops";
}

inline std::string nsToLatency(double ns)
{
    if (ns < define::K)
    {
        return std::to_string(ns) + " ns";
    }
    else if (ns < define::M)
    {
        return std::to_string(ns / define::K) + " us";
    }
    else if (ns < define::G)
    {
        return std::to_string(ns / define::M) + " ms";
    }
    return std::to_string(ns / define::G) + " s";
}
}  // namespace smart
}  // namespace zeno
#endif