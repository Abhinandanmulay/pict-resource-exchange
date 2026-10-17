#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <vector>
#include <functional>
#include <stdexcept>

/**
 * @brief Node for Singly Linked List
 */
template <typename T>
struct ListNode {
    T data;
    ListNode<T>* next;

    ListNode(const T& val) : data(val), next(nullptr) {}
};

/**
 * @brief Custom Singly Linked List implementation
 * Used for dynamic resource collections, category-wise catalog filtering,
 * and the student wanted/requests board.
 */
template <typename T>
class LinkedList {
private:
    ListNode<T>* head;
    ListNode<T>* tail;
    int count;

public:
    LinkedList() : head(nullptr), tail(nullptr), count(0) {}

    // Destructor to free all allocated memory
    ~LinkedList() {
        clear();
    }

    // Copy constructor
    LinkedList(const LinkedList<T>& other) : head(nullptr), tail(nullptr), count(0) {
        ListNode<T>* current = other.head;
        while (current != nullptr) {
            insertTail(current->data);
            current = current->next;
        }
    }

    // Copy assignment operator
    LinkedList<T>& operator=(const LinkedList<T>& other) {
        if (this != &other) {
            clear();
            ListNode<T>* current = other.head;
            while (current != nullptr) {
                insertTail(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    // Clear all elements
    void clear() {
        ListNode<T>* current = head;
        while (current != nullptr) {
            ListNode<T>* temp = current;
            current = current->next;
            delete temp;
        }
        head = nullptr;
        tail = nullptr;
        count = 0;
    }

    // Insert at front (O(1))
    void insertHead(const T& element) {
        ListNode<T>* newNode = new ListNode<T>(element);
        if (head == nullptr) {
            head = tail = newNode;
        } else {
            newNode->next = head;
            head = newNode;
        }
        count++;
    }

    // Insert at end (O(1))
    void insertTail(const T& element) {
        ListNode<T>* newNode = new ListNode<T>(element);
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        count++;
    }

    // Remove first occurrence of element (O(N))
    bool remove(const T& element) {
        if (head == nullptr) return false;

        // If head holds the element
        if (head->data == element) {
            ListNode<T>* temp = head;
            head = head->next;
            if (head == nullptr) tail = nullptr;
            delete temp;
            count--;
            return true;
        }

        // Traverse to find the element
        ListNode<T>* current = head;
        while (current->next != nullptr && !(current->next->data == element)) {
            current = current->next;
        }

        if (current->next != nullptr) {
            ListNode<T>* temp = current->next;
            current->next = temp->next;
            if (temp == tail) {
                tail = current;
            }
            delete temp;
            count--;
            return true;
        }

        return false;
    }

    // Check if element exists
    bool contains(const T& element) const {
        ListNode<T>* current = head;
        while (current != nullptr) {
            if (current->data == element) return true;
            current = current->next;
        }
        return false;
    }

    // Search and return pointer to node data using a predicate
    T* findIf(std::function<bool(const T&)> predicate) {
        ListNode<T>* current = head;
        while (current != nullptr) {
            if (predicate(current->data)) {
                return &(current->data);
            }
            current = current->next;
        }
        return nullptr;
    }

    // Remove first node matching predicate
    bool removeIf(std::function<bool(const T&)> predicate) {
        if (head == nullptr) return false;

        if (predicate(head->data)) {
            ListNode<T>* temp = head;
            head = head->next;
            if (head == nullptr) tail = nullptr;
            delete temp;
            count--;
            return true;
        }

        ListNode<T>* current = head;
        while (current->next != nullptr && !predicate(current->next->data)) {
            current = current->next;
        }

        if (current->next != nullptr) {
            ListNode<T>* temp = current->next;
            current->next = temp->next;
            if (temp == tail) {
                tail = current;
            }
            delete temp;
            count--;
            return true;
        }

        return false;
    }

    // Get number of elements
    int size() const {
        return count;
    }

    bool isEmpty() const {
        return count == 0;
    }

    // Export list contents as vector (useful for API serialization)
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        ListNode<T>* current = head;
        while (current != nullptr) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }
};

#endif // LINKED_LIST_H
