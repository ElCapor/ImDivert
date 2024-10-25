#include <LuaEngine.h>
#include <NetworkEngine.h>

void LuaEngine::Init()
{
    _lstate.open_libraries(sol::lib::base);
}


WINDIVERT_ADDRESS tester;

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
    windivert_meta.new_usertype<WINDIVERT_ADDRESS>("Address",
        
        
    );

    tester.Layer |= 0;
    std::cout << tester.Layer << std::endl;
    tester.Timestamp = INT64_MAX;
    _lstate["tester"] = std::ref(tester);

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
