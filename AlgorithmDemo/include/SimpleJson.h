#pragma once

#include <map>
#include <string>
#include <vector>

namespace simple_json {

class Value {
public:
    enum Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    typedef std::vector<Value> ArrayType;
    typedef std::map<std::string, Value> ObjectType;

    Value();
    explicit Value(bool value);
    explicit Value(double value);
    explicit Value(const std::string& value);
    explicit Value(const ArrayType& value);
    explicit Value(const ObjectType& value);

    Type type() const;
    bool is_null() const;
    bool is_bool() const;
    bool is_number() const;
    bool is_string() const;
    bool is_array() const;
    bool is_object() const;

    bool as_bool(bool defaultValue = false) const;
    double as_number(double defaultValue = 0.0) const;
    std::string as_string(const std::string& defaultValue = "") const;
    const ArrayType& as_array() const;
    const ObjectType& as_object() const;

    const Value* get(const std::string& key) const;
    const Value* at(size_t index) const;

private:
    Type type_;
    bool boolValue_;
    double numberValue_;
    std::string stringValue_;
    ArrayType arrayValue_;
    ObjectType objectValue_;
};

Value Parse(const std::string& text);
Value ParseFile(const std::string& path);
std::string EscapeString(const std::string& text);

} // namespace simple_json
