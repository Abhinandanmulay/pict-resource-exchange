#include "HttpServer.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstring>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

// URL decoding helper
static std::string urlDecode(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size()) {
            int value;
            std::istringstream is(in.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                out += static_cast<char>(value);
                i += 2;
            } else {
                out += in[i];
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return out;
}

// Extract query parameter value
static std::string getQueryParam(const std::string& query, const std::string& key) {
    std::string search = key + "=";
    size_t pos = query.find(search);
    if (pos == std::string::npos) return "";
    pos += search.length();
    size_t endPos = query.find('&', pos);
    if (endPos == std::string::npos) {
        return urlDecode(query.substr(pos));
    }
    return urlDecode(query.substr(pos, endPos - pos));
}

// Quick JSON value extractor
static std::string getJsonStr(const std::string& json, const std::string& key) {
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

static double getJsonNum(const std::string& json, const std::string& key) {
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
    try {
        return std::stod(json.substr(pos, endPos - pos));
    } catch (...) {
        return 0.0;
    }
}

HttpServer::HttpServer(int port, ExchangeManager& mgr)
    : port(port), manager(mgr), listenSocket(INVALID_SOCKET), isRunning(false) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    WSADATA wsaData;
    int wsResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsResult != 0) {
        std::cerr << "[HttpServer] WSAStartup failed: " << wsResult << "\n";
        return;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "[HttpServer] Error creating socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    // Allow address reuse
    int optval = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);

    if (bind(listenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cerr << "[HttpServer] Bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[HttpServer] Listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    isRunning = true;
    std::cout << "=========================================================\n";
    std::cout << "   PICT RESOURCE EXCHANGE - SERVER RUNNING\n";
    std::cout << "   Local Storefront URL: http://localhost:" << port << "/\n";
    std::cout << "   API Base URL:         http://localhost:" << port << "/api\n";
    std::cout << "=========================================================\n";

    while (isRunning) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            if (!isRunning) break;
            std::cerr << "[HttpServer] Accept failed: " << WSAGetLastError() << "\n";
            continue;
        }

        handleClient(clientSocket);
        closesocket(clientSocket);
    }
}

void HttpServer::stop() {
    if (isRunning) {
        isRunning = false;
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        WSACleanup();
    }
}

void HttpServer::handleClient(SOCKET clientSocket) {
    char buffer[8192];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) return;
    buffer[bytesReceived] = '\0';

    std::string rawRequest(buffer, bytesReceived);

    // Parse Request Line (Method, URI, Protocol)
    std::istringstream reqStream(rawRequest);
    std::string method, fullUri, protocol;
    reqStream >> method >> fullUri >> protocol;

    // Split path and query
    std::string path = fullUri;
    std::string query = "";
    size_t qPos = fullUri.find('?');
    if (qPos != std::string::npos) {
        path = fullUri.substr(0, qPos);
        query = fullUri.substr(qPos + 1);
    }

    // Extract Headers and Body
    size_t bodyPos = rawRequest.find("\r\n\r\n");
    std::string body = "";
    if (bodyPos != std::string::npos) {
        body = rawRequest.substr(bodyPos + 4);
    }

    // Check Content-Length for large bodies
    size_t clPos = rawRequest.find("Content-Length: ");
    if (clPos != std::string::npos) {
        size_t clEnd = rawRequest.find("\r\n", clPos);
        int contentLength = std::stoi(rawRequest.substr(clPos + 16, clEnd - (clPos + 16)));
        while ((int)body.length() < contentLength) {
            int moreBytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (moreBytes <= 0) break;
            body.append(buffer, moreBytes);
        }
    }

    // Handle CORS Pre-flight Options request
    if (method == "OPTIONS") {
        sendResponse(clientSocket, 204, "text/plain", "");
        return;
    }

    // Handle Static UI Files
    if (path == "/" || path == "/index.html") {
        sendFile(clientSocket, "frontend/index.html", "text/html; charset=utf-8");
        return;
    } else if (path == "/style.css") {
        sendFile(clientSocket, "frontend/style.css", "text/css; charset=utf-8");
        return;
    } else if (path == "/app.js") {
        sendFile(clientSocket, "frontend/app.js", "application/javascript; charset=utf-8");
        return;
    }

    // Handle REST API requests
    if (path.rfind("/api/", 0) == 0) {
        std::string jsonResponse = handleApiRequest(method, path, query, body);
        sendResponse(clientSocket, 200, "application/json; charset=utf-8", jsonResponse);
        return;
    }

    // Fallback 404
    sendResponse(clientSocket, 404, "text/plain", "404 Not Found");
}

