#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "../backend/include/LinkedList.h"
#include "../backend/include/Queue.h"
#include "../backend/include/Stack.h"
#include "../backend/include/HashTable.h"

void testLinkedList() {
    std::cout << "[TEST] Running Singly Linked List tests...\n";
    LinkedList<std::string> list;
    assert(list.size() == 0);
    assert(list.isEmpty());

    list.insertTail("ITEM_1");
    list.insertTail("ITEM_2");
    list.insertHead("ITEM_0");

    assert(list.size() == 3);
    assert(list.contains("ITEM_1"));
    assert(list.contains("ITEM_0"));
    assert(!list.contains("ITEM_99"));

    std::vector<std::string> v = list.toVector();
    assert(v.size() == 3);
    assert(v[0] == "ITEM_0");
    assert(v[1] == "ITEM_1");
    assert(v[2] == "ITEM_2");

    bool removed = list.remove("ITEM_1");
    assert(removed);
    assert(list.size() == 2);
    assert(!list.contains("ITEM_1"));

    list.clear();
    assert(list.size() == 0);
    assert(list.isEmpty());
    std::cout << "  --> LinkedList PASSED!\n\n";
}

void testQueue() {
    std::cout << "[TEST] Running FIFO Queue (Waitlist) tests...\n";
    Queue<std::string> q;
    assert(q.isEmpty());
    assert(q.size() == 0);

    // Enqueue 3 students into waitlist
    q.enqueue("31105"); // First
    q.enqueue("31142"); // Second
    q.enqueue("31201"); // Third

    assert(q.size() == 3);
    assert(!q.isEmpty());
    assert(q.front() == "31105");

    // Dequeue must preserve strict FIFO order
    std::string firstOut = q.dequeue();
    assert(firstOut == "31105");
    assert(q.size() == 2);
    assert(q.front() == "31142");

    std::string secondOut = q.dequeue();
    assert(secondOut == "31142");
    assert(q.size() == 1);

    std::string thirdOut = q.dequeue();
    assert(thirdOut == "31201");
    assert(q.isEmpty());

    std::cout << "  --> FIFO Queue PASSED!\n\n";
}

void testStack() {
    std::cout << "[TEST] Running LIFO Stack (Recently Viewed & Undo) tests...\n";
    Stack<std::string> st;
    assert(st.isEmpty());
    assert(st.size() == 0);

    // Push browsed items
    st.push("PICT-101");
    st.push("PICT-102");
    st.push("PICT-103");

    assert(st.size() == 3);
    assert(st.peek() == "PICT-103");

    // Vector export must be Top to Bottom (most recent first)
    std::vector<std::string> v = st.toVector();
    assert(v[0] == "PICT-103");
    assert(v[1] == "PICT-102");
    assert(v[2] == "PICT-101");

    // Pop returns most recently pushed (LIFO)
    std::string popped = st.pop();
    assert(popped == "PICT-103");
    assert(st.peek() == "PICT-102");
    assert(st.size() == 2);

    st.clear();
    assert(st.isEmpty());
    std::cout << "  --> LIFO Stack PASSED!\n\n";
}

void testHashTable() {
    std::cout << "[TEST] Running Hash Table (Separate Chaining) tests...\n";
    HashTable<std::string, std::string> ht(5); // small table to force collisions
    assert(ht.isEmpty());
    assert(ht.size() == 0);

    ht.insert("31105", "Rohan Shinde");
    ht.insert("31142", "Priya Patil");
    ht.insert("31201", "Amit Deshmukh");
    ht.insert("31350", "Tanvi Joshi");

    assert(ht.size() == 4);
    assert(ht.contains("31105"));
    assert(ht.contains("31142"));
    assert(!ht.contains("99999"));

    std::string name;
    assert(ht.search("31105", name));
    assert(name == "Rohan Shinde");

    assert(ht.search("31201", name));
    assert(name == "Amit Deshmukh");

    // Test in-place modification
    std::string* pVal = ht.find("31350");
    assert(pVal != nullptr);
    *pVal = "Tanvi Joshi (Updated)";

    assert(ht.search("31350", name));
    assert(name == "Tanvi Joshi (Updated)");

    // Test removal
    bool removed = ht.remove("31142");
    assert(removed);
    assert(ht.size() == 3);
    assert(!ht.contains("31142"));

    std::cout << "  --> Hash Table PASSED!\n\n";
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "  PICT RESOURCE EXCHANGE - DATA STRUCTURE TEST SUITE     \n";
    std::cout << "=========================================================\n\n";

    testLinkedList();
    testQueue();
    testStack();
    testHashTable();

    std::cout << "=========================================================\n";
    std::cout << "  ALL DATA STRUCTURE UNIT TESTS PASSED SUCCESSFULLY!     \n";
    std::cout << "=========================================================\n";
    return 0;
}
