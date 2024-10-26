#ifndef LUA_ENGINE_HPP
#define LUA_ENGINE_HPP
#include <sol/sol.hpp>
#include <luajit.h>
#include <string>
#include <windivert.h>

/// @brief Because the original structs are not aligned,
/// we can't access bitfields , so we gotta wrap all
struct LuaWindivertAddress : WINDIVERT_ADDRESS
{
    LuaWindivertAddress() : WINDIVERT_ADDRESS() {};
    LuaWindivertAddress(const WINDIVERT_ADDRESS& other) : WINDIVERT_ADDRESS(other) {};

    sol::object get(sol::stack_object key, sol::this_state L);
};

class LuaEngine
{
public:
    void Init();

    bool RegisterEnv();

    bool RunCodeUnsafe(std::string code);

    void ShutDown();

    sol::state& GetState();

private:
    sol::state _lstate;
};
#endif