# Valkyrie Engine

Valkyrie Engine is a game engine written in modern C++ that is designed to be as modular as possible and as lightweight as you want.

Valkyrie Engine's Core module is designed to be small and easy to implement. Most of the functionality that you would expect from a game engine is defined across Valkyrie Engine's various modules. These modules are collections of systems, events and components designed to help accomplish a specific functionality or behaviour within a game. 

The eventual goal of Valkyrie Engine is to create a thriving ecosystem of modules from various developers that have either been torn out of previous Valkyrie Engine projects or developed completely independently of one. 

## 1 Manifesto

The supply of video games in the current market is increasing at a rate that is far outpacing the demand. Consumers are beginning to raise their overall standards for quality as various niches are quickly becoming saturated. As games become more complex and their specifications more refined, it becomes increasingly difficult to produce a single tool that adequately meets this wide range of requirements and yet most publicly-available engines are trying to do exactly that. 

This is where Valkyrie Engine comes in. Instead of giving developers one general-purpose system, Valkyrie Engine is designed to give them a choice between several specialised systems so they can use the best option for their game. Valkyrie Engine lets the developer decide how it's implemented and in what capacity. You can mix-and-match existing modules and use Valkyrie Engine as a fully-featured game engine or forgo them  and elect to make your own.

## 2 Documentation

Documentation is currently live [here.](https://vd-15.github.io/ValkyrieEngineCore/html/index.html)

## 3 Examples

### 3.1 Initialization

Valkyrie Engine doesn't require much in terms of initialization, currently it only requires a call to `vlk::Application::Start()` at the beginning of your program. This is the function that starts the main update loop. This loop will run continuously until `vlk::Application::Stop()` is called, in which case it will exit once the current update loop is completed.

```cpp
#include "ValkyrieEngine/ValkyrieEngine.hpp"

int main()
{
	vlk::ApplicationArgs args{};
	args.applicationName = "My Game";

    vlk::Application::Start(args);
}
```

### 3.2 Events

In Valkyrie Engine, events are the primary way that inter-module communication occurs. Instead of calling a function from another module directly, send an event. Events help keep Valkyrie Engine modular and your code maintainable, both by yourself and others.

#### 3.2.1 Creating Events

In Valkyrie Engine, any object or class can be an event, though usually they are just POD structs.

```cpp
struct MyEvent
{
    const vlk::Int foo = 10;
    const vlk::Float bar = 1.0f;
};

```

#### 3.2.2 Pre-Defined Events

Valkyrie Engine Core comes with a few events that help form the main update loop of the engine. All of these events are simply empty structs with no data but their usage varies significantly:

* `ApplicationStartEvent` is sent once, just before the first event loop of the application runs. This is used primarily for second-stage module initialization and to set up the initial state of your game.
* `ApplicationExitEvent` is sent once, after the event loop exits and the application is due to terminate. This is used to perform cleanup on any entities or systems that need it.
* `PreUpdateEvent` is sent at the beginning of every update loop but should not be considered part of it. It is mostly used to update modules and should not be used to change the state of your game.
* `EarlyUpdateEvent` is sent after `PreUpdateEvent` and is mostly used to read the state of other entities before `UpdateEvent` is called.
* `UpdateEvent` is sent after `EarlyUpdateEvent` and is used to update the state of most of entities present in the game.
* `LateUpdateEvent` is sent after `UpdateEvent` and is mostly used to react to the changes that occured during the `UpdateEvent`.
* `PostUpdateEvent` is sent after `LateUpdateEvent` and is similar to the `PreUpdateEvent` in that it is mostly used to update modules and should not be used to alter the game state in any way, redering would usually take place during a PostUpdate..

#### 3.2.3 Sending Events

Sending events in Valkyrie Engine is done through the `EventBus` class. This will raise the event handler for every listener of your event.

```cpp
vlk::SendEvent(MyEvent{});
```

#### 3.2.4 Recieving Events

In order to recieve an event, you need to define a class that inherits from `vlk::EventListener` and construct an instance of it.

This instance will then recieve any events of the appropriate type you send down the event bus.

```cpp
class MyEventListener : public EventListener<MyEvent>
{
    /* ... */

    void OnEvent(const MyEvent& ev) final override
    {
        /* Do something in here :) */
    }
};
```

#### 3.2.5 Putting it all together

```cpp
#include "ValkyrieEngine/ValkyrieEngine.hpp"
#include <iostream>
#include <string>

struct MyEvent
{
    const vlk::Int foo;
    const std::string bar;
};

class MyEventListener : public vlk::EventListener<MyEvent>
{
    public:
    MyEventListener() = default;

    private:
    void OnEvent(const MyEvent& ev) final override
    {
        std::cout << "Event handler raised: " << std::to_string(ev.foo) << std::endl;
        std::cout << "Cow says: " << ev.bar << std::endl;
    }
};

int main()
{
    MyEventListener listener;

    vlk::SendEvent(MyEvent{ 51, "Moo!" });

}
```

### 3.3 Modules

> I'm workin' on it! ;)

### 3.4 Entity-Component System

Valkyrie Engine core defines a basic entity-component system that serves as the underlying framework for any game produced in it.

#### 3.4.1 Components

Components are data types that define a particular behaviour or property for the entity they're attached to. It is important to note that components shouldn't be event listeners. This is mostly to avoid congestion on the event bus as constantly adding and removing event listeners can significantly impact performance.

The constructor for `Component` takes a single pointer to the entity the component belongs to in the form of an `IEntity*`. Any constructors for your component should accept an `IEntity*` as their first argument, this to to help compatability with certain helper functions you may wind up using.

```cpp
#include "ValkyrieEngine/ValkyrieEngine.hpp"

class ScoreCounterComponent : public vlk::Component<ScoreCounterComponent>
{
    public:
    MyComponent(IEntity* e, vlk::UInt _score) :
        Component<MyComponent>(e)
    {
        this->score = _score;
    }

    vlk::UInt score;
};
```

#### 3.4.2 Entities

Entities are simply data types used to composite several components into one structure, they will usually not hold any data aside from components and should never really be used outside of their constructor. `Entity` defines several member functions, one of which is a useful way of attaching components to an entity.

```cpp
#include "ValkyrieEngine/ValkyrieEngine.hpp"

class MyEntity : public vlk::Entity<MyEntity>
{
    public:
    MyEntity()
    {
		myComponent = new MyComponent(this);
    }

    ~MyEntity()
    {
		delete myComponent();
    }

    private:
    MyComponent* myComponent;
};
```

#### 3.4.3 Systems

Where components declare behaviour for entities, systems are where it's implemented. Most systems will take the form of several event listeners with any necessary data stored in member variables or the like. Since systems perform operations on/with components, they need a way to access every instance of that component at runtime. `Component` has a static member function called `ForEach` that does exactly that. It accepts a const reference to anything that can serve as an std::function, usually lambdas. Because events can be sent from any thread, it is important to consider thread-safety. Thankfully, every component comes with a mutex that can be locked to ensure thread-safe access.

```
#include "ValkyrieEngine/ValkyrieEngine.hpp"

class MySystem : public vlk::EventListener<vlk::UpdateEvent>
{
    public:
    MySystem() = default;

    private:
    void OnEvent(const vlk::UpdateEvent& ev) final override
    {
        MyComponent::ForEach([](MyComponent* c)
        {
			//Lock the component
			std::unique_lock ulock = c->Lock();

			//Modify the component
            c->score++;

			//the unique_lock is unlocked when it falls out of scope
        });
    }
};
```
