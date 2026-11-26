#pragma once
#include <cstddef>
using namespace std;

template <typename T>
struct DefaultLess {
    bool operator()(const T& first, const T& second) const { return first < second; }
};

// Less(a, b) must return true when a has LOWER priority than b; top() is the highest.
template <typename T, typename Less = DefaultLess<T> >
class PriorityQueue {
    T* items;
    size_t length, capacity;
    Less comparer;

    void grow() {
        size_t newcapacity = capacity ? capacity * 2 : 8;
        T* newitems = new T[newcapacity];
        for (size_t i = 0; i < length; ++i) newitems[i] = items[i];
        delete[] items;
        items = newitems;
        capacity = newcapacity;
    }
    void swapitems(size_t first, size_t second) { T temporary = items[first]; items[first] = items[second]; items[second] = temporary; }
    void moveup(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (!comparer(items[parent], items[index])) break;
            swapitems(parent, index);
            index = parent;
        }
    }
    void movedown(size_t index) {
        for (;;) {
            size_t leftchild = 2 * index + 1, rightchild = leftchild + 1, largest = index;
            if (leftchild < length && comparer(items[largest], items[leftchild])) largest = leftchild;
            if (rightchild < length && comparer(items[largest], items[rightchild])) largest = rightchild;
            if (largest == index) break;
            swapitems(index, largest);
            index = largest;
        }
    }

public:
    PriorityQueue() : items(nullptr), length(0), capacity(0), comparer() {}
    ~PriorityQueue() { delete[] items; }
    PriorityQueue(const PriorityQueue&) = delete;
    PriorityQueue& operator=(const PriorityQueue&) = delete;

    void push(const T& value) {
        if (length == capacity) grow();
        items[length] = value;
        moveup(length);
        ++length;
    }
    bool pop(T& output) {
        if (length == 0) return false;
        output = items[0];
        items[0] = items[--length];
        if (length) movedown(0);
        return true;
    }
    const T* top() const { return length ? &items[0] : nullptr; }
    bool empty() const { return length == 0; }
    size_t size() const { return length; }
    void clear() { length = 0; }
};
