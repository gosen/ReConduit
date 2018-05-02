#pragma once

#include <variant>
#include <type_traits>

namespace mock_state_machine {

struct Listen;
struct SynSent;
struct SynReceived;
struct Established;
struct FinWait_1;
struct FinWait_2;
struct TimeWait;
struct Closing;
struct CloseWait;
struct LastAck;

struct Closed {
    Closed() {}
    explicit Closed(const Listen&) {}
    explicit Closed(const SynSent&) {}
    explicit Closed(const LastAck&) {}
    explicit Closed(const TimeWait&) {}
};

struct Listen {
    explicit Listen(const Closed&) {}
    explicit Listen(const SynReceived&) {}
};

struct SynSent {
    explicit SynSent(const Listen&) {}
    explicit SynSent(const Closed&) {}
};

struct SynReceived {
    explicit SynReceived(const Listen&) {}
    explicit SynReceived(const SynSent&) {}
};

struct Established {
    explicit Established(const SynSent&) {}
    explicit Established(const SynReceived&) {}
};

struct FinWait_1 {
    explicit FinWait_1(const SynReceived&) {}
    explicit FinWait_1(const Established&) {}
};

struct FinWait_2 {
    explicit FinWait_2(const FinWait_1&) {}
};

struct TimeWait {
    explicit TimeWait(const FinWait_1&) {}
    explicit TimeWait(const FinWait_2&) {}
    explicit TimeWait(const Closing&) {}
};

struct Closing {
    explicit Closing(const FinWait_1&) {}
};

struct CloseWait {
    explicit CloseWait(const Established&) {}
};

struct LastAck {
    explicit LastAck(const CloseWait&) {}
};

class Event
{
public:
    constexpr Event(
        bool ack,
        bool syn = false,
        bool fin = false,
        bool timeout = false,
        bool rst = false)
        : ack_{ ack }
        , syn_{ syn }
        , fin_{ fin }
        , rst_{ rst }
        , timeout_{ timeout }
    {}

private:

    friend class TCPStateMachine;

    template<typename T, typename S>
    constexpr bool is(S&& s) const noexcept { return std::get_if<T>( &s ); }

    constexpr auto toClose(auto&& s) const noexcept {
        return ( ( is<SynSent>( s ) || is<TimeWait>( s ) ) && timeout_ ) ||
                 ( is<LastAck>( s )                        && ack_     );
    }
    constexpr auto toListen(auto&& s) const noexcept { return rst_; }
    constexpr auto toSynSent(auto&& s) const noexcept { return syn_; }
    constexpr auto toSynReceived(auto&& s) const noexcept { return syn_; }
    constexpr auto toEstablished(auto&& s) const noexcept { return ack_; }
    constexpr auto toFinWait_1(auto&& s) const noexcept { return fin_; }
    constexpr auto toFinWait_2(auto&& s) const noexcept { return ack_; }
    constexpr auto toTimeWait(auto&& s) const noexcept
    {
        return ( is<FinWait_1>( s ) && fin_ && ack_ ) ||
               ( is<FinWait_2>( s ) && fin_         ) ||
               ( is<Closing>( s )   && ack_         );
    }
    constexpr auto toClosing(auto&& s) const noexcept { return fin_; }
    constexpr auto toCloseWait(auto&& s) const noexcept { return fin_; }
    constexpr auto toLastAck(auto&& s) const noexcept { return fin_; }

    bool ack_;
    bool syn_;
    bool fin_;
    bool rst_;
    bool timeout_;
};

template<typename T, typename... U>
constexpr auto enable_for() { return ( std::is_same_v<T, U> || ... ); }

class TCPStateMachine
{
    template<class T> struct always_false : std::false_type {};

    using states_type = std::variant< Closed, Listen, SynSent, SynReceived,
                                      Established, FinWait_1, FinWait_2,
                                      TimeWait, Closing, CloseWait, LastAck >;

public:

    template<typename S>
    bool is_current() const { return std::get_if<S>( &state_ ); }

    constexpr void transition(Event e)
    {
        std::visit([ & ](auto&& state)
        {
            using T = std::decay_t<decltype( state )>;
            if constexpr ( enable_for<T, Listen, SynSent, LastAck, TimeWait>() ) {
                if( e.toClose  ( state_ ) ) { state_ = Closed ( state ); return; }
            }

            if constexpr ( enable_for<T, Closed, SynReceived>() ) {
                if( e.toListen ( state_ ) ) { state_ = Listen ( state ); return; }
            }

            if constexpr ( enable_for<T, Listen, Closed>() ) {
                if( e.toSynSent( state_ ) ) { state_ = SynSent( state ); return; }
            }

            if constexpr ( enable_for<T, Listen, SynSent>() ) {
                if( e.toSynReceived( state_ ) ) { state_ = SynReceived( state ); return; }
            }

            if constexpr ( enable_for<T, SynSent, SynReceived>() ) {
                if( e.toEstablished( state_ ) ) { state_ = Established( state ); return; }
            }

            if constexpr ( enable_for<T, SynReceived, Established>() ) {
                if( e.toFinWait_1( state_ ) ) { state_ = FinWait_1( state ); return; }
            }

            if constexpr ( enable_for<T, FinWait_1>() ) {
                if( e.toFinWait_2( state_ ) ) { state_ = FinWait_2( state ); return; }
            }

            if constexpr ( enable_for<T, FinWait_1, FinWait_2, Closing>() ) {
                if( e.toTimeWait( state_ ) ) { state_ = TimeWait( state ); return; }
            }

            if constexpr ( enable_for<T, FinWait_1>() ) {
                if( e.toClosing( state_ ) ) { state_ = Closing( state ); return; }
            }

            if constexpr ( enable_for<T, Established>() ) {
                if( e.toCloseWait( state_ ) ) { state_ = CloseWait( state ); return; }
            }

            if constexpr ( enable_for<T, CloseWait>() ) {
                if( e.toLastAck( state_ ) ) { state_ = LastAck( state ); return; }
            }
        }, state_);
    }

private:

    states_type state_ = Closed{};
};

}
