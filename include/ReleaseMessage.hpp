#ifndef __RELEASE_MESSAGE_CONDUIT__HPP__
#define __RELEASE_MESSAGE_CONDUIT__HPP__

#include "EmbeddedMessage.hpp"

#include <iostream>
#include <type_traits>

namespace conduits {

template<typename T>
class Release : public EmbeddedMessage<T>
{
public:

    Release(T& m, Conduit* c)
        : EmbeddedMessage<T>{ m }
        , conduit_origin_{ c }
    {}

    auto getOrigin() const { return conduit_origin_; }

    static void* operator new(std::size_t sz)
    {
        return getFromPool<sizeof(Release)>();
    }

    static void operator delete(void* p)
    {
        putToPool<sizeof(Release)>( p );
    }

private:

    Conduit* conduit_origin_;
};

}

#endif  // __RELEASE_MESSAGE_CONDUIT__HPP__
