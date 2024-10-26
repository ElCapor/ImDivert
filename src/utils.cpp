#include <utils.h>
#include <string>
#include <sstream>

namespace xtd
{
    std::string ipToStr(uint32_t ip)
    {
        std::stringstream ipStr;
        for (short c = 0; c <= 24; c += 8)
        {
            ipStr << ((ip >> c) & 0xff) << ((c < 24) ? "." : "");
        }
        return ipStr.str();
    }
}