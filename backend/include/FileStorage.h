#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include <string>
#include <vector>
#include "Models.h"
#include "HashTable.h"
#include "LinkedList.h"

/**
 * @brief File Storage Manager for persistence
 * Reads and writes JSON data files for items, students, and requests
 */
class FileStorage {
private:
    std::string dataDir;

public:
    explicit FileStorage(const std::string& directory = "data");

    // Load functions
    bool loadItems(HashTable<std::string, ResourceItem>& itemTable);
    bool loadStudents(HashTable<std::string, Student>& studentTable);
    bool loadRequests(LinkedList<ResourceRequest>& requestList);

    // Save functions
    bool saveItems(const HashTable<std::string, ResourceItem>& itemTable);
    bool saveStudents(const HashTable<std::string, Student>& studentTable);
    bool saveRequests(const LinkedList<ResourceRequest>& requestList);

    // Seed defaults if files don't exist
    void initializeDefaultData(HashTable<std::string, ResourceItem>& itemTable,
                               HashTable<std::string, Student>& studentTable,
                               LinkedList<ResourceRequest>& requestList);
};

#endif // FILE_STORAGE_H
