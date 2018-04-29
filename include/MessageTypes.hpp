#ifndef __RECONDUIT_MESSAGE_TYPES__HPP__
#define __RECONDUIT_MESSAGE_TYPES__HPP__

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

constexpr auto make_variant_message(auto&& msg)
{
    return message_type<embedded_t<decltype( msg )>>{ &msg };
}

template<typename MSG>
auto make_variant_message(auto&& msg, Conduit* conduit_origin)
{
    using msg_type = MSG;
    thread_local static msg_type* msg_ptr;
    if( msg_ptr ) {
        msg_ptr->~msg_type();
        new ( msg_ptr ) msg_type(msg.get(), conduit_origin);
    } else {
        msg_ptr = new msg_type(msg.get(), conduit_origin); // From Pool...
    }
    return message_type<embedded_t<decltype( msg )>>{ msg_ptr };
}

auto make_variant_setup_message(auto&& msg, Conduit* conduit_origin)
{
    using setup_type = Setup<embedded_t<decltype( msg )>>;
    return make_variant_message<setup_type>(msg, conduit_origin);
}

auto make_variant_release_message(auto&& msg, Conduit* conduit_origin)
{
    using release_type = Release<embedded_t<decltype( msg )>>;
    return make_variant_message<release_type>(msg, conduit_origin);
}

auto make_variant_alerting_message(auto&& msg, Conduit* conduit_origin)
{
    using alerting_type = Alerting<embedded_t<decltype( msg )>>;
    return make_variant_message<alerting_type>(msg, conduit_origin);
}

}

#endif //__RECONDUIT_MESSAGE_TYPES__HPP__
