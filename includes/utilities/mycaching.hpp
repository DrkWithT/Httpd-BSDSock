#pragma once

#include <sstream>
#include <string>

namespace MyHttpd::Utilities {
    class GMTGen {
    private:
        std::ostringstream m_sout;
        std::time_t m_timing;

    public:
        GMTGen();

        [[nodiscard]] std::string operator()();
    };
}