#include <ui.h>
#include <imgui.h>
#include <NetworkEngine.h>
#include <iostream>
#include <sstream>
#include <thread>
NetworkEngine ntEngine;

void ui::Init()
{
    ImGui::SetNextWindowSize({ 800, 600 }, ImGuiCond_FirstUseEver); //{ 800, 600 }
}

std::vector<NetPacket> nPackets;

const char* pktype2str(NetPacketType typ)
{
    switch (typ)
    {
        case NT_TCP:
            return "TCP";
        case NT_UDP:
            return "UDP";
        default:
            return "Unknown";
    }
}

const char* transport2type(NetTransportType typ)
{
    switch (typ)
    {
        case NT_IP:
            return "IP";
        case NT_IPV6:
            return "IPV6";
        case NT_ICMP:
            return "ICMP";
        case NT_ICMPV6:
            return "ICMPV6";
        default:
            return "Unknown";
    }
}
std::string ipToStr(uint32_t ip)
{
    std::stringstream ipStr;
    for(short c = 0; c <= 24; c += 8){
        ipStr << ((ip >> c) & 0xff) << ((c < 24) ? "." : "");    
    }
    return ipStr.str();
}

const char* packet_summary(NetPacket pkt)
{
    char src_str[46], dst_str[46];
    switch (pkt.typ)
    {
        case NT_IP:
            WinDivertHelperFormatIPv4Address(htonl(pkt.ip_header->SrcAddr),
                src_str, sizeof(src_str));
            WinDivertHelperFormatIPv4Address(htonl(pkt.ip_header->DstAddr),
                dst_str, sizeof(dst_str));
    }
    
    std::stringstream ss;
    ss << "[" << pktype2str(pkt.kind) << "] " << ipToStr(pkt.ip_header->SrcAddr) << " -> " << ipToStr(pkt.ip_header->DstAddr) << std::endl;
    return ss.str().c_str(); 
}

void OnPktRecieved(NetworkEngine nt, NetPacket& pkt)
{
    UINT64 hash = WinDivertHelperHashPacket(pkt.data, pkt.len, 0);
    printf("Packet [Timestamp=%.8g, Direction=%s IfIdx=%u SubIfIdx=%u "
            "Loopback=%u Hash=0x%.16llX]\n",
            0, (pkt.addr.Outbound?  "outbound": "inbound"),
            pkt.addr.Network.IfIdx, pkt.addr.Network.SubIfIdx, pkt.addr.Loopback, hash);
    nPackets.push_back(pkt);
    //nt.Send(pkt);
}

// has the network thread started
bool isStarted = false;
void network_thread()
{
    while (true)
    {
        if (ntEngine.isRunning)
        {
            auto pkt = ntEngine.Step();
            if (pkt.has_value())
            {
                printf("Packet recieved\n");
                ntEngine.onPktRecieved(ntEngine, pkt.value());
            } else {
                
            }
        }
        Sleep(200);
    }
}

void ui::Render()
{
    if (!isStarted)
    {
        ntEngine.onPktRecieved = OnPktRecieved;
        std::thread(network_thread).detach();
        isStarted = true;
    }

    if (ImGui::Begin("ImDivert | ElCapor###Menu"))
    {
        // Main menu bar
        ImGui::BeginChild("child1", { -1, 44 }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings);
        {
            /// @separator

            /// @begin Text
            ImGui::TextUnformatted("Status : Running");
            /// @end Text

            /// @begin Spacer
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::Dummy({ 424, 0 });
            /// @end Spacer

            /// @begin Button
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::BeginDisabled(ntEngine.isRunning);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xff008000);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xff669933);
            if (ImGui::Button("Run", { 64, 0 }))
                ntEngine.Init("tcp.DstPort == 12345", 0);
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
            /// @end Button

            /// @begin Button
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::BeginDisabled(!ntEngine.isRunning);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xff0066ff);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xff0000ff);
            if (ImGui::Button("Stop", { 64, 0 }))
                ntEngine.Stop();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
            /// @end Button

            /// @begin Button
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xffff00fe);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xffff9acc);
            ImGui::Button("Clear", { 56, 0 });
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            /// @end Button

            /// @separator
            ImGui::EndChild();
            ImGui::Dummy({ 0, 0 });
            ImGui::Separator();

            if (ImGui::BeginTabBar("##tools"))
            {
                if (ImGui::BeginTabItem("Packets"))
                {
                    for (int i=0; i < nPackets.size(); i++)
                    {
                        ImGui::PushID(i);
                        if (ImGui::CollapsingHeader(packet_summary(nPackets[i])))
                        {
                            ImGui::Text("Holder");
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Settings"))
                {
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

        }
        ImGui::End();
    }
}

void ui::Shutdown()
{

}