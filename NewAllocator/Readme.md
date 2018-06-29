# Easy Allocator：Customized STL Allocator & Memp Pool

Author： 信息管理与信息系统 3150101123 李嘉宁

## 任务需求

实现基于内存池的分配器，使之可以替代std::allocator作为vector的第二个类模板参数。

## 实现思路

- 完成一个内存池，这个内存池首先会请求一片非常大的连续内存，vector需要分配内存时直接从内存池中请求内存空间。
- 内存池里面的内存空间是固定大小的，为了适应不同体量的内存分配请求，将内存池划分成若干个子内存池，每个子内存池处理不同size的内存请求。
- 将内存池作为allocator的一个成员，并实现allocator的其它公共接口。

## 功能描述

整体方案自上而下划分为三个层次：EasyAllocator,MemPool,SubMemPool.

### 分配器 EasyAllocator

- 功能概述
  - 调用更低一层的内存池进行内存空间的分配
  - 提供在指定的内存位置进行构造和析构的方法
  - 提供其它STL所要求的allocator的公共接口
- 内容结构
  - 成员变量
    - (静态私有)内存池对象memp
  - 成员函数
    - allocate
      - 指定所需分配的对象数目，调用memp的malloc函数，返回对象的指针
    - deallocate
      - 调用memp的free函数
    - construct
      - 调用new(p)arg，在指针p的位置以arg为参数构造对象
    - destroy
      - 接受对象的指针，调用该对象的析构函数
    - 其它成员函数
      - max_size() 返回地址空间的最大值
      - 其余函数令我费解，根据参考文档胡乱实现了一遍
      - 引用参考：<http://stdcxx.apache.org/doc/stdlibug/15-3.html>

### 总内存池 MemPool

- 功能概述
  - 初始化六个不同级别的分内存池
  - 根据所需分配的对象数目，选择合适的分内存池，调用它的malloc和free函数
  - 对于超出上限的空间分配请求，调用new和delete操作符
- 内容结构
  - 成员变量
    - 适合各种内存大小的空间请求的分内存池：
  - 成员方法
    - malloc和free：使用一组"if else"子句选择node大小相称的分内存池

```cpp
- SubMemPool<4 * sizeof(Ty),16> mp4;
- SubMemPool<1024 * sizeof(Ty)> mp1024;
- SubMemPool<2048 * sizeof(Ty),512> mp2048;
- SubMemPool<4096 * sizeof(Ty)> mp4096;
- SubMemPool<6144 * sizeof(Ty)> mp6144;
- SubMemPool<8192 * sizeof(Ty)> mp8192;
```

### 分内存池 SubMemPool

- 功能概述
  - 包含两个层次：Block和Node
    - Node是内存分配的最小单位
    - Block是向系统请求预分配内存空间的单位
  - 以数组和链表相结合的方式管理内存，它在物理存储层面上是连续的定长数组，但抽象逻辑层面上使用链表维护可用的Node的信息。
  - 需要分配内存时，从空闲Node的链表中剥离一个Node；释放内存时将它所在的Node其放回链表。
  - 若空闲Node用尽，以Block为单位新建一组Node。
- 内容结构
  - 类模板参数
    - NodeData_sz:每个Node的内存空间体量
    - Block_NodeCount：每个block上面的node的数量
  - 成员变量
    - FreeNode和MemBlock的链表头
  - 构造析构
    - 析构时将MemBlock链表上的每一个block都释放掉

## 性能优化

根据需求，调整总内存池里面的各个子内存池的模板参数，选定合适的内存大小的级别划分

因为测试用例里面，数组长度服从0与10000之间的均匀分布，所以划分为5个主要分内存池，每个分内存池负责一个区间的需求。

比如，mp4096负责处理数组长度在2049到4096之间时的内存分配请求。

另外，由于vector内部会有很多分配单个对象的操作（初始化迭代器），因而单独设置一个处理内存大小1~4个对象长度的分内存池。

为了尽量减少新开辟memblock的操作，将每个block的节点数量设为768。（平均来讲，每个分内存池需要处理2000个数组的分配，因而将block设置得更大一些）

### 运行结果

>EasyAllocator MemPool Seting: 0 4 1024 2048 8192
(进程内存 174MB)  
correct assignment in vecints: 3338  
correct assignment in vecpts: 2599  
Elapsed time: 1.59096 seconds.
>
>std::allocator
> (进程内存 93MB)  
correct assignment in vecints: 5676  
correct assignment in vecpts: 6471  
Elapsed time: 1.39847 seconds.
>
>EasyAllocator MemPool Seting: 6个子内存池 0 4 1024 2048 4096 6144 8192
(进程内存 168MB)  
correct assignment in vecints: 6826  
correct assignment in vecpts: 2391  
Elapsed time: 1.55401 seconds.  

## 附：测试日志

### 检验SubMemPool的基本功能

```cpp
using namespace std;
EasyAllocator<int> allc;
int* p=allc.allocate(30);
p[0] = 1;
p[1] = 2;
p[29] = 30;
cout << p[0] << ";" << p[1] << ";" << p[29] << endl;
allc.deallocate(p)；
```

allc可以正确读写int数据。

### 检验vector与EasyAllocator是否相容

