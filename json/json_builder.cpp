#include "json_builder.h"

namespace json {

    Builder::Builder() : root_() {
        nodes_stack_.emplace_back(&root_);
    }
    /*
    Node Builder::BuildNode(Node::Value value) {
        switch (value.index()) { // std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        case 1: return Node{ std::get<Array>(value) };
        case 2: return Node{ std::get<Dict>(value) };
        case 3: return Node{ std::get<bool>(value) };
        case 4: return Node{ std::get<int>(value) };
        case 5: return Node{ std::get<double>(value) };
        case 6: return Node{ std::get<std::string>(value) };
        }
        return Node{ std::get<std::nullptr_t>(value) };
    }
    */
    Node::Value& Builder::GetCurrentValue() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Attempt to change finalized JSON");
        }
        return nodes_stack_.back()->GetValue();
    }

    Builder& Builder::Key(std::string key) {
        Node::Value& current_value = GetCurrentValue();

        if (!std::holds_alternative<Dict>(current_value)) {
            throw std::logic_error("Key() outside a dict");
        }

        nodes_stack_.emplace_back(
            &std::get<Dict>(current_value)[std::move(key)]
        );
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        return AddObject(value, false);
    }

    DictItemContext Builder::StartDict() {
        return DictItemContext(AddObject(Dict(), true));
    }

    ArrayItemContext Builder::StartArray() {
        return ArrayItemContext(AddObject(Array(), true));
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("EndDict is not allowed here");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("EndArray is not allowed here");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Incomplete JSON structure");
        }
        return root_;
    }

    Builder& Builder::AddObject(Node::Value value, bool complex) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("JSON structure has been completed");
        }
        Node::Value& current_value = GetCurrentValue();

        if (nodes_stack_.back()->IsArray()) {
            Node& tmp = std::get<Array>(current_value).emplace_back(std::move(value));;
            //tmp.emplace_back(std::move(BuildNode(value)));
            if (complex) {
                nodes_stack_.emplace_back(&tmp);
            }
        }
        else {
            current_value = std::move(value);
            if (!complex) {
                nodes_stack_.pop_back();
            }
        }
        return *this;
    }

    // ItemContext

    KeyItemContext BaseItemContext::Key(std::string key) {
        return KeyItemContext(builder_.Key(key));
    }

    Builder& BaseItemContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    DictItemContext BaseItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext BaseItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& BaseItemContext::EndDict() {
        return builder_.EndDict();
    }
    Builder& BaseItemContext::EndArray() {
        return builder_.EndArray();
    }

} // namespace json