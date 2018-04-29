#pragma once

#include "ReConduitTypesGenerators.hpp"
#include "MockLogger.hpp"
#include "MockMessage.hpp"

#include "sol.hpp"

#include <unordered_map>
#include <type_traits>
#include <string>
#include <sstream>

namespace mock_conduits {

//////////////////////////////////////
// Muxes
//////////////////////////////////////

class ConnectionsMux
{
public:

    using key_type   = int;
    using value_type = reconduits::Conduit*;

    constexpr auto accept(auto&& msg)
    {
        using namespace reconduits;
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        emsg.append( "ConnectionsMux" );
        using T = std::decay_t<decltype(msg)>;
        if constexpr (std::is_same_v<T, reconduits::Release<Message>>)
            return std::pair{ NextSide::b0, make_variant_message( msg ) };
        else
            return std::pair{ NextSide::b,  make_variant_message( msg ) };
    }

    auto find(auto&& msg) const
    {
        auto it = mux_table_.find( msg.get().getId() );
        bool found = it != mux_table_.cend();
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{0:p}] has{1}found key {2}.", static_cast<const void*>(this), (found ? " ":" NOT "), msg.get().getId());
        return std::pair{(found ? it->second : nullptr), found};
    }

    auto insert(auto&& key, reconduits::Conduit& c)
    {
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] is going to connect new conduits.", static_cast<void*>(this));
        auto [it, inserted] = mux_table_.emplace(key, &c);
        if( inserted ) return it->second;
        else {
            getLogger()->warn("The new connection for key {} was not possible.", key);
            return static_cast<reconduits::Conduit*>( nullptr );
        }
    }

    auto setup(auto&& msg, reconduits::Conduit* conduit_origin) const
    {
        using namespace reconduits;
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] is going to create new conduits.", static_cast<const void*>(this));
        return make_variant_setup_message(msg, conduit_origin);
    }

    auto erase(auto&& key)
    {
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] is going to release connected conduits.", static_cast<void*>(this));
        auto it = mux_table_.find( key );
        if( it != mux_table_.cend() ) {
            mux_table_.erase( it );
            return it->second;
        } else
            return static_cast<reconduits::Conduit*>( nullptr );
    }

protected:

    using mux_table_type = std::unordered_map<key_type, reconduits::Conduit*>;
    mux_table_type mux_table_;
};

class ConnectionsLUAMux : public ConnectionsMux
{
public:
    constexpr auto accept(auto&& msg)
    {
        using namespace reconduits;
        SPDLOG_DEBUG(getLogger(), "ConnectionsLUAMux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        lua.set_function("getId", [ & ]{ emsg.append( "ConnectionsLUAMux" ); });
        lua.script("getId()");
        return std::pair{ (emsg.isUpLink() ? NextSide::b : NextSide::a), make_variant_message( msg ) };
    }

private:
    sol::state lua;
};

}
