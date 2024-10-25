#include <NetworkEngine.h>
#include <windivert.h>
bool NetworkEngine::IsValid()
{
    return hHandle != INVALID_HANDLE_VALUE;
}

bool NetworkEngine::Init(const char* filters, unsigned long layr)
{
    hHandle = WinDivertOpen(filters, (WINDIVERT_LAYER)layr, 0, 0);
    isRunning = true;
    return IsValid();
}

std::optional<NetPacket> NetworkEngine::Step()
{
    if (!IsValid())
        return std::nullopt;

    NetPacket pkt;
    if (!WinDivertRecv(hHandle, pkt.data, sizeof(pkt.data), &pkt.len, &pkt.addr))
    {

        return std::nullopt;
    }

    WINDIVERT_ADDRESS addr;
    PWINDIVERT_IPHDR ip_header;
    PWINDIVERT_IPV6HDR ipv6_header;
    PWINDIVERT_ICMPHDR icmp_header;
    PWINDIVERT_ICMPV6HDR icmpv6_header;
    PWINDIVERT_TCPHDR tcp_header;
    PWINDIVERT_UDPHDR udp_header;

    WinDivertHelperParsePacket(pkt.data, pkt.len, &ip_header, &ipv6_header,
            NULL, &icmp_header, &icmpv6_header, &tcp_header, &udp_header, NULL,
            NULL, NULL, NULL);

    if (ip_header != NULL)
    {
        pkt.typ = NetTransportType::NT_IP;
        pkt.ip_header = ip_header;
    } else if (ipv6_header != NULL)
    {
        pkt.typ = NetTransportType::NT_IPV6;
        pkt.ipv6_header = ipv6_header;
    } else if (icmp_header != NULL)
    {
        pkt.typ = NetTransportType::NT_ICMP;
        pkt.icmp_header = icmp_header;
    } else if (icmpv6_header != NULL)
    {
        pkt.typ = NetTransportType::NT_ICMPV6;
        pkt.icmpv6_header = icmpv6_header;
    } else
    {
        return std::nullopt; // unhandled packet
    }
    
    if (tcp_header != NULL)
    {
        pkt.kind = NetPacketType::NT_TCP;
        pkt.tcp_header = tcp_header;
    } 
    else if (udp_header != NULL)
    {
        pkt.kind = NetPacketType::NT_UDP;
        pkt.udp_header = udp_header;
    } else
    {
        return std::nullopt; // unhandled packet
    }

    return pkt;
}

/// @brief Careful ! User is responsible for recomputing the checksum
/// @param pkt 
/// @return true if success
bool NetworkEngine::Send(NetPacket pkt)
{
    //WinDivertHelperCalcChecksums(pkt.data, pkt.len, &pkt.addr, 0);
    return IsValid() && WinDivertSend(hHandle, pkt.data, pkt.len, NULL, &pkt.addr);
}

bool NetworkEngine::Stop()
{
    isRunning = false;
    return WinDivertClose(hHandle);
}

std::vector<uint8_t> GetRawData(NetPacket &pkt)
{
    

    int extra_size = sizeof(WINDIVERT_ADDRESS);

    auto ret = xtd::slice(pkt.data, GetHeadersLen(pkt), 4095);
    return ret;
}

uint32_t GetHeadersLen(NetPacket &pkt)
{
    int header_size = 0;
    switch (pkt.typ)
    {
        case NT_IP:
            header_size = sizeof(WINDIVERT_IPHDR);
            break;
        case NT_IPV6:
            header_size = sizeof(WINDIVERT_IPV6HDR);
            break;
        case NT_ICMP:
            header_size = sizeof(WINDIVERT_ICMPHDR);
            break;
        case NT_ICMPV6:
            header_size = sizeof(WINDIVERT_ICMPV6HDR);
            break;
    };

    int kind_size = 0;
    switch (pkt.kind)
    {
        case NT_TCP:
            kind_size = sizeof(WINDIVERT_TCPHDR);
            break;
        
        case NT_UDP:
            kind_size = sizeof(WINDIVERT_UDPHDR);
            break;
    }

    return header_size + kind_size;
}
