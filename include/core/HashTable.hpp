#pragma once
#include <cstddef>
#include <string>
using namespace std;

template <typename V>
class HashTable {
    struct Node {
        string key;
        V value;
        Node* next;
        Node(const string& givenkey, const V& givenvalue, Node* following) : key(givenkey), value(givenvalue), next(following) {}
    };
    Node** buckets;
    size_t capacity, length;

    static size_t hashof(const string& text) {
        size_t hashvalue = 5381;
        for (size_t i = 0; i < text.size(); ++i) hashvalue = ((hashvalue << 5) + hashvalue) + (unsigned char)text[i];
        return hashvalue;
    }
    void rehash(size_t newcapacity) {
        Node** newbuckets = new Node*[newcapacity];
        for (size_t i = 0; i < newcapacity; ++i) newbuckets[i] = nullptr;
        for (size_t i = 0; i < capacity; ++i) {
            Node* node = buckets[i];
            while (node) {
                Node* following = node->next;
                size_t index = hashof(node->key) % newcapacity;
                node->next = newbuckets[index];
                newbuckets[index] = node;
                node = following;
            }
        }
        delete[] buckets;
        buckets = newbuckets;
        capacity = newcapacity;
    }

public:
    explicit HashTable(size_t startcapacity = 16)
        : buckets(nullptr), capacity(startcapacity ? startcapacity : 16), length(0) {
        buckets = new Node*[capacity];
        for (size_t i = 0; i < capacity; ++i) buckets[i] = nullptr;
    }
    ~HashTable() { clear(); delete[] buckets; }
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // Returns true if a new key was inserted, false if an existing key was updated.
    bool insert(const string& key, const V& value) {
        size_t index = hashof(key) % capacity;
        for (Node* node = buckets[index]; node; node = node->next)
            if (node->key == key) { node->value = value; return false; }
        buckets[index] = new Node(key, value, buckets[index]);
        ++length;
        if (loadfactor() > 0.75) rehash(capacity * 2);
        return true;
    }
    V* find(const string& key) {
        for (Node* node = buckets[hashof(key) % capacity]; node; node = node->next)
            if (node->key == key) return &node->value;
        return nullptr;
    }
    const V* find(const string& key) const {
        for (Node* node = buckets[hashof(key) % capacity]; node; node = node->next)
            if (node->key == key) return &node->value;
        return nullptr;
    }
    bool contains(const string& key) const { return find(key) != nullptr; }
    bool remove(const string& key) {
        size_t index = hashof(key) % capacity;
        Node* previous = nullptr;
        for (Node* node = buckets[index]; node; previous = node, node = node->next) {
            if (node->key == key) {
                if (previous) previous->next = node->next; else buckets[index] = node->next;
                delete node;
                --length;
                return true;
            }
        }
        return false;
    }
    size_t size() const { return length; }
    size_t bucketcount() const { return capacity; }
    double loadfactor() const { return (double)length / (double)capacity; }
    void clear() {
        for (size_t i = 0; i < capacity; ++i) {
            Node* node = buckets[i];
            while (node) { Node* following = node->next; delete node; node = following; }
            buckets[i] = nullptr;
        }
        length = 0;
    }
};
