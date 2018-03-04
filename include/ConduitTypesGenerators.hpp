#ifndef __CONDUIT_TYPES_GENERATORS__HPP__
#define __CONDUIT_TYPES_GENERATORS__HPP__

#include "ConduitFwd.hpp"

#include <variant>
#include <type_traits>

namespace reconduits {

template<typename... T>
using generate_type_from = std::variant<std::add_pointer_t<std::decay_t<T>>...>;

}

#define GENERATE_ADAPTER_CONDUITS( A... )  namespace reconduits { using adapter_type  = generate_type_from<A>; }
#define GENERATE_FACTORY_CONDUITS( F... )  namespace reconduits { using factory_type  = generate_type_from<F>; }
#define GENERATE_MUX_CONDUITS( M... )      namespace reconduits { using mux_type      = generate_type_from<M>; }
#define GENERATE_PROTOCOL_CONDUITS( P... ) namespace reconduits { using protocol_type = generate_type_from<P>; }

#endif //__CONDUIT_TYPES_GENERATORS__HPP__
