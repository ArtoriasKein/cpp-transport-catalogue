#include "json.h"
#include <sstream>

using namespace std;

namespace json {

    namespace {

        void CheckInputStream(istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            if (it == end) {
                throw ParsingError("Parsing error");
            }
        }

        Node LoadNode(istream& input);

        Node LoadNull(istream& input) {
            CheckInputStream(input);
            string word;
            for (char c; input >> c && c != ',';) {
                word.push_back(c);
            }
            if (word != "null"s) {
                throw ParsingError("The uploaded text is not null"s);
            }
            return Node(nullptr);
        }

        Node LoadNumber(std::istream& input) {
            CheckInputStream(input);
            using namespace std::literals;
            std::string parsed_num;
            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) {
            CheckInputStream(input);
            using namespace std::literals;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        Node LoadBool(istream& input) {
            CheckInputStream(input);
            string word;
            for (int i = 0; i < 4; ++i) {
                char c;
                input >> c;
                word.push_back(c);
            }
            if (word == "true"s) {
                return Node(true);
            }
            if (word == "fals"s) {
                char c;
                input >> c;
                if (c == 'e') {
                    return Node(false);
                }
            }
            throw ParsingError("The uploaded text is not boolean"s);
        }

        Node LoadArray(istream& input) {
            CheckInputStream(input);
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            CheckInputStream(input);
            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (isdigit(c) || c == '-') {
                input.putback(c);
                return LoadNumber(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else {
                throw ParsingError("Invalid first character"s);
            }
        }

    }  // namespace

    Node::Node(nullptr_t)
        : value_() {
    }

    Node::Node(Array array)
        : value_(move(array)) {
    }

    Node::Node(Dict map)
        : value_(move(map)) {
    }

    Node::Node(int value)
        : value_(move(value)) {
    }

    Node::Node(string value)
        : value_(move(value)) {
    }

    Node::Node(double value)
        : value_(move(value)) {

    }
    Node::Node(bool value)
        : value_(move(value)) {

    }

    bool Node::IsInt() const {
        if (holds_alternative<int>(value_)) {
            return true;
        }
        return false;
    }
    bool Node::IsDouble() const {
        if (holds_alternative<int>(value_)) {
            return true;
        }
        if (holds_alternative<double>(value_)) {
            return true;
        }
        return false;
    }
    bool Node::IsPureDouble() const {
        if (holds_alternative<double>(value_)) {
            return true;
        }
        return false;
    }
    bool Node::IsBool() const {
        if (holds_alternative<bool>(value_)) {
            return true;
        }
        return false;
    }
    bool Node::IsString() const {
        if (holds_alternative<string>(value_)) {
            return true;
        }
        return false;
    }
    bool Node::IsNull() const {
        if (holds_alternative<nullptr_t>(value_)) {
            return true;
        }
        return false;
    }
    bool Node::IsArray() const {
        if (holds_alternative<Array>(value_)) {
            return true;
        }
        return false;
    }
    bool Node::IsMap() const {
        if (holds_alternative<Dict>(value_)) {
            return true;
        }
        return false;
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("Invalid type");
        }
        return get<Array>(value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw logic_error("Invalid type");
        }
        return get<Dict>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("Invalid type");
        }
        return get<int>(value_);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("Invalid type");
        }
        return get<string>(value_);
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return get<int>(value_) * 1.0;
        }
        if (!IsPureDouble()) {
            throw logic_error("Invalid type");
        }
        return get<double>(value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("Invalid type");
        }
        return get<bool>(value_);
    }

    const Node::Value& Node::GetValue() const {
        return value_;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintValue(nullptr_t, ostream& out) {
        out << "null";
    }
    void PrintValue(const Array& as_array, ostream& out) {
        out << "[";
        for (auto it = as_array.begin(); it < as_array.end(); ++it) {
            if (it + 1 == as_array.end()) {
                PrintNode(*it, out);
                break;
            }
            PrintNode(*it, out);
            out << ",";
        }
        out << "]";
    }
    void PrintValue(const Dict& as_map, ostream& out) {
        out << "{";
        int size = as_map.size();
        for (const auto [key, value] : as_map) {
            if (size == 1) {
                out << "\"" << key << "\"" << ":";
                PrintNode(value, out);
                break;
            }
            out << "\"" << key << "\"" << ":";
            PrintNode(value, out);
            out << ",";
            --size;
        }
        out << "}";
    }
    void PrintValue(bool as_bool, ostream& out) {
        if (as_bool) {
            out << "true"s;
            return;
        }
        out << "false"s;
    }
    void PrintValue(int as_int, ostream& out) {
        out << as_int;
    }
    void PrintValue(double as_double, ostream& out) {
        out << as_double;
    }
    void PrintValue(string as_string, ostream& out) {
        string result = as_string;
        int pos = 0;
        while (pos < static_cast<int>(result.size())) {
            if (result.find('\\', pos) != ::std::string::npos) {
                pos = result.find('\\', pos);
                result.replace(pos, 1, "\\\\");
                pos += 3;
            }
            else {
                break;
            }
        }
        pos = 0;
        while (pos < static_cast<int>(result.size())) {
            if (result.find('\"', pos) != ::std::string::npos) {
                pos = result.find('\"', pos);
                result.replace(pos, 1, "\\\"");
                pos += 2;
            }
            else {
                break;
            }
        }
        pos = 0;
        while (pos < static_cast<int>(result.size())) {
            if (result.find('\n', pos) != ::std::string::npos) {
                pos = result.find('\n', pos);
                result.replace(pos, 1, "\\n");
                pos += 2;
            }
            else {
                break;
            }
        }
        pos = 0;
        while (pos < static_cast<int>(result.size())) {
            if (result.find('\t', pos) != ::std::string::npos) {
                pos = result.find('\t', pos);
                result.replace(pos, 1, "\t");
                pos += 2;
            }
            else {
                break;
            }
        }
        pos = 0;
        while (pos < static_cast<int>(result.size())) {
            if (result.find('\r', pos) != ::std::string::npos) {
                pos = result.find('\r', pos);
                result.replace(pos, 1, "\\r");
                pos += 2;
            }
            else {
                break;
            }
        }
        out << "\"" << result << "\"";
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value) { PrintValue(value, out); },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

    bool operator==(const Node& l, const Node& r) noexcept {
        if (holds_alternative<int>(l.GetValue()) && holds_alternative<int>(r.GetValue())) {
            return l.AsInt() == r.AsInt();
        }
        if (holds_alternative<bool>(l.GetValue()) && holds_alternative<bool>(r.GetValue())) {
            return l.AsBool() == r.AsBool();
        }
        if (holds_alternative<double>(l.GetValue()) && holds_alternative<double>(r.GetValue())) {
            return l.AsDouble() == r.AsDouble();
        }
        if (holds_alternative<string>(l.GetValue()) && holds_alternative<string>(r.GetValue())) {
            return l.AsString() == r.AsString();
        }
        if (holds_alternative<Array>(l.GetValue()) && holds_alternative<Array>(r.GetValue())) {
            return l.AsArray() == r.AsArray();
        }
        if (holds_alternative<Dict>(l.GetValue()) && holds_alternative<Dict>(r.GetValue())) {
            return l.AsMap() == r.AsMap();
        }
        if (holds_alternative<nullptr_t>(l.GetValue()) && holds_alternative<nullptr_t>(r.GetValue())) {
            return true;
        }
        return false;
    }

    bool operator!=(const Node& l, const Node& r) noexcept {
        return !(l == r);
    }

    bool operator==(const Document& l, const Document& r) noexcept {
        return l.GetRoot() == r.GetRoot();
    }

    bool operator!=(const Document& l, const Document& r) noexcept {
        return !(l.GetRoot() == r.GetRoot());
    }

}  // namespace json
