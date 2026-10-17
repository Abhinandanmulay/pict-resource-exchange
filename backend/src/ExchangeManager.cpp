#include "ExchangeManager.h"
#include <algorithm>
#include <sstream>
#include <ctime>
#include <iostream>

static std::string getCurrentTimestamp() {
    time_t now = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buf);
}

static std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

ExchangeManager::ExchangeManager(const std::string& dataDir) : storage(dataDir) {
    bool loadedItems = storage.loadItems(itemTable);
    bool loadedStudents = storage.loadStudents(studentTable);
    bool loadedRequests = storage.loadRequests(requestList);

    // If data files don't exist yet, seed initial realistic PICT data
    if (!loadedItems || itemTable.isEmpty() || !loadedStudents || studentTable.isEmpty()) {
        std::cout << "[ExchangeManager] Seeding default PICT store data...\n";
        storage.initializeDefaultData(itemTable, studentTable, requestList);
    }

    rebuildCategoryIndices();
    std::cout << "[ExchangeManager] Initialized with " << itemTable.size() 
              << " items, " << studentTable.size() << " students, and " 
              << requestList.size() << " requests.\n";
}

void ExchangeManager::rebuildCategoryIndices() {
    categoryIndex.clear();
    std::vector<ResourceItem> items = itemTable.getAllValues();
    for (const auto& item : items) {
        LinkedList<std::string>* list = categoryIndex.find(item.category);
        if (list == nullptr) {
            LinkedList<std::string> newList;
            newList.insertTail(item.id);
            categoryIndex.insert(item.category, newList);
        } else {
            list->insertTail(item.id);
        }
    }
}

std::vector<ResourceItem> ExchangeManager::getAllItems() const {
    return itemTable.getAllValues();
}

ResourceItem* ExchangeManager::getItem(const std::string& itemId) {
    return itemTable.find(itemId);
}

std::vector<ResourceItem> ExchangeManager::searchAndFilter(const std::string& query,
                                                          const std::string& category,
                                                          const std::string& availability) const {
    std::vector<ResourceItem> result;
    std::string lowerQuery = toLower(query);

    auto matchesFilter = [&](const ResourceItem& item) -> bool {
        if (!category.empty() && category != "All" && item.category != category) {
            return false;
        }

        if (availability == "available" && (!item.isAvailable || item.isSold)) return false;
        if (availability == "borrowed" && (item.isAvailable || item.isSold)) return false;
        if (availability == "sold" && !item.isSold) return false;
        if (availability == "sale" && (item.listingType != "SELL" && item.listingType != "BOTH")) return false;
        if (availability == "rent" && (item.listingType != "RENT" && item.listingType != "BOTH")) return false;

        if (!lowerQuery.empty()) {
            std::string lowerTitle = toLower(item.title);
            std::string lowerDesc = toLower(item.description);
            if (lowerTitle.find(lowerQuery) == std::string::npos &&
                lowerDesc.find(lowerQuery) == std::string::npos) {
                return false;
            }
        }
        return true;
    };

    // If category is provided and exists in our category linked list index,
    // we can optimize traversal by only checking items in that category list
    if (!category.empty() && category != "All") {
        const LinkedList<std::string>* catList = categoryIndex.find(category);
        if (catList != nullptr) {
            std::vector<std::string> itemIds = catList->toVector();
            for (const auto& id : itemIds) {
                const ResourceItem* item = itemTable.find(id);
                if (item != nullptr && matchesFilter(*item)) {
                    result.push_back(*item);
                }
            }
            return result;
        }
    }

    // Otherwise, traverse all items from the Hash Table
    std::vector<ResourceItem> allItems = itemTable.getAllValues();
    for (const auto& item : allItems) {
        if (matchesFilter(item)) {
            result.push_back(item);
        }
    }
    return result;
}

