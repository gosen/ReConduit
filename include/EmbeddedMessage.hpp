#ifndef __EMBEDDED_MESSAGE_CONDUIT__HPP__
#define __EMBEDDED_MESSAGE_CONDUIT__HPP__

#include "ConduitPool.hpp"
#include <type_traits>

namespace conduits {

template<typename T>
using embedded_t = typename std::decay_t<T>::value_type;

template<typename T>
class EmbeddedMessage
{
public:

    using value_type = T;

    explicit EmbeddedMessage(T& em) : embedded_message_{ em } {}

    const T& get() const { return embedded_message_; }
    T& get() { return embedded_message_; }

private:

    T& embedded_message_;
};

}

#endif  // __EMBEDDED_MESSAGE_CONDUIT__HPP__
