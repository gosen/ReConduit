#ifndef __INFORMATION_CHUNK_MESSAGE_CONDUIT__HPP__
#define __INFORMATION_CHUNK_MESSAGE_CONDUIT__HPP__

#include "EmbeddedMessage.hpp"

#include <iostream>
#include <any>

namespace reconduits {

template<typename T>
class InformationChunk : public EmbeddedMessage<T>
{
public:

    explicit InformationChunk(T& m)
        : EmbeddedMessage<T>{ m } {}

    static void* operator new(std::size_t sz)
    {
        return getFromPool<sizeof(InformationChunk)>();
    }

    static void operator delete(void* p)
    {
        putToPool<sizeof(InformationChunk)>( p );
    }

};

}

#endif  // __INFORMATION_CHUNK_MESSAGE_CONDUIT__HPP__
