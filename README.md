# PICT RESOURCE EXCHANGE
> **Peer-to-Peer E-Commerce Campus Resource Sharing Platform**  
> *Pune Institute of Computer Technology (PICT), Dhankawadi, Pune*

---

## 📌 Project Overview
**PICT Resource Exchange** is a full-featured campus e-commerce store designed for engineering students to **borrow**, **lend**, and **request** academic tools and resources (e.g., engineering graphics mini-drafters, scientific calculators, reference textbooks, workshop aprons, and electronics lab kits).

Instead of relying on generic collections or STL black-box containers, the core exchange engine is built in **pure C++ using custom data structures implemented from scratch**:
1. **Hash Table** with DJB2 Hashing & Separate Chaining
2. **FIFO Queue** with dynamic node pointers
3. **LIFO Stack** with dynamic node pointers
4. **Singly Linked List** with dynamic node pointers

The engine serves a modern, responsive web storefront via a lightweight native C++ Windows Sockets (`ws2_32`) HTTP server, requiring **zero Node.js or external server dependencies**.

---

## 🧠 Data Structures Breakdown & Implementation

| Data Structure | Implementation File | Academic & Practical Justification | Time Complexity | How It Is Used in the Platform |
| :--- | :--- | :--- | :--- | :--- |
| **Hash Table** | `backend/include/HashTable.h` | Fast direct access without scanning entire inventory. | $O(1)$ Average Lookup / Insert / Delete | **Catalog & Student Indexing**: Maps `itemId` $\rightarrow$ `ResourceItem*` and `rollNo` $\rightarrow$ `Student*`. Uses DJB2 polynomial hash function with separate chaining collision resolution. |
| **FIFO Queue** | `backend/include/Queue.h` | Strict First-In, First-Out resource allocation fairness. | $O(1)$ Enqueue / Dequeue | **Resource Waiting List**: When a high-demand item (e.g. Mini-Drafter during exam week) is already borrowed, subsequent students are enqueued in order. When the current borrower returns the item, the student at the front of the queue is **automatically allocated** the resource. |
| **LIFO Stack** | `backend/include/Stack.h` | Reversing chronology and transactional rollback. | $O(1)$ Push / Pop / Peek | **Recently Viewed Shelf & Undo**: As students inspect items in the store, they are pushed onto their personal navigation stack (top = most recent). Also powers **Undo Last Action**, allowing a student to pop and roll back their last listed item. |
| **Linked List** | `backend/include/LinkedList.h` | Dynamic memory sizing without array reallocations. | $O(1)$ Head/Tail Insert, $O(N)$ Search | **Category Catalogs & Request Board**: Categorizes resources (e.g., Drawing Tools, Books, Aprons) and manages the community Wanted Request Board with seamless node additions and removals. |

---

## 🏗️ Architecture & Class Design

```
pict-resource-exchange/
├── backend/
│   ├── include/
│   │   ├── LinkedList.h        <- Custom generic Singly Linked List
│   │   ├── Queue.h             <- Custom generic FIFO Queue
│   │   ├── Stack.h             <- Custom generic LIFO Stack
│   │   ├── HashTable.h         <- Custom generic Hash Table (Separate Chaining)
│   │   ├── Models.h            <- ResourceItem, Student, ResourceRequest models
│   │   ├── FileStorage.h       <- JSON serialization and disk persistence
│   │   ├── ExchangeManager.h   <- Core business logic orchestrator
│   │   └── HttpServer.h        <- Lightweight native Winsock REST HTTP server
│   └── src/
│       ├── Models.cpp
│       ├── FileStorage.cpp
│       ├── ExchangeManager.cpp
│       ├── HttpServer.cpp
│       └── main.cpp            <- Application startup and console logs
├── frontend/
│   ├── index.html              <- Responsive E-Commerce storefront UI
│   ├── style.css               <- Modern dark glassmorphism styling
│   └── app.js                  <- Dynamic UI controller & API client
├── tests/
│   └── test_ds.cpp             <- Comprehensive unit test suite for all 4 DS
├── data/                       <- Seeded JSON storage (persists on disk)
│   ├── items.json
│   ├── students.json
│   └── requests.json
├── build.bat                   <- One-click Windows build script
└── README.md                   <- Documentation
```

---

## ⚡ How to Build & Run

### 1. Requirements
- Windows 10/11
- MinGW GCC (`g++`) with C++14 support

### 2. Build Main Application
Double-click `build.bat` or run in terminal:
```powershell
./build.bat
```
Alternatively, compile manually with:
```powershell
g++ -std=c++14 -O2 -Ibackend/include backend/src/*.cpp -o pict_exchange.exe -lws2_32
```

### 3. Run the Server
```powershell
./pict_exchange.exe
```

### 4. Open the E-Commerce Storefront
Open any web browser and visit:
```
http://localhost:8080/
```

---

## 🧪 Running Data Structure Unit Tests
To independently verify the custom data structures:
```powershell
g++ -std=c++14 -Itests tests/test_ds.cpp -o tests/test_ds.exe
./tests/test_ds.exe
```

Expected output:
```
[TEST] Running Singly Linked List tests...
  --> LinkedList PASSED!
[TEST] Running FIFO Queue (Waitlist) tests...
  --> FIFO Queue PASSED!
[TEST] Running LIFO Stack (Recently Viewed & Undo) tests...
  --> LIFO Stack PASSED!
[TEST] Running Hash Table (Separate Chaining) tests...
  --> Hash Table PASSED!

ALL DATA STRUCTURE UNIT TESTS PASSED SUCCESSFULLY!
```

---

## 🌐 REST API Endpoints

| Endpoint | Method | Description |
| :--- | :--- | :--- |
| `GET /api/items` | `GET` | Retrieve all items or filtered by `?query=...&category=...` |
| `GET /api/item?id=...` | `GET` | Direct $O(1)$ Hash Table lookup for item |
| `POST /api/items/lend` | `POST` | List a new item (adds to Hash Table & category Linked List) |
| `POST /api/items/borrow` | `POST` | Borrow item or enqueue into FIFO Waiting Queue |
| `POST /api/items/return` | `POST` | Return item (auto-advances next student from FIFO Queue) |
| `POST /api/undo` | `POST` | Pops action from LIFO Stack to undo last listing |
| `GET /api/students` | `GET` | Get all registered student profiles |
| `GET /api/student?roll=...` | `GET` | Get single student details |
| `GET /api/student/recent?roll=...` | `GET` | Retrieve recently viewed items from student's LIFO Stack |
| `POST /api/student/view` | `POST` | Push item view event onto student's LIFO Stack |
| `GET /api/requests` | `GET` | View all community requests (Linked List) |
| `POST /api/requests/create` | `POST` | Post new resource request to Linked List |
| `POST /api/requests/fulfill` | `POST` | Mark a request as fulfilled |
| `GET /api/stats` | `GET` | Real-time C++ Data Structure engine metrics |
