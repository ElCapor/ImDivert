#include <ui.h>
#include <imgui.h>
#include <NetworkEngine.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <imgui_hex_editor.h>
#include <TextEditor.h>
#include <LuaEngine.h>
TextEditor editor;
NetworkEngine ntEngine;
LuaEngine lEngine;

void ui::Init()
{
    ImGui::SetNextWindowSize({800, 600}, ImGuiCond_FirstUseEver); //{ 800, 600 }
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
    editor.SetPalette(TextEditor::GetDarkPalette());
    lEngine.Init();
    lEngine.RegisterEnv();
    
}

std::vector<NetPacket> nPackets;

const char *pktype2str(NetPacketType typ)
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

const char *transport2type(NetTransportType typ)
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
    for (short c = 0; c <= 24; c += 8)
    {
        ipStr << ((ip >> c) & 0xff) << ((c < 24) ? "." : "");
    }
    return ipStr.str();
}

const char *packet_summary(NetPacket pkt)
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

void OnPktRecieved(NetworkEngine nt, NetPacket &pkt)
{
    UINT64 hash = WinDivertHelperHashPacket(pkt.data, pkt.len, 0);
    printf("Packet [Timestamp=%.8g, Direction=%s IfIdx=%u SubIfIdx=%u "
           "Loopback=%u Hash=0x%.16llX]\n",
           0, (pkt.addr.Outbound ? "outbound" : "inbound"),
           pkt.addr.Network.IfIdx, pkt.addr.Network.SubIfIdx, pkt.addr.Loopback, hash);
    auto fn = lEngine.GetState()["onPktRecieve"];
    // user registered a callback to register packets
    if (fn.is<sol::function>())
    {
        fn.call(std::ref(pkt.addr));   
    }
    nPackets.push_back(pkt);
    // nt.Send(pkt);
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
            }
            else
            {
            }
        }
        Sleep(200);
    }
}

uint16_t reverseEndian16(uint16_t value)
{
    return (value >> 8) | (value << 8);
}

