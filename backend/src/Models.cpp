#include "Models.h"
#include <sstream>
#include <iomanip>

static std::string escapeJsonString(const std::string& input) {
    std::ostringstream ss;
    for (char c : input) {
        if (c == '"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\b') ss << "\\b";
        else if (c == '\f') ss << "\\f";
        else if (c == '\n') ss << "\\n";
        else if (c == '\r') ss << "\\r";
        else if (c == '\t') ss << "\\t";
        else ss << c;
    }
    return ss.str();
}

Feedback::Feedback() : rating(5) {}

Feedback::Feedback(const std::string& id,
                   const std::string& roll,
                   const std::string& name,
                   int rating,
                   const std::string& comment,
                   const std::string& time)
    : id(id), reviewerRollNo(roll), reviewerName(name),
      rating(rating), comment(comment), timestamp(time) {}

std::string Feedback::toJson() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"id\": \"" << escapeJsonString(id) << "\",\n";
    ss << "  \"reviewerRollNo\": \"" << escapeJsonString(reviewerRollNo) << "\",\n";
    ss << "  \"reviewerName\": \"" << escapeJsonString(reviewerName) << "\",\n";
    ss << "  \"rating\": " << rating << ",\n";
    ss << "  \"comment\": \"" << escapeJsonString(comment) << "\",\n";
    ss << "  \"timestamp\": \"" << escapeJsonString(timestamp) << "\"\n";
    ss << "}";
    return ss.str();
}

ResourceItem::ResourceItem() 
    : borrowPrice(0.0), depositAmount(0.0), maxDays(7), isAvailable(true),
      listingType("RENT"), sellPrice(0.0), isSold(false),
      buyerRollNo(""), buyerName(""), soldTimestamp("") {}

ResourceItem::ResourceItem(const std::string& id,
                           const std::string& title,
                           const std::string& category,
                           const std::string& condition,
                           const std::string& description,
                           const std::string& ownerRoll,
                           const std::string& ownerNm,
                           const std::string& ownerPhone,
                           double borrowFee,
                           double deposit,
                           int days,
                           const std::string& img,
                           const std::string& listType,
                           double priceToSell)
    : id(id), title(title), category(category), condition(condition),
      description(description), ownerRollNo(ownerRoll), ownerName(ownerNm),
      ownerContact(ownerPhone), currentBorrowerRollNo(""), currentBorrowerName(""),
      borrowPrice(borrowFee), depositAmount(deposit), maxDays(days), isAvailable(true), imageUrl(img),
      listingType(listType.empty() ? "RENT" : listType), sellPrice(priceToSell), isSold(false),
      buyerRollNo(""), buyerName(""), soldTimestamp("") {}

double ResourceItem::getAverageRating() const {
    std::vector<Feedback> list = feedbacks.toVector();
    if (list.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& fb : list) {
        sum += fb.rating;
    }
    return sum / list.size();
}

int ResourceItem::getFeedbackCount() const {
    return feedbacks.size();
}

std::string ResourceItem::toJson() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"id\": \"" << escapeJsonString(id) << "\",\n";
    ss << "  \"title\": \"" << escapeJsonString(title) << "\",\n";
    ss << "  \"category\": \"" << escapeJsonString(category) << "\",\n";
    ss << "  \"condition\": \"" << escapeJsonString(condition) << "\",\n";
    ss << "  \"description\": \"" << escapeJsonString(description) << "\",\n";
    ss << "  \"ownerRollNo\": \"" << escapeJsonString(ownerRollNo) << "\",\n";
    ss << "  \"ownerName\": \"" << escapeJsonString(ownerName) << "\",\n";
    ss << "  \"ownerContact\": \"" << escapeJsonString(ownerContact) << "\",\n";
    ss << "  \"currentBorrowerRollNo\": \"" << escapeJsonString(currentBorrowerRollNo) << "\",\n";
    ss << "  \"currentBorrowerName\": \"" << escapeJsonString(currentBorrowerName) << "\",\n";
    ss << "  \"borrowPrice\": " << std::fixed << std::setprecision(2) << borrowPrice << ",\n";
    ss << "  \"depositAmount\": " << std::fixed << std::setprecision(2) << depositAmount << ",\n";
    ss << "  \"maxDays\": " << maxDays << ",\n";
    ss << "  \"isAvailable\": " << (isAvailable ? "true" : "false") << ",\n";
    ss << "  \"listingType\": \"" << escapeJsonString(listingType) << "\",\n";
    ss << "  \"sellPrice\": " << std::fixed << std::setprecision(2) << sellPrice << ",\n";
    ss << "  \"isSold\": " << (isSold ? "true" : "false") << ",\n";
    ss << "  \"buyerRollNo\": \"" << escapeJsonString(buyerRollNo) << "\",\n";
    ss << "  \"buyerName\": \"" << escapeJsonString(buyerName) << "\",\n";
    ss << "  \"soldTimestamp\": \"" << escapeJsonString(soldTimestamp) << "\",\n";
    ss << "  \"imageUrl\": \"" << escapeJsonString(imageUrl) << "\",\n";

    // Waitlist Queue elements to JSON Array
    std::vector<std::string> waitlistVector = waitlist.toVector();
    ss << "  \"waitlist\": [";
    for (size_t i = 0; i < waitlistVector.size(); ++i) {
        ss << "\"" << escapeJsonString(waitlistVector[i]) << "\"";
        if (i + 1 < waitlistVector.size()) ss << ", ";
    }
    ss << "],\n";
    ss << "  \"waitlistCount\": " << waitlistVector.size() << ",\n";

    // Feedbacks Linked List elements to JSON Array
    std::vector<Feedback> fbVector = feedbacks.toVector();
    ss << "  \"feedbacks\": [\n";
    for (size_t i = 0; i < fbVector.size(); ++i) {
        ss << fbVector[i].toJson();
        if (i + 1 < fbVector.size()) ss << ",\n";
    }
    ss << "\n  ],\n";
    ss << "  \"averageRating\": " << std::fixed << std::setprecision(1) << getAverageRating() << ",\n";
    ss << "  \"reviewCount\": " << fbVector.size() << "\n";
    ss << "}";
    return ss.str();
}

