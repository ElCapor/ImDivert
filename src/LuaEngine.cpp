#include <LuaEngine.h>
#include <NetworkEngine.h>

/// @brief Because the original structs are not aligned,
/// we can't access bitfields , so we gotta wrap all
struct LuaWindivertAddress : WINDIVERT_ADDRESS
{
    LuaWindivertAddress() : WINDIVERT_ADDRESS()
    {

    }
    LuaWindivertAddress(const WINDIVERT_ADDRESS& other) : WINDIVERT_ADDRESS(other)
    {

    }

    sol::object get(sol::stack_object key, sol::this_state L)
    {
        auto key_str = key.as<sol::optional<std::string>>();
        std::cout << "got key " << *key_str << std::endl;
        if (key_str)
        {
            const auto str = *key_str;
            if (str == "Layer")
            {
                uint32_t val = this->Layer;
                return sol::make_object(L, val);
            } else if (str == "Timestamp")
            {
                return sol::make_object(L, this->Timestamp);
            }
        }
        return sol::object(L, sol::in_place, sol::lua_nil);

    }
};

void LuaEngine::Init()
{
    _lstate.open_libraries(sol::lib::base);
}


WINDIVERT_ADDRESS tester;
LuaWindivertAddress add;

bool LuaEngine::RegisterEnv()
{
    _lstate.new_enum<NetTransportType>(
        "NetTransportType",
        {
        {"IP", NetTransportType::NT_IP},
        {"IPV6", NetTransportType::NT_IPV6},
        {"ICMP", NetTransportType::NT_ICMP},
        {"ICMPV6", NetTransportType::NT_ICMPV6}
        }
    );

    _lstate.new_enum<NetPacketType>(
        "NetPacketType",
        {
            {"TCP", NetPacketType::NT_TCP},
            {"UDP", NetPacketType::NT_UDP}
        }
    );
    auto windivert_meta = _lstate["Windivert"].get_or_create<sol::table>();
    windivert_meta.new_usertype<LuaWindivertAddress>("Address",
        sol::meta_function::index, &LuaWindivertAddress::get
    );

    tester.Layer = 2;
    std::cout << tester.Layer << std::endl;
    tester.Timestamp = INT64_MAX;
    add = tester;
    _lstate["tester"] = std::ref(add);
    std::cout << add.Timestamp << std::endl;

    return true;
}

bool LuaEngine::RunCodeUnsafe(std::string code)
{
    auto bad_code_result = _lstate.script(code, [](lua_State*, sol::protected_function_result pfr) {
		// pfr will contain things that went wrong, for either loading or executing the script
		// Can throw your own custom error
		// You can also just return it, and let the call-site handle the error if necessary.
		return pfr;
	});
    return bad_code_result.valid();
}

void LuaEngine::ShutDown()
{
}
