#include <ctime>
#include <iomanip>
#include <sstream>
#include "utilities/mycaching.hpp"

namespace MyHttpd::Utilities {
    GMTGen::GMTGen()
    : m_sout {}, m_timing {} {}

    std::string GMTGen::operator()() {
        m_timing = std::time(nullptr);

        std::tm* gmt_data = std::gmtime(&m_timing);
        
        m_sout << std::put_time(gmt_data, "%a, %d %b %Y %T %Z");

        auto result = m_sout.str();
        m_sout.str("");

        return result;
    }
}