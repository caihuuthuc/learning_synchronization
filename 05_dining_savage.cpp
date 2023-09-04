#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <vector>
using namespace std::chrono_literals;

constexpr unsigned short maxServing = 10;
unsigned short numServing = maxServing;

std::mutex m_;
std::mutex emptyPotMutex_;
std::mutex fullPotMutex_;

std::condition_variable emptyPotCV_;
std::condition_variable fullPotCV_;

bool cookDone = false;
void cook() {
    for (;;) {
        std::unique_lock emptyPotLock(emptyPotMutex_);
        emptyPotCV_.wait(emptyPotLock, [&] {return numServing==0;});

        std::cout << "Cooking.\n";
        std::this_thread::sleep_for(2s);
        std::cout << "Cooking is done\n";
        cookDone = true;
        fullPotCV_.notify_all();
    }

}

void savage() {
    for (;;) {
        std::lock_guard savageLock(m_);
        if (numServing == 0) {
            std::cout << "Empty pot.\nWake up the cook\n";
            emptyPotCV_.notify_one();
            {
                std::cout << "Savage is waiting..\n";
                std::unique_lock fullPotLock(fullPotMutex_);
                fullPotCV_.wait(fullPotLock, [] {return cookDone;});
                cookDone = false;
                numServing = maxServing;
            }
        }
        std::cout << "Get serving from pot. Eating..\n";
        numServing -= 1;
    }
    // get mutex
    // if numServing == 0
    //  emptyPot.signal
    // fullPot.wait()
    // numServing = maxServing;
    // endif
    // eat -- decrease numServing
}
int main() {
    std::vector<std::thread> v_t;
    for (int i = 0; i < 10; i++) {
        v_t.emplace_back(savage);
    }
    v_t.emplace_back(cook);

    for (auto &t: v_t) {
        t.join();
    }
    return 0;
}