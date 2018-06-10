#pragma once

#include <variant>
#include <type_traits>

namespace mock_state_machine {

inline auto getLogger(std::string logger_name = "reconduit_test_fsm")
{
    static auto logger{ spdlog::stdout_color_mt( logger_name ) };
    spdlog::set_level(spdlog::level::warn);
    return logger;
}

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

template<typename FROM, typename TO>
void print_transist(TO to)
{
    std::cout << "Transist from \""<< FROM::name() << "\" to \""<< to->name() <<"\" state\n";
}

struct Closed {
    Closed() {}
    explicit Closed(const Listen&) { print_transist<Listen>(this); }
    explicit Closed(const SynSent&) { print_transist<SynSent>(this); }
    explicit Closed(const LastAck&) { print_transist<LastAck>(this); }
    explicit Closed(const TimeWait&) { print_transist<TimeWait>(this); }
    static const char* name() { return "Closed"; }
};

struct Listen {
    explicit Listen(const Closed&) { print_transist<Closed>(this); }
    explicit Listen(const SynReceived&) { print_transist<SynReceived>(this); }
    static const char* name() { return "Listen";}
};

struct SynSent {
    explicit SynSent(const Listen&) { print_transist<Listen>(this); }
    explicit SynSent(const Closed&) { print_transist<Closed>(this); }
    static const char* name() { return "SynSent";}
};

struct SynReceived {
    explicit SynReceived(const Listen&) { print_transist<Listen>(this); }
    explicit SynReceived(const SynSent&) { print_transist<SynSent>(this); }
    static const char* name() { return "SynReceived";}
};

struct Established {
    explicit Established(const SynSent&) { print_transist<SynSent>(this); }
    explicit Established(const SynReceived&) { print_transist<SynReceived>(this); }
    static const char* name() { return "Established";}
};

struct FinWait_1 {
    explicit FinWait_1(const SynReceived&) { print_transist<SynReceived>(this); }
    explicit FinWait_1(const Established&) { print_transist<Established>(this); }
    static const char* name() { return "FinWait_1";}
};

struct FinWait_2 {
    explicit FinWait_2(const FinWait_1&) { print_transist<FinWait_1>(this); }
    static const char* name() { return "FinWait_2";}
};

struct TimeWait {
    explicit TimeWait(const FinWait_1&) { print_transist<FinWait_1>(this); }
    explicit TimeWait(const FinWait_2&) { print_transist<FinWait_2>(this); }
    explicit TimeWait(const Closing&) { print_transist<Closing>(this); }
    static const char* name() { return "TimeWait";}
};

struct Closing {
    explicit Closing(const FinWait_1&) { print_transist<FinWait_1>(this); }
    static const char* name() { return "Closing";}
};

struct CloseWait {
    explicit CloseWait(const Established&) { print_transist<Established>(this); }
    static const char* name() { return "CloseWait";}
};

struct LastAck {
    explicit LastAck(const CloseWait&) { print_transist<CloseWait>(this); }
    static const char* name() { return "LastAck";}
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
    constexpr auto toSynReceived(auto&& s) const noexcept { return syn_ && ack_; }
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
    constexpr bool is_current() const { return std::get_if<S>( &state_ ); }

    constexpr auto transition(Event e)
    {
        return std::visit([ & ](auto&& state)
        {
            auto prev_state = *this;
            using T = std::decay_t<decltype( state )>;
            if constexpr ( enable_for<T, Listen, SynSent, LastAck, TimeWait>() ) {
                if( e.toClose  ( state_ ) ) { state_ = Closed ( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, Closed, SynReceived>() ) {
                if( e.toListen ( state_ ) ) { state_ = Listen ( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, Listen, Closed>() ) {
                if( e.toSynSent( state_ ) ) { state_ = SynSent( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, Listen, SynSent>() ) {
                if( e.toSynReceived( state_ ) ) { state_ = SynReceived( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, SynSent, SynReceived>() ) {
                if( e.toEstablished( state_ ) ) { state_ = Established( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, SynReceived, Established>() ) {
                if( e.toFinWait_1( state_ ) ) { state_ = FinWait_1( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, FinWait_1>() ) {
                if( e.toFinWait_2( state_ ) ) { state_ = FinWait_2( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, FinWait_1, FinWait_2, Closing>() ) {
                if( e.toTimeWait( state_ ) ) { state_ = TimeWait( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, FinWait_1>() ) {
                if( e.toClosing( state_ ) ) { state_ = Closing( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, Established>() ) {
                if( e.toCloseWait( state_ ) ) { state_ = CloseWait( state ); return prev_state; }
            }

            if constexpr ( enable_for<T, CloseWait>() ) {
                if( e.toLastAck( state_ ) ) { state_ = LastAck( state ); return prev_state; }
            }

            return prev_state;

        }, state_);
    }

private:

    states_type state_ = Closed{};
};

}
