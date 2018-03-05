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
// Factories
//////////////////////////////////////

class TCPConnectionFactory
{
public:

    constexpr reconduits::Conduit* accept(auto&& msg, reconduits::Conduit* a, reconduits::Conduit* b)
    {
        using T = std::decay_t<decltype(msg)>;
        if      constexpr (std::is_same_v<T, reconduits::Setup<Message>>) return create(msg, a, b);
        else if constexpr (std::is_same_v<T, reconduits::Release<Message>>) clean(msg, a, b);
        return nullptr;
    }

private:

    reconduits::Conduit* create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b);
    void clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b);
};

}
