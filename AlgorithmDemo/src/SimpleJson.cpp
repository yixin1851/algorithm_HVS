#include "SimpleJson.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace simple_json {

namespace {

const Value::ArrayType& EmptyArray() {
    static const Value::ArrayType value;
    return value;
}

const Value::ObjectType& EmptyObject() {
    static const Value::ObjectType value;
    return value;
}

class Parser {
public:
    explicit Parser(const std::string& text) : text_(text), pos_(0) {}

    Value parse() {
        skip_ws();
        Value value = parse_value();
        skip_ws();
        if (pos_ != text_.size()) {
            fail("unexpected trailing characters");
        }
        return value;
    }

private:
    const std::string& text_;
    size_t pos_;

    void skip_ws() {
        while (pos_ < text_.size() &&
               std::isspace(static_cast<unsigned char>(text_[pos_]))) {
            ++pos_;
        }
    }

    bool consume(char ch) {
        if (pos_ < text_.size() && text_[pos_] == ch) {
            ++pos_;
            return true;
        }
        return false;
    }

    void expect(char ch) {
        if (!consume(ch)) {
            std::ostringstream message;
            message << "expected '" << ch << "'";
            fail(message.str());
        }
    }

    void fail(const std::string& message) const {
        std::ostringstream out;
        out << "json parse error at " << pos_ << ": " << message;
        throw std::runtime_error(out.str());
    }

    bool starts_with(const char* text) const {
        size_t i = 0;
        while (text[i] != '\0') {
            if (pos_ + i >= text_.size() || text_[pos_ + i] != text[i]) {
                return false;
            }
            ++i;
        }
        return true;
    }

    Value parse_value() {
        skip_ws();
        if (pos_ >= text_.size()) {
            fail("unexpected end of input");
        }
        const char ch = text_[pos_];
        if (ch == '"') {
            return Value(parse_string());
        }
        if (ch == '{') {
            return parse_object();
        }
        if (ch == '[') {
            return parse_array();
        }
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch))) {
            return Value(parse_number());
        }
        if (starts_with("true")) {
            pos_ += 4;
            return Value(true);
        }
        if (starts_with("false")) {
            pos_ += 5;
            return Value(false);
        }
        if (starts_with("null")) {
            pos_ += 4;
            return Value();
        }
        fail("unexpected value");
        return Value();
    }

    std::string parse_string() {
        expect('"');
        std::string out;
        while (pos_ < text_.size()) {
            const char ch = text_[pos_++];
            if (ch == '"') {
                return out;
            }
            if (ch != '\\') {
                out.push_back(ch);
                continue;
            }
            if (pos_ >= text_.size()) {
                fail("unfinished escape sequence");
            }
            const char escaped = text_[pos_++];
            switch (escaped) {
            case '"': out.push_back('"'); break;
            case '\\': out.push_back('\\'); break;
            case '/': out.push_back('/'); break;
            case 'b': out.push_back('\b'); break;
            case 'f': out.push_back('\f'); break;
            case 'n': out.push_back('\n'); break;
            case 'r': out.push_back('\r'); break;
            case 't': out.push_back('\t'); break;
            case 'u':
                // Keep non-ASCII unicode escapes simple; profile paths and keys are ASCII.
                if (pos_ + 4 > text_.size()) {
                    fail("unfinished unicode escape");
                }
                pos_ += 4;
                out.push_back('?');
                break;
            default:
                fail("invalid escape sequence");
            }
        }
        fail("unterminated string");
        return out;
    }

    double parse_number() {
        const size_t start = pos_;
        if (consume('-')) {}
        if (consume('0')) {
        } else {
            if (pos_ >= text_.size() ||
                !std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                fail("invalid number");
            }
            while (pos_ < text_.size() &&
                   std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                ++pos_;
            }
        }
        if (consume('.')) {
            if (pos_ >= text_.size() ||
                !std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                fail("invalid number fraction");
            }
            while (pos_ < text_.size() &&
                   std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                ++pos_;
            }
        }
        if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) {
                ++pos_;
            }
            if (pos_ >= text_.size() ||
                !std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                fail("invalid number exponent");
            }
            while (pos_ < text_.size() &&
                   std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                ++pos_;
            }
        }
        std::istringstream in(text_.substr(start, pos_ - start));
        double value = 0.0;
        in >> value;
        if (!in) {
            fail("invalid number");
        }
        return value;
    }

    Value parse_array() {
        expect('[');
        Value::ArrayType values;
        skip_ws();
        if (consume(']')) {
            return Value(values);
        }
        while (true) {
            values.push_back(parse_value());
            skip_ws();
            if (consume(']')) {
                break;
            }
            expect(',');
        }
        return Value(values);
    }

    Value parse_object() {
        expect('{');
        Value::ObjectType values;
        skip_ws();
        if (consume('}')) {
            return Value(values);
        }
        while (true) {
            skip_ws();
            if (pos_ >= text_.size() || text_[pos_] != '"') {
                fail("expected object key");
            }
            const std::string key = parse_string();
            skip_ws();
            expect(':');
            values[key] = parse_value();
            skip_ws();
            if (consume('}')) {
                break;
            }
            expect(',');
        }
        return Value(values);
    }
};

} // namespace

