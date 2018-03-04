#ifndef __ALERTING_MESSAGE_RECONDUIT__HPP__
#define __ALERTING_MESSAGE_RECONDUIT__HPP__

#include "EmbeddedMessage.hpp"

#include <iostream>
#include <type_traits>

namespace reconduits {

template<typename T>
class Alerting : public EmbeddedMessage<T>
{
public:

    Alerting(T& m, Conduit* c)
        : EmbeddedMessage<T>{ m }
        , conduit_origin_{ c }
    {}

    static void* operator new(std::size_t sz)
    {
        return getFromPool<sizeof(Alerting)>();
    }

    static void operator delete(void* p)
    {
        putToPool<sizeof(Alerting)>( p );
    }

private:

    Conduit* conduit_origin_;
};

}

#endif  // __ALERTING_MESSAGE_RECONDUIT__HPP__
