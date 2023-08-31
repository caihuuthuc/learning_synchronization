#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <stdlib.h>

#define _AGENT_PRINT_
#define _PUSHER_PRINT_
#define _SMOKER_PRINT_

// #define _AGENT_DEBUG_
// #define _PUSHER_DEBUG_

using namespace std::chrono_literals;

using std::srand;

std::mutex m_;

std::mutex agentMutex_;
std::condition_variable agentCV_;

int agent_turn = 0;

std::mutex tobacco_;
std::mutex paper_;
std::mutex match_;

std::condition_variable tobaccoCV_;
std::condition_variable paperCV_;
std::condition_variable matchCV_;

std::mutex smokerHavingTobaccoMutex_;
std::mutex smokerHavingPaperMutex_;
std::mutex smokerHavingMatchMutex_;

std::condition_variable smokerHavingTobaccoCV_;
std::condition_variable smokerHavingMatchCV_;
std::condition_variable smokerHavingPaperCV_;

unsigned int numPaperProduced = 0;
unsigned int numMatchProduced = 0;
unsigned int numTobaccoProduced = 0;

unsigned int numPaperOnTable = 0;
unsigned int numMatchOnTable = 0;
unsigned int numTobaccoOnTable = 0;

bool smokerWithMatchCond = false;
bool smokerWithPaperCond = false;
bool smokerWithTobaccoCond = false;

// agent code
void agent_producing_paper_and_tobacco() {
    for (;;) {
        #ifdef _AGENT_DEBUG_
            std::cout << "Agent is waiting to produce paper and tobacco\n";
        #endif
        
        std::unique_lock agentLock(agentMutex_);
        agentCV_.wait(agentLock, [] {return agent_turn == 0;});
        #ifdef _AGENT_DEBUG_
            std::cout << "Agent is producing paper and tobacco\n";
        #endif
        
        numPaperProduced += 1;
        numTobaccoProduced  += 1;

        #ifdef _AGENT_PRINT_
            std::cout << "Agent produced paper and tobacco\n";
            std::cout << (    "numMatchProduced: " + std::to_string(numMatchProduced) 
                            + " numPaperProduced: " + std::to_string(numPaperProduced) 
                            + " numTobaccoProduced: " + std::to_string(numTobaccoProduced) 
                            + "\n\n");
        #endif

        paperCV_.notify_one();
        tobaccoCV_.notify_one();

        agent_turn += 1;
        agent_turn %= 3;
        std::this_thread::sleep_for(5s);
        agentCV_.notify_all();
    }    
}


void agent_producing_tobacco_and_match() {
    for (;;) {
        #ifdef _AGENT_DEBUG_
            std::cout << "Agent is waiting to produce tobacco and match\n";
        #endif
        
        std::unique_lock agentLock(agentMutex_);
        agentCV_.wait(agentLock, [] {return agent_turn == 1;});
        #ifdef _AGENT_DEBUG_
            std::cout << "Agent is producing tobacco and match\n";
        #endif
        

        numTobaccoProduced  += 1;
        numMatchProduced  += 1;

        #ifdef _AGENT_PRINT_
            std::cout << "Agent produced tobacco and match\n";
            std::cout << (    "numMatchProduced: " + std::to_string(numMatchProduced) 
                            + " numPaperProduced: " + std::to_string(numPaperProduced) 
                            + " numTobaccoProduced: " + std::to_string(numTobaccoProduced) 
                            + "\n\n");
        #endif
        tobaccoCV_.notify_one();
        matchCV_.notify_one();

        agent_turn += 1;
        agent_turn %= 3;
        std::this_thread::sleep_for(5s);
        agentCV_.notify_all();
    }    
}


void agent_producing_match_and_paper() {
    for (;;) {
        #ifdef _AGENT_DEBUG_
            std::cout << "Agent is waiting to produce match and paper\n";
        #endif
        std::unique_lock agentLock(agentMutex_);
        agentCV_.wait(agentLock, [] {return agent_turn == 2;});
        #ifdef _AGENT_DEBUG_
            std::cout << "Agent is producing match and paper\n";
        #endif
        

        numMatchProduced  += 1;
        numPaperProduced  += 1;
        #ifdef _AGENT_PRINT_
            std::cout << "Agent produced match and paper\n";
            std::cout << (    "numMatchProduced: " + std::to_string(numMatchProduced) 
                            + " numPaperProduced: " + std::to_string(numPaperProduced) 
                            + " numTobaccoProduced: " + std::to_string(numTobaccoProduced) 
                            + "\n\n");
        #endif
        matchCV_.notify_one();
        paperCV_.notify_one();

        agent_turn += 1;
        agent_turn %= 3;
        std::this_thread::sleep_for(5s);
        agentCV_.notify_all();
    }    
}


