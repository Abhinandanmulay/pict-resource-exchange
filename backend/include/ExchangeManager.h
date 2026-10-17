#ifndef EXCHANGE_MANAGER_H
#define EXCHANGE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include "Models.h"
#include "HashTable.h"
#include "LinkedList.h"
#include "Queue.h"
#include "Stack.h"
#include "FileStorage.h"

/**
 * @brief Core business logic controller for PICT Resource Exchange
 * Orchestrates Hash Tables, Linked Lists, Queues, and Stacks
 */
class ExchangeManager {
private:
    // Core in-memory Data Structures
    HashTable<std::string, ResourceItem> itemTable;       // O(1) Lookup: ItemId -> ResourceItem
    HashTable<std::string, Student> studentTable;         // O(1) Lookup: RollNo -> Student
    LinkedList<ResourceRequest> requestList;              // Dynamic list of student wanted requests
    Stack<ActionRecord> undoStack;                        // LIFO undo stack for reversible actions
    
    // Category indexing using Linked Lists: CategoryName -> List of ItemIds
    HashTable<std::string, LinkedList<std::string>> categoryIndex;

    FileStorage storage;

    void rebuildCategoryIndices();

public:
    explicit ExchangeManager(const std::string& dataDir = "data");

    // --- Resource Item Management ---
    std::vector<ResourceItem> getAllItems() const;
    ResourceItem* getItem(const std::string& itemId);
    std::vector<ResourceItem> searchAndFilter(const std::string& query,
                                             const std::string& category,
                                             const std::string& availability) const;

    bool listItem(const ResourceItem& item, std::string& outMessage);
    bool cancelLendedItem(const std::string& itemId, const std::string& ownerRollNo, std::string& outMessage);
    std::string borrowItem(const std::string& itemId, const std::string& studentRollNo);
    std::string buyItem(const std::string& itemId, const std::string& buyerRollNo);
    std::string returnItem(const std::string& itemId, int rating = 0, const std::string& comment = "", const std::string& reviewerRollNo = "", const std::string& reviewerName = "");
    bool addFeedback(const std::string& itemId, const Feedback& feedback, std::string& outMessage);

    // --- Student & Profile Management ---
    std::vector<Student> getAllStudents() const;
    Student* getStudent(const std::string& rollNo);
    bool registerStudent(const Student& student, std::string& outMessage);
    void recordItemView(const std::string& studentRollNo, const std::string& itemId);
    std::vector<ResourceItem> getRecentlyViewed(const std::string& studentRollNo);

    // --- Resource Requests Management ---
    std::vector<ResourceRequest> getAllRequests() const;
    bool createRequest(const ResourceRequest& req, std::string& outMessage);
    bool fulfillRequest(const std::string& reqId, std::string& outMessage);
    bool deleteRequest(const std::string& reqId, const std::string& requesterRollNo, std::string& outMessage);

    // --- Undo Operation (Stack LIFO) ---
    bool undoLastAction(std::string& outMessage);

    // Helper statistics
    std::string getSystemStatsJson() const;
};

#endif // EXCHANGE_MANAGER_H
