#pragma once

#include "network/IProtocolMessage.h"
#include "cavepacker/shared/network/ProtocolMessageTypes.h"

namespace cavepacker {

PROTOCOL_CLASS_SIMPLE(AutoSolveStartedMessage, protocol::PROTO_AUTOSOLVE);
PROTOCOL_CLASS_SIMPLE(AutoSolveAbortedMessage, protocol::PROTO_AUTOSOLVEABORT);
PROTOCOL_CLASS_SIMPLE(UndoMessage, protocol::PROTO_UNDO);

}
