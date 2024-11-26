#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <string>

// Global variables
std::atomic<bool> isRunning = false;  // Clicker active state
std::atomic<bool> isClickerActive = true; // Global program state
POINT coordinates = { 1025, 757 };
COLORREF lastColor;

// Get the color of a pixel on the screen
COLORREF getPixelColor(int x, int y) {
    HDC hdcScreen = GetDC(NULL);
    COLORREF color = GetPixel(hdcScreen, x, y);
    ReleaseDC(NULL, hdcScreen);
    return color;
}

// Simulate a mouse click
void performClick(int x, int y) {
    SetCursorPos(x, y);
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));
    Sleep(10); // Short delay to simulate click
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Clicker thread function
void clicker() {
    while (isClickerActive) {
        if (isRunning) {
            COLORREF currentColor = getPixelColor(coordinates.x, coordinates.y);
            if (currentColor != lastColor) {
                performClick(coordinates.x, coordinates.y);
                lastColor = currentColor;
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

// Function to update coordinates
void updateCoordinates() {
    int x, y;
    std::wcout << L"Введите координаты (X Y): ";
    std::wcin >> x >> y;
    coordinates.x = x;
    coordinates.y = y;
    lastColor = getPixelColor(coordinates.x, coordinates.y);
    std::wcout << L"Координаты обновлены: (" << coordinates.x << L", " << coordinates.y << L")\n";
}

// Main function
int main() {
    setlocale(LC_ALL, "Russian");

    std::wcout << L"Программа запущена.\n";
    std::wcout << L"Нажмите F2 для включения/приостановки кликера.\n";
    std::wcout << L"Введите 'u' для обновления координат.\n";
    std::wcout << L"Закройте окно, чтобы завершить.\n";

    // Initialize last color
    lastColor = getPixelColor(coordinates.x, coordinates.y);

    // Register F2 as a hotkey
    if (!RegisterHotKey(NULL, 1, 0, VK_F2)) {
        std::wcerr << L"Ошибка: не удалось зарегистрировать горячую клавишу F2.\n";
        return 1;
    }

    // Start clicker thread
    std::thread clickerThread(clicker);

    // Input handling thread
    std::thread inputThread([]() {
        while (isClickerActive) {
            std::wstring command;
            std::wcin >> command;
            if (command == L"u") {
                updateCoordinates();
            }
        }
        });

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == 1) {
            isRunning = !isRunning;
            if (isRunning) {
                std::wcout << L"Кликер включен.\n";
            }
            else {
                std::wcout << L"Кликер приостановлен.\n";
            }
        }
    }

    // Clean up
    isClickerActive = false;
    if (clickerThread.joinable()) {
        clickerThread.join();
    }
    if (inputThread.joinable()) {
        inputThread.join();
    }
    UnregisterHotKey(NULL, 1);
    std::wcout << L"Программа завершена.\n";
    return 0;
}
