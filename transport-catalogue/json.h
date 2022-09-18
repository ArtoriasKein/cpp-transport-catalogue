#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        explicit Node() = default;
        Node(int value);
        Node(double value);
        Node(bool value);
        Node(std::string value);
        Node(std::nullptr_t);
        Node(Array array);
        Node(Dict map);
        bool IsInt() const;
        int AsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        double AsDouble() const;
        bool IsBool() const;
        bool AsBool() const;
        bool IsString() const;
        const std::string& AsString() const;
        bool IsNull() const;
        bool IsArray() const;
        const Array& AsArray() const;
        bool IsMap() const;
        const Dict& AsMap() const;
        const Value& GetValue() const;
    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void PrintValue(std::nullptr_t, std::ostream& out);
    void PrintValue(const Array& as_array, std::ostream& out);
    void PrintValue(const Dict& as_map, std::ostream& out);
    void PrintValue(bool as_bool, std::ostream& out);
    void PrintValue(int as_int, std::ostream& out);
    void PrintValue(double as_double, std::ostream& out);
    void PrintValue(std::string as_string, std::ostream& out);
    void PrintNode(const Node& node, std::ostream& out);
    void Print(const Document& doc, std::ostream& output);

    bool operator==(const Node& l, const Node& r) noexcept;
    bool operator!=(const Node& l, const Node& r) noexcept;
    bool operator==(const Document& l, const Document& r) noexcept;
    bool operator!=(const Document& l, const Document& r) noexcept;

}  // namespace json
