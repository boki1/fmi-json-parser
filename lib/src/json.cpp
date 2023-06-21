#include <json-parser/json.h>

namespace json_parser {

///
/// Special member functions
///

json::json(const json &rhs)
{
    if (rhs.m_root_node)
        m_root_node = rhs.m_root_node->clone();
}

json& json::operator=(const json &rhs) {
    auto clone {rhs};
    m_root_node = std::move(clone.m_root_node);
    return *this;
}

bool json::operator==(const json &) const noexcept {
    return false;
}

///
/// Serialization
///

void json::number::serialize(std::ostream &os, std::size_t depth) const {
    os << std::string(depth, ' ');
    m_data.serialize(os);
}

void json::string::serialize(std::ostream &os, std::size_t depth) const {
    os << std::string(depth, ' ');
    m_data.serialize(os);
}

void json::keyword::serialize(std::ostream &os, std::size_t depth) const {
    os << std::string(depth, ' ');
    m_data.serialize(os);
}

} // namespace json_parser
