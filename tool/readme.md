tool
===
这是个放轮子的地方。

*文件说明*
---
* tmp.h<p>
定义了一些用于模板元编程的辅助类型，其位于命名空间PO::Tmp内。<p>
以及一些用来过程化模板元编程的辅助库，其位于命名空间PO::TmpCall内。<p>
    + namespace Tmp
        + set\_t——用来储存参数包的类型。
        + set\_i——用来储存size_t类型的参数包的类型。
        + is_one_of——用以查看第一个类型是否是后面的参数包中的一个。
            ```cpp
            class c_A;
            class c_B;
            class c_C;
            PO::Tmp::is_one_of<c_A, c_B, c_C, c_A>::value // true
            PO::Imp::is_one_of<>::value //false
            ```
        + is_repeat——用来查看是否参数包中有相同的类型，用法同上（带cv属性的类型被认为是不同的类型
        + itself——用来在cpp17以下替代std\:\:in_places和std::declval的类型，具体用法如下：
            ```cpp
            decltype(PO::Tmp::itself<A>{}()); // A
            using type = typename PO::Tmp::itself<A>::type; // = A;
            ```
        + instant——用来做类型拼接的类型。
            ```cpp
            class ca; class cb; class cc;
            using pre = instant<std::is_same, ca>;
            using pre2 = pre::template in<cb>; // instant<std::is_same, ca, cb>;
            using pre2_t = pre::template in_t<cb>; // std::is_same<ca, cb>;
            using pre3 = pre2::template form<cc>; // instant<std::is_same, cc, ca>;
            using pre3_t = pre2::template form_t<cc>; // std::is_same<cc, ca>;
            //ca,cb,cc可以是一个类型参数或者是一个参数包。
            ```