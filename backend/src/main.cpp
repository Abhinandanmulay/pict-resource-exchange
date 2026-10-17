#include <iostream>
#include <string>
#include "ExchangeManager.h"
#include "HttpServer.h"

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    std::cout << "\n";
    std::cout << "=====================================================================\n";
    std::cout << "        ____ ___ ____ _____   ____  _____ ____   ___  _   _ ____   ____ _____ \n";
    std::cout << "       |  _ \\_ _/ ___|_   _| |  _ \\| ____/ ___| / _ \\| | | |  _ \\ / ___| ____|\n";
    std::cout << "       | |_) | | |     | |   | |_) |  _| \\___ \\| | | | | | | |_) | |   |  _|  \n";
    std::cout << "       |  __/| | |___  | |   |  _ <| |___ ___) | |_| | |_| |  _ <| |___| |___ \n";
    std::cout << "       |_|  |___\\____| |_|   |_| \\_\\_____|____/ \\___/ \\___/|_| \\_\\\\____|_____|\n";
    std::cout << "                   PUNE INSTITUTE OF COMPUTER TECHNOLOGY (PICT)        \n";
    std::cout << "=====================================================================\n";
    std::cout << "[DATA STRUCTURES ACTIVE IN ENGINE]:\n";
    std::cout << "  1. HASH TABLE   -> O(1) Student Roll No & Item Catalog ID indexing (DJB2 + Chaining)\n";
    std::cout << "  2. FIFO QUEUE   -> High-demand resource waiting lists with fair auto-allocation\n";
    std::cout << "  3. LIFO STACK   -> Recently viewed items shelf & action rollback (Undo)\n";
    std::cout << "  4. LINKED LIST  -> Dynamic category catalogs & student request board\n";
    std::cout << "=====================================================================\n\n";

    try {
        ExchangeManager manager("data");
        HttpServer server(port, manager);

        std::cout << "[INIT] Server initialized successfully on port " << port << ".\n";
        std::cout << "[INFO] Open your web browser and visit:\n";
        std::cout << "       --> http://localhost:" << port << "/\n\n";
        std::cout << "[INFO] Press Ctrl+C in this console window to gracefully stop.\n\n";

        server.start();
    } catch (const std::exception& ex) {
        std::cerr << "[CRITICAL ERROR] " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
