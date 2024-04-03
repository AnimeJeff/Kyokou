#include "video.h"
#include <sstream>

void Video::addHeader(const std::string &key, const std::string &value) {
    m_headers[key] = value;
}

std::string Video::headers() const {
    std::stringstream ss;
    for (auto it = m_headers.begin(); it != m_headers.end(); ++it) {
        if (it != m_headers.begin()) {
            ss << ","; // Add comma separator except before the first element
        }
        ss << it->first << ": " << it->second; // Append "key: value" to the string stream
    }

    return ss.str();
}
