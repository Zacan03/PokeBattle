#pragma once
#include <cstddef>
using namespace std;

template <typename T>
class Stack {
    struct Node {
        T data;
        Node* next;
        Node(const T& value, Node* following) : data(value), next(following) {}
    };
    Node* head;
    size_t length;

public:
    Stack() : head(nullptr), length(0) {}
    ~Stack() { clear(); }
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    void push(const T& value) {
        head = new Node(value, head);
        ++length;
    }
    bool pop(T& output) {
        if (!head) return false;
        Node* node = head;
        output = node->data;
        head = node->next;
        delete node;
        --length;
        return true;
    }
    T* top() { return head ? &head->data : nullptr; }
    const T* top() const { return head ? &head->data : nullptr; }
    bool empty() const { return head == nullptr; }
    size_t size() const { return length; }
    void clear() {
        while (head) {
            Node* node = head;
            head = node->next;
            delete node;
        }
        length = 0;
    }
};
