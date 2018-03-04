#ifndef __CONDUIT_MESSAGE_TYPES__HPP__
#define __CONDUIT_MESSAGE_TYPES__HPP__

#include "SetupMessage.hpp"
#include "ReleaseMessage.hpp"
#include "AlertingMessage.hpp"
#include "InformationChunkMessage.hpp"

#include <variant>

namespace reconduits {

template<typename T>
using message_type = std::variant<
                                  Setup<T>*,
                                  Release<T>*,
                                  Alerting<T>*,
                                  InformationChunk<T>*
                                 >;

}



#endif //__CONDUIT_MESSAGE_TYPES__HPP__
