#include <thread>
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cctype>
#include <mutex>
#include <condition_variable>
#include <array>

volatile char spam_key;

volatile boolean spam_enabled = false;
volatile boolean dance_enabled = false;

std::mutex spam_mtx;
std::mutex dance_mtx;

std::condition_variable spam_cv;
std::condition_variable dance_cv;

HWND hwnd;

void spam_loop()
{    
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.dwFlags = KEYEVENTF_SCANCODE;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    while (true)
    {
        if (!spam_enabled) 
        {
            std::unique_lock<std::mutex> lk(spam_mtx);
            while (!spam_enabled)
            {
                spam_cv.wait(lk);
            }
        }

        HWND this_fg = GetForegroundWindow();
        if (hwnd == this_fg)
            continue;

        int BASE_DELAY = 100;
        int VAR = 100;
        int delay = rand() % VAR + BASE_DELAY; 
        Sleep(delay);

        DWORD key = spam_key; 
        ip.ki.wVk = key;
        ip.ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC);

        ip.ki.dwFlags = KEYEVENTF_SCANCODE;
        SendInput(1, &ip, sizeof(INPUT));
        ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
    }
}

void dance_loop()
{
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.dwFlags = KEYEVENTF_SCANCODE;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    std::array<uint8_t, 5> key_list = { 'A', 'D', VK_UP, VK_DOWN , VK_SPACE};

    const int BASE_DELAY = 50;
    const int VAR = 100;

    while (true)
    {
        if (!dance_enabled) 
        {
            std::unique_lock<std::mutex> lk(dance_mtx);
            while (!dance_enabled)
            {
                dance_cv.wait(lk);
            }
        }

        HWND this_fg = GetForegroundWindow();
        if (hwnd == this_fg)
            continue;

        DWORD key = key_list[rand() % key_list.size()];
        ip.ki.wVk = key;
        ip.ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC);

        ip.ki.dwFlags = KEYEVENTF_SCANCODE;
        SendInput(1, &ip, sizeof(INPUT));
        uint32_t delay = rand() % VAR + BASE_DELAY; 
        Sleep(delay);
        ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));

        delay = rand() % VAR + BASE_DELAY; 
        Sleep(delay);
    }
}

int main()
{
    hwnd = GetForegroundWindow();

    std::thread spam(spam_loop);
    spam.detach();

    std::thread dance(dance_loop);
    dance.detach();

    boolean run = true;
    while (run)
    {
        std::cout << "Toggle one of the following: " << std::endl;
        std::cout << "1. Spam [" << (spam_enabled ? "On" : "Off") << "]" << std::endl; 
        std::cout << "2. Dance [" << (dance_enabled ? "On" : "Off") << "]" << std::endl; 
        std::cout << "3. Exit" << std::endl;

        std::string line;
        std::getline(std::cin, line);
        if (line.empty())
        {
            std::cout << "Invalid input" << std::endl;
            continue;
        }
        char input = line[0];

        if (input == '1')
        {
            std::unique_lock<std::mutex> lk(spam_mtx);
            if (spam_enabled)
            {
                spam_enabled = false;
            }
            else
            {
                std::cout << "Key to spam: ";
                std::getline(std::cin, line);
                if (line.empty())
                {
                    std::cout << "Invalid input." << std::endl;
                    continue;
                }
                spam_key = std::toupper(line[0]);
                std::cout << "Spamming " << spam_key << " will start when this terminal has lost focus." << std::endl;
                spam_enabled = true;
                spam_cv.notify_one();
            }
        }
        else if (input == '2')
        {
            std::unique_lock<std::mutex> lk(dance_mtx);
            if (dance_enabled)
            {
                dance_enabled = false;
            }
            else
            {
                std::cout << "Dancing will start when this terminal has lost focus." << std::endl;
                dance_enabled = true;
                dance_cv.notify_one();
            }
        }
        else if (input == '3')
        {
            run = false;
        }
        else 
        {
            std::cout << "invalid option.";
        }
    }
    return 0;
}