bool ExchangeManager::listItem(const ResourceItem& item, std::string& outMessage) {
    if (item.id.empty()) {
        outMessage = "Error: Item ID cannot be empty";
        return false;
    }
    if (itemTable.contains(item.id)) {
        outMessage = "Error: Item with ID '" + item.id + "' already exists";
        return false;
    }

    // Verify owner student exists or create fallback profile
    Student* owner = studentTable.find(item.ownerRollNo);
    if (owner == nullptr) {
        Student newStudent(item.ownerRollNo, item.ownerName, "General", 2, "", item.ownerContact);
        studentTable.insert(item.ownerRollNo, newStudent);
        owner = studentTable.find(item.ownerRollNo);
    }

    if (owner != nullptr) {
        owner->lentItemIds.insertTail(item.id);
    }

    itemTable.insert(item.id, item);

    // Update category linked list
    LinkedList<std::string>* catList = categoryIndex.find(item.category);
    if (catList == nullptr) {
        LinkedList<std::string> newList;
        newList.insertTail(item.id);
        categoryIndex.insert(item.category, newList);
    } else {
        catList->insertTail(item.id);
    }

    // Record action onto Undo Stack (LIFO)
    ActionRecord act;
    act.actionType = "LIST_ITEM";
    act.entityId = item.id;
    act.studentRollNo = item.ownerRollNo;
    act.timestamp = getCurrentTimestamp();
    undoStack.push(act);

    storage.saveItems(itemTable);
    storage.saveStudents(studentTable);

    outMessage = "Item '" + item.title + "' listed successfully!";
    return true;
}

bool ExchangeManager::cancelLendedItem(const std::string& itemId, const std::string& ownerRollNo, std::string& outMessage) {
    ResourceItem* item = itemTable.find(itemId);
    if (item == nullptr) {
        outMessage = "Item not found in catalog";
        return false;
    }

    if (!ownerRollNo.empty() && item->ownerRollNo != ownerRollNo) {
        outMessage = "Unauthorized: Only the lender (Roll: " + item->ownerRollNo + ") can cancel this listing";
        return false;
    }

    if (item->isSold) {
        std::string buyer = item->buyerName.empty() ? item->buyerRollNo : item->buyerName;
        outMessage = "Cannot cancel listing: Item has already been sold to " + buyer + ".";
        return false;
    }

    if (!item->isAvailable) {
        std::string borrower = item->currentBorrowerName.empty() ? item->currentBorrowerRollNo : item->currentBorrowerName;
        outMessage = "Cannot cancel listing: Item is currently borrowed by " + borrower + ". Please wait for it to be returned.";
        return false;
    }

    std::string title = item->title;
    std::string category = item->category;
    std::string actualOwnerRoll = item->ownerRollNo;

    // 1. Remove from category linked list
    LinkedList<std::string>* catList = categoryIndex.find(category);
    if (catList != nullptr) {
        catList->remove(itemId);
    }

    // 2. Remove from student's lent list
    Student* student = studentTable.find(actualOwnerRoll);
    if (student != nullptr) {
        student->lentItemIds.remove(itemId);
    }

    // 3. Remove from item hash table
    itemTable.remove(itemId);

    // 4. Persist changes to disk storage
    storage.saveItems(itemTable);
    storage.saveStudents(studentTable);

    outMessage = "Listing for '" + title + "' (ID: " + itemId + ") was successfully cancelled.";
    return true;
}

std::string ExchangeManager::buyItem(const std::string& itemId, const std::string& buyerRollNo) {
    ResourceItem* item = itemTable.find(itemId);
    if (item == nullptr) {
        return "{\"status\":\"ERROR\",\"message\":\"Resource item not found in catalog\"}";
    }

    Student* buyer = studentTable.find(buyerRollNo);
    if (buyer == nullptr) {
        return "{\"status\":\"ERROR\",\"message\":\"Student roll number not registered\"}";
    }

    if (item->ownerRollNo == buyerRollNo) {
        return "{\"status\":\"ERROR\",\"message\":\"You cannot purchase your own listed item\"}";
    }

    if (item->isSold) {
        return "{\"status\":\"ERROR\",\"message\":\"This item has already been sold\"}";
    }

    if (!item->isAvailable) {
        return "{\"status\":\"ERROR\",\"message\":\"Item is currently unavailable (borrowed by a peer)\"}";
    }

    if (item->listingType != "SELL" && item->listingType != "BOTH") {
        return "{\"status\":\"ERROR\",\"message\":\"This item is listed for borrowing only and is not for sale\"}";
    }

    // Process Purchase
    item->isAvailable = false;
    item->isSold = true;
    item->buyerRollNo = buyer->rollNo;
    item->buyerName = buyer->name;
    item->soldTimestamp = getCurrentTimestamp();

    buyer->purchasedItemIds.insertTail(itemId);

    // Record action onto Undo Stack (LIFO)
    ActionRecord act;
    act.actionType = "BUY_ITEM";
    act.entityId = itemId;
    act.studentRollNo = buyerRollNo;
    act.timestamp = getCurrentTimestamp();
    undoStack.push(act);

    storage.saveItems(itemTable);
    storage.saveStudents(studentTable);

    std::ostringstream ss;
    ss << "{\n"
       << "  \"status\": \"BOUGHT\",\n"
       << "  \"message\": \"Purchase successful! You have reserved '" << item->title << "' for ₹" << (long long)item->sellPrice << ".\",\n"
       << "  \"itemId\": \"" << itemId << "\",\n"
       << "  \"title\": \"" << item->title << "\",\n"
       << "  \"sellPrice\": " << item->sellPrice << ",\n"
       << "  \"buyerRollNo\": \"" << buyer->rollNo << "\",\n"
       << "  \"buyerName\": \"" << buyer->name << "\",\n"
       << "  \"ownerRollNo\": \"" << item->ownerRollNo << "\",\n"
       << "  \"ownerName\": \"" << item->ownerName << "\",\n"
       << "  \"ownerContact\": \"" << item->ownerContact << "\"\n"
       << "}";
    return ss.str();
}

