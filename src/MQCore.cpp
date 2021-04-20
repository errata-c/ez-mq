#include <ez/MQCore.hpp>
#include <exception>
#include <cassert>
#include <iostream>
#include <cinttypes>
#include <zmq.h>

#include <ez/serialize.hpp>
#include <ez/deserialize.hpp>

namespace ez {
	MQCore::MQCore()
		: ctx(nullptr)
		, deferSock(nullptr)
	{
		ctx = zmq_ctx_new();
		if (ctx == nullptr) {
			throw std::logic_error("Failed to create ZeroMQ context!");
		}

		int res = zmq_ctx_set(ctx, ZMQ_IO_THREADS, 0);
		assert(res == 0);

		deferSock = zmq_socket(ctx, ZMQ_PULL);
		if (deferSock == nullptr) {
			res = zmq_errno();
			switch (res) {
			case EINVAL:
			case EFAULT:
				throw std::logic_error("If you get this error message, there is a bug in the ez-mq library.");
				break;
			case EMFILE:
				throw std::logic_error("The total number of ZeroMQ sockets has been reached.");
				break;
			case ETERM:
				throw std::logic_error("The ZeroMQ context was terminated before the defer socket could be created!");
				break;
			default:
				throw std::logic_error("Some unknown kind of error with libzeromq.");
				break;
			}
		}
		else {
			res = zmq_bind(deferSock, "inproc://defer");
			if (res == -1) {
				throw std::logic_error("Failed to bind the defer socket to 'inproc://defer'");
			}
		}
	}
	MQCore::MQCore(MQCore&& other) noexcept 
		: ctx(other.ctx)
		, deferSock(other.deferSock)
	{}
	MQCore& MQCore::operator=(MQCore&& other) noexcept {
		ctx = other.ctx;
		deferSock = other.deferSock;
		other.ctx = nullptr;

		return *this;
	}
	MQCore::~MQCore() {
		if (deferSock) {
			int res = zmq_close(deferSock);
			assert(res == 0); // Should not fail, if it does the socket was not initialized correctly.
		}

		if (ctx) {
			int res = zmq_ctx_destroy(ctx);
			if (res != 0) {
				res = zmq_errno();

				std::cerr << "Failed to destroy ZeroMQ context with error message:\n";
				std::cerr << zmq_strerror(res) << '\n';

				assert(res != EFAULT);

				while (res == EINTR) {
					res = zmq_ctx_destroy(ctx);
					if (res != 0) {
						res = zmq_errno();
					}
				}
			}
			ctx = nullptr;
		}
	}

	void MQCore::runDeferred() {
		std::uint8_t arr[sizeof(void*)];

		int res = 0;
		while (res >= 0) {
			res = zmq_recv(deferSock, arr, sizeof(arr), ZMQ_DONTWAIT);
			if (res == sizeof(arr)) {
				const std::uint8_t* read = arr;
				std::function<void()>* ptr = (std::function<void()>*)deserialize::ptr(read, read + sizeof(nullptr));

				(*ptr)();

				delete ptr;
			}
		}
		res = zmq_errno();
		switch (res) {
		case EAGAIN: // No messages.
			break;
		case EINTR: // Signal interrupted delivery.
			break;
		case ETERM:
		case ENOTSUP:
		case EFSM:
		case ENOTSOCK:
			assert(false);
			break;
		}
	}

	void MQCore::defer(std::function<void()>&& func) {
		if (!func) {
			return;
		}

		void* sock = zmq_socket(ctx, ZMQ_PUSH);
		assert(sock != nullptr);

		int res = zmq_connect(sock, "inproc://defer");
		assert(res == 0);

		std::function<void()>* ptr = new std::function<void()>(std::move(func));
		assert(ptr != nullptr);
		
		std::uint8_t arr[sizeof(ptr)];
		serialize::ptr(ptr, arr, arr + sizeof(ptr));

		res = zmq_send(sock, arr, sizeof(arr), 0);
		assert(res != -1);

		res = zmq_close(sock);
		assert(res == 0);
	}

};