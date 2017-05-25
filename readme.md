
*po*
=======

这个是自用的基于cpp14并准备升级到cpp17的个人用图形库&渲染实验库。

目前，该库只支持vs2017编译器。

*目录说明*
---
* vs2017存放了工程文件。
* frame存放框架主文件。
* gui存放默认图形接口文件（只作为框架的补充。
* tool存放了全部的轮子。
* demo里边放了实验示例

*快速开始*
---
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