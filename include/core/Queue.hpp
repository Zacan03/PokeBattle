#pragma once
#include <cstddef>
using namespace std;

template <typename T>
class Queue {
    struct Node {
        T data;
        Node* next;
        explicit Node(const T& value) : data(value), next(nullptr) {}
    };
    Node* head;
    Node* tail;
    size_t length;

    void copyfrom(const Queue& other) {
        for (Node* node = other.head; node; node = node->next) enqueue(node->data);
    }

public:
    Queue() : head(nullptr), tail(nullptr), length(0) {}
    Queue(const Queue& other) : head(nullptr), tail(nullptr), length(0) { copyfrom(other); }
    Queue& operator=(const Queue& other) {
        if (this != &other) { clear(); copyfrom(other); }
        return *this;
    }
    ~Queue() { clear(); }

    void enqueue(const T& value) {
        Node* node = new Node(value);
        if (tail) tail->next = node; else head = node;
        tail = node;
        ++length;
    }
    bool dequeue(T& output) {
        if (!head) return false;
        Node* node = head;
        output = node->data;
        head = node->next;
        if (!head) tail = nullptr;
        delete node;
        --length;
        return true;
    }
    const T* front() const { return head ? &head->data : nullptr; }
    bool empty() const { return head == nullptr; }
    size_t size() const { return length; }
    template <typename Visitor>
    void visitall(Visitor& visitor) const { for (Node* node = head; node; node = node->next) visitor(node->data); }
    void clear() {
        while (head) {
            Node* node = head;
            head = node->next;
            delete node;
        }
        tail = nullptr;
        length = 0;
    }
};
