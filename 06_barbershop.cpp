#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <vector>
using namespace std::chrono_literals;

constexpr int maxCustomerInShop = 4; // total number of customers that can be in the shop: three in the waiting room and one in the chair.
int customerInShop = 0; // counts the number of customers in the shop; protected by the mutex m_

constexpr int barberChair = 1; //number of barber chair
int barberChairAvailable = barberChair; 

std::mutex m_;

std::mutex barberChairM_;
std::condition_variable barberChairCV_;

bool customerReady = true;

std::mutex barberchairReadyM_;
std::condition_variable barberchairReadyCV_;

bool barberChairReady = true;

std::mutex customerDoneM_;
std::mutex barberDoneM_;

bool barberDone = true;

std::condition_variable customerDoneCV_;
std::condition_variable barberDoneCV_;

void customer (int id) {
    bool isBalk = false;
    { // a customer walk to barbershop, if barbershop is full, customer balk. Else, enter and wait in waiting room
        std::lock_guard l(m_);
        if (customerInShop == maxCustomerInShop) {
            std::cout << "Shop is full. Balk.\n";
            return;
        }
        else {
            customerInShop += 1;
            std::cout << "New customer is entered.\n";
        }
    }
     std::cout << "customer " + std::to_string(id) + "  is waiting\n";
    {
        // customer is waiting in waiting room. Customer ask barber to take a sit at barber chair
        barberChairCV_.notify_one();
    }
    
    {
        std::unique_lock barberChairReadyLock(barberchairReadyM_);
        barberchairReadyCV_.wait(barberChairReadyLock);
    }

    std::cout << "getHairCut\n";
    
    { //customer is done
        customerDoneCV_.notify_one();
    }
    
    { //waiting for barber to cleanup
        std::unique_lock barberDoneLock(barberDoneM_);
        barberDoneCV_.wait(barberDoneLock);
    }
    { //customer is leaving
        std::lock_guard l(m_);
        customerInShop -= 1;
    }
    std::cout << "Customer " + std::to_string(id) + " is left\n";
    
}

void barber() {
    std::cout << "\tBarber\n";
    for (;;) {
        //waiting for customer
        {
            std::unique_lock barberChairLock(barberChairM_);
            barberChairCV_.wait(barberChairLock);
        }
        std::cout << "\tcustomer is sit on seat\n";
        {
            barberchairReadyCV_.notify_one();
        }
        // ready to cut hair
        std::cout << "\tCut hair\n";
        //waiting for customer is done
        {
            std::unique_lock customerDoneLock(customerDoneM_);
            customerDoneCV_.wait(customerDoneLock);
        }
        //cleanup done
        {
            barberDoneCV_.notify_one();
        }

    }
}
int main() {
    std::vector<std::thread> v_t;
    v_t.emplace_back(barber);

    for (int i = 0; i< 10; i++) {
        v_t.emplace_back(customer, i);
    }


    for (auto & t: v_t) {
        t.join();
    }
}