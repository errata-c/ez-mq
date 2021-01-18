#pragma once
#include <functional>

namespace ez {
	class MQCore {
	public:
		MQCore();
		MQCore(MQCore&&) noexcept;
		MQCore& operator=(MQCore&&) noexcept;
		~MQCore();

		void runDeferred();

		void defer(std::function<void()> && func);
	private:
		void* ctx, * deferSock;
	};
}