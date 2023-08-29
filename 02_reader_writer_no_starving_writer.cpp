#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <vector>

using namespace std::chrono_literals;


std::mutex m_;
std::mutex roomEmptyMutex_;
std::condition_variable cv;

bool roomEmpty_ = true;
int roomCounter_;

std::mutex newWriterComeMutex_;
std::condition_variable newWriterComeCV_;

int nWriterCome_ = 0;

void reader(int id) {
    std::this_thread::sleep_for(1s);
    { //turntile
        std::unique_lock<std::mutex> newWriterComeLock(newWriterComeMutex_);
        newWriterComeCV_.wait(newWriterComeLock, [&]{return nWriterCome_ == 0;});
        newWriterComeCV_.notify_one();
    }
    {
        std::lock_guard<std::mutex> l(m_);
        roomCounter_++;
        if (roomCounter_ == 1) // first in lock
        {   
            roomEmpty_ = false;
        }
    }    
    std::cout << "Reader id #" << id <<" join critical section" << std::endl;
    std::this_thread::sleep_for(2s);
    std::cout << "Leaving critical section" << std::endl;
    {
        std::lock_guard<std::mutex> l(m_);
        roomCounter_--;
        if (roomCounter_ == 0) // last out unlock
        {   
            roomEmpty_ = true;
            cv.notify_one();
        }
    }
    std::this_thread::sleep_for(1s);

}

void writer(int id) {
    std::this_thread::sleep_for(1s);

    //turntile
    {
        std::unique_lock<std::mutex> newWriterComeLock(newWriterComeMutex_);
        nWriterCome_++;
    }
    

    std::unique_lock<std::mutex> roomLock(roomEmptyMutex_);
    cv.wait(roomLock, [&]{return roomEmpty_;});


    std::cout << "Writer id #" << id <<" join critical section" << std::endl;
    std::this_thread::sleep_for(2s);
    std::cout << "Leaving critical section" << std::endl;
    cv.notify_one();

    {
        std::unique_lock<std::mutex> newWriterComeLock(newWriterComeMutex_);
        nWriterCome_--;
        newWriterComeCV_.notify_one();
    }

    std::this_thread::sleep_for(1s);

}
int main() {
    std::vector<std::thread> v_t;
    for (int i = 0; i < 5; i++) {
        v_t.emplace_back(reader, i);
        v_t.emplace_back(writer, i);
    }
    for (auto& t: v_t) {
        t.join();
    }
    return 0;
}

