set(network_SRCS
	NoNetwork.cpp
	Network.cpp
	ProtocolMessageFactory.cpp
	INetwork.cpp
)
add_library(network ${network_SRCS})
set_target_properties(network PROPERTIES FOLDER network)
cp_target_link_libraries(TARGET network LIBS common SDL2_net)