```cpp
using IntVec = std::vector<int, EasyAllocator<int>>;
IntVec vint(20);
cout << vint.size() << endl;
std::vector<IntVec, EasyAllocator<IntVec>> vecints(20);
for (int i = 0; i <20; i++) {
  vecints[i].resize(300);
}
cout << vecints.size() << endl;
```

- 此处报错了...耗费了很长时间来找问题。
- 报错信息：
  - vector<>内部释放迭代器时用到的_orphan_all函数崩溃
  - 崩溃原因是出现了空指针

```cpp
//xutility
inline void _Container_base12::_Orphan_all() _NOEXCEPT
{//orphan all iterators
#if _ITERATOR_DEBUG_LEVEL == 2
  if (_Myproxy != 0)
    {// proxy allocated, drain it
    _Lockit _Lock(_LOCK_DEBUG);

    for (_Iterator_base12 **_Pnext = &_Myproxy->_Myfirstiter;
      *_Pnext != 0; *_Pnext = (*_Pnext)->_Mynextiter)
      (*_Pnext)->_Myproxy = 0;
    _Myproxy->_Myfirstiter = 0;
    }
#endif /* _ITERATOR_DEBUG_LEVEL == 2 */
}
//调用vec.resize()时运行到这里会出错，但vec初始化时不会。
```

![Result](Result.png)

#### 纠错经过

- 出问题的这一段代码是干什么的呢？"std::_Container_base::_Orphan_all"
> Searching the source code reveals it is involved with iterator debugging;
it appears to be used by vector and deque to invalidate all iterators into
the container at those times when they really are invalidated, so that
future use of those iterators will cause an error.

```cpp
void _Alloc_proxy()
  {// construct proxy
  _Alproxy _Proxy_allocator(_Getal());
  _Myproxy() = _Unfancy(_Proxy_allocator.allocate(1));}
```

- 使用断点，逐步跟踪调试；修改代码，比较差异，试图找出问题。
  - 如果allocator调用的是::operator new，那么不会报错；但是在vector析构时会出错。
  - 将::operator new()在allocator的层面替换内存池的malloc，发现可以运行

```cpp
//在alloc.malloc的位置添加原生的分配内存返回指针的函数，并添加断点
pointer allocate(size_type n, const void * = 0) throw() {
  std::allocator<T> alloc;
  p = static_cast<pointer>(malloc(sizeof(T)*n));
  pointer r = static_cast<pointer>(::operator new (sizeof(T)*n));
  pointer s = alloc.allocate(n);

  pointer q = (memp.malloc(n));//my_proxy可读。

  return q;
}
```

- 分析过程
  - 通过变量浏览器看分配得来的指针，发现跟原生malloc与std::alloc  ator的区别在于：
    - memp.malloc返回的指针的_Mycont的_Myporxy可读，而原生分配的指针中，_Myproxy<无法读取内存>
    - 此处返回的指针的内容为： 0x0000025f6c6df430 {_Mycont=0xcdcdcdcdcdcdcdcd {_Myproxy=??? } _Myfirstiter=0xcdcdcdcdcdcdcdcd  
    - 它的成员变量 Myproxy 类型为 ...std::_Container_proxy *
  - 会不会是构造proxy类的时候有差错？
    - 应该不是；跟踪运行时没有运行到构造函数的位置
    - 报错也可能是free的时候报错。
  - 仔细考虑之后，想到malloc返回指针时，proxy肯定还没有调用到构造函数，那么它的成员怎么会有值呢？
    - 可能还是我分配内存时有溢出问题
  - 各个层次的分配函数中的参数sz到底是指什么？
    - allocator.allocate: 所需分配的T对象的个数
    - memp.malloc: 应该是个数，但调用时...
      - 调用new时好像不应该是个数？参数sz:Size in bytes of the requested memory block.
    - submp.malloc: 实际内存大小
  - 修改后，没有报错！问题在于，allocator调用memp时分配的内存大小是错误的。
    - 这样子，memp分配的Freenode大小不太对；但是返回的结点应该是一样的，allocator对指针的处理也一样，为什么会错？
    - 发现，问题在于所分配的最小内存？
      - FreeNode的数据大小为4*16时，proxy正常；为128*16时，出错。
  - 小规模测试时没有问题，但放上testallocator后又崩溃了。
    - 完整阅读代码后，发现初始化block时，对空闲node链表的最后一项分配了错误pNext指针
    - 但是仍然搞不懂这里为什么会造成vector的崩溃
- `结论`
  - 第一个错误：allocator执行allocate方法时，传给下一层次的memp的参数应该是所需新建的对象个数，而非size_t
  - 第二个错误，初始化block将新的node加入空闲节点链表时，错误的处理了尾部的结点。

```cpp
if (freeNodeHeader == NULL) {
  MemBlock* pBlock = new MemBlock();
  pBlock->nodes[0].pNext = NULL;
  for (int i = 0; i < Block_NodeCount; i++) {//这一行有错，应该从int i = 1开始。
    pBlock->nodes[i].pNext = &pBlock->nodes[i-1];
  }
  freeNodeHeader= &pBlock->nodes[Block_NodeCount - 1];
  pBlock->pNext = memBlockHeader;
  memBlockHeader = pBlock;
}
```