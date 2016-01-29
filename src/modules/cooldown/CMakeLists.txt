set(cooldown_SRCS
	CooldownMgr.h CooldownMgr.cpp
	CooldownType.h
	CooldownTriggerState.h
)

add_library(cooldown ${cooldown_SRCS})
set_target_properties(cooldown PROPERTIES FOLDER cooldown)
cp_target_link_libraries(TARGET cooldown LIBS common)
