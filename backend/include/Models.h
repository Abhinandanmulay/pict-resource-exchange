#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include "Queue.h"
#include "Stack.h"
#include "LinkedList.h"

/**
 * @brief Borrower Feedback & Rating Record
 */
struct Feedback {
    std::string id;
    std::string reviewerRollNo;
    std::string reviewerName;
    int rating; // 1 to 5
    std::string comment;
    std::string timestamp;

    Feedback();
    Feedback(const std::string& id,
             const std::string& roll,
             const std::string& name,
             int rating,
             const std::string& comment,
             const std::string& time = "");

    std::string toJson() const;
};

/**
 * @brief Campus Resource Item
 * Represents an item listed on the PICT Resource Exchange
 */
struct ResourceItem {
    std::string id;
    std::string title;
    std::string category;     // "Books", "Drawing Tools", "Electronics", "Stationery", "Aprons", "Notes"
    std::string condition;    // "Brand New", "Like New", "Good", "Fair"
    std::string description;
    std::string ownerRollNo;
    std::string ownerName;
    std::string ownerContact;
    std::string currentBorrowerRollNo;
    std::string currentBorrowerName;
    double borrowPrice;       // Rental / Borrowing fee in INR (0 for free)
    double depositAmount;     // Refundable deposit in INR (0 for free)
    int maxDays;              // Max allowed borrowing duration
    bool isAvailable;
    std::string imageUrl;
    std::string listingType;  // "RENT", "SELL", "BOTH"
    double sellPrice;         // Selling price in INR for permanent sale (0 if rent only)
    bool isSold;              // True if purchased permanently
    std::string buyerRollNo;  // Roll number of buyer
    std::string buyerName;    // Name of buyer
    std::string soldTimestamp;// Timestamp when purchased
    Queue<std::string> waitlist; // FIFO Queue of student roll numbers waiting for this resource
    LinkedList<Feedback> feedbacks; // Custom Linked List of reviews & ratings

    ResourceItem();
    ResourceItem(const std::string& id,
                 const std::string& title,
                 const std::string& category,
                 const std::string& condition,
                 const std::string& description,
                 const std::string& ownerRoll,
                 const std::string& ownerNm,
                 const std::string& ownerPhone,
                 double borrowFee = 0.0,
                 double deposit = 0.0,
                 int days = 7,
                 const std::string& img = "",
                 const std::string& listType = "RENT",
                 double priceToSell = 0.0);

    double getAverageRating() const;
    int getFeedbackCount() const;

    // Serialization helper to JSON string
    std::string toJson() const;
};

/**
 * @brief PICT Student Profile
 */
struct Student {
    std::string rollNo;       // Unique PICT Roll number (e.g. "31105")
    std::string name;
    std::string department;   // "Computer", "IT", "EnTC", "AI&DS"
    int year;                 // 1 (FE), 2 (SE), 3 (TE), 4 (BE)
    std::string email;
    std::string contactNo;
    LinkedList<std::string> borrowedItemIds;     // Dynamic list of currently borrowed items
    LinkedList<std::string> lentItemIds;         // Dynamic list of items listed by student
    LinkedList<std::string> purchasedItemIds;    // Dynamic list of items bought by student
    Stack<std::string> recentlyViewedItemIds;    // LIFO stack of item IDs browsed

    Student();
    Student(const std::string& roll,
            const std::string& nm,
            const std::string& dept,
            int yr,
            const std::string& em,
            const std::string& phone);

    std::string toJson() const;
};

/**
 * @brief Student Resource Request / Wanted Post
 * Allows students to broadcast a request for an item not currently listed
 */
struct ResourceRequest {
    std::string requestId;
    std::string requesterRollNo;
    std::string requesterName;
    std::string requesterContact;
    std::string itemTitle;
    std::string category;
    std::string description;
    std::string urgency;      // "Urgent (Exam Soon)", "Moderate", "Normal"
    std::string datePosted;
    bool isFulfilled;

    ResourceRequest();
    ResourceRequest(const std::string& reqId,
                    const std::string& roll,
                    const std::string& name,
                    const std::string& contact,
                    const std::string& title,
                    const std::string& cat,
                    const std::string& desc,
                    const std::string& urg,
                    const std::string& date);

    std::string toJson() const;
};

/**
 * @brief Audit Log / Undo Record
 */
struct ActionRecord {
    std::string actionType; // "LIST_ITEM", "BORROW_ITEM", "CREATE_REQUEST"
    std::string entityId;
    std::string studentRollNo;
    std::string timestamp;

    std::string toJson() const;
};

#endif // MODELS_H
