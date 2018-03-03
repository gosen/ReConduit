#ifndef __CONDUIT_TYPES_GENERATORS__HPP__
#define __CONDUIT_TYPES_GENERATORS__HPP__

#include "ConduitFwd.hpp"

#include <variant>
#include <type_traits>

namespace conduits {

template<typename... T>
using generate_type_from = std::variant<std::add_pointer_t<std::decay_t<T>>...>;

}

#define GENERATE_ADAPTER_CONDUITS( A... )  namespace conduits { using adapter_type  = generate_type_from<A>; }
#define GENERATE_FACTORY_CONDUITS( F... )  namespace conduits { using factory_type  = generate_type_from<F>; }
#define GENERATE_MUX_CONDUITS( M... )      namespace conduits { using mux_type      = generate_type_from<M>; }
#define GENERATE_PROTOCOL_CONDUITS( P... ) namespace conduits { using protocol_type = generate_type_from<P>; }

#endif //__CONDUIT_TYPES_GENERATORS__HPP__
