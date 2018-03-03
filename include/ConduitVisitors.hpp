#ifndef __CONDUIT_VISITORS__HPP__
#define __CONDUIT_VISITORS__HPP__

#include <variant>
#include <type_traits>

namespace conduits {

template<typename V>
constexpr void dispatch(V v, auto&& f)
{
    std::visit([ &f ](auto&& conduit_ptr) { f( conduit_ptr ); }, v);
}

template<typename V, typename M>
constexpr void doubleDispatch(V v, M m, auto&& f)
{
    std::visit([ & ](auto&& conduit_ptr) {
        std::visit([ & ](auto&& message_ptr) {
            f( conduit_ptr, message_ptr );
        }, m);
    }, v);
}

template<typename... U, typename V>
constexpr void dispatchFor(V v, auto&& f)
{
    std::visit([ &f ](auto&& conduit_ptr) {
        using T = std::decay_t<decltype(conduit_ptr)>;
        constexpr bool enable_for = ( std::is_same_v<T, U> || ... );
        if constexpr ( enable_for ) f( conduit_ptr );
    }, v);
}

}

#endif //__CONDUIT_VISITORS__HPP__
