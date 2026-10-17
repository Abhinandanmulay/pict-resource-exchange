#ifndef QUEUE_H
#define QUEUE_H

#include <vector>
#include <stdexcept>

/**
 * @brief Node for Queue
 */
template <typename T>
struct QueueNode {
    T data;
    QueueNode<T>* next;

    QueueNode(const T& val) : data(val), next(nullptr) {}
};

/**
 * @brief Custom FIFO Queue implementation
 * Used for resource waiting lists: when an item is already borrowed,
 * students join a FIFO waiting queue ensuring fair allocation upon return.
 */
template <typename T>
class Queue {
private:
    QueueNode<T>* frontNode;
    QueueNode<T>* rearNode;
    int count;

public:
    Queue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

    ~Queue() {
        clear();
    }

    Queue(const Queue<T>& other) : frontNode(nullptr), rearNode(nullptr), count(0) {
        QueueNode<T>* current = other.frontNode;
        while (current != nullptr) {
            enqueue(current->data);
            current = current->next;
        }
    }

    Queue<T>& operator=(const Queue<T>& other) {
        if (this != &other) {
            clear();
            QueueNode<T>* current = other.frontNode;
            while (current != nullptr) {
                enqueue(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    void clear() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    // Enqueue an element at the rear (O(1))
    void enqueue(const T& element) {
        QueueNode<T>* newNode = new QueueNode<T>(element);
        if (rearNode == nullptr) {
            frontNode = rearNode = newNode;
        } else {
            rearNode->next = newNode;
            rearNode = newNode;
        }
        count++;
    }

    // Dequeue an element from the front (O(1))
    T dequeue() {
        if (isEmpty()) {
            throw std::runtime_error("Cannot dequeue from an empty queue");
        }
        QueueNode<T>* temp = frontNode;
        T val = temp->data;
        frontNode = frontNode->next;
        if (frontNode == nullptr) {
            rearNode = nullptr;
        }
        delete temp;
        count--;
        return val;
    }

    // Peek front element (O(1))
    T front() const {
        if (isEmpty()) {
            throw std::runtime_error("Queue is empty");
        }
        return frontNode->data;
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    // Convert queue contents to vector (from front to rear) without modifying the queue
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        QueueNode<T>* current = frontNode;
        while (current != nullptr) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }
};

#endif // QUEUE_H
