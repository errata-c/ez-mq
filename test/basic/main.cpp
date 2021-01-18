#include <fmt/core.h>
#include <thread>
#include <ez/MQCore.hpp>

int main() {
	ez::MQCore core;

	std::thread 
	one([&]() {
		std::this_thread::sleep_for(std::chrono::seconds(3));
		core.defer([]() {
				fmt::print("Hello, ");
			});
	}),
	two([&]() {
		std::this_thread::sleep_for(std::chrono::seconds(6));
		core.defer([]() {
				fmt::print("World!\n");
			});
	});

	one.join();
	two.join();

	core.runDeferred();

	return 0;
}