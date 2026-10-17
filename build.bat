@echo off
echo ========================================================
echo  PICT RESOURCE EXCHANGE - BUILD SCRIPT
echo ========================================================
echo Compiling C++ Backend Engine with Winsock2 support...

g++ -std=c++14 -O2 -Ibackend/include backend/src/Models.cpp backend/src/FileStorage.cpp backend/src/ExchangeManager.cpp backend/src/HttpServer.cpp backend/src/main.cpp -o pict_exchange.exe -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Compilation succeeded!
    echo Executable created: pict_exchange.exe
    echo.
    echo To run the server:
    echo   pict_exchange.exe
    echo.
    echo Then open your browser at:
    echo   http://localhost:8080/
) else (
    echo.
    echo [FAILED] Compilation failed. See error output above.
)
