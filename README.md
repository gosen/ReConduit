ReConduit
=====

Polymorphic compile-time header library based on conduit+ framework

Motivation
----------

Well-designed frameworks evolve naturally. That being the case, it seems like a good idea to take a well-defined pattern design which has been proved once and again in a particular area and then try to improve it by using newer technologies. And this is our case here.

[Conduit+](https://www.glue.ch/~hueni/conduits/) is a framework for network software that has been used to implement the signalling system of a multi-protocol ATM access switch. An earlier version was used to implement TCP/IP. It reduces the complexity of network software, makes it easier to extend or modify network protocols, and is sufficiently efficient. Conduits+ shows the power of a componentized object-oriented framework and of common object-oriented design patterns.

But how it might be improved this design pattern? The answer is getting *compile-time polymorphism* to the next step.

From [**A Framework for Network Protocol Software** paper](doc/A_Framework_for_Network_Protocol_Software.pdf) (Hermann Hüni. Ralph Johnson and Robert Engel. 1995.) we read:

*Conduit+ framework is made up of two sorts of objects, conduits and information chunks. A conduit is a software component with two distinct sides, *sideA* and *sideB*. A conduit may be connected on each of its sides to other conduits, which are its neighbor conduits. A conduit accepts chunks of information from a neighbor conduit on one side and delivers them to a conduit on the opposite side. Conduits are bidirectional, so both its neighbors can send it information.

There are four kinds of conduits, all of which have one neighbor on sideA. A *Mux* can have many neighbors on sideB, an *Adapter* has no neighbor conduit on sideB, and a *Protocol* and *Factory* have exactly one.*

The basic idea of Conduit framework is to get *InformationChunk* messages from one conduit to another back and forward sidaA or sideB interfaces. As long as all kind of conduits support those conduit interconnection, messages are able to transist over them.

How would be a Conduit base class:

```cpp
struct Conduit
{
    virtual void setSideA(Conduit*) = 0;
    virtual void setSideA(Conduit*) = 0;
};

```

A Protocol conduit should then derive from it:

```cpp
struct Protocol : Conduit
{
    void setSideA(Conduit*) override;
    void setSideA(Conduit*) override;
};

```

This way, they will be able to being connected each other:

```cpp
void connect_conduits()
{
    Adapter a1, a2;
    Protocol p1, p2;
    Mux m1, m2,
    Factory f1;

    a1.setSideA( &p1 );

    p1.setSideA( &a1 );
    p1.setSideB( &m1 );

    m1.setSideA( &p1 );
    m1.setSideB( &f1 );

    // ...

    // Start main loop...
};
```

Another important aspect of Conduit+ framework is the role of messages as they are in charge of convey the information. Each conduit has a function member which accept a InformationChunck message:

```cpp
class InformationChunk;

struct Conduit
{
    virtual void setSideA(Conduit*) = 0;
    virtual void setSideA(Conduit*) = 0;

    virtual void accept(InformationChunk*) = 0;
};

struct Protocol : Conduit
{
    void setSideA(Conduit*) override;
    void setSideA(Conduit*) override;

    void accept(InformationChunk*) override;
};

```

So, once the conduits have been connected, they are ready to exchange InformationChunks:


```cpp
void connect_conduits()
{
    Adapter a1, a2;
    Protocol p1, p2;
    Mux m1, m2,
    Factory f1;

    a1.setSideA( &p1 );

    p1.setSideA( &a1 );
    p1.setSideB( &m1 );

    m1.setSideA( &p1 );
    m1.setSideB( &f1 );

    // ...

    // Start main loop...

    InformationChunk* info = deque_new_message();

    a1.accept( info );

    // ...
};
```

Of course, Conduit+ framework allow to create new conduits on events, their interconnection and removal. Mux conduits are in charge of telling conduit factories to create new conduits and then, factories tell them back to Mux for connect them properly. They conduit design uses double dispaching techniques to address this so flexible and dynamic approach. Mux conduit meets how newer conduit that factory conduit has just created must be connected each other, but it has no idea about how they are created. However, factories do. They just need to know when they have to perform their work. Double dispaching is a powerful technique which improves decoupling. (See [visitor pattern](https://en.wikipedia.org/wiki/Visitor_pattern) and *A Framework for Network Proto col Software* paper for further info).

Compile Time Polimorphism
-------------------------

Once conduit basic concepts have been presented, it is clear that making a conduit implementation using object-oriented language support heavily requires virtual function calls.

The advantage of having scalable and flexible designs makes worthy to pay virtual call penalties. For some environments, however, the cost of invoking virtual functions might be simply unacceptable.

Here is where the lastest C++ features come into play:

- Available since C++11 release, [*parameter pack*](http://en.cppreference.com/w/cpp/language/parameter_pack).
- Available since C++14 release, [*generic lambdas*](https://isocpp.org/wiki/faq/cpp14-language#generic-lambdas)
- Available since C++17 release, [ *variants* types](http://en.cppreference.com/w/cpp/utility/variant).

How can polimorphism be performed at compile time? Being **V** a variant type:

```cpp
template<typename V>
constexpr void dispatch(V v, auto&& f)
{
    std::visit([ &f ](auto&& conduit_ptr) { f( conduit_ptr ); }, v);
}
```

What about double dispaching?


```cpp
template<typename V, typename M>
constexpr void doubleDispatch(V v, M m, auto&& f)
{
    std::visit([ & ](auto&& conduit_ptr) {
        std::visit([ & ](auto&& message_ptr) {
            f( conduit_ptr, message_ptr );
        }, m);
    }, v);
}

```

Requirements
------------

This library relies heavily on function return type deduction and other of the newest C++17 features. Any C++17 compliant compiler is fine.

You also need lua5.3 library to run UT.

Usage
-----

To use ReConduit header library, include *ConduitTypesGenerators.hpp*. Then, define your own Adaptor, Protocol, Mux and Factory conduits....

```cpp
struct NetworkAdapter
{
    constexpr auto accept(auto&& msg)
    {
        // ...
    }
}

// ...

struct IPProtocol
{
    constexpr auto accept(auto&& msg)
    {
        // ...
    }
}

// ...
```

Set up variants using

```cpp
#include "MyAdaptersTypes.hpp"
#include "MyFactoriesTypes.hpp"
#include "MyMuxesTypes.hpp"
#include "MyProtocolsTypes.hpp"

#include "ConduitTypesGenerators.hpp"

GENERATE_ADAPTER_CONDUITS(  NetworkAdapter, ApplicationAdapter    );
GENERATE_FACTORY_CONDUITS(  TCPConnectionFactory                  );
GENERATE_MUX_CONDUITS(      ConnectionsMux, ConnectionsLUAMux     );
GENERATE_PROTOCOL_CONDUITS( IPProtocol, TCPProtocol, HTTPProtocol );

#include "ConduitTypes.hpp" // <---- Always after GENERATE macros
```

Eventually, generate your setup:

```cpp
TEST(ConduitTest, SimpleMessages) {

    using namespace conduits;

    Conduit a1{ Adapter{ NetworkAdapter{} } }, a2{ Adapter{ ApplicationAdapter{} } };
    Conduit p1{ Protocol{ IPProtocol{} } };
    Conduit m1{ Mux{ ConnectionsLUAMux{} } }, m2{ Mux{ ConnectionsMux{} } };
    Conduit f{ Factory{ TCPConnectionFactory{} } };

    a1.setSideA( p1 );

    p1.setSideA( a1 );
    p1.setSideB( m1 );

    m1.setSideA( p1 );
    m1.setSideB( f );

    f.setSideA( m1 );
    f.setSideB( m2 );

    m2.setSideA( a2 );
    m2.setSideB( f );

    a2.setSideA( m2 );

    for( auto msg_cnt = 0; msg_cnt < 10; ++msg_cnt) {
        // Create a message...
        Message greeting{msg_cnt, "Hello word!!!", true};

        // ... then, accept a message.
        a1.accept( InformationChunk<Message>{ greeting } );

        // Print out the result.
        greeting();
    }
}
```

See a whole example in test directory.
