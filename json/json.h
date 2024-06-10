#pragma once

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final
        : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;
        using Value = variant;

        Node(Value value) : variant(std::move(value)) {}

#define ALT std::holds_alternative
        bool IsInt()        const { return ALT<int>(*this); }
        bool IsDouble()     const { return ALT<double>(*this) || ALT<int>(*this); }
        bool IsPureDouble() const { return ALT<double>(*this); }
        bool IsBool()       const { return ALT<bool>(*this); }
        bool IsString()     const { return ALT<std::string>(*this); }
        bool IsNull()       const { return ALT<std::nullptr_t>(*this); }
        bool IsArray()      const { return ALT<Array>(*this); }
        bool IsDict()        const { return ALT<Dict>(*this); }

        int AsInt()                   const;
        bool AsBool()                 const;
        double AsDouble()             const;
        const std::string& AsString() const;
        const Array& AsArray()        const;
        const Dict& AsDict()           const;

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

        const Value& GetValue() const { return *this; }
        Value& GetValue() { return *this; }
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;

    private:
        Node root_;
    };

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;
        void PrintIndent() const;
        PrintContext Indented() const;
    };

    Document Load(std::istream& input);
    void Print(const Document& doc, std::ostream& output);
    void PrintNode(const Node& node, const PrintContext& ctx);

}  // namespace json