#ifndef STACK_H
#define STACK_H

#include <vector>
#include <stdexcept>

/**
 * @brief Node for Stack
 */
template <typename T>
struct StackNode {
    T data;
    StackNode<T>* next;

    StackNode(const T& val) : data(val), next(nullptr) {}
};

/**
 * @brief Custom LIFO Stack implementation
 * Used for "Recently Viewed Items" navigation and "Undo Last Action" history.
 */
template <typename T>
class Stack {
private:
    StackNode<T>* topNode;
    int count;

public:
    Stack() : topNode(nullptr), count(0) {}

    ~Stack() {
        clear();
    }

    Stack(const Stack<T>& other) : topNode(nullptr), count(0) {
        // To maintain order, extract other to vector and push in reverse
        std::vector<T> items = other.toVector(); // top to bottom
        for (int i = (int)items.size() - 1; i >= 0; --i) {
            push(items[i]);
        }
    }

    Stack<T>& operator=(const Stack<T>& other) {
        if (this != &other) {
            clear();
            std::vector<T> items = other.toVector();
            for (int i = (int)items.size() - 1; i >= 0; --i) {
                push(items[i]);
            }
        }
        return *this;
    }

    void clear() {
        while (!isEmpty()) {
            pop();
        }
    }

    // Push element onto top of stack (O(1))
    void push(const T& element) {
        StackNode<T>* newNode = new StackNode<T>(element);
        newNode->next = topNode;
        topNode = newNode;
        count++;
    }

    // Pop element from top of stack (O(1))
    T pop() {
        if (isEmpty()) {
            throw std::runtime_error("Cannot pop from an empty stack");
        }
        StackNode<T>* temp = topNode;
        T val = temp->data;
        topNode = topNode->next;
        delete temp;
        count--;
        return val;
    }

    // Peek top element (O(1))
    T peek() const {
        if (isEmpty()) {
            throw std::runtime_error("Stack is empty");
        }
        return topNode->data;
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    // Export stack contents from top (most recent) to bottom
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        StackNode<T>* current = topNode;
        while (current != nullptr) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }
};

#endif // STACK_H
