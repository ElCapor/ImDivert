#include <Windows.h>
#include <windivert.h>
#include <optional>
#include <functional>

enum NetTransportType
{
    NT_IP,
    NT_IPV6,
    NT_ICMP,
    NT_ICMPV6
};

enum NetPacketType
{
    NT_TCP,
    NT_UDP
};

struct NetPacket
{
    WINDIVERT_ADDRESS addr;
    UINT len;
    unsigned char data[40 + 0xFFF];

    NetTransportType typ;
    union {
        PWINDIVERT_IPHDR ip_header;
        PWINDIVERT_IPV6HDR ipv6_header;
        PWINDIVERT_ICMPHDR icmp_header;
        PWINDIVERT_ICMPV6HDR icmpv6_header;
    };

    NetPacketType kind;
    union
    {
        PWINDIVERT_TCPHDR tcp_header;
        PWINDIVERT_UDPHDR udp_header;
    };
};

#include <vector>

namespace xtd {
    template <typename T>
    std::vector<T> slice(const T* array, int start, int end) {
        int length = end - start;
        
        if (length <= 0) return {};

        std::vector<T> result(length);

        for (int i = 0; i < length; ++i) {
            result[i] = array[start + i];
        }

        return result;
    }
}

std::vector<uint8_t> GetRawData(NetPacket& pkt);
uint32_t GetHeadersLen(NetPacket& pkt);

class NetworkEngine
{
    public:
    bool Init(const char* filters, unsigned long layr);
    bool IsValid();

    std::optional<NetPacket> Step();
    bool Send(NetPacket pkt);

    std::function<void(NetworkEngine, NetPacket&)> onPktRecieved;

    bool isRunning = false;
    bool isDirty = false;

    bool Stop();

    private:
    HANDLE hHandle;
};