Value::Value()
    : type_(Null), boolValue_(false), numberValue_(0.0) {}

Value::Value(bool value)
    : type_(Bool), boolValue_(value), numberValue_(0.0) {}

Value::Value(double value)
    : type_(Number), boolValue_(false), numberValue_(value) {}

Value::Value(const std::string& value)
    : type_(String), boolValue_(false), numberValue_(0.0), stringValue_(value) {}

Value::Value(const ArrayType& value)
    : type_(Array), boolValue_(false), numberValue_(0.0), arrayValue_(value) {}

Value::Value(const ObjectType& value)
    : type_(Object), boolValue_(false), numberValue_(0.0), objectValue_(value) {}

Value::Type Value::type() const { return type_; }
bool Value::is_null() const { return type_ == Null; }
bool Value::is_bool() const { return type_ == Bool; }
bool Value::is_number() const { return type_ == Number; }
bool Value::is_string() const { return type_ == String; }
bool Value::is_array() const { return type_ == Array; }
bool Value::is_object() const { return type_ == Object; }

bool Value::as_bool(bool defaultValue) const {
    return type_ == Bool ? boolValue_ : defaultValue;
}

double Value::as_number(double defaultValue) const {
    return type_ == Number ? numberValue_ : defaultValue;
}

std::string Value::as_string(const std::string& defaultValue) const {
    return type_ == String ? stringValue_ : defaultValue;
}

const Value::ArrayType& Value::as_array() const {
    return type_ == Array ? arrayValue_ : EmptyArray();
}

const Value::ObjectType& Value::as_object() const {
    return type_ == Object ? objectValue_ : EmptyObject();
}

const Value* Value::get(const std::string& key) const {
    if (type_ != Object) {
        return 0;
    }
    ObjectType::const_iterator it = objectValue_.find(key);
    return it == objectValue_.end() ? 0 : &it->second;
}

const Value* Value::at(size_t index) const {
    if (type_ != Array || index >= arrayValue_.size()) {
        return 0;
    }
    return &arrayValue_[index];
}

Value Parse(const std::string& text) {
    Parser parser(text);
    return parser.parse();
}

Value ParseFile(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in) {
        throw std::runtime_error("failed to open json file: " + path);
    }
    std::ostringstream text;
    text << in.rdbuf();
    return Parse(text.str());
}

std::string EscapeString(const std::string& text) {
    std::ostringstream out;
    for (size_t i = 0; i < text.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(text[i]);
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (ch < 0x20) {
                out << "?";
            } else {
                out << static_cast<char>(ch);
            }
            break;
        }
    }
    return out.str();
}

} // namespace simple_json
