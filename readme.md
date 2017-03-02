
*po*
=======

这个是基于C++14的图形框架，目前并未完工。


*文件说明*
---
* po_vs2015存放了能被vs2015打开的PO框架的工程。
* frame存放框架主文件。
* gui存放默认图形接口文件（只作为框架的补充。
* tool存放了全部的轮子。
* sample存放了demo的文件和工程。



*框架设计*
-------

本框架主要处理plugin，frame，context之间的关系。<p>
* *frame* ——自定义的窗口和控制器集。<p>
  一般而言，frame应当由库的使用者来进行定义，其可以定义三个类，form，viewer和ticker。<p>
  最简单的定义如下:<p>
    ```C++
    struct custom_frame{
        using form = XXXform;
        using ticker = XXXticker;
        using viewer = XXXviewer;
    }; 
    ```
    其中，form是自定义的窗口实体，viewer是自定义的窗口管理器，ticker是自定义的渲染管理器。viewer和ticker是可选的。<p>
    form由context在另一个线程中构造。context会将创建时传入的参数的引用传送到另一个线程中，并在参数列表之前添加一个form\_self的引用来尝试构造form。form\_self是储存了部分控制参数的类型。若加上form\_self失败，则会试用参数列表的引用来构造form。在每一帧时，均会尝试调用form的void tick(form\_ticker&)成员函数，若无法调用，则忽略。<p>
    ticker则在其之后在同一线程被构造，以构造出来的form的引用作为其构造参数。该访问器用以在当前线程给予form所附属的plugin访问。一般而言，ticker的访问总是和form在同一个线程，所以可以不用做线程同步。<p>
    viewer则作为form的线程外的一个访问器，其必须提供一个移动构造函数或复制构造函数，和一个以form的引用为参数的构造函数。在创建form时，第一次将会在form的相同线程中以form的引用为参数创建，并通过移动语义传递到线程外。viewer控制form的代码的线程安全性由用户提供。<p>

* *context* ——form的创建者和管理者。<p>
    form由context来指定创建，并且创建在另一个线程，并返回一个包含了viewer的viewer\_packet，该packet提供了线程安全的控制访问，能够用来创建plugin。若创建多个form，那么每个form均在不同的线程中。而在一个可执行程序中，context可以存在多个，但由该context创建的form，在context析构时，会先行强行析构，除非context使用void wait\_all\_form\_close()成员函数堵塞当前线程，直到所有form主动析构。或试用void detach()成员函数，将所有的form的管理权放到全局，此时context的析构将不会引起其创建的form的析构，但程序的结束也会引起form的析构。

* *plugin* ——代码执行者。<p>
    plugin是能够动态创建和执行的部件，用来执行所有具体的渲染和逻辑代码，通常一个form会关联多个plugin，而一个plugin只会从属于一个form。一般而言，plugin的创建和析构不会对form有影响，但是form的析构将会引发其所关联的所有plugin的析构。<p>
    一般，plugin必须与frame中所指定的ticker所匹配，当然还有一种通配的无类型plugin，这种plugin无法访问ticker。<p>
    在尝试构造的时候，会先在给定的参数列表之前添加一个控制类型的引用，这个控制类型与plugin的创建类型和创建位置有关。若使用viewer_packet创建，则plugin在form所在的线程外创建，其参数为constor\_outside\<frame\>，若使用ticker创建，则其在form所在的线程中被创建，类型为constor\_inside\<ticker\>，该两种类型都能退化成constor，同时若使用无类型创建，那么其控制类型将强制为这个。若加入该控制类型后无法构造，则会自动使用传入的参数的引用来进行创建。<p>
    在创建plugin之后，将会进入一个等待列表，等待当前所有plugin轮询完毕后，将会调用plugin的void init(ticker\<ticker\>&)成员函数。该函数与form在同一个线程内，若是使用无类型创建，则会调用void init(ticker\_tl&)成员函数，若无则忽略。该函数被调用一次，然后加入轮询列表。<p>
    在plugin被轮询时，会调用void tick(ticker\<ticker\>&)成员函数，若使用无类型创建，则会调用void tick(ticker\_tl&)成员函数，若无则忽略。<p>
    在任意时刻，均可试用constor或者ticker\_tl的self()成员函数来获取本身的控制类型的引用，并用此来控制plugin的析构。当然也可以通过form()成员函数来获取得到一个窗口的控制类型的引用，ticker()成员函数来获取frame的ticker的类型的引用（无类型创建无此函数）。

*使用方法*
----

首先，必须先指定一个frame。该frame包含三个类型定义，分别是form, viewer, ticker。其中只有form是必须的，若使用提供的默认图形接口，例如说dx11的图形接口：
```cpp
struct frame{
    using form = Dx11_form;
    using ticker = Dx11_ticker;
};
```
然后再创建一个PO::contex;
```cpp
PO::context co;
auto view = co.create_window<frame>();
```
此时view既是包含viewer的一个packet。<p>
然后，定义一个plugin：
```cpp
class custom_plugin
{
    int coun;
public:
    custom_plugin(constor_outside<Dx11_ticker>& co, int c)
    {
        coun = c;
    }
    void init(ticker<Dx11_ticker>& t)
    {
        //call once
    }
    void tick(ticker<Dx11_ticker>& t)
    {
        ++coun;
        if(coun >100)
        {
            t.self().close();
            t.form().close();
        }
    }
}
```
用packet创建plugin，并等待窗口关闭。
```cpp
view.lock([](PO::viewer<frame>& v)
{
    v.cleate_plugin(PO::plugin_type<custom_plugin>{}, 1);
});
co.wait_all_form_close();
```