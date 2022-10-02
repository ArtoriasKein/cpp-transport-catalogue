#include "json_builder.h"
#include <iostream>

using namespace std;

namespace json {

    Builder::DictItemContext& Builder::StartDict() {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw logic_error("Attempt to call method \"StartDict\" with a ready object"s);
        }
        AddNode(Dict());
        return *(new DictItemContext(*this));
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw logic_error("Attempt to call method \"EndDict\" with a ready object"s);
        }
        if (!nodes_stack_.back()->IsMap()) {
            throw logic_error("Attempt to call method \"EndDict\" in the context of another container"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder::ArrayItemContext& Builder::StartArray() {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw logic_error("Attempt to call method \"StartArray\" with a ready object"s);
        }
        AddNode(Array());
        return *(new ArrayItemContext(*this));
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw logic_error("Attempt to call method \"EndArray\" with a ready object"s);
        }
        if (!nodes_stack_.back()->IsArray()) {
            throw logic_error("Attempt to call method \"EndArray\" in the context of another container"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder::KeyItemContext& Builder::Key(string&& key) {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw logic_error("Attempt to call method \"Key\" with a ready object"s);
        }
        if (key_.has_value()) {
            throw logic_error("Attempt to call method \"Key\" immediately after another method \"Key\""s);
        }
        if (!nodes_stack_.back()->IsMap()) {
            throw logic_error("Attempt to call method \"Key\" outside of Dict"s);
        }

        if (nodes_stack_.back()->IsMap()) {
            Dict& dict = get<Dict>(nodes_stack_.back()->GetNonConstValue());
            dict[key];
            key_ = key;
        }
        return *(new KeyItemContext(*this));
    }

    Builder& Builder::Value(Node::Value&& value) {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw logic_error("Attempt to call method \"Value\" with a ready object"s);
        }
        if (!key_.has_value() && !nodes_stack_.empty() && !nodes_stack_.back()->IsArray()) {
            throw logic_error("Method \"Value\" is called outside of Array or Dict"s);
        }

        Node node;
        if (holds_alternative<string>(value)) {
            node = Node(get<string>(value));
        }
        else if (holds_alternative<Array>(value)) {
            node = Node(get<Array>(value));
        }
        else if (holds_alternative<Dict>(value)) {
            node = Node(get<Dict>(value));
        }
        else if (holds_alternative<int>(value)) {
            node = Node(get<int>(value));
        }
        else if (holds_alternative<double>(value)) {
            node = Node(get<double>(value));
        }
        else if (holds_alternative<bool>(value)) {
            node = Node(get<bool>(value));
        }

        AddNode(node);
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw logic_error("The described object is not ready"s);
        }
        if (root_.IsNull()) {
            throw logic_error("Attempt to build an empty JSON"s);
        }
        return root_;
    }

    void Builder::AddNode(Node node) {
        if (nodes_stack_.empty()) {
            root_ = node;
            nodes_stack_.push_back(&root_);
        }
        else if (nodes_stack_.back()->IsMap() && key_.has_value()) {
            Dict& dict = get<Dict>(nodes_stack_.back()->GetNonConstValue());
            dict[key_.value()] = node;
            nodes_stack_.push_back(&dict[key_.value()]);
            key_.reset();
        }
        else if (nodes_stack_.back()->IsArray()) {
            Array& arr = get<Array>(nodes_stack_.back()->GetNonConstValue());
            arr.push_back(node);
            nodes_stack_.push_back(&arr.back());
        }
    }

    Builder::KeyItemContext& Builder::DictItemContext::Key(std::string&& value) {
        return builder_.Key(std::move(value));
    }

    Builder& Builder::DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::KeyValueItemContext& Builder::KeyItemContext::Value(Node::Value&& value) {
        return *(new KeyValueItemContext(builder_.Value(std::move(value))));
    }

    Builder::DictItemContext& Builder::KeyItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext& Builder::KeyItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder::ArrayValueItemContext& Builder::ArrayItemContext::Value(Node::Value&& value) {
        return *(new ArrayValueItemContext(builder_.Value(std::move(value))));
    }

    Builder::DictItemContext& Builder::ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext& Builder::ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

    Builder::ArrayValueItemContext& Builder::ArrayValueItemContext::Value(Node::Value&& value) {
        return *(new ArrayValueItemContext(builder_.Value(std::move(value))));
    }

    Builder::DictItemContext& Builder::ArrayValueItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext& Builder::ArrayValueItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::ArrayValueItemContext::EndArray() {
        return builder_.EndArray();
    }

    Builder::KeyItemContext& Builder::KeyValueItemContext::Key(std::string&& value) {
        return builder_.Key(std::move(value));
    }

    Builder& Builder::KeyValueItemContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::KeyItemContext& Builder::BaseContext::Key(std::string&& value) {
        return builder_.Key(std::move(value));
    }
    Builder::ValueItemContext& Builder::BaseContext::Value(Node::Value&& value) {
        return *(new ValueItemContext(builder_.Value(std::move(value))));
    }
    Builder::ArrayItemContext& Builder::BaseContext::StartArray() {
        return builder_.StartArray();
    }
    Builder& Builder::BaseContext::EndArray() {
        return builder_.EndArray();
    }
    Builder::DictItemContext& Builder::BaseContext::StartDict() {
        return builder_.StartDict();
    }
    Builder& Builder::BaseContext::EndDict() {
        return builder_.EndDict();
    }

} // namespace json
