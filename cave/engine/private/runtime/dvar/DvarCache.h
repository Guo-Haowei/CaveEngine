#pragma once
#include "Dvar.h"

#if USING(ENABLE_DVAR)
namespace cave {

class CommandRegistry;

class DvarCache {
public:
    static void Serialize(std::string_view p_path);
    static void Deserialize(std::string_view p_path);
    static bool Parse(std::span<const std::string_view> p_commands);
    static void RegisterCmd(CommandRegistry& p_reg);
};

}  // namespace cave
#endif
