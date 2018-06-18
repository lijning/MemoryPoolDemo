#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H
#include <cstddef>
#include <type_traits>
#include <climits>
#include <utility>
#include <stdexcept>

const size_t BlockSize = 4096;
template <typename T>
class MyAllocator
{
public:
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef const T* const_pointer;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	//typedef std::false_type propagate_on_container_copy_assignment;
	//typedef std::true_type propagate_on_container_move_assignment;
	//typedef std::true_type  propagate_on_container_swap;
	template <typename U> struct rebind
	{
		typedef MyAllocator<U> Other;
	};
	//构造
	MyAllocator() _NOEXCEPT;
	MyAllocator(const MyAllocator& myallocator) _NOEXCEPT;
	MyAllocator(MyAllocator&& myallocator) _NOEXCEPT;
	template <class U>MyAllocator(const MyAllocator<U>& myallocator) _NOEXCEPT;
	//析构
	~MyAllocator() _NOEXCEPT;
	MyAllocator& operator=(const MyAllocator& myallocator) = delete;
	MyAllocator& operator=(MyAllocator&& myallocator) _NOEXCEPT;
	//元素取址
	pointer address(reference x) const _NOEXCEPT;
	const_pointer address(const_reference x) const _NOEXCEPT;
	//分配和回收一个元素 的内存空间
	pointer allocate(size_type n = 1, const_pointer hint = 0);
	void deallocate(pointer p, size_type n = 1);
	//可以达到的最多元素数
	size_type max_size() const _NOEXCEPT;
	//内存池的构造和析构
	template <class U, class... Args> void construct(U* p, Args&&... args); /*pointer newElement(const_reference val);*/
	void destroy(pointer p);
	// 自带申请内存和释放内存的构造和析构  
	pointer newElement(const_reference val);
	void deleteElement(pointer p);
private:
	// union 结构体,用于存放元素或 next 指针  
	union Slot_ {
		value_type element;
		Slot_* next;
	};
	typedef char* data_pointer_;  // char* 指针，主要用于指向内存首地址  
	typedef Slot_ slot_type_;     // Slot_ 值类型  
	typedef Slot_* slot_pointer_; // Slot_* 指针类型  

	slot_pointer_ currentBlock_;  // 内存块链表的头指针  
	slot_pointer_ currentSlot_;   // 元素链表的头指针  
	slot_pointer_ lastSlot_;      // 可存放元素的最后指针  
	slot_pointer_ freeSlots_;     // 元素构造后释放掉的内存链表头指针  

