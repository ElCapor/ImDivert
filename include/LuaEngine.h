#include <sol/sol.hpp>
#include <luajit.h>
#include <string>

class LuaEngine
{
public:
    void Init();

    bool RegisterEnv();

    bool RunCodeUnsafe(std::string code);

    void ShutDown();

private:
    sol::state _lstate;
};