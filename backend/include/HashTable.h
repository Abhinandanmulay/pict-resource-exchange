#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>

/**
 * @brief Node for Hash Table Chaining
 */
template <typename K, typename V>
struct HashNode {
    K key;
    V value;
    HashNode<K, V>* next;

    HashNode(const K& k, const V& val) : key(k), value(val), next(nullptr) {}
};

/**
 * @brief Default Hash Functor
 * Specializations for string and integral types
 */
template <typename K>
struct HashFunction {
    size_t operator()(const K& key) const {
        return std::hash<K>{}(key);
    }
};

// Explicit DJB2 String Hash Specialization
template <>
struct HashFunction<std::string> {
    size_t operator()(const std::string& str) const {
        // DJB2 Hash Algorithm: hash * 33 + c
        size_t hash = 5381;
        for (char c : str) {
            hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
        }
        return hash;
    }
};

/**
 * @brief Custom Hash Table with Separate Chaining for Collision Resolution
 * Provides average O(1) key lookups, insertions, and deletions.
 */
template <typename K, typename V, typename Hasher = HashFunction<K>>
class HashTable {
private:
    HashNode<K, V>** buckets;
    int capacity;
    int elementCount;
    Hasher hasher;

    size_t getBucketIndex(const K& key) const {
        return hasher(key) % capacity;
    }

    void rehash(int newCapacity) {
        HashNode<K, V>** newBuckets = new HashNode<K, V>*[newCapacity]();
        
        for (int i = 0; i < capacity; ++i) {
            HashNode<K, V>* current = buckets[i];
            while (current != nullptr) {
                HashNode<K, V>* nextNode = current->next;
                size_t newIndex = hasher(current->key) % newCapacity;
                current->next = newBuckets[newIndex];
                newBuckets[newIndex] = current;
                current = nextNode;
            }
        }

        delete[] buckets;
        buckets = newBuckets;
        capacity = newCapacity;
    }

public:
    explicit HashTable(int initialCapacity = 101) 
        : capacity(initialCapacity), elementCount(0) {
        buckets = new HashNode<K, V>*[capacity]();
    }

    ~HashTable() {
        clear();
        delete[] buckets;
    }

    // Copy Constructor
    HashTable(const HashTable<K, V, Hasher>& other) 
        : capacity(other.capacity), elementCount(0) {
        buckets = new HashNode<K, V>*[capacity]();
        for (int i = 0; i < other.capacity; ++i) {
            HashNode<K, V>* current = other.buckets[i];
            while (current != nullptr) {
                insert(current->key, current->value);
                current = current->next;
            }
        }
    }

    // Copy Assignment
    HashTable<K, V, Hasher>& operator=(const HashTable<K, V, Hasher>& other) {
        if (this != &other) {
            clear();
            delete[] buckets;
            capacity = other.capacity;
            elementCount = 0;
            buckets = new HashNode<K, V>*[capacity]();
            for (int i = 0; i < other.capacity; ++i) {
                HashNode<K, V>* current = other.buckets[i];
                while (current != nullptr) {
                    insert(current->key, current->value);
                    current = current->next;
                }
            }
        }
        return *this;
    }

    void clear() {
        for (int i = 0; i < capacity; ++i) {
            HashNode<K, V>* current = buckets[i];
            while (current != nullptr) {
                HashNode<K, V>* temp = current;
                current = current->next;
                delete temp;
            }
            buckets[i] = nullptr;
        }
        elementCount = 0;
    }

    // Insert or update key-value pair (O(1) average)
    void insert(const K& key, const V& value) {
        // Load factor check: if elements / capacity > 0.75, double capacity
        if ((float)(elementCount + 1) / capacity > 0.75f) {
            rehash(capacity * 2 + 1);
        }

        size_t index = getBucketIndex(key);
        HashNode<K, V>* current = buckets[index];

        // Check if key already exists, update value if found
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        // Insert new node at bucket head
        HashNode<K, V>* newNode = new HashNode<K, V>(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
        elementCount++;
    }

    // Search key and retrieve value (O(1) average)
    bool search(const K& key, V& outValue) const {
        size_t index = getBucketIndex(key);
        HashNode<K, V>* current = buckets[index];
        while (current != nullptr) {
            if (current->key == key) {
                outValue = current->value;
                return true;
            }
            current = current->next;
        }
        return false;
    }

    // Get pointer to value for in-place modifications
    V* find(const K& key) {
        size_t index = getBucketIndex(key);
        HashNode<K, V>* current = buckets[index];
        while (current != nullptr) {
            if (current->key == key) {
                return &(current->value);
            }
            current = current->next;
        }
        return nullptr;
    }

    const V* find(const K& key) const {
        size_t index = getBucketIndex(key);
        HashNode<K, V>* current = buckets[index];
        while (current != nullptr) {
            if (current->key == key) {
                return &(current->value);
            }
            current = current->next;
        }
        return nullptr;
    }

    // Check if key exists
    bool contains(const K& key) const {
        size_t index = getBucketIndex(key);
        HashNode<K, V>* current = buckets[index];
        while (current != nullptr) {
            if (current->key == key) return true;
            current = current->next;
        }
        return false;
    }

    // Remove key from hash table (O(1) average)
    bool remove(const K& key) {
        size_t index = getBucketIndex(key);
        HashNode<K, V>* current = buckets[index];
        HashNode<K, V>* prev = nullptr;

        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    buckets[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                elementCount--;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    int size() const {
        return elementCount;
    }

    int getCapacity() const {
        return capacity;
    }

    bool isEmpty() const {
        return elementCount == 0;
    }

    // Retrieve all values in the hash table
    std::vector<V> getAllValues() const {
        std::vector<V> values;
        values.reserve(elementCount);
        for (int i = 0; i < capacity; ++i) {
            HashNode<K, V>* current = buckets[i];
            while (current != nullptr) {
                values.push_back(current->value);
                current = current->next;
            }
        }
        return values;
    }

    // Retrieve all keys in the hash table
    std::vector<K> getAllKeys() const {
        std::vector<K> keys;
        keys.reserve(elementCount);
        for (int i = 0; i < capacity; ++i) {
            HashNode<K, V>* current = buckets[i];
            while (current != nullptr) {
                keys.push_back(current->key);
                current = current->next;
            }
        }
        return keys;
    }
};

#endif // HASH_TABLE_H