std::string ExchangeManager::borrowItem(const std::string& itemId, const std::string& studentRollNo) {
    ResourceItem* item = itemTable.find(itemId);
    if (item == nullptr) {
        return "{\"status\":\"ERROR\",\"message\":\"Resource item not found\"}";
    }

    Student* student = studentTable.find(studentRollNo);
    if (student == nullptr) {
        return "{\"status\":\"ERROR\",\"message\":\"Student roll number not registered\"}";
    }

    if (item->ownerRollNo == studentRollNo) {
        return "{\"status\":\"ERROR\",\"message\":\"You are the lender/owner of this item\"}";
    }

    if (item->currentBorrowerRollNo == studentRollNo) {
        return "{\"status\":\"ERROR\",\"message\":\"You already have borrowed this item\"}";
    }

    // If available: borrow immediately!
    if (item->isAvailable) {
        item->isAvailable = false;
        item->currentBorrowerRollNo = student->rollNo;
        item->currentBorrowerName = student->name;
        student->borrowedItemIds.insertTail(itemId);

        storage.saveItems(itemTable);
        storage.saveStudents(studentTable);

        std::ostringstream ss;
        ss << "{\n"
           << "  \"status\": \"BORROWED\",\n"
           << "  \"message\": \"Successfully borrowed '" << item->title << "'!\",\n"
           << "  \"itemId\": \"" << itemId << "\",\n"
           << "  \"borrower\": \"" << student->name << "\"\n"
           << "}";
        return ss.str();
    } else {
        // Resource is currently unavailable -> Enqueue into FIFO Waitlist Queue!
        std::vector<std::string> currentWaitlist = item->waitlist.toVector();
        for (const auto& roll : currentWaitlist) {
            if (roll == studentRollNo) {
                return "{\"status\":\"ERROR\",\"message\":\"You are already in the waiting list for this resource\"}";
            }
        }

        item->waitlist.enqueue(studentRollNo);
        storage.saveItems(itemTable);

        std::ostringstream ss;
        ss << "{\n"
           << "  \"status\": \"WAITLISTED\",\n"
           << "  \"message\": \"Resource is currently borrowed. Added to FIFO Waiting Queue!\",\n"
           << "  \"itemId\": \"" << itemId << "\",\n"
           << "  \"queuePosition\": " << item->waitlist.size() << ",\n"
           << "  \"studentRollNo\": \"" << studentRollNo << "\"\n"
           << "}";
        return ss.str();
    }
}

