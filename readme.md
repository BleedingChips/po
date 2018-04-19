#PO - 异步ECS框架

需要编译器开启`C++17`与`RTTI`

##例子
见 demo/po_demo/po_demo.sln (vs2017)

##教程

1. 包含头文件
	```
	#include "po/po_implement.h"
	```

1. 主循环
	```cpp
	PO::context_implement imp;
	imp.init([](PO::context& con){
		//在这里创建你的ECS资源，并且最少添加一个System。
	});
	//设置每个循环后等待2000毫秒，若设置为0则不等待。
	imp.set_duration(PO::duration_ms{ 2000 });
	//设置预留的线程数。最终线程数为CPU核数*2 - 预留数，并至少拥有本线程加上一个额外的线程。
	imp.set_thread_reserved(100);
	//创建线程并开始主循环，除非调用PO::context::destory()或者system数为0，否则一直循环。
	//创建的所有外部线程会在loop()返回时关闭。
	imp.loop();
	```

1. 资源接口`PO::context`
	通过`PO::context_implement`构建的线程安全的资源操作接口，所有的ECS资源分配均需要通过该接口。该接口只能在 `void PO::context_implement::init(callable_object&&)`中与System中被访问。
	使用该接口创建资源时，会立即对资源进行初始化，并且将其放入到指令缓冲中，在下一帧的开始加入到主循环中。
	使用该接口销毁资源时，并**不会**对资源立即析构，只是将其放入到指令缓冲中，在下一帧的开始执行。
	不同线程的指令缓冲相互独立，并且按线程依次执行。执行的顺序依赖于System的执行完毕的顺序。

