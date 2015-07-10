#pragma once

#include "network/IProtocolMessage.h"
#include "miniracer/shared/network/ProtocolMessageTypes.h"

namespace miniracer {

PROTOCOL_CLASS_SIMPLE(AutoSolveStartedMessage, protocol::PROTO_AUTOSOLVE);
PROTOCOL_CLASS_SIMPLE(AutoSolveAbortedMessage, protocol::PROTO_AUTOSOLVEABORT);
PROTOCOL_CLASS_SIMPLE(UndoMessage, protocol::PROTO_UNDO);

}
