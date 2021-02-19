
if(NOT TARGET libzmq)
	find_dependency(ZeroMQ CONFIG)
endif()

if(NOT TARGET ez::serialize)
	find_dependency(ez-serialize CONFIG)
endif()