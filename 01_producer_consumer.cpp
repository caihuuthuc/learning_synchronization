#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <condition_variable>
#include <chrono>

using namespace std::chrono_literals;

std::mutex m;
std::condition_variable cv;
bool itemAvailable = false;


void consumer() {
	std::unique_lock lk(m);
	cv.wait(lk, []{return itemAvailable;});
	std::cout << "Consumer side. Processing items...." << std::endl;
	std::this_thread::sleep_for(2s);
	std::cout << "Consumer side. Processed items" << std::endl;
}

void producer() {
	std::cout << "Producer side." << std::endl;
	{
		std::lock_guard lk(m);
		std::cout << "Producing item" << std::endl;
		std::this_thread::sleep_for(2s);
		itemAvailable = true;
		std::cout << "Item is available" << std::endl;
		cv.notify_one();
	}
}

int main() {
	std::thread t_p(producer);
	std::thread t_c(consumer);
	t_p.join();
	t_c.join();

	return 0;
}