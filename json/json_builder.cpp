#include "json_builder.h"

namespace json {

    Builder::Builder() : root_() {
        nodes_stack_.emplace_back(&root_);
    }

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
        return AddObject(std::move(value), false);
    }

    Builder::DictItemContext Builder::StartDict() {
        return DictItemContext(AddObject(Dict(), true));
    }

    Builder::ArrayItemContext Builder::StartArray() {
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

    Builder::KeyItemContext Builder::BaseItemContext::Key(std::string key) {
        return KeyItemContext(builder_.Key(key));
    }

    Builder& Builder::BaseItemContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    Builder::DictItemContext Builder::BaseItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext Builder::BaseItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::BaseItemContext::EndDict() {
        return builder_.EndDict();
    }
    Builder& Builder::BaseItemContext::EndArray() {
        return builder_.EndArray();
    }

} // namespace json