Student::Student() : year(1) {}

Student::Student(const std::string& roll,
                 const std::string& nm,
                 const std::string& dept,
                 int yr,
                 const std::string& em,
                 const std::string& phone)
    : rollNo(roll), name(nm), department(dept), year(yr), email(em), contactNo(phone) {}

std::string Student::toJson() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"rollNo\": \"" << escapeJsonString(rollNo) << "\",\n";
    ss << "  \"name\": \"" << escapeJsonString(name) << "\",\n";
    ss << "  \"department\": \"" << escapeJsonString(department) << "\",\n";
    ss << "  \"year\": " << year << ",\n";
    ss << "  \"email\": \"" << escapeJsonString(email) << "\",\n";
    ss << "  \"contactNo\": \"" << escapeJsonString(contactNo) << "\",\n";

    // Borrowed Items (Linked List)
    std::vector<std::string> borrowed = borrowedItemIds.toVector();
    ss << "  \"borrowedItemIds\": [";
    for (size_t i = 0; i < borrowed.size(); ++i) {
        ss << "\"" << escapeJsonString(borrowed[i]) << "\"";
        if (i + 1 < borrowed.size()) ss << ", ";
    }
    ss << "],\n";

    // Lent Items (Linked List)
    std::vector<std::string> lent = lentItemIds.toVector();
    ss << "  \"lentItemIds\": [";
    for (size_t i = 0; i < lent.size(); ++i) {
        ss << "\"" << escapeJsonString(lent[i]) << "\"";
        if (i + 1 < lent.size()) ss << ", ";
    }
    ss << "],\n";

    // Purchased Items (Linked List)
    std::vector<std::string> bought = purchasedItemIds.toVector();
    ss << "  \"purchasedItemIds\": [";
    for (size_t i = 0; i < bought.size(); ++i) {
        ss << "\"" << escapeJsonString(bought[i]) << "\"";
        if (i + 1 < bought.size()) ss << ", ";
    }
    ss << "],\n";

    // Recently Viewed (Stack - LIFO order)
    std::vector<std::string> recent = recentlyViewedItemIds.toVector();
    ss << "  \"recentlyViewedItemIds\": [";
    for (size_t i = 0; i < recent.size(); ++i) {
        ss << "\"" << escapeJsonString(recent[i]) << "\"";
        if (i + 1 < recent.size()) ss << ", ";
    }
    ss << "]\n";
    ss << "}";
    return ss.str();
}

ResourceRequest::ResourceRequest() : isFulfilled(false) {}

ResourceRequest::ResourceRequest(const std::string& reqId,
                                 const std::string& roll,
                                 const std::string& name,
                                 const std::string& contact,
                                 const std::string& title,
                                 const std::string& cat,
                                 const std::string& desc,
                                 const std::string& urg,
                                 const std::string& date)
    : requestId(reqId), requesterRollNo(roll), requesterName(name),
      requesterContact(contact), itemTitle(title), category(cat),
      description(desc), urgency(urg), datePosted(date), isFulfilled(false) {}

std::string ResourceRequest::toJson() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"requestId\": \"" << escapeJsonString(requestId) << "\",\n";
    ss << "  \"requesterRollNo\": \"" << escapeJsonString(requesterRollNo) << "\",\n";
    ss << "  \"requesterName\": \"" << escapeJsonString(requesterName) << "\",\n";
    ss << "  \"requesterContact\": \"" << escapeJsonString(requesterContact) << "\",\n";
    ss << "  \"itemTitle\": \"" << escapeJsonString(itemTitle) << "\",\n";
    ss << "  \"category\": \"" << escapeJsonString(category) << "\",\n";
    ss << "  \"description\": \"" << escapeJsonString(description) << "\",\n";
    ss << "  \"urgency\": \"" << escapeJsonString(urgency) << "\",\n";
    ss << "  \"datePosted\": \"" << escapeJsonString(datePosted) << "\",\n";
    ss << "  \"isFulfilled\": " << (isFulfilled ? "true" : "false") << "\n";
    ss << "}";
    return ss.str();
}

std::string ActionRecord::toJson() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"actionType\": \"" << escapeJsonString(actionType) << "\",\n";
    ss << "  \"entityId\": \"" << escapeJsonString(entityId) << "\",\n";
    ss << "  \"studentRollNo\": \"" << escapeJsonString(studentRollNo) << "\",\n";
    ss << "  \"timestamp\": \"" << escapeJsonString(timestamp) << "\"\n";
    ss << "}";
    return ss.str();
}