1. ECS资源
	* Entity，内置类型。作为一个集合保存Component的所属关系。对于一个Entity，其绑定的同一种类型的Component的实例只能有一个。用户拿到的只有一个指针，并且必须保证在`PO::context_implement`析构时，所有的指针必须为空。
		```cpp
		imp.init([](PO::context& con){
			//创建 entity
			PO::entity ent = con.create_entity();
			//销毁 entity，该entity所关联的所有component均会被销毁，
			con.destory_entity(ent);
		});
		```
	* Component，自定义类型，纯数据，有专门的对象池进行管理。其必须要绑定Entity后才能被System捕获。
		```cpp
		struct component1{  component1(int){} };
		...
		imp.init([](PO::context& con){
			PO::entity ent = con.create_entity();
			// 创建一个普通 component1 并绑定到该 entity。
			con.create_component<component1>(ent, 1);
			// 删除 ent 中的component1
			// con.destory_component<component1>(ent);
			...
		});
		```
	* 全局Component，不需要以来于Entity，但是一个主循环中，每一种类型只允许拥有一个实例。
		```cpp
		imp.init([](PO::context& con){
			...
			// 创建一个全局 component1 ，不需要绑定。
			con.create_singleton_component<component1>();
			// 删除 全局 component1 
			// con.destory_singleton_component<component1>();
			...
		});
		```
	* System ，捕获Component，执行逻辑。
		System要求提供三个成员函数，分别是:
		```cpp
		struct SystemDemo
		{
			PO::SystemSequence check_sequence(std::type_index ti) noexcept;
			static PO::SystemLayout layout() noexcept;
			void operator()(PO::context&, .../*捕获器*/){ /*logic code here*/}
		};
		```
		其中`PO::SystemSequence`用来控制两个System之间的执行顺序，`PO::SystemLayout`用来定义System的层次，这两个均用来判定System之间的依赖关系，以方便异步执行。
		可以继承一个 `PO::system_default` 来启用默认值
		```cpp
		struct SystemDemo : public PO::system_default
		{
			// SystemSequence = UNDEFINE; SystemLayout = UPDATE;
			void operator()(PO::context&, .../*捕获器*/){ /*logic code here*/}
		};
		ci.init([](PO::context& c)
		{
			//创建一个System
			c.create_system<SystemDemo>(...);
		};
		```
		`operator()`的参数是用来收集System对Component的读写属性，以决定各System的依赖，其函数内部为执行的逻辑。
	
	* Temporary System 只运行一次的System。
		创建效率比System要高，只在主线程种按创建顺序运行一次，没有唯一性要求，其运行顺序在下一帧种领先于所有的system

		```
		ci.init([](PO::context& c)
		{
			//创建一个Temporary System
			c.create_temporary_system<SystemDemo>(...);
		};
		```

	* 捕获器
		* `template<typeanme ...component> class pre_filter`预捕获器。
			其类型不能带`&`与`&&`属性，其中带`const`属性的类型为读，否则为写。其直接捕获所有的entity，挑选出其中带有给定Component组合的，并且提取出对应的Component。
			```cpp
			// 对component1写，对component2读
			void operator()(PO::context& c, PO::pre_filter<component1, const component2> f)
			{
				//参数类型必须在前加entity，其余与预捕器一致，返回捕获到多少个Entity
				f << [](entity e, component1& c1, const component2&){
					//执行多次，直到所有符合要求的entity被轮询完毕后。
				};
			}
			```

		* `template<typeanme ...component> class filter`捕获器。
			与预捕获器类似，但是其只对一个entity有效。
			```cpp
			void operator()(PO::context& c, PO::filter<component1, const component2> f)
			{
				entity e = ...//从其他位置获取到的一个entity，返回是否捕获该Enity
				f[e]<<[](entity e, component1& c1, const component2&){
					//如果该entity带有所示组合，则会调用一次，否则不调用。
				};
			}
			```

		* 全局Component捕获器。
			```cpp
			// component1 为写，component2 与 component3 均为读。
			// 如果有任意一个全局component不存在的话，即该operator()不会被执行。
			void operator()(PO::context& c, component1& c1, const component2& c2, component3 c3);
			```

		* `template<typename event_t> class provider`事件提供者。
			对 event_t 为写。每次使用需手动清空。
			```cpp
			void operator()(PO::context& c, PO::provider<event_t> r)
			{
				//清空所有该System产生的所有event
				r.clear();
				r.push_back(event_t{});
			}
			```

		* `template<typename event_t> class receiver`事件接受者。
			对 event_t 为读。
			```cpp
			void operator()(PO::context& c, PO::receiver<event_t> r)
			{
				//轮询所有System产生的所有event_t，单个System产生的event的顺序是固定的，但是不同System之间则不固定。
				r << [](const event_t& t) { ... };
			}
			``` 
* 依赖判定
	* 如果存在读写冲突或者写写冲突（其中filter与pre_filter一个通道，singleton一个通道，event一个通道，互不影响），则需要计算依赖，否则不需要。
	* 若需要计算依赖，且`PO::SystemLayout`优先级不同，则按优先级计算依赖。(`PRE_UPDATE > UPDATE > POS_UPDATE`)
	* 若`PO::SystemLayout`优先级相同，则按各自的`PO::SystemSequence`中优先级最大的计算依赖。(`BEFORE == AFTER >  NOT_CARE > UNDE
	FINE`，若同时为`BEFORE`或者同时为`AFTER`，则抛出异常`PO::Error::system_dependence_confuse`) 
	* 若`PO::SystemSequence`为`UNDEFINE`或者`NOT_CARE`，且为写读冲突，则按读依赖于写。
	* 若`PO::SystemSequence`为`NOT_CARE`，且为写写冲突，则为互斥但无依赖关系。
	* 若`PO::SystemSequence`为`UNDEFINE`，且为写写冲突，则抛出异常`PO::Error::system_dependence_confuse`。
	* 若存在循环依赖，则抛出异常`PO::Error::system_dependence_circle`。

* 异常
	```cpp
	try
	{
		imp.loop();
	}
	catch(const PO::Error::system_dependence_circle&)
	{
		//循环依赖
	}
	catch(const PO::Error::system_dependence_confuse&)
	{
		//依赖冲突
	}
	```

