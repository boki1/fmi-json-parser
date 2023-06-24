#include <string>

#include <json-parser/json.h>

namespace json_parser {

///
/// Following paths
///

json::value *json::follow_impl(const path &path) const {
    json::value *node_ptr = m_root_node.get();
    for (const auto &component: path) {
        if (node_ptr == nullptr || node_ptr->trivial())
            throw json_exception("Cannot follow given path, because it does not exist.");

        // Safety: We just verified that it is not a trivial node, so if it is valid
        // then its only possibility is to be a compound.
        assert(node_ptr->compound());

        if (auto *node_as_object = dynamic_cast<json::object *>(node_ptr); node_as_object) {
            const json::string *component_as_str = dynamic_cast<json::string *>(component.get());
            if (!component_as_str)
                throw json_exception("JSON objects are indexed only when the key is an instance of json::string.");
            try {
                node_ptr = &(*node_as_object)[*component_as_str];
            } catch (const std::out_of_range &oor) {
                throw json_exception("Trying to index JSON object with non-existent key.");
            }

            continue;
        }

        if (auto *node_as_array = dynamic_cast<json::array *>(node_ptr); node_as_array) {
            static const std::string int_error_msg = "JSON objects are indexed only when the key is an integral.";
            const json::number *component_as_num = dynamic_cast<json::number *>(component.get());
            if (!component_as_num)
                throw json_exception(int_error_msg);

            const double num = (double) *component_as_num;
            const std::size_t index = static_cast<std::size_t>(num);
            if ((double) index != num)
                throw json_exception(int_error_msg);
            try {
                node_ptr = &(*node_as_array)[index];
            } catch (const std::out_of_range &oor) {
                throw json_exception("Trying to index JSON object with index that is out of bounds.");
            }
            continue;
        }

        mystd::unreachable();
    }

    return node_ptr;
}

[[nodiscard]] json::value *json::follow(const path &path) {
    return follow_impl(path);
}

[[nodiscard]] const json::value *json::follow(const path &path) const {
    return follow_impl(path);
}
///
/// Other operations
///

[[nodiscard]] json::pmrvalue json::take() {
    auto root_node = std::move(m_root_node);
    m_root_node.reset();
    return root_node;
}

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

void json::dump(std::ostream &os) const {
    if (m_root_node)
        m_root_node->serialize(os, /* depth */ 0);
}

void json::number::serialize(std::ostream &os, std::size_t depth, bool in_object) const {
    if (!in_object)
        os << std::string(depth, ' ');
    m_data.serialize(os);
}

void json::string::serialize(std::ostream &os, std::size_t depth, bool in_object) const {
    if (!in_object)
        os << std::string(depth, ' ');
    m_data.serialize(os);
}

void json::boolean::serialize(std::ostream &os, std::size_t depth, bool in_object) const {
    if (!in_object)
        os << std::string(depth, ' ');
    m_data.serialize(os);
}

void json::null::serialize(std::ostream &os, std::size_t depth, bool in_object) const {
    if (!in_object)
        os << std::string(depth, ' ');
    m_data.serialize(os);
}

} // namespace json_parser
