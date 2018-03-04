#ifndef __SETUP_MESSAGE_RECONDUIT__HPP__
#define __SETUP_MESSAGE_RECONDUIT__HPP__

#include "EmbeddedMessage.hpp"

#include <iostream>
#include <type_traits>

namespace reconduits {

class Conduit;

template<typename T>
class Setup : public EmbeddedMessage<T>
{
public:

    Setup(T& m, Conduit* c)
        : EmbeddedMessage<T>{ m }
        , conduit_origin_{ c }
    {}

    auto getOrigin() const { return conduit_origin_; }

    static void* operator new(std::size_t sz)
    {
        return getFromPool<sizeof(Setup)>();
    }

    static void operator delete(void* p)
    {
        putToPool<sizeof(Setup)>( p );
    }

private:

    Conduit* conduit_origin_;
};

}

#endif  // __SETUP_MESSAGE_RECONDUIT__HPP__
