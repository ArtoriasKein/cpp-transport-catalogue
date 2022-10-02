#pragma once
#include "json.h"
#include <optional>

namespace json {

    class Builder {
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::optional<std::string> key_;
        class BaseContext;
        class KeyValueItemContext;
        class KeyItemContext;
        class DictItemContext;
        class ValueItemContext;
        class ArrayItemContext;
        class ArrayValueItemContext;
        class BaseContext {
        public:

            BaseContext(Builder& builder)
                : builder_(builder)
            {}
            KeyItemContext& Key(std::string&& value);
            ValueItemContext& Value(Node::Value&& value);
            ArrayItemContext& StartArray();
            Builder& EndArray();
            DictItemContext& StartDict();
            Builder& EndDict();
        protected:
            Builder& builder_;
        };

        class KeyValueItemContext : public BaseContext {
        public:
            KeyValueItemContext(Builder& builder)
                : BaseContext(builder)
            {
            }
            KeyItemContext& Key(std::string&& value);
            ValueItemContext& Value(Node::Value&& value) = delete;
            DictItemContext& StartDict() = delete;
            Builder& EndDict();
            ArrayItemContext& StartArray() = delete;
            Builder& EndArray() = delete;
        };
        class KeyItemContext : public BaseContext {
        public:
            KeyItemContext(Builder& builder)
                : BaseContext(builder)
            {
            }
            KeyItemContext& Key(std::string&& value) = delete;
            KeyValueItemContext& Value(Node::Value&& value);
            DictItemContext& StartDict();
            Builder& EndDict() = delete;
            ArrayItemContext& StartArray();
            Builder& EndArray() = delete;
        };

        class DictItemContext : public BaseContext {
        public:
            DictItemContext(Builder& builder)
                : BaseContext(builder)
            {
            }
            KeyItemContext& Key(std::string&& value);
            ValueItemContext& Value(Node::Value&& value) = delete;
            DictItemContext& StartDict() = delete;
            Builder& EndDict();
            ArrayItemContext& StartArray() = delete;
            Builder& EndArray() = delete;
        };
        class ValueItemContext : public BaseContext {
        public:
            ValueItemContext(Builder& builder)
                : BaseContext(builder)
            {
            }
            KeyItemContext& Key(std::string&& value) = delete;
            ValueItemContext& Value(Node::Value&& value);
            DictItemContext& StartDict();
            Builder& EndDict() = delete;
            ArrayItemContext& StartArray();
            Builder& EndArray() = delete;
        };
        class ArrayItemContext : public BaseContext {
        public:
            ArrayItemContext(Builder& builder)
                : BaseContext(builder)
            {
            }
            KeyItemContext& Key(std::string&& value) = delete;
            ArrayValueItemContext& Value(Node::Value&& value);
            DictItemContext& StartDict();
            Builder& EndDict() = delete;
            ArrayItemContext& StartArray();
            Builder& EndArray();
        };
        class ArrayValueItemContext : public ArrayItemContext {
        public:
            ArrayValueItemContext(Builder& builder)
                : ArrayItemContext(builder)
            {
            }
            KeyItemContext& Key(std::string&& value) = delete;
            ArrayValueItemContext& Value(Node::Value&& value);
            DictItemContext& StartDict();
            Builder& EndDict() = delete;
            ArrayItemContext& StartArray();
            Builder& EndArray();
        };

    public:
        Node Build();
        KeyItemContext& Key(std::string&& value);
        Builder& Value(Node::Value&& value);
        DictItemContext& StartDict();
        Builder& EndDict();
        ArrayItemContext& StartArray();
        Builder& EndArray();
        void AddNode(Node node);
    };

}
