
*po*
=======

这个是自用的基于cpp14并准备升级到cpp17的个人用图形库&渲染实验库，目前任然在开发当中。

该框架主要处理了form, renderer, plugin三者的关系。
* form 底层窗口管理者，负责具体窗口与系统的交互，以及进行输入消息的捕获和分发。
* renderer 依赖于form，负责所有渲染任务的执行。
* plugin 依赖于renderer，负责对所需要渲染的物体的管理。

目前，该库只提供了vs2017的工程文件，以及DX11的基本封装。

---

本文结构
---
* 目录说明
* 快速开始（DX11）
* API说明

*目录说明*
---
* demo里边存放了用于演示的demo工程。
* include存放了框架代码。
    * po 框架主代码。
        * frame 框架使用的通用功能封装。
        * tool 一些工具和轮子。
    * dx 基于DX的通用功能封装（扩展）。
    * dxgi 默认提供的，对DXGI的封装（扩展）。
    * win32 默认提供的，对WIN32 API中与窗口相关的函数的封装（扩展）。
    * dx11 对DX11的功能的封装（扩展）。
* project存放了框架各部件的工程文件和属性表。
    * vs2017 vs2017的各部件工程文件和属性表。

*快速开始（DX11）*
---
下面尝试使用默认提供的dx11扩展进行开发。

1. *配置*
    1. 新建一个解决解决方案和工程，除了本身创建的工程外，再将po/project/vs2017/ 下的dx, dxgi, dx11, win32工程都加入到解决方案中。
    2. 然后将PO框架的include目录放置加入附加头文件目录下。
    3. 将po.lib, dx.lib, dxgi.lib, win32.lib, dx11.lib加入到连接器的输入中（其位置位于po/project/vs2017/{对应的项目名，配置，平台}/下）。
    4. 这是要使用的配置和平台。

2. *创建窗口*
    1. 在工程中添加TestingPlugin.h, 并加入如下代码：
        ```cpp
        #include "po/plugin.h"
        #include "dx11/dx11_renderer.h"
        #include <iostream>

        struct TestingPlugin
        {
            PO::adapter_map mapping(PO::self&);
            void Init(PO::Dx11::simple_renderer&);
            void Tick(PO::Dx11::simple_renderer&);
        }
        ```

    2. 在工程中添加TestingPlugin.cpp, 并加入如下代码：
        ```cpp
        #include "TestingPlugin.h"
        PO::adapter_map TestingPlugin::mapping(PO::self&){
            return {
                PO::make
            }
        }
        ```
    1. 在新建的工程中添加一个main.cpp，加入以下头文件：
        ```cpp
        #include "po/po.h"
        #include "dx11/dx11_form.h"
        #include "dx11/dx11_renderer.h"
        ```
    2. 新建一个plugin
    2. 在main.cpp中添加一个main函数，并写入如下代码：
        ```cpp
        int main()
        {
            PO::context po;
            auto to = po.create_form(form<PO::Dx11::Dx11_form>{});
            fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<simple_renderer>{});
			ui.create(plugin<UE4_testing>{});
		}); 
            return 0;
        }
        ```


要使用该框架，必须先定义


作为用户，你需要先定义一个frame。<p>
一个frame是一个类型，里边定义了三种东西，一个窗口管理器form，一个用于多线程的窗口访问器viewer，一个单线程的渲染器renderer。窗口管理器无法被直接访问，如果是在框架外部访问，需要通过viewer访问，若是在内部访问，那么需要通过renderer来进行访问。<p>
（一般来讲，form和renderer是必须的，而viewer是可省略的）<p>
在gui/dx11中能找目前来说封装的比较简陋的一个dx11的窗口管理器和窗口渲染器。<p>
为了说明其使用方法，先采用dx11的form和renderer进行说明：<p>

```C++
struct custom_frame{
    using viewer = PO::Dx11::Dx11_viewer;
    using form = PO::Dx11::Dx11_form;
    using renderer = PO::Dx11::Dx11_ticker;
}; 
```

这样便定义了一个frame。<p>
接下来我们还需要一个context，并使用该context以默认参数创建frame：<p>
```C++
PO::context con;
auto fo = con.create_frame(PO::frame<custom_frame>{});
```
此时将会以默认参数在**另一个单独的线程**中创建custom_frame中的东西，并且返回一个被包装过的viewer，在代码中，就是fo。当然，还可以创建多个frame。但不能保证多个frame不会相互干涉。<p>
要使得窗口能画上你所想要的图案，还需要创建一个plugin，我们接下来先定义一个啥都不干的plugin：<p>
```C++
struct test_plugin{
    void tick(PO::self_depute<PO::Dx11::Dx11_ticker, void>, duration da);
    void init(PO::self_depute<PO::Dx11::Dx11_ticker, void>);
    test_plugin(PO::self_depute<PO::Dx11::Dx11_ticker, void> p){
        init(p);
        p.self.auto_bind_tick(&test_plugin::tick, this);
    }
    test_plugin(PO::peek<PO::Dx11::Dx11_ticker, void> p){
        p.self.auto_bind_init(&test_plugin::init, this);
        p.self.auto_bind_tick(&test_plugin::tick, this);
    }
};
```
plugin除了需要其自定义的参数外，还需要在开头添加一个self\_depute类型或者peek模板类型。该模板后面接两个类型参数，分别表示其所能支持renderer类型和viewer类型，若为void，则表示不需要访问renderer/viewer，能够匹配任意的类型。<p>

若从viewer中创建，那么plugin将会在调用的时候在viewer所在的线程下，在输入的参数的开头加入peek，并调用plugin的构造函数。peek无法访问renderer，那么就需要用peek::self::auto\_bind\_init注册一个初始化函数。初始化函数只会延迟到frame所在的线程中的每一帧的开始调用，并且只会调用一次。<p>

若是从其他plugin的tick中创建，将会在输入的参数的开头加入self\_depute，并调用plugin的构造函数，但其初始化函数任然会在下一帧的开始时调用。或者是试用viewer延时创建，调用时将会拷贝一份参数，并在每一帧的开始进行创建。<p>

在peek或者self\_depute里边控制自己的生存或者销毁，并且绑定初始化函数或者心跳函数。<p>

初始化函数的参数为self\_depute，可以被省略。心跳函数的参数为self\_depute和duration，顺序任意，并且可以被省略。<p>

接下来我们可以使用上面获得的viewer来创建plugin了:
```C++
fo.lock([](auto& ui){
    ui.depute_create_plugin(PO::plugin<test_plugin>{});
});
```

最后等待所有frame线程的结束：
```C++
con.wait_all_form_close();
```