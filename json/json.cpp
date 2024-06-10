#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (input.fail() && input.eof()) {
                throw ParsingError("Array parsing error: missing closing ']'");
            }
            return Node(move(result));
        }

        Node LoadNumber(istream& input) {
            string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream");
                }
                };

            auto read_digits = [&input, read_char] {
                if (!isdigit(input.peek())) {
                    throw ParsingError("A digit is expected");
                }
                while (isdigit(input.peek())) {
                    read_char();
                }
                };

            if (input.peek() == '-') {
                read_char();
            }
            if (input.peek() == '0') {
                read_char();
            }
            else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

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
                    return Node(stoi(parsed_num));
                }
                return Node(stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert " + parsed_num + " to number");
            }
        }

        Node LoadString(istream& input) {
            using namespace literals;

            auto it = istreambuf_iterator<char>(input);
            auto end = istreambuf_iterator<char>();
            string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
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
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }
            return Node(move(s));
        }

        Node LoadDict(istream& input) {
            Dict result;
            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                string key = get<string>(LoadString(input).GetValue());
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (input.fail() && input.eof()) {
                throw ParsingError("Dict parsing error: missing closing '}'");
            }
            return Node(move(result));
        }

        Node LoadBoolOrNull(istream& input) {
            string value;
            while (isalpha(input.peek())) {
                value += input.get();
            }

            if (value == "true") {
                return Node(true);
            }
            else if (value == "false") {
                return Node(false);
            }
            else if (value == "null") {
                return Node(nullptr);
            }
            else {
                throw ParsingError("Invalid value");
            }
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
            else if (isdigit(c) || c == '-' || c == '.') {
                input.putback(c);
                return LoadNumber(input);
            }
            else if (isalpha(c)) {
                input.putback(c);
                return LoadBoolOrNull(input);
            }
            else {
                throw ParsingError("Invalid character");
            }
        }

    }  // namespace

    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("Not an int");
        }
        return get<int>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("Not a bool");
        }
        return get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return get<int>(*this);
        }
        else if (IsPureDouble()) {
            return get<double>(*this);
        }
        else {
            throw logic_error("Not a double");
        }
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("Not a string");
        }
        return get<string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("Not an Array");
        }
        return get<Array>(*this);
    }

    const Dict& Node::AsDict() const {
        if (!IsDict()) {
            throw logic_error("Not a Dict");
        }
        return get<Dict>(*this);
    }

    bool Node::operator==(const Node& other) const {
        return GetValue() == other.GetValue();
    }

    bool Node::operator!=(const Node& other) const {
        return !(*this == other);
    }

    bool Document::operator==(const Document& other) const {
        return GetRoot() == other.GetRoot();
    }

    bool Document::operator!=(const Document& other) const {
        return !(*this == other);
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

    void PrintValue(const bool& value, const PrintContext& ctx) {
        ctx.out << (value ? "true" : "false");
    }

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintValue(nullptr_t, const PrintContext& ctx) {
        ctx.out << "null";
    }

    void PrintValue(const string& value, const PrintContext& ctx) {
        ctx.out << '"';
        for (const char c : value) {
            switch (c) {
            case '"':
                ctx.out << "\\\"";
                break;
            case '\\':
                ctx.out << "\\\\";
                break;
            case '\n':
                ctx.out << "\\n";
                break;
            case '\r':
                ctx.out << "\\r";
                break;
            case '\t':
                ctx.out << "\\t";
                break;
            default:
                ctx.out << c;
                break;
            }
        }
        ctx.out << '"';
    }

    void PrintValue(const Array& array, const PrintContext& ctx) {
        ctx.out << "[\n";
        auto new_ctx = ctx.Indented();
        bool first = true;
        for (const auto& elem : array) {
            if (!first) {
                ctx.out << ",\n";
            }
            first = false;
            new_ctx.PrintIndent();
            PrintNode(elem, new_ctx);
        }
        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << ']';
    }

    void PrintValue(const Dict& dict, const PrintContext& ctx) {
        ctx.out << "{\n";
        auto new_ctx = ctx.Indented();
        bool first = true;
        for (const auto& [key, value] : dict) {
            if (!first) {
                ctx.out << ",\n";
            }
            first = false;
            new_ctx.PrintIndent();
            PrintValue(key, new_ctx);
            ctx.out << ": ";
            PrintNode(value, new_ctx);
        }
        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << '}';
    }


    void PrintContext::PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext PrintContext::Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        visit(
            [&ctx](const auto& value) {
                PrintValue(value, ctx);
            },
            node.GetValue());
    }

    void Print(const Document& doc, ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }


}  // namespace json