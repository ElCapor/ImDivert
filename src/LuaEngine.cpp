#include <LuaEngine.h>
#include <NetworkEngine.h>

/// @brief Because the original structs are not aligned,
/// we can't access bitfields , so we gotta wrap all
struct LuaWindivertAddress : WINDIVERT_ADDRESS
{
    LuaWindivertAddress() : WINDIVERT_ADDRESS()
    {
    }
    LuaWindivertAddress(const WINDIVERT_ADDRESS &other) : WINDIVERT_ADDRESS(other)
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
                WINDIVERT_LAYER val = (WINDIVERT_LAYER)this->Layer;
                return sol::make_object(L, val);
            }
            else if (str == "Timestamp")
            {
                return sol::make_object(L, this->Timestamp);
            }
            else if (str == "Event")
            {
                uint32_t val = this->Event;
                return sol::make_object(L, val);
            }
            else if (str == "Sniffed")
            {
                bool val = this->Sniffed;
                return sol::make_object(L, val);
            }
            else if (str == "Outbound")
            {
                bool val = this->Outbound;
                return sol::make_object(L, val);
            }
            else if (str == "Loopback")
            {
                bool val = this->Loopback;
                return sol::make_object(L, val);
            }
            else if (str == "Impostor")
            {
                bool val = this->Impostor;
                return sol::make_object(L, val);
            }
            else if (str == "IPv6")
            {
                bool isIPV6 = this->IPv6;
                return sol::make_object(L, isIPV6);
            }
            else if (str == "IPChecksum")
            {
                bool val = this->IPChecksum;
                return sol::make_object(L, val);
            }
            else if (str == "TCPChecksum")
            {
                bool val = this->TCPChecksum;
                return sol::make_object(L, val);
            }
            else if (str == "UDPChecksum")
            {
                bool val = this->UDPChecksum;
                return sol::make_object(L, val);
            }
            else if (str == "Network")
            {
                // not a network packet
                if (Layer != 0 || Layer != 1)
                    return sol::object(L, sol::in_place, sol::lua_nil);
                
                return sol::make_object(L, this->Network);
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
        {{"IP", NetTransportType::NT_IP},
         {"IPV6", NetTransportType::NT_IPV6},
         {"ICMP", NetTransportType::NT_ICMP},
         {"ICMPV6", NetTransportType::NT_ICMPV6}});

    _lstate.new_enum<NetPacketType>(
        "NetPacketType",
        {{"TCP", NetPacketType::NT_TCP},
         {"UDP", NetPacketType::NT_UDP}});

    _lstate.new_enum<WINDIVERT_LAYER>(
        "WINDIVERT_LAYER",
        {
            {"NETWORK", WINDIVERT_LAYER::WINDIVERT_LAYER_NETWORK},
            {"NETWORK_FORWARD", WINDIVERT_LAYER::WINDIVERT_LAYER_NETWORK_FORWARD},
            {"FLOW", WINDIVERT_LAYER::WINDIVERT_LAYER_FLOW},
            {"SOCKET", WINDIVERT_LAYER::WINDIVERT_LAYER_SOCKET},
            {"REFLECT", WINDIVERT_LAYER::WINDIVERT_LAYER_REFLECT},
        }
    );
    auto windivert_meta = _lstate["Windivert"].get_or_create<sol::table>();

    windivert_meta.new_usertype<WINDIVERT_DATA_NETWORK>("Network",
            "IfIdx", &WINDIVERT_DATA_NETWORK::IfIdx,
            "SubIfIdx", &WINDIVERT_DATA_NETWORK::SubIfIdx
    );

    windivert_meta.new_usertype<WINDIVERT_DATA_FLOW>("Flow",
    
        "EndpointId", &WINDIVERT_DATA_FLOW::EndpointId,
        "ParentEndpointId", &WINDIVERT_DATA_FLOW::ParentEndpointId,
        "ProcessId", &WINDIVERT_DATA_FLOW::ProcessId,
        "LocalAddr", &WINDIVERT_DATA_FLOW::LocalAddr,
        "RemoteAddr", &WINDIVERT_DATA_FLOW::RemoteAddr,
        "Protocol", &WINDIVERT_DATA_FLOW::Protocol
    );

    windivert_meta.new_usertype<WINDIVERT_DATA_SOCKET>("Socket",
    
        "EndpointId", &WINDIVERT_DATA_SOCKET::EndpointId,
        "ParentEndpointId", &WINDIVERT_DATA_SOCKET::ParentEndpointId,
        "ProcessId", &WINDIVERT_DATA_SOCKET::ProcessId,
        "LocalAddr", &WINDIVERT_DATA_SOCKET::LocalAddr,
        "RemoteAddr", &WINDIVERT_DATA_SOCKET::RemoteAddr,
        "Protocol", &WINDIVERT_DATA_SOCKET::Protocol
    );

    windivert_meta.new_usertype<WINDIVERT_DATA_REFLECT>("Reflect",
        "Timestamp", &WINDIVERT_DATA_REFLECT::Timestamp,
        "ProcessId", &WINDIVERT_DATA_REFLECT::ProcessId,
        "Layer", &WINDIVERT_DATA_REFLECT::Layer,
        "Flags", &WINDIVERT_DATA_REFLECT::Flags,
        "Priority", &WINDIVERT_DATA_REFLECT::Priority
    );

    windivert_meta.new_usertype<LuaWindivertAddress>("Address",
                                                     sol::meta_function::index, &LuaWindivertAddress::get);

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
    auto bad_code_result = _lstate.script(code, [](lua_State *, sol::protected_function_result pfr)
                                          {
		// pfr will contain things that went wrong, for either loading or executing the script
		// Can throw your own custom error
		// You can also just return it, and let the call-site handle the error if necessary.
		return pfr; });
    return bad_code_result.valid();
}

void LuaEngine::ShutDown()
{
}
