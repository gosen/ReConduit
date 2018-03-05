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
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        emsg.append( "ConnectionsMux" );
        using T = std::decay_t<decltype(msg)>;
        if constexpr (std::is_same_v<T, reconduits::Release<Message>>)
            return reconduits::NextSide::b0;
        else
            return emsg.isUpLink() ? reconduits::NextSide::a : reconduits::NextSide::b;
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

    auto create(reconduits::Conduit* conduit_origin, auto&& msg) const
    {
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] is going to create new conduits.", static_cast<const void*>(this));
        auto& emsg = msg.get();
        return reconduits::Setup<reconduits::embedded_t<decltype(msg)>>{emsg, conduit_origin};
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
        SPDLOG_DEBUG(getLogger(), "ConnectionsLUAMux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        lua.set_function("getId", [ & ]{ emsg.append( "ConnectionsLUAMux" ); });
        lua.script("getId()");
        return emsg.isUpLink() ? reconduits::NextSide::b : reconduits::NextSide::a;
    }

private:
    sol::state lua;
};

}
