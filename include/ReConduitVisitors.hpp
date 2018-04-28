#ifndef __RECONDUIT_VISITORS__HPP__
#define __RECONDUIT_VISITORS__HPP__

#include <variant>
#include <type_traits>

namespace reconduits {

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

template<typename V>
constexpr auto dispatch_r(V v, auto&& f)
{
    return std::visit([ &f ](auto&& conduit_ptr) { return f( conduit_ptr ); }, v);
}

template<typename V, typename M>
constexpr auto doubleDispatch_r(V v, M m, auto&& f)
{
    return std::visit([ & ](auto&& conduit_ptr) {
        return std::visit([ & ](auto&& message_ptr) {
            return f( conduit_ptr, message_ptr );
        }, m);
    }, v);
}

template<class T> struct always_false : std::false_type {};

template<typename R, typename... U, typename V>
constexpr auto dispatchFor_r(V v, auto&& f)
{
    return std::visit([ &f ](auto&& conduit_ptr) {
        using T = std::decay_t<decltype(conduit_ptr)>;
        constexpr bool enable_for = ( std::is_same_v<T, U> || ... );
        if constexpr ( enable_for ) return f( conduit_ptr );
        else return R{};
    }, v);
}
}

#endif //__RECONDUIT_VISITORS__HPP__
