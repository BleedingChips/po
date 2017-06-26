
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
            void Tick(PO::Dx11::simple_renderer&, duration);
        }
        ```

    2. 在工程中添加TestingPlugin.cpp, 并加入如下代码：
        ```cpp
        #include "TestingPlugin.h"
        //为对应的renderer绑定初始化函数与心跳函数
        PO::adapter_map TestingPlugin::mapping(PO::self&){
            return {
                PO::make_member_adapter<PO::Dx11::simple_renderer>(
                    this,
                    &TestingPlugin::init, 
                    &TestingPlugin::tick
                    )
            };
        }

        void TestingPlugin::Init(PO::Dx11::simple_renderer&)
        {
            //初始化函数
        }
        void TestingPlugin::Tick(PO::Dx11::simple_renderer&, duration)
        {
            //心跳函数
        }
        ```
    3. 在新建的工程中添加一个main.cpp，加入以下代码：
        ```cpp
        #include "po/po.h"
        #include "dx11/dx11_form.h"
        #include "dx11/dx11_renderer.h"
        #include "TestingPlugin.h"
        int main()
        {
            PO::context po;
            auto to = po.create_form(form<PO::Dx11::Dx11_form>{});
            fo.lock([](decltype(fo)::type& ui) {
			ui.create(renderer<simple_renderer>{});
			ui.create(plugin<TestingPlugin>{});
		}); 
            //堵塞直到所有的窗口被关闭。
            po.wait_all_form_close();
            return 0;
        }
        ```