std::string ExchangeManager::returnItem(const std::string& itemId, int rating, const std::string& comment, const std::string& reviewerRollNo, const std::string& reviewerName) {
    ResourceItem* item = itemTable.find(itemId);
    if (item == nullptr) {
        return "{\"status\":\"ERROR\",\"message\":\"Resource item not found\"}";
    }

    if (item->isAvailable) {
        return "{\"status\":\"ERROR\",\"message\":\"Item is already marked as available\"}";
    }

    std::string previousBorrowerRoll = item->currentBorrowerRollNo;
    Student* prevStudent = studentTable.find(previousBorrowerRoll);
    if (prevStudent != nullptr) {
        prevStudent->borrowedItemIds.remove(itemId);
    }

    // Record feedback into the item's custom Linked List if rating is provided
    if (rating >= 1 && rating <= 5) {
        std::string fbId = "FB-" + std::to_string(time(nullptr) % 100000);
        std::string authorRoll = reviewerRollNo.empty() ? previousBorrowerRoll : reviewerRollNo;
        std::string authorName = reviewerName.empty() ? (prevStudent ? prevStudent->name : "Student") : reviewerName;
        std::string fbTime = getCurrentTimestamp();

        Feedback fb(fbId, authorRoll, authorName, rating, comment, fbTime);
        item->feedbacks.insertTail(fb); // Encapsulated in generic Singly Linked List
    }

    // Check if the FIFO Waitlist Queue has waiting students!
    if (!item->waitlist.isEmpty()) {
        std::string nextRoll = item->waitlist.dequeue(); // FIFO dequeue!
        Student* nextStudent = studentTable.find(nextRoll);

        item->currentBorrowerRollNo = nextRoll;
        item->currentBorrowerName = nextStudent ? nextStudent->name : nextRoll;
        item->isAvailable = false; // Remains borrowed by the next waiting student

        if (nextStudent != nullptr) {
            nextStudent->borrowedItemIds.insertTail(itemId);
        }

        storage.saveItems(itemTable);
        storage.saveStudents(studentTable);

        std::ostringstream ss;
        ss << "{\n"
           << "  \"status\": \"AUTO_REASSIGNED\",\n"
           << "  \"message\": \"Item returned! Next student in FIFO Queue (" << (nextStudent ? nextStudent->name : nextRoll) << ") has been automatically assigned the resource.\",\n"
           << "  \"itemId\": \"" << itemId << "\",\n"
           << "  \"newBorrowerRoll\": \"" << nextRoll << "\",\n"
           << "  \"remainingWaitlist\": " << item->waitlist.size() << "\n"
           << "}";
        return ss.str();
    } else {
        // No one waiting, item becomes available for anyone
        item->isAvailable = true;
        item->currentBorrowerRollNo = "";
        item->currentBorrowerName = "";

        storage.saveItems(itemTable);
        storage.saveStudents(studentTable);

        std::ostringstream ss;
        ss << "{\n"
           << "  \"status\": \"RETURNED_AVAILABLE\",\n"
           << "  \"message\": \"Item returned successfully and is now available for borrowing in the store.\",\n"
           << "  \"itemId\": \"" << itemId << "\"\n"
           << "}";
        return ss.str();
    }
}

bool ExchangeManager::addFeedback(const std::string& itemId, const Feedback& feedback, std::string& outMessage) {
    ResourceItem* item = itemTable.find(itemId);
    if (item == nullptr) {
        outMessage = "Item not found";
        return false;
    }
    item->feedbacks.insertTail(feedback);
    storage.saveItems(itemTable);
    outMessage = "Feedback submitted successfully!";
    return true;
}

std::vector<Student> ExchangeManager::getAllStudents() const {
    return studentTable.getAllValues();
}

Student* ExchangeManager::getStudent(const std::string& rollNo) {
    return studentTable.find(rollNo);
}

bool ExchangeManager::registerStudent(const Student& student, std::string& outMessage) {
    if (student.rollNo.empty()) {
        outMessage = "Roll number cannot be empty";
        return false;
    }
    if (studentTable.contains(student.rollNo)) {
        outMessage = "Student already registered";
        return false;
    }
    studentTable.insert(student.rollNo, student);
    storage.saveStudents(studentTable);
    outMessage = "Student profile created successfully!";
    return true;
}

void ExchangeManager::recordItemView(const std::string& studentRollNo, const std::string& itemId) {
    Student* student = studentTable.find(studentRollNo);
    if (student == nullptr) return;

    // Check if item exists
    if (!itemTable.contains(itemId)) return;

    // Push to student's Recently Viewed Stack (LIFO)
    student->recentlyViewedItemIds.push(itemId);
    storage.saveStudents(studentTable);
}

std::vector<ResourceItem> ExchangeManager::getRecentlyViewed(const std::string& studentRollNo) {
    std::vector<ResourceItem> result;
    Student* student = studentTable.find(studentRollNo);
    if (student == nullptr) return result;

    // Stack to vector extracts from Top (most recent) to Bottom
    std::vector<std::string> itemIds = student->recentlyViewedItemIds.toVector();
    for (const auto& id : itemIds) {
        ResourceItem* item = itemTable.find(id);
        if (item != nullptr) {
            // Avoid duplicate display in UI if viewed multiple times consecutively
            bool alreadyInList = false;
            for (const auto& res : result) {
                if (res.id == item->id) {
                    alreadyInList = true;
                    break;
                }
            }
            if (!alreadyInList) {
                result.push_back(*item);
            }
        }
    }
    return result;
}

std::vector<ResourceRequest> ExchangeManager::getAllRequests() const {
    return requestList.toVector();
}

bool ExchangeManager::createRequest(const ResourceRequest& req, std::string& outMessage) {
    if (req.requestId.empty() || req.itemTitle.empty()) {
        outMessage = "Request ID and Item Title are required";
        return false;
    }

    requestList.insertTail(req);
    storage.saveRequests(requestList);
    outMessage = "Resource request posted to the student board!";
    return true;
}

bool ExchangeManager::fulfillRequest(const std::string& reqId, std::string& outMessage) {
    ResourceRequest* target = requestList.findIf([&](const ResourceRequest& r) {
        return r.requestId == reqId;
    });

    if (target == nullptr) {
        outMessage = "Request with ID '" + reqId + "' not found";
        return false;
    }

    target->isFulfilled = true;
    storage.saveRequests(requestList);
    outMessage = "Request marked as fulfilled!";
    return true;
}

bool ExchangeManager::deleteRequest(const std::string& reqId, const std::string& requesterRollNo, std::string& outMessage) {
    ResourceRequest* target = requestList.findIf([&](const ResourceRequest& r) {
        return r.requestId == reqId;
    });

    if (target == nullptr) {
        outMessage = "Request with ID '" + reqId + "' not found";
        return false;
    }

    if (!requesterRollNo.empty() && target->requesterRollNo != requesterRollNo) {
        outMessage = "Only the creator of this request can cancel it";
        return false;
    }

    requestList.removeIf([&](const ResourceRequest& r) {
        return r.requestId == reqId;
    });

    storage.saveRequests(requestList);
    outMessage = "Request successfully removed from the Wanted Board!";
    return true;
}

bool ExchangeManager::undoLastAction(std::string& outMessage) {
    if (undoStack.isEmpty()) {
        outMessage = "No recent actions to undo";
        return false;
    }

    ActionRecord act = undoStack.pop(); // LIFO Pop!
    if (act.actionType == "LIST_ITEM") {
        ResourceItem* item = itemTable.find(act.entityId);
        if (item != nullptr) {
            std::string title = item->title;
            // Remove from category list
            LinkedList<std::string>* catList = categoryIndex.find(item->category);
            if (catList != nullptr) catList->remove(act.entityId);

            // Remove from student's lent list
            Student* student = studentTable.find(act.studentRollNo);
            if (student != nullptr) student->lentItemIds.remove(act.entityId);

            // Remove from hash table
            itemTable.remove(act.entityId);

            storage.saveItems(itemTable);
            storage.saveStudents(studentTable);
            outMessage = "Undid listing: '" + title + "' removed from catalog.";
            return true;
        }
    } else if (act.actionType == "BUY_ITEM") {
        ResourceItem* item = itemTable.find(act.entityId);
        if (item != nullptr) {
            std::string title = item->title;
            item->isSold = false;
            item->isAvailable = true;
            item->buyerRollNo = "";
            item->buyerName = "";
            item->soldTimestamp = "";

            Student* buyer = studentTable.find(act.studentRollNo);
            if (buyer != nullptr) {
                buyer->purchasedItemIds.remove(act.entityId);
            }

            storage.saveItems(itemTable);
            storage.saveStudents(studentTable);
            outMessage = "Undid purchase: '" + title + "' is now back in the store.";
            return true;
        }
    }

    outMessage = "Undid action: " + act.actionType;
    return true;
}

std::string ExchangeManager::getSystemStatsJson() const {
    std::ostringstream ss;
    int totalItems = itemTable.size();
    int availableItems = 0;
    int waitlistedStudents = 0;

    std::vector<ResourceItem> items = itemTable.getAllValues();
    for (const auto& item : items) {
        if (item.isAvailable) availableItems++;
        waitlistedStudents += item.waitlist.size();
    }

    std::vector<ResourceRequest> allReqs = requestList.toVector();
    int openRequests = 0;
    for (const auto& r : allReqs) {
        if (!r.isFulfilled) openRequests++;
    }

    ss << "{\n"
       << "  \"totalItems\": " << totalItems << ",\n"
       << "  \"availableItems\": " << availableItems << ",\n"
       << "  \"borrowedItems\": " << (totalItems - availableItems) << ",\n"
       << "  \"totalStudents\": " << studentTable.size() << ",\n"
       << "  \"totalRequests\": " << openRequests << ",\n"
       << "  \"allRequestsCount\": " << allReqs.size() << ",\n"
       << "  \"waitlistQueueCount\": " << waitlistedStudents << ",\n"
       << "  \"undoStackDepth\": " << undoStack.size() << "\n"
       << "}";
    return ss.str();
}
