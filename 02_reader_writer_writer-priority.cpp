// The idea is
// use two mutex: noReader and noWriter
// if noReader is unlocked, reader thread can come into critical section
// if noWriter is unlocked, writer thread can come into critical section
// the reader thread only owns the noWriter mutex. The meaning is, one or more reader can come into critical section at a same time and do not allow writer thread enters.
// the writer thread owns both noReader and noWriter mutex. The mearning is, only one writer can come into critcal section at a particular time, any is disallowed.


// In implementation side,
// We create two running room
// One for reader and another for writer

// A reader thread have to lock noReader mutex to enter the running room of reader.
// In reader running room, the reader thread lock the noWriter to disallow writer thread enter.

// A writer thread have to lock noWriter mutex to enter the running room of writer
// In writer running room, the writer thread lock the noReader to disallow reader thread enter.
// By owning both noReader and noWriter mutexes, only one writer can come into critical section at a particular time.

#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <vector>

using namespace std::chrono_literals;

std::mutex m_;
std::mutex noReaderMutex_;
std::mutex noWriterMutex_;


std::condition_variable noWriterSwitchCV_;

std::condition_variable noReaderSwitchCV_;

bool noWriter_ = false;
bool noReader_ = false;

unsigned int numReader_ = 0;
unsigned int numWriter_ = 0;

void reader(int id) {
    std::this_thread::sleep_for(1s);
    
    std::cout << ("Reader #" + std::to_string(id) + " is coming" + "\n");

    { 
        std::unique_lock noReaderLock(noReaderMutex_);
        noReaderSwitchCV_.wait(noReaderLock, [] {return !noReader_;});
        {
            std::lock_guard l(m_);
            numReader_++;
            if (numReader_ == 1) {
                noWriter_ = true; // if it is the first one came, lock the writer thread
            }
        }
        
    }

    std::cout << ("Reader #" + std::to_string(id) + " entering critical section...\n");
    std::this_thread::sleep_for(5s);
    std::cout << ("Reader #" + std::to_string(id) + " left critical section.\n");

    {
        std::lock_guard l(m_);
        numReader_--;
        if (numReader_ == 0) {
            noWriter_ = false;
            if (numWriter_ > 0) { //at least a writer is waiting
                noWriterSwitchCV_.notify_all();
            }
            if (!noReader_) { 
                noReaderSwitchCV_.notify_one();
            }
        }
    }

    std::this_thread::sleep_for(1s);

    // noReader.wait()
    //    readLightSwitch.lock(noWriter) // first reader in, lock noWriter
    // noReader.signal()
    // ... critical section ...
    // readLightSwitch.unlock(noWriter) // last reader out, unlock noWriter
}


void writer(int id) {
    std::this_thread::sleep_for(1s);
    std::cout << ("Writer #" + std::to_string(id) + " is coming" + "\n");
    
    {
        std::lock_guard l(m_);
        numWriter_ += 1;
        if (numWriter_ == 1) {
            noReader_ = true;
        }
    }
    std::lock_guard noReaderLock(noReaderMutex_);
    
    {
        std::unique_lock noWriterLock(noWriterMutex_);
        noWriterSwitchCV_.wait(noWriterLock, [] {return !noWriter_;});

        std::cout << ("Writer #" + std::to_string(id) + " entering critical section...\n");
        std::this_thread::sleep_for(5s);
    }


    {
        std::lock_guard l(m_);
        std::cout << ("Writer #" + std::to_string(id) + " left critical section.\n");

        numWriter_ -= 1;
        if (numWriter_ == 0) {
            noReader_ = false;
            noReaderSwitchCV_.notify_one();
        } else {
            noWriterSwitchCV_.notify_one();
        }
    }

    std::this_thread::sleep_for(1s);

    // writeLightSwitch.lock(noReader) // first writer in, lock noReader
    // noWriter.wait() //exclusive mutual access to critical section
    // ... critical section ...
    // noWriter.signal()
    // writerLightSwitch.unlock(noReader)
}


int main() {
    std::vector<std::thread> v_t;
    for (int i = 0; i < 5; i++) {
        v_t.emplace_back(reader, i);
        v_t.emplace_back(writer, i);
    }
    for (auto & i: v_t) {
        i.join();
    }

    return 0;
}