set(network_SRCS
	NoNetwork.cpp
	Network.cpp
	ProtocolMessageFactory.cpp
	INetwork.cpp
)
add_library(network ${network_SRCS})
set_target_properties(network PROPERTIES FOLDER network)
target_link_libraries(network libs common)