static MemoryEditor mem_edit;
NetPacket currentPkt;
bool showPktHex = false;

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
        ImGui::BeginChild("child1", {-1, 44}, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoSavedSettings);
        {
            /// @separator

            /// @begin Text
            ImGui::TextUnformatted("Status : Running");
            /// @end Text

            /// @begin Spacer
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::Dummy({424, 0});
            /// @end Spacer

            /// @begin Button
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::BeginDisabled(ntEngine.isRunning);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xff008000);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xff669933);
            if (ImGui::Button("Run", {64, 0}))
                ntEngine.Init("udp.DstPort == 12345", 0);
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
            /// @end Button

            /// @begin Button
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::BeginDisabled(!ntEngine.isRunning);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xff0066ff);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xff0000ff);
            if (ImGui::Button("Stop", {64, 0}))
                ntEngine.Stop();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
            /// @end Button

            /// @begin Button
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0xffff00fe);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0xffff9acc);
            ImGui::Button("Clear", {56, 0});
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            /// @end Button

            /// @separator
            ImGui::EndChild();
            ImGui::Dummy({0, 0});
            ImGui::Separator();

            if (ImGui::BeginTabBar("##tools"))
            {
                if (ImGui::BeginTabItem("Packets"))
                {
                    for (int i = 0; i < nPackets.size(); i++)
                    {
                        NetPacket pkt = nPackets[i];
                        ImGui::PushID(i);
                        if (ImGui::CollapsingHeader(packet_summary(nPackets[i])))
                        {
                            switch (pkt.typ)
                            {
                            case NT_IP:
                            {
                                if (ImGui::TreeNode("IP Header"))
                                {
                                    ImGui::Text("Header Length %i", pkt.ip_header->HdrLength);
                                    ImGui::Text("IP Version %i", pkt.ip_header->Version);
                                    ImGui::Text("TOS %i", pkt.ip_header->TOS);
                                    ImGui::Text("TTL %i", pkt.ip_header->TTL);
                                    ImGui::Text("Protocol %i", pkt.ip_header->Protocol);
                                    ImGui::Text("Checksum %i", pkt.ip_header->Checksum);
                                    ImGui::Text("SrcAddr %s", ipToStr(pkt.ip_header->SrcAddr).c_str());
                                    ImGui::Text("DestAddr %s", ipToStr(pkt.ip_header->DstAddr).c_str());
                                    ImGui::TreePop();
                                }
                            }
                            break;

                            default:
                                ImGui::Text("Unknown Header");
                                break;
                            }

                            switch (pkt.kind)
                            {
                            case NT_TCP:
                            {
                                if (ImGui::TreeNode("TCP Header"))
                                {
                                    ImGui::Text("Src Port %i", pkt.tcp_header->SrcPort);
                                    ImGui::Text("Dst Port %i", pkt.tcp_header->DstPort);
                                    ImGui::Text("Checksum %i", pkt.tcp_header->Checksum);
                                    ImGui::Text("SeqNum %i", pkt.tcp_header->SeqNum);
                                    ImGui::Text("AckNum %i", pkt.tcp_header->AckNum);
                                    ImGui::Text("HdrLength %i", pkt.tcp_header->HdrLength);
                                    ImGui::Text("Fin %b", pkt.tcp_header->Fin);
                                    ImGui::Text("Syn %b", pkt.tcp_header->Syn);
                                    ImGui::Text("Rst %b", pkt.tcp_header->Rst);
                                    ImGui::Text("Psh %b", pkt.tcp_header->Psh);
                                    ImGui::Text("Ack %b", pkt.tcp_header->Ack);
                                    ImGui::Text("Urg %b", pkt.tcp_header->Urg);
                                    ImGui::Text("Window %b", pkt.tcp_header->Window);

                                    ImGui::Text("Reserved1 %i", pkt.tcp_header->Reserved1);
                                    ImGui::Text("Reserved2 %i", pkt.tcp_header->Reserved2);
                                    ImGui::TreePop();
                                }
                            }
                            break;

                            case NT_UDP:
                            {
                                if (ImGui::TreeNode("UDP Header"))
                                {
                                    ImGui::Text("Src Port %i", reverseEndian16(pkt.udp_header->SrcPort));
                                    ImGui::Text("Dst Port %i", reverseEndian16(pkt.udp_header->DstPort));
                                    ImGui::Text("Length %i", pkt.udp_header->Length);
                                    ImGui::Text("Checksum %i", pkt.udp_header->Checksum);
                                    ImGui::TreePop();
                                }
                            }
                            break;

                            default:
                                ImGui::Text("Unknown Protocol");
                                break;
                            }
                            if (ImGui::TreeNode("Raw Data"))
                            {
                                if (ImGui::Button("Edit hex"))
                                {
                                    currentPkt = pkt;
                                    mem_edit.Open = true;
                                }
                                ImGui::TreePop();
                            }
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Scripting"))
                {
                    if (ImGui::BeginChild("Cap", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, ImGuiWindowFlags_MenuBar))
                    {
                        if (ImGui::BeginMenuBar())
                        {
                            if (ImGui::BeginMenu("File"))
                            {
                                ImGui::EndMenu();
                            }
                            if (ImGui::BeginMenu("Edit"))
                            {
                                if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, editor.CanUndo()))
                                    editor.Undo();
                                if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, editor.CanRedo()))
                                    editor.Redo();

                                ImGui::Separator();

                                if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.AnyCursorHasSelection()))
                                    editor.Copy();
                                if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, editor.AnyCursorHasSelection()))
                                    editor.Cut();
                                if (ImGui::MenuItem("Delete", "Del", nullptr, editor.AnyCursorHasSelection()))
                                    editor.ReplaceTextInCurrentCursor("");
                                if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr))
                                    editor.Paste();

                                ImGui::Separator();

                                if (ImGui::MenuItem("Select all", "Ctrl-A", nullptr))
                                    editor.SelectAll();

                                ImGui::EndMenu();
                            }
                            if (ImGui::BeginMenu("Go"))
                            {
                                if (ImGui::MenuItem("Execute"))
                                {
                                    lEngine.RunCodeUnsafe(editor.GetText());
                                }
                                ImGui::EndMenu();
                            }
                            ImGui::EndMenuBar();
                        }
                        editor.Render("");
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
        if (mem_edit.Open)
        {
            mem_edit.DrawWindow("Memory Editor", GetRawData(currentPkt).data(), currentPkt.len - GetHeadersLen(currentPkt));
        }
    }
}

void ui::Shutdown()
{
}