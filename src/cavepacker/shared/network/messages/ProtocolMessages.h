#pragma once

#include "engine/common/network/IProtocolMessage.h"
#include "cavepacker/shared/network/ProtocolMessageTypes.h"

PROTOCOL_CLASS_SIMPLE(AutoSolveStartedMessage, protocol::PROTO_AUTOSOLVE);
PROTOCOL_CLASS_SIMPLE(AutoSolveAbortedMessage, protocol::PROTO_AUTOSOLVEABORT);
