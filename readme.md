# PO
Some Toys

### Contents

1. Automatic Parallelization ECS FrameWork


### Automatic Parallelization ECS FrameWork

Demo code in `po/demo/ecs_demo/ecs_demo.sln (vs2019)`.

#### How to use

1. Include `po/include/frame/ecs/implement.h` and link `po.lib`.

1. Define your Components and Systems.

	```cpp
	struct Component1;
	struct Component2;
	struct GobalComponent;
	struct System1
	{

		//Defines system's priority layout. It could be ignored. 
		//Default Priority is PO::ECS::TickPriority::Normal
		PO::ECS::TickPriority tick_layout();

		//Defines system's priority. It could be ignored. 
		//Default Priority is PO::ECS::TickPriority::Normal
		PO::ECS::TickPriority tick_priority();

		//Defines system's priority with specific system. It could be ignored.
		//Use PO::ECS::TypeLayout::create<X>() to create typelayout for specific system.
		//Default Order is PO::ECS::TickOrder Undefine.
		PO::ECS::TickOrder tick_order(const PO::ECS::TypeLayout& layout);

		//Define operator() function with special Filter.
		void operator()(PO::ECS::Filter<const Component1>&, PO::ECS::Context&);

		//Would be call before operator(). It could be ignored.
		void pre_tick(PO::ECS::Context&);

		//Would be call after operator() and no matter wheather operator() has been call or not. It could be ignored.
		void pos_tick(PO::ECS::Context&);
	};

	// lambda or normal function are also an acceptable system.
	```

1. Define and set up ecs context

	```cpp
	PO::ECS::ContextImplement imp;
	// Setting up minimum duration with each cycle. Default value is 16ms
	imp.set_duration(PO::ECS::duration_ms{16});

	// Setting up reserve threads for other uses. Default value is 0;
	imp.set_thread_reserved(2);
	```

1. Create Components and Systems

	```cpp
	PO::ECS::Entity entity = imp.create_entity();
	Component1& ref1 = imp.create_component<Component1>(entity, /*contructional parameter*/);
	Component2& ref2 = imp.create_component<Component2>(entity, /*contructional parameter*/);
	GobalComponent& ref2 = imp.create_gobal_component<Component2>(entity, /*contructional parameter*/);
	System1& sys1 = imp.create_system<System1>(/*contructional parameter*/);
	//Callable object
	imp.create_system([](Filter<Component1>& f){});
	```

1. Have Fun!

	```cpp
	try{
		imp.loop();
	}catch(const PO::ECS::Error::SystemOrderConflig&)
	{
		//Handle system order conflig
	}catch(const PO::ECS::Error::SystemOrderRecursion&)
	{
		//Handle system order recursion
	}
	```

#### For Dynamic Link Library

* in DLL:

	```cpp
	// Include interface
	#include "po/include/frame/ecs/interface.h"
	// Define Components or Systems
	extern "C" {
		void __declspec(dllexport) init(PO::ECS::Context*);
	}
	void __declspec(dllexport) init(PO::ECS::Context*)
	{
		// Createing Systems and Components
	}
	```

* in EXE

	```cpp
	PO::ECS::ContextImplement imp;
	/*
	Do some thing here
	*/
	auto handle = LoadLibrary(...);
	auto init = (void(*)(PO::ECS::Context*))GetProcAddress(handle, "init");
	init(imp);
	imp.loop();
	//You must free library after imp.loop() to make sure that every data has been destructed.
	FreeLibrary(handle);
	```

#### Special Filter

* EntityFilter

	Access components form specific Entity only if it has those components.

	```cpp
	void s1::operator()(PO::ECS::EntityFilter<const Component1, Component2>& f)
	{
		PO::ECS::Entity entity;
		f(entity) << [](const Component1&, Component2&){};
	}
	```
	

* Filter

	Access components form all Entity only if it has those components.

	```cpp
	void s1::operator()(PO::ECS::Filter<const Component1, Component2>& f)
	{
		for(auto ite = f.begin(); ite != f.end(); ++ ite)
		{
			PO::ECS::Entity entity = std::get<0>(*ite);
			std::tuple<const Component1&, Component2&> ref_tuple = std::get<1>(*ite);
			...
		}
	}

	void s1::operator()(PO::ECS::Filter<const Component1>& f)
	{
		for(auto ite = f.begin(); ite != f.end(); ++ ite)
		{
			PO::ECS::Entity entity = std::get<0>(*ite);
			const Component1& ref = std::get<1>(*ite);
			...
		}
	}
	```

* EventProvider

	Provide event to next frame for other systems to access.

	```cpp
	void s1::operator()(PO::ECS::EventProvider<int>& f)
	{
		f.push(1);
	}
	```

