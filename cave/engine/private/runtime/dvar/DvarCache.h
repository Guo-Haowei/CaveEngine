#pragma once
#include "Dvar.h"

#if USING(ENABLE_DVAR)
namespace cave {

class CommandRegistry;

class DvarCache {
public:
    static void serialize(std::string_view path);
    static void deserialize(std::string_view path);
    static bool parse(std::span<const std::string_view> commands);
    static void registerCmd(CommandRegistry& reg);
};

}  // namespace cave
#endif
