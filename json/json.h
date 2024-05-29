#pragma once

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict  = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;        
        
        template<typename T>
        Node(T value) : value_(std::move(value)) {}
        Node(std::nullptr_t) : value_(nullptr)   {}

        #define ALT std::holds_alternative
        bool IsInt()        const { return ALT<int>(value_); }
        bool IsDouble()     const { return ALT<double>(value_) || ALT<int>(value_); }
        bool IsPureDouble() const { return ALT<double>(value_); }
        bool IsBool()       const { return ALT<bool>(value_); }
        bool IsString()     const { return ALT<std::string>(value_); }
        bool IsNull()       const { return ALT<std::nullptr_t>(value_); }
        bool IsArray()      const { return ALT<Array>(value_); }
        bool IsMap()        const { return ALT<Dict>(value_); }

        int AsInt()                   const;
        bool AsBool()                 const;
        double AsDouble()             const;
        const std::string& AsString() const;
        const Array& AsArray()        const;
        const Dict& AsMap()           const;
        
        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

        const Value& GetValue() const { return value_; }

    private:
        Value value_;
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