std::string HttpServer::handleApiRequest(const std::string& method,
                                         const std::string& path,
                                         const std::string& query,
                                         const std::string& body) {
    // 1. GET /api/items (All items or filtered)
    if (path == "/api/items" && method == "GET") {
        std::string q = getQueryParam(query, "query");
        std::string cat = getQueryParam(query, "category");
        std::string avail = getQueryParam(query, "availability");

        std::vector<ResourceItem> items;
        if (!q.empty() || !cat.empty() || !avail.empty()) {
            items = manager.searchAndFilter(q, cat, avail);
        } else {
            items = manager.getAllItems();
        }

        std::ostringstream ss;
        ss << "[\n";
        for (size_t i = 0; i < items.size(); ++i) {
            ss << items[i].toJson();
            if (i + 1 < items.size()) ss << ",\n";
        }
        ss << "\n]";
        return ss.str();
    }

    // 2. GET /api/item (Single item details by ID using Hash Table)
    if (path == "/api/item" && method == "GET") {
        std::string id = getQueryParam(query, "id");
        ResourceItem* item = manager.getItem(id);
        if (item != nullptr) {
            return item->toJson();
        }
        return "{\"status\":\"ERROR\",\"message\":\"Item not found\"}";
    }

    // 3. POST /api/items/lend (List new resource item)
    if (path == "/api/items/lend" && method == "POST") {
        std::string id = getJsonStr(body, "id");
        if (id.empty()) {
            // Generate ID based on timestamp
            id = "PICT-" + std::to_string(time(nullptr) % 100000);
        }

        ResourceItem item;
        item.id = id;
        item.title = getJsonStr(body, "title");
        item.category = getJsonStr(body, "category");
        item.condition = getJsonStr(body, "condition");
        item.description = getJsonStr(body, "description");
        item.ownerRollNo = getJsonStr(body, "ownerRollNo");
        item.ownerName = getJsonStr(body, "ownerName");
        item.ownerContact = getJsonStr(body, "ownerContact");
        item.borrowPrice = getJsonNum(body, "borrowPrice");
        item.depositAmount = getJsonNum(body, "depositAmount");
        item.maxDays = (int)getJsonNum(body, "maxDays");
        if (item.maxDays <= 0) item.maxDays = 7;
        item.isAvailable = true;
        item.imageUrl = getJsonStr(body, "imageUrl");

        item.listingType = getJsonStr(body, "listingType");
        if (item.listingType.empty()) item.listingType = "RENT";
        item.sellPrice = getJsonNum(body, "sellPrice");
        item.isSold = false;

        std::string outMsg;
        bool ok = manager.listItem(item, outMsg);
        if (ok) {
            return "{\"status\":\"SUCCESS\",\"message\":\"" + outMsg + "\",\"itemId\":\"" + id + "\"}";
        } else {
            return "{\"status\":\"ERROR\",\"message\":\"" + outMsg + "\"}";
        }
    }

    // 3b. POST /api/items/cancel-lend (Cancel/Delist a listed resource item)
    if (path == "/api/items/cancel-lend" && method == "POST") {
        std::string itemId = getJsonStr(body, "itemId");
        std::string ownerRollNo = getJsonStr(body, "ownerRollNo");
        std::string outMsg;
        bool ok = manager.cancelLendedItem(itemId, ownerRollNo, outMsg);
        if (ok) {
            return "{\"status\":\"SUCCESS\",\"message\":\"" + outMsg + "\"}";
        } else {
            return "{\"status\":\"ERROR\",\"message\":\"" + outMsg + "\"}";
        }
    }

    // 4. POST /api/items/borrow (Borrow resource or join FIFO waitlist)
    if (path == "/api/items/borrow" && method == "POST") {
        std::string itemId = getJsonStr(body, "itemId");
        std::string rollNo = getJsonStr(body, "studentRollNo");
        return manager.borrowItem(itemId, rollNo);
    }

    // 4b. POST /api/items/buy (Purchase resource item permanently)
    if (path == "/api/items/buy" && method == "POST") {
        std::string itemId = getJsonStr(body, "itemId");
        std::string rollNo = getJsonStr(body, "studentRollNo");
        return manager.buyItem(itemId, rollNo);
    }

    // 5. POST /api/items/return (Return resource with optional feedback and notify waitlist)
    if (path == "/api/items/return" && method == "POST") {
        std::string itemId = getJsonStr(body, "itemId");
        int rating = (int)getJsonNum(body, "rating");
        std::string comment = getJsonStr(body, "comment");
        std::string reviewerRollNo = getJsonStr(body, "reviewerRollNo");
        std::string reviewerName = getJsonStr(body, "reviewerName");
        return manager.returnItem(itemId, rating, comment, reviewerRollNo, reviewerName);
    }

    // 5b. POST /api/items/feedback (Leave review/rating for an item)
    if (path == "/api/items/feedback" && method == "POST") {
        std::string itemId = getJsonStr(body, "itemId");
        std::string fbId = "FB-" + std::to_string(time(nullptr) % 100000);
        int rating = (int)getJsonNum(body, "rating");
        if (rating < 1) rating = 5;
        if (rating > 5) rating = 5;
        std::string comment = getJsonStr(body, "comment");
        std::string reviewerRollNo = getJsonStr(body, "reviewerRollNo");
        std::string reviewerName = getJsonStr(body, "reviewerName");
        std::string timestamp = "Just now";

        Feedback fb(fbId, reviewerRollNo, reviewerName, rating, comment, timestamp);
        std::string outMsg;
        bool ok = manager.addFeedback(itemId, fb, outMsg);
        if (ok) {
            return "{\"status\":\"SUCCESS\",\"message\":\"" + outMsg + "\"}";
        } else {
            return "{\"status\":\"ERROR\",\"message\":\"" + outMsg + "\"}";
        }
    }

    // 6. POST /api/undo (Undo last action from LIFO Stack)
    if (path == "/api/undo" && method == "POST") {
        std::string outMsg;
        bool ok = manager.undoLastAction(outMsg);
        if (ok) {
            return "{\"status\":\"SUCCESS\",\"message\":\"" + outMsg + "\"}";
        } else {
            return "{\"status\":\"ERROR\",\"message\":\"" + outMsg + "\"}";
        }
    }

    // 7. GET /api/students (All students)
    if (path == "/api/students" && method == "GET") {
        std::vector<Student> students = manager.getAllStudents();
        std::ostringstream ss;
        ss << "[\n";
        for (size_t i = 0; i < students.size(); ++i) {
            ss << students[i].toJson();
            if (i + 1 < students.size()) ss << ",\n";
        }
        ss << "\n]";
        return ss.str();
    }

    // 7b. POST /api/student/login (Login/verify student by roll number using Hash Table)
    if (path == "/api/student/login" && method == "POST") {
        std::string roll = getJsonStr(body, "rollNo");
        Student* s = manager.getStudent(roll);
        if (s != nullptr) {
            std::ostringstream ss;
            ss << "{\n"
               << "  \"status\": \"SUCCESS\",\n"
               << "  \"message\": \"Welcome back, " << s->name << "!\",\n"
               << "  \"student\": " << s->toJson() << "\n"
               << "}";
            return ss.str();
        }
        return "{\"status\":\"ERROR\",\"message\":\"No student found with Roll Number '" + roll + "'. Please check the roll number or register a new profile.\"}";
    }

    // 7c. POST /api/students/register (Register a new student into C++ Hash Table)
    if (path == "/api/students/register" && method == "POST") {
        std::string roll = getJsonStr(body, "rollNo");
        std::string name = getJsonStr(body, "name");
        std::string dept = getJsonStr(body, "department");
        int year = (int)getJsonNum(body, "year");
        if (year <= 0) year = 1;
        std::string email = getJsonStr(body, "email");
        std::string contact = getJsonStr(body, "contactNo");

        Student newStudent(roll, name, dept, year, email, contact);
        std::string outMsg;
        bool ok = manager.registerStudent(newStudent, outMsg);
        if (ok) {
            std::ostringstream ss;
            ss << "{\n"
               << "  \"status\": \"SUCCESS\",\n"
               << "  \"message\": \"" << outMsg << "\",\n"
               << "  \"student\": " << newStudent.toJson() << "\n"
               << "}";
            return ss.str();
        } else {
            return "{\"status\":\"ERROR\",\"message\":\"" + outMsg + "\"}";
        }
    }

    // 8. GET /api/student (Single student profile by roll number)
    if (path == "/api/student" && method == "GET") {
        std::string roll = getQueryParam(query, "roll");
        Student* s = manager.getStudent(roll);
        if (s != nullptr) {
            return s->toJson();
        }
        return "{\"status\":\"ERROR\",\"message\":\"Student not found\"}";
    }

    // 9. POST /api/student/view (Record recently viewed item onto Stack)
    if (path == "/api/student/view" && method == "POST") {
        std::string roll = getJsonStr(body, "studentRollNo");
        std::string itemId = getJsonStr(body, "itemId");
        manager.recordItemView(roll, itemId);
        return "{\"status\":\"SUCCESS\"}";
    }

    // 10. GET /api/student/recent (Get recently viewed items from Stack)
    if (path == "/api/student/recent" && method == "GET") {
        std::string roll = getQueryParam(query, "roll");
        std::vector<ResourceItem> recent = manager.getRecentlyViewed(roll);
        std::ostringstream ss;
        ss << "[\n";
        for (size_t i = 0; i < recent.size(); ++i) {
            ss << recent[i].toJson();
            if (i + 1 < recent.size()) ss << ",\n";
        }
        ss << "\n]";
        return ss.str();
    }

    // 11. GET /api/requests (Community Wanted Board - Linked List)
    if (path == "/api/requests" && method == "GET") {
        std::vector<ResourceRequest> reqs = manager.getAllRequests();
        std::ostringstream ss;
        ss << "[\n";
        for (size_t i = 0; i < reqs.size(); ++i) {
            ss << reqs[i].toJson();
            if (i + 1 < reqs.size()) ss << ",\n";
        }
        ss << "\n]";
        return ss.str();
    }

    // 12. POST /api/requests/create (Post new request to Linked List)
    if (path == "/api/requests/create" && method == "POST") {
        std::string reqId = "REQ-" + std::to_string(time(nullptr) % 100000);
        ResourceRequest req(
            reqId,
            getJsonStr(body, "requesterRollNo"),
            getJsonStr(body, "requesterName"),
            getJsonStr(body, "requesterContact"),
            getJsonStr(body, "itemTitle"),
            getJsonStr(body, "category"),
            getJsonStr(body, "description"),
            getJsonStr(body, "urgency"),
            "Today"
        );
        std::string outMsg;
        bool ok = manager.createRequest(req, outMsg);
        if (ok) {
            return "{\"status\":\"SUCCESS\",\"message\":\"" + outMsg + "\",\"requestId\":\"" + reqId + "\"}";
        } else {
            return "{\"status\":\"ERROR\",\"message\":\"" + outMsg + "\"}";
        }
    }

    // 13. POST /api/requests/fulfill (Mark request as fulfilled)
    if (path == "/api/requests/fulfill" && method == "POST") {
        std::string reqId = getJsonStr(body, "requestId");
        std::string outMsg;
        bool ok = manager.fulfillRequest(reqId, outMsg);
        if (ok) {
            return "{\"status\":\"SUCCESS\",\"message\":\"" + outMsg + "\"}";
        } else {
            return "{\"status\":\"ERROR\",\"message\":\"" + outMsg + "\"}";
        }
    }

    // 14. POST /api/requests/delete (Cancel/delete a request)
    if (path == "/api/requests/delete" && method == "POST") {
        std::string reqId = getJsonStr(body, "requestId");
        std::string roll = getJsonStr(body, "requesterRollNo");
        std::string outMsg;
        bool ok = manager.deleteRequest(reqId, roll, outMsg);
        if (ok) {
            return "{\"status\":\"SUCCESS\",\"message\":\"" + outMsg + "\"}";
        } else {
            return "{\"status\":\"ERROR\",\"message\":\"" + outMsg + "\"}";
        }
    }

    // 15. GET /api/stats (Data Structures System Metrics)
    if (path == "/api/stats" && method == "GET") {
        return manager.getSystemStatsJson();
    }

    return "{\"status\":\"ERROR\",\"message\":\"Unknown endpoint\"}";
}

void HttpServer::sendResponse(SOCKET clientSocket,
                              int statusCode,
                              const std::string& contentType,
                              const std::string& content) {
    std::string statusText = "OK";
    if (statusCode == 204) statusText = "No Content";
    else if (statusCode == 404) statusText = "Not Found";
    else if (statusCode == 500) statusText = "Internal Server Error";

    std::ostringstream ss;
    ss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    ss << "Content-Type: " << contentType << "\r\n";
    ss << "Content-Length: " << content.length() << "\r\n";
    ss << "Connection: close\r\n";
    ss << "Access-Control-Allow-Origin: *\r\n";
    ss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    ss << "Access-Control-Allow-Headers: Content-Type\r\n";
    ss << "\r\n";
    ss << content;

    std::string response = ss.str();
    send(clientSocket, response.c_str(), (int)response.length(), 0);
}

void HttpServer::sendFile(SOCKET clientSocket,
                          const std::string& filepath,
                          const std::string& contentType) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        sendResponse(clientSocket, 404, "text/plain", "File not found: " + filepath);
        return;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    sendResponse(clientSocket, 200, contentType, content);
}
