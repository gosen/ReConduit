#ifndef __CONDUIT_POOL__HPP__
#define __CONDUIT_POOL__HPP__

#include "ReConduitLogger.hpp"

#include <memory>
#include <forward_list>
#include <unordered_set>
#include <cstdint>

namespace reconduits {

template<std::size_t RequestedSize>
class ConduitsPool
{
public:

    constexpr ConduitsPool(std::size_t n = 0)
        : in_used_{}
        , free_{}
        , pool_{}
    {
        SPDLOG_DEBUG(getLogger(), "{}[{}] free is {}empty. in_use {} elements of size {}", __func__, static_cast<void*>(this), (free_.empty()?"":"NO "), in_used_.size(), RequestedSize);
        augmentPoolBy( n );
    }

    ~ConduitsPool()
    {
        for( auto p : pool_ ) std::free( p );
        SPDLOG_DEBUG(getLogger(), "{}[{}] free is {}empty. in_use {} elements of size {}", __func__, static_cast<void*>(this), (free_.empty()?"":"NO "), in_used_.size(), RequestedSize);
    }

    void* get()
    {
        SPDLOG_DEBUG(getLogger(), "{}[{}] free is {}empty. in_use {} elements of size {}", __func__, static_cast<void*>(this), (free_.empty()?"":"NO "), in_used_.size(), RequestedSize);
        if( free_.empty() ) augmentPoolBy();
        auto [it, inserted] = in_used_.insert( free_.front() );
        if( inserted ) {
            SPDLOG_DEBUG(getLogger(), "{}[{}] free is {}empty. in_use {} elements of size {}", __func__, static_cast<void*>(this), (free_.empty()?"":"NO "), in_used_.size(), RequestedSize);
            free_.pop_front();
            return *it;
        }
        throw std::bad_alloc();
    }

    void put(void* elem)
    {
        if( elem ) {
            SPDLOG_DEBUG(getLogger(), "{}[{}] free is {}empty. in_use {} elements of size {}", __func__, static_cast<void*>(this), (free_.empty()?"":"NO "), in_used_.size(), RequestedSize);
            free_.push_front( in_used_.extract( elem ).value() );
        }
    }

private:

    auto augmentPoolBy(std::size_t n = 1)
    {
        for( auto i = 0u; i < n; ++i ) free_.push_front( pool_.emplace_front( std::malloc( RequestedSize )) );
        SPDLOG_DEBUG(getLogger(), "{}[{}] free is {}empty. in_use {} elements of size {}", __func__, static_cast<void*>(this), (free_.empty()?"":"NO "), in_used_.size(), RequestedSize);
    }

    std::unordered_set<void*> in_used_;
    std::forward_list<void*>  free_;
    std::forward_list<void*>  pool_;
};

template<std::size_t RequestedSize>
inline ConduitsPool<RequestedSize>& getPoolInstance(std::size_t n = 0)
{
    static ConduitsPool<RequestedSize> pool{ n };
    return pool;
}

template<std::size_t RequestedSize>
inline void* getFromPool()
{
    auto value = getPoolInstance<RequestedSize>().get();
    SPDLOG_DEBUG(getLogger(), "{}[{}] value of size {}", __func__, value, RequestedSize);
    return value;
}

template<std::size_t RequestedSize>
inline void putToPool(void* value)
{
    SPDLOG_DEBUG(getLogger(), "{}[{}] value of size {}", __func__, value, RequestedSize);
    return getPoolInstance<RequestedSize>().put( value );
}

}

#endif //__CONDUIT_POOL__HPP__