	//size_t padPointer(data_pointer_ p, size_type align) const _NOEXCEPT();  // 计算对齐所需空间  
	void allocateBlock();  // 申请内存块放进内存池  
};
template<typename T>
MyAllocator<T>::MyAllocator() _NOEXCEPT
{
	currentBlock_ = 0;
	currentSlot_ = 0;
	lastSlot_ = 0;
	freeSlots_ = 0;
}
template<typename T>
MyAllocator<T>::MyAllocator(const MyAllocator & myallocator) _NOEXCEPT :MyAllocator()
{
	MyAllocator();
}
template<typename T>
MyAllocator<T>::MyAllocator(MyAllocator && myallocator) _NOEXCEPT
{}
//复制构造函数
template<typename T>
template <class U>
MyAllocator<T>::MyAllocator(const MyAllocator<U>& myallocator) _NOEXCEPT :MyAllocator()
{
	MyAllocator();
}
template<typename T>
MyAllocator<T> & MyAllocator<T>::operator=(MyAllocator && myallocator) _NOEXCEPT
{}
template<typename T>
MyAllocator<T>::~MyAllocator() _NOEXCEPT
{
	slot_pointer_ curr = currentBlock_;
	while (curr != 0) {
		slot_pointer_ prev = curr->next;
		// 转化为 void 指针，是因为 void 类型不需要调用析构函数,只释放空间  
		operator delete(reinterpret_cast<void*>(curr));
		curr = prev;
	}
	//返回地址
	template<typename T>
	inline typename MyAllocator<T>::pointer MyAllocator<T>::address(reference x) const _NOEXCEPT
	{
		return &x;
	}
		//返回地址const重载
		template <typename T>
	inline typename MyAllocator<T>::const_pointer MyAllocator<T>::address(const_reference x) const _NOEXCEPT
	{
		return &x;
	}
		//返回 指向分配新元素所需的内存 的指针
		template<typename T>
	inline typename MyAllocator<T>::pointer MyAllocator<T>::allocate(size_type n, const_pointer hint)
	{
		// 如果 freeSlots_ 非空，就在 freeSlots_ 中取内存  
		if (freeSlots_ != 0) {
			pointer result = reinterpret_cast<pointer>(freeSlots_);
			// 更新 freeSlots_  
			freeSlots_ = freeSlots_->next;
			return result;
		}
		else {
			if (currentSlot_ >= lastSlot_)
				// 之前申请的内存用完了，分配新的 block  
				allocateBlock();
			// 从分配的 block 中划分出去  
			return reinterpret_cast<pointer>(currentSlot_++);
		}
	}
	//将元素的内存返还给内存链表
	template<typename T>
	inline void MyAllocator<T>::deallocate(pointer p, size_type n)
	{
		if (p != nullptr)
		{
			// 转换成 slot_pointer_ 指针，next 指向 freeSlots_ 链表  
			reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
			// 新的 freeSlots_ 头为 p  
			freeSlots_ = reinterpret_cast<slot_pointer_>(p);
		}
	}
	//计算最大元素上限
	template<typename T>
	inline typename MyAllocator<T>::size_type MyAllocator<T>::max_size() const _NOEXCEPT
	{
		size_type maxBlocks = -1 / BlockSize;
		return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
	}
		// 在已分配内存上构造对象 
		template<typename T>
	template<class U, class... Args>
	inline void MyAllocator<T>::construct(U* p, Args&&... args)
	{
		new (p)U(std::forward<Args>(args)...);
	}
	//销毁对象
	template<typename T>
	inline void MyAllocator<T>::destroy(pointer p)
	{
		p->~T();
	}
	//template <typename T>
	//inline size_t MyAllocator<T>::padPointer(data_pointer_ p, size_type align) const _NOEXCEPT(){
	//	size_t result = reinterpret_cast<size_t>(p);
	//	return ((align - result) % align);
	//}
	template <typename T>
	void MyAllocator<T>::allocateBlock(){
		// operator new 申请对应大小内存，返回 void* 指针  
		data_pointer_ newBlock = reinterpret_cast<data_pointer_>
			(operator new(BlockSize));
		// 原来的 block 链头接到 newblock  
		reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
		// 新的 currentblock_  
		currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
		// Pad block body to staisfy the alignment requirements for elements  
		data_pointer_ body = newBlock + sizeof(slot_pointer_);
		// 计算为了对齐应该空出多少位置  
		size_t bodyPadding = (sizeof(slot_type_) - reinterpret_cast<size_t>body) % sizeof(slot_type_); /*padPointer(body, sizeof(slot_type_));*/
		// currentslot_ 为该 block 开始的地方加上 bodypadding 个 char* 空间  
		currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
		// 计算最后一个能放置 slot_type_ 的位置  
		lastSlot_ = reinterpret_cast<slot_pointer_>
			(newBlock + BlockSize - sizeof(slot_type_) + 1);
	}
	template <typename T>
	inline pointer MyAllocator<T>::newElement(const_reference val){
		// 申请内存  
		pointer result = allocate();
		// 在内存上构造对象  
		construct(result, val);
		return result;
	}
	template <typename T>
	inline void MyAllocator<T>::deleteElement(pointer p){
		if (p != 0) {
			// placement new 中需要手动调用元素 T 的析构函数  
			p->~value_type();
			// 归还内存  
			deallocate(p);
		}
	}

#endif // !MY_ALLOCATOR_H