// pusher code
void pusher_receiving_paper() {
    for (;;) {
        std::unique_lock PusherPaperLock(paper_);
        paperCV_.wait(PusherPaperLock, []{return numPaperProduced > 0;});
        
        #ifdef _PUSHER_PRINT_
                std::cout << "\tPusher received paper\n";
        #endif

        numPaperProduced -= 1;

        std::lock_guard l(m_);
        if (numMatchOnTable > 0) {
            numMatchOnTable -= 1;
            smokerWithTobaccoCond = true;
            smokerHavingTobaccoCV_.notify_one();
        } else if (numTobaccoOnTable > 0) {
            numTobaccoOnTable -= 1;
            smokerWithMatchCond = true;
            smokerHavingMatchCV_.notify_one();
        } else {
            numPaperOnTable += 1;

        }
    }
}


void pusher_receiving_tobacco() {
    for (;;) {
        std::unique_lock PusherTobaccoLock(tobacco_); 
        tobaccoCV_.wait(PusherTobaccoLock, []{return numTobaccoProduced > 0;});

        #ifdef _PUSHER_PRINT_
                std::cout << "\tPusher received tobacco\n";
        #endif
        
        numTobaccoProduced -= 1;

        std::lock_guard l(m_);
        if (numMatchOnTable > 0) {
            numMatchOnTable -= 1;
            smokerWithPaperCond = true;
            smokerHavingPaperCV_.notify_one();
        } else if (numPaperOnTable > 0) {
            numPaperOnTable -= 1;
            smokerWithMatchCond = true;
            smokerHavingMatchCV_.notify_one();
        } else {
            numTobaccoOnTable += 1;
        }
    }
}


void pusher_receiving_match() {
    for (;;) {
        std::unique_lock PusherMatchLock(match_);
        matchCV_.wait(PusherMatchLock, []{return numMatchProduced > 0;});

        #ifdef _PUSHER_PRINT_
            std::cout << "\tPusher received match\n";
                
        #endif


        numMatchProduced -= 1;

        std::lock_guard l(m_);
        if (numTobaccoOnTable > 0) {
            numTobaccoOnTable -= 1;
            smokerWithPaperCond = true;
            smokerHavingPaperCV_.notify_one();
        } else if (numPaperOnTable > 0) {
            numPaperOnTable -= 1;
            smokerWithTobaccoCond = true;
            smokerHavingTobaccoCV_.notify_one();
        } else {
            numMatchOnTable += 1;
        }
    }
}


void smoker_with_match() {
    for (;;) {
        std::this_thread::sleep_for(100ms);
        std::unique_lock smokerMatchLock(smokerHavingMatchMutex_);

        smokerHavingMatchCV_.wait(smokerMatchLock, []{return smokerWithMatchCond;});

        smokerWithMatchCond = false;
        #ifdef _SMOKER_PRINT_
            std::cout << ("\t\tSmoker with match is making cigarette\n");
            std::cout << ("\t\tSmoker with match is smoking\n");
        #endif
    }
}


void smoker_with_tobacco() {
    for (;;) {
        std::this_thread::sleep_for(100ms);
        std::unique_lock smokerTobaccoLock(smokerHavingTobaccoMutex_);

        smokerHavingTobaccoCV_.wait(smokerTobaccoLock, []{return smokerWithTobaccoCond;});
        smokerWithTobaccoCond = false;

        #ifdef _SMOKER_PRINT_
            std::cout << ("\t\tSmoker with tobacco is making cigarette\n");
            std::cout << ("\t\tSmoker with tobacco is smoking\n");
        #endif
    }
}


void smoker_with_paper() {
    for (;;) {
        std::this_thread::sleep_for(100ms);
        std::unique_lock smokerPaperLock(smokerHavingPaperMutex_);

        smokerHavingPaperCV_.wait(smokerPaperLock, []{return smokerWithPaperCond;});
        smokerWithPaperCond = false;

        #ifdef _SMOKER_PRINT_
            std::cout << ("\t\tSmoker with paper is making cigarette\n");
            std::cout << ("\t\tSmoker with paper is smoking\n");
        #endif
    }
}


int main() {
    std::vector<std::thread> v_t;

    v_t.emplace_back(agent_producing_paper_and_tobacco);
    v_t.emplace_back(agent_producing_tobacco_and_match);
    v_t.emplace_back(agent_producing_match_and_paper);

    v_t.emplace_back(pusher_receiving_match);
    v_t.emplace_back(pusher_receiving_paper);
    v_t.emplace_back(pusher_receiving_tobacco);

    v_t.emplace_back(smoker_with_match);
    v_t.emplace_back(smoker_with_tobacco);
    v_t.emplace_back(smoker_with_paper);

    for (auto & t: v_t) {
        t.join();
    }

    return 0;
}