* EventViewer

	Access event form last frame.

	```cpp
	void s1::operator()(PO::ECS::EventProvider<int>& f)
	{
		for(auto ite = f.begin(); ite != f.end(); ++ite)
		{
			const int& event = *ite;
		}
	}
	```

* Context

	Access ecs context.

	```cpp
	void s1::operator()(PO::ECS::Context& f)
	{
		//Jump out of the loop
		f.exit();
	}
	```

* SystemWrapper

	Access other system. Functions would be call only if those systems exist.

	```cpp
	void s1::operator()(PO::ECS::SystemWrapper<System2>& f, PO::ECS::SystemWrapper<const System2>& f2)
	{
		System2* sys = f.operator->();
		const System2* sys = f2.operator->();
	}
	```

* Other

	Access gobal component. Functions would be call only if those gobal components exist.

	```cpp
	void s1::operator()(GobalComponent& g1, const GobalComponent2& g2);
	```


#### System Parallelized

Framework uses the type of special filter from system's `operator()` funtion to detect read-write-property of specific data type, and those read-write-property will be used to make a order graphic, which control the order of system calling and whether two system will be parallelized or not. 

|Filter Type|Read-Write-Property|
|:---:|:---:|
|`Filter<A>`or`EntityFilter<A>`|Write To ComponentA|
|`Filter<const A>`or`EntityFilter<const A>`|Read From ComponentA|
|`SystemWrapper<A>`| Write To SystemA|
|`SystemWrapper<const A>`|Read From SystemA|
|`A&`|Write To GobalComponentA|
|`const A&`|Read From GobalComponentA|
|`EntityProvider<A>`|Write To EventA|
|`EntityViewer<A>`|None|

Framework uses the following step to decide system order of two specific system.

1. Compares system layout priority.

	System layout priority comes from function `PO::ECS::TickPriority s1::tick_layout()`. `PO::ECS::TickPriority` has following 5 values which sorted by its priority.
	> **HighHigh** > **High** > **Normal** > **Low** > **LowLow**

	System with higher priority will be called **Before** system with lower priority. In the same time, two systems with difference priority will never be parallelized. 
	
	If is the same, jump to step 2.

2. Calculate system's read-write-property

	If system s1 needs to write to s2, like `void s1::operator(PO::ECS::SystemWrapper<s2>)`, means s1 **may** be called **Before** s2, while if system s1 needs to read from s2, like `void s1::operator(const PO::ECS::SystemWrapper<s2>)`, means s1 **may** be called **After** s2. 

	If two systems need to write to each other, an exception of `PO::ECS::Error::SystemOrderConflig` **may** be threw. 

	In fact, if those situations occurr, framework will jume to step 4 to calculate user-define priority. If undefine, framework will use the reault from this step, else will use the result of user-define priority.

	If thos situations is not occurred, jump to step 3.

3. Calculate read-write-property of same data type.

	Data type is sperated into following 4 independent channels, sorted by calculating order.

	> **System** > **GobalComponent** > **Component** > **Event**

	If s1 needs to write to DataTypeA, and s2 needs to read from DataTypeA, means s1 **may** be called **Before** s2. 
	
	If s1 needs to write to DataTypeA, read from DataTypeB, and s2 needs to write to DataTypeB, read from DataTypeA, or both of s1 and s2 need to write to s3, an exception of `PO::ECS::Error::SystemOrderConflig` **may** be threw. 

	Just like step 2, if those situations occurr, framework will jume to step 4 to calculate user-define priority. 
	
	But event channle has little different. If s1 needs to write to EventA and s2 needs to write to EventB, s1 and s2 **will** not be call at the same time.

	If thos situations is not occurred, those two system **may** be parallelized.
	
4. User-define priority

	System priority just like system layout priority but it came from `PO::ECS::TickPriority s1::tick_priority()`. If is the same, framework will call following two functions to get the order.
	
	```cpp
	auto t1 = PO::ECS::TickOrder s1::tick_order(PO::ECS::TypeLayout::create<s2>());
	auto t2 = PO::ECS::TickOrder s2::tick_order(PO::ECS::TypeLayout::create<s1>());
	```

	If t1 and t2 is `PO::ECS::TickOrder::Undefine`, will not overwrite order.
	
	If t1 and t2 is conflig, an exception of `PO::ECS::Error::SystemOrderConflig` **will** be threw.

	Else, overwrite order.

After system order is beening decide, an order graphic will be made. But if s1 before s2, s2 before s3 and s3 before s1, an exception of `PO::ECS::Error::SystemOrderRecursion` will be threw.

With the help of order graphic, systems can be Parallelized.

> PS: 英语不太好的我实在是尽力了。。。