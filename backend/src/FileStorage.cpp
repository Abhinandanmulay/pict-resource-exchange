#include "FileStorage.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <direct.h> // for _mkdir on Windows

// Simple JSON extraction helper routines
static std::string extractString(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos);
    if (pos == std::string::npos) return "";
    size_t endPos = json.find('"', pos + 1);
    while (endPos != std::string::npos && json[endPos - 1] == '\\') {
        endPos = json.find('"', endPos + 1);
    }
    if (endPos == std::string::npos) return "";
    return json.substr(pos + 1, endPos - pos - 1);
}

static double extractDouble(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return 0.0;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return 0.0;
    while (pos < json.size() && (json[pos] == ':' || json[pos] == ' ' || json[pos] == '\t')) pos++;
    size_t endPos = pos;
    while (endPos < json.size() && ((json[endPos] >= '0' && json[endPos] <= '9') || json[endPos] == '.' || json[endPos] == '-')) {
        endPos++;
    }
    std::string valStr = json.substr(pos, endPos - pos);
    try {
        return std::stod(valStr);
    } catch (...) {
        return 0.0;
    }
}

static int extractInt(const std::string& json, const std::string& key) {
    return (int)extractDouble(json, key);
}

static bool extractBool(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return false;
    while (pos < json.size() && (json[pos] == ':' || json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (json.substr(pos, 4) == "true") return true;
    return false;
}

static std::vector<std::string> extractStringArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return result;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return result;
    size_t endArray = json.find(']', pos);
    if (endArray == std::string::npos) return result;

    size_t curr = pos + 1;
    while (curr < endArray) {
        size_t startQuote = json.find('"', curr);
        if (startQuote == std::string::npos || startQuote >= endArray) break;
        size_t endQuote = json.find('"', startQuote + 1);
        while (endQuote != std::string::npos && json[endQuote - 1] == '\\' && endQuote < endArray) {
            endQuote = json.find('"', endQuote + 1);
        }
        if (endQuote == std::string::npos || endQuote > endArray) break;
        result.push_back(json.substr(startQuote + 1, endQuote - startQuote - 1));
        curr = endQuote + 1;
    }
    return result;
}

// Splits a JSON array of objects into individual object strings
static std::vector<std::string> splitObjectArray(const std::string& content) {
    std::vector<std::string> objects;
    size_t start = content.find('[');
    if (start == std::string::npos) return objects;

    int braceCount = 0;
    size_t objStart = std::string::npos;
    bool inQuote = false;

    for (size_t i = start + 1; i < content.size(); ++i) {
        char c = content[i];
        if (c == '"' && (i == 0 || content[i - 1] != '\\')) {
            inQuote = !inQuote;
        }
        if (!inQuote) {
            if (c == '{') {
                if (braceCount == 0) objStart = i;
                braceCount++;
            } else if (c == '}') {
                braceCount--;
                if (braceCount == 0 && objStart != std::string::npos) {
                    objects.push_back(content.substr(objStart, i - objStart + 1));
                    objStart = std::string::npos;
                }
            } else if (c == ']' && braceCount == 0) {
                break;
            }
        }
    }
    return objects;
}

FileStorage::FileStorage(const std::string& directory) : dataDir(directory) {
    _mkdir(dataDir.c_str());
}

bool FileStorage::loadItems(HashTable<std::string, ResourceItem>& itemTable) {
    std::ifstream file(dataDir + "/items.json");
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    std::vector<std::string> objStrings = splitObjectArray(content);
    for (const auto& obj : objStrings) {
        ResourceItem item;
        item.id = extractString(obj, "id");
        if (item.id.empty()) continue;

        item.title = extractString(obj, "title");
        item.category = extractString(obj, "category");
        item.condition = extractString(obj, "condition");
        item.description = extractString(obj, "description");
        item.ownerRollNo = extractString(obj, "ownerRollNo");
        item.ownerName = extractString(obj, "ownerName");
        item.ownerContact = extractString(obj, "ownerContact");
        item.currentBorrowerRollNo = extractString(obj, "currentBorrowerRollNo");
        item.currentBorrowerName = extractString(obj, "currentBorrowerName");
        item.borrowPrice = extractDouble(obj, "borrowPrice");
        item.depositAmount = extractDouble(obj, "depositAmount");
        item.maxDays = extractInt(obj, "maxDays");
        item.isAvailable = extractBool(obj, "isAvailable");
        item.imageUrl = extractString(obj, "imageUrl");

        item.listingType = extractString(obj, "listingType");
        if (item.listingType.empty()) item.listingType = "RENT";
        item.sellPrice = extractDouble(obj, "sellPrice");
        item.isSold = extractBool(obj, "isSold");
        item.buyerRollNo = extractString(obj, "buyerRollNo");
        item.buyerName = extractString(obj, "buyerName");
        item.soldTimestamp = extractString(obj, "soldTimestamp");

        // Load waitlist queue
        std::vector<std::string> waitlistEntries = extractStringArray(obj, "waitlist");
        for (const auto& studentRoll : waitlistEntries) {
            item.waitlist.enqueue(studentRoll);
        }

        // Load feedbacks Linked List
        size_t fbPos = obj.find("\"feedbacks\"");
        if (fbPos != std::string::npos) {
            std::vector<std::string> fbObjs = splitObjectArray(obj.substr(fbPos));
            for (const auto& fbObj : fbObjs) {
                Feedback fb;
                fb.id = extractString(fbObj, "id");
                fb.reviewerRollNo = extractString(fbObj, "reviewerRollNo");
                fb.reviewerName = extractString(fbObj, "reviewerName");
                fb.rating = extractInt(fbObj, "rating");
                if (fb.rating < 1) fb.rating = 5;
                fb.comment = extractString(fbObj, "comment");
                fb.timestamp = extractString(fbObj, "timestamp");
                item.feedbacks.insertTail(fb);
            }
        }

        itemTable.insert(item.id, item);
    }
    return true;
}

bool FileStorage::loadStudents(HashTable<std::string, Student>& studentTable) {
    std::ifstream file(dataDir + "/students.json");
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    std::vector<std::string> objStrings = splitObjectArray(content);
    for (const auto& obj : objStrings) {
        Student s;
        s.rollNo = extractString(obj, "rollNo");
        if (s.rollNo.empty()) continue;

        s.name = extractString(obj, "name");
        s.department = extractString(obj, "department");
        s.year = extractInt(obj, "year");
        s.email = extractString(obj, "email");
        s.contactNo = extractString(obj, "contactNo");

        std::vector<std::string> borrowed = extractStringArray(obj, "borrowedItemIds");
        for (const auto& id : borrowed) s.borrowedItemIds.insertTail(id);

        std::vector<std::string> lent = extractStringArray(obj, "lentItemIds");
        for (const auto& id : lent) s.lentItemIds.insertTail(id);

        std::vector<std::string> bought = extractStringArray(obj, "purchasedItemIds");
        for (const auto& id : bought) s.purchasedItemIds.insertTail(id);

        std::vector<std::string> recent = extractStringArray(obj, "recentlyViewedItemIds");
        // Rebuild stack in correct order (vector is from top to bottom)
        for (int i = (int)recent.size() - 1; i >= 0; --i) {
            s.recentlyViewedItemIds.push(recent[i]);
        }

        studentTable.insert(s.rollNo, s);
    }
    return true;
}

bool FileStorage::loadRequests(LinkedList<ResourceRequest>& requestList) {
    std::ifstream file(dataDir + "/requests.json");
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    std::vector<std::string> objStrings = splitObjectArray(content);
    for (const auto& obj : objStrings) {
        ResourceRequest r;
        r.requestId = extractString(obj, "requestId");
        if (r.requestId.empty()) continue;

        r.requesterRollNo = extractString(obj, "requesterRollNo");
        r.requesterName = extractString(obj, "requesterName");
        r.requesterContact = extractString(obj, "requesterContact");
        r.itemTitle = extractString(obj, "itemTitle");
        r.category = extractString(obj, "category");
        r.description = extractString(obj, "description");
        r.urgency = extractString(obj, "urgency");
        r.datePosted = extractString(obj, "datePosted");
        r.isFulfilled = extractBool(obj, "isFulfilled");

        requestList.insertTail(r);
    }
    return true;
}

bool FileStorage::saveItems(const HashTable<std::string, ResourceItem>& itemTable) {
    std::ofstream file(dataDir + "/items.json");
    if (!file.is_open()) return false;

    std::vector<ResourceItem> items = itemTable.getAllValues();
    file << "[\n";
    for (size_t i = 0; i < items.size(); ++i) {
        file << items[i].toJson();
        if (i + 1 < items.size()) file << ",\n";
    }
    file << "\n]\n";
    file.close();
    return true;
}

bool FileStorage::saveStudents(const HashTable<std::string, Student>& studentTable) {
    std::ofstream file(dataDir + "/students.json");
    if (!file.is_open()) return false;

    std::vector<Student> students = studentTable.getAllValues();
    file << "[\n";
    for (size_t i = 0; i < students.size(); ++i) {
        file << students[i].toJson();
        if (i + 1 < students.size()) file << ",\n";
    }
    file << "\n]\n";
    file.close();
    return true;
}

bool FileStorage::saveRequests(const LinkedList<ResourceRequest>& requestList) {
    std::ofstream file(dataDir + "/requests.json");
    if (!file.is_open()) return false;

    std::vector<ResourceRequest> requests = requestList.toVector();
    file << "[\n";
    for (size_t i = 0; i < requests.size(); ++i) {
        file << requests[i].toJson();
        if (i + 1 < requests.size()) file << ",\n";
    }
    file << "\n]\n";
    file.close();
    return true;
}

void FileStorage::initializeDefaultData(HashTable<std::string, ResourceItem>& itemTable,
                                        HashTable<std::string, Student>& studentTable,
                                        LinkedList<ResourceRequest>& requestList) {
    // 1. Seed Students
    Student s1("31105", "Rohan Shinde", "Computer Engineering", 2, "rohan.31105@pict.edu", "+91 98220 12345");
    Student s2("31142", "Priya Patil", "IT Engineering", 3, "priya.31142@pict.edu", "+91 98221 23456");
    Student s3("31201", "Amit Deshmukh", "EnTC Engineering", 2, "amit.31201@pict.edu", "+91 98222 34567");
    Student s4("31350", "Tanvi Joshi", "AI & Data Science", 1, "tanvi.31350@pict.edu", "+91 98223 45678");

    // 2. Seed Items (id, title, category, condition, description, ownerRoll, ownerNm, ownerPhone, borrowPrice, depositAmount, maxDays, imageUrl)
    ResourceItem item1("PICT-101",
                       "Omega Engineering Mini Drafter (Steel Arm)",
                       "Drawing Tools",
                       "Like New",
                       "Precision mini drafter used in FE Graphic Drawing. In original carrying case with clamp.",
                       "31105",
                       "Rohan Shinde",
                       "+91 98220 12345",
                       20.0,
                       150.0,
                       14,
                       "https://images.unsplash.com/photo-1581291518857-4e27b48ff24e?w=500&auto=format&fit=crop&q=60");
    item1.feedbacks.insertTail(Feedback("FB-101", "31142", "Priya Patil", 5, "Steel arm is super smooth and accurate. Helped immensely in FE drawing exam!", "2026-10-15 14:30"));
    item1.feedbacks.insertTail(Feedback("FB-102", "31350", "Tanvi Joshi", 4, "Great condition, clamp holds tightly to the drawing board.", "2026-10-16 11:10"));

    ResourceItem item2("PICT-102",
                       "Casio fx-991CW ClassWiz Scientific Calculator",
                       "Stationery",
                       "Brand New",
                       "Approved for SPPU engineering exams. High-res LCD, 540+ functions, 4x4 matrix, complex numbers.",
                       "31142",
                       "Priya Patil",
                       "+91 98221 23456",
                       30.0,
                       200.0,
                       7,
                       "https://images.unsplash.com/photo-1594980596870-8aa52a78d8cd?w=500&auto=format&fit=crop&q=60");
    item2.feedbacks.insertTail(Feedback("FB-103", "31105", "Rohan Shinde", 5, "Battery is full and display is crisp. Matrix solving is super fast.", "2026-10-14 16:45"));

    ResourceItem item3("PICT-103",
                       "Database System Concepts (Silberschatz & Korth - 7th Ed)",
                       "Books",
                       "Good",
                       "Standard textbook for SE DBMS. Complete with SQL queries and relational algebra chapters highlighted.",
                       "31105",
                       "Rohan Shinde",
                       "+91 98220 12345",
                       15.0,
                       100.0,
                       21,
                       "https://images.unsplash.com/photo-1544716278-ca5e3f4abd8c?w=500&auto=format&fit=crop&q=60");
    item3.feedbacks.insertTail(Feedback("FB-104", "31201", "Amit Deshmukh", 5, "Very neat book with all important ER diagrams and normalization topics marked.", "2026-10-13 09:20"));

    ResourceItem item4("PICT-104",
                       "Arduino Uno R3 Starter Lab Kit with Sensors",
                       "Electronics",
                       "Good",
                       "Includes breadboard, jumper wires, ultrasonic sensor, DHT11, and servo motor for IoT/microprocessor labs.",
                       "31201",
                       "Amit Deshmukh",
                       "+91 98222 34567",
                       40.0,
                       250.0,
                       10,
                       "https://images.unsplash.com/photo-1553406830-ef2513450d76?w=500&auto=format&fit=crop&q=60");

    ResourceItem item5("PICT-105",
                       "PICT Workshop Cotton Apron (Navy Blue - Size M)",
                       "Aprons",
                       "Like New",
                       "Required for FE Mechanical Workshop (Welding, Carpentry, Fitting). Freshly washed.",
                       "31142",
                       "Priya Patil",
                       "+91 98221 23456",
                       10.0,
                       50.0,
                       30,
                       "https://images.unsplash.com/photo-1582719478250-c89cae4dc85b?w=500&auto=format&fit=crop&q=60");

    ResourceItem item6("PICT-106",
                       "Operating System Concepts (Galvin - 10th Ed)",
                       "Books",
                       "Good",
                       "Covers Process Management, Concurrency, Virtual Memory, and Linux Kernels. Great for In-Sem prep.",
                       "31350",
                       "Tanvi Joshi",
                       "+91 98223 45678",
                       15.0,
                       100.0,
                       14,
                       "https://images.unsplash.com/photo-1532012164546-f432f2e3edd9?w=500&auto=format&fit=crop&q=60");

    // Connect item ownership
    s1.lentItemIds.insertTail("PICT-101");
    s1.lentItemIds.insertTail("PICT-103");
    s2.lentItemIds.insertTail("PICT-102");
    s2.lentItemIds.insertTail("PICT-105");
    s3.lentItemIds.insertTail("PICT-104");
    s4.lentItemIds.insertTail("PICT-106");

    // 3. Seed Requests
    ResourceRequest req1("REQ-201",
                         "31350",
                         "Tanvi Joshi",
                         "+91 98223 45678",
                         "Wanted: 8051 Microcontroller Trainer Kit",
                         "Electronics",
                         "Urgent requirement for TE EnTC Microcontroller Practical exam this Friday.",
                         "Urgent (Exam in 2 days)",
                         "2026-10-07");

    ResourceRequest req2("REQ-202",
                         "31201",
                         "Amit Deshmukh",
                         "+91 98222 34567",
                         "Looking for Computer Networks 5th Edition (Andrew Tanenbaum)",
                         "Books",
                         "Needed for preparing unit 3 & 4 network layer protocols.",
                         "Moderate",
                         "2026-10-06");

    itemTable.insert(item1.id, item1);
    itemTable.insert(item2.id, item2);
    itemTable.insert(item3.id, item3);
    itemTable.insert(item4.id, item4);
    itemTable.insert(item5.id, item5);
    itemTable.insert(item6.id, item6);

    studentTable.insert(s1.rollNo, s1);
    studentTable.insert(s2.rollNo, s2);
    studentTable.insert(s3.rollNo, s3);
    studentTable.insert(s4.rollNo, s4);

    requestList.insertTail(req1);
    requestList.insertTail(req2);

    saveItems(itemTable);
    saveStudents(studentTable);
    saveRequests(requestList);
}
