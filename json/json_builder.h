#pragma once

#include <optional>

#include "json.h"

namespace json {
    
    class ArrayItemContext;
    class DictItemContext;    
    class KeyItemContext;

    class Builder {
    public:
        Builder();
                
        Builder& Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();
        
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Node::Value& GetCurrentValue();
        //static Node BuildNode(Node::Value value);
        Builder& AddObject(Node::Value value, bool complex);
    };

    class BaseItemContext {
    public:                
        BaseItemContext(Builder& builder) : builder_(builder) {} 

        KeyItemContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();

    protected:
        Builder& builder_;
    };

    class ValuetemContext : public BaseItemContext {
    public:
        ValuetemContext(Builder& builder) : BaseItemContext(builder) {}

        Builder& Value(Node::Value value) = delete;
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;

    };
    
    class DictItemContext : public BaseItemContext {
    public:
        DictItemContext(Builder& builder) : BaseItemContext(builder) {}

        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;

    };
    
    class ArrayItemContext : public BaseItemContext {
    public:
        ArrayItemContext(Builder& builder) : BaseItemContext(builder) {}
        ArrayItemContext Value(Node::Value value) {
            return ArrayItemContext(builder_.Value(value));
        }
        KeyItemContext Key(std::string key) = delete;
        Builder& EndDict() = delete;

    };

    class KeyItemContext : public BaseItemContext {
    public:
        KeyItemContext(Builder& builder) : BaseItemContext(builder) {}
        KeyItemContext Key(std::string key) = delete;
        ValuetemContext Value(Node::Value value) {
            return ValuetemContext(builder_.Value(value));
        }
        Builder& EndDict() = delete;
        Builder& EndArray() = delete;

    };

} // namespace json
