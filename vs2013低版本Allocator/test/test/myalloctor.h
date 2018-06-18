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
	//����
	MyAllocator() _NOEXCEPT;
	MyAllocator(const MyAllocator& myallocator) _NOEXCEPT;
	MyAllocator(MyAllocator&& myallocator) _NOEXCEPT;
	template <class U>MyAllocator(const MyAllocator<U>& myallocator) _NOEXCEPT;
	//����
	~MyAllocator() _NOEXCEPT;
	MyAllocator& operator=(const MyAllocator& myallocator) = delete;
	MyAllocator& operator=(MyAllocator&& myallocator) _NOEXCEPT;
	//Ԫ��ȡַ
	pointer address(reference x) const _NOEXCEPT;
	const_pointer address(const_reference x) const _NOEXCEPT;
	//����ͻ���һ��Ԫ�� ���ڴ�ռ�
	pointer allocate(size_type n = 1, const_pointer hint = 0);
	void deallocate(pointer p, size_type n = 1);
	//���Դﵽ�����Ԫ����
	size_type max_size() const _NOEXCEPT;
	//�ڴ�صĹ��������
	template <class U, class... Args> void construct(U* p, Args&&... args); /*pointer newElement(const_reference val);*/
	void destroy(pointer p);
	// �Դ������ڴ���ͷ��ڴ�Ĺ��������  
	pointer newElement(const_reference val);
	void deleteElement(pointer p);
private:
	// union �ṹ��,���ڴ��Ԫ�ػ� next ָ��  
	union Slot_ {
		value_type element;
		Slot_* next;
	};
	typedef char* data_pointer_;  // char* ָ�룬��Ҫ����ָ���ڴ��׵�ַ  
	typedef Slot_ slot_type_;     // Slot_ ֵ����  
	typedef Slot_* slot_pointer_; // Slot_* ָ������  

	slot_pointer_ currentBlock_;  // �ڴ�������ͷָ��  
	slot_pointer_ currentSlot_;   // Ԫ�������ͷָ��  
	slot_pointer_ lastSlot_;      // �ɴ��Ԫ�ص����ָ��  
	slot_pointer_ freeSlots_;     // Ԫ�ع�����ͷŵ����ڴ�����ͷָ��  

	//size_t padPointer(data_pointer_ p, size_type align) const _NOEXCEPT();  // �����������ռ�  
	void allocateBlock();  // �����ڴ��Ž��ڴ��  
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
//���ƹ��캯��
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
		// ת��Ϊ void ָ�룬����Ϊ void ���Ͳ���Ҫ������������,ֻ�ͷſռ�  
		operator delete(reinterpret_cast<void*>(curr));
		curr = prev;
	}
	//���ص�ַ
	template<typename T>
	inline typename MyAllocator<T>::pointer MyAllocator<T>::address(reference x) const _NOEXCEPT
	{
		return &x;
	}
		//���ص�ַconst����
		template <typename T>
	inline typename MyAllocator<T>::const_pointer MyAllocator<T>::address(const_reference x) const _NOEXCEPT
	{
		return &x;
	}
		//���� ָ�������Ԫ��������ڴ� ��ָ��
		template<typename T>
	inline typename MyAllocator<T>::pointer MyAllocator<T>::allocate(size_type n, const_pointer hint)
	{
		// ��� freeSlots_ �ǿգ����� freeSlots_ ��ȡ�ڴ�  
		if (freeSlots_ != 0) {
			pointer result = reinterpret_cast<pointer>(freeSlots_);
			// ���� freeSlots_  
			freeSlots_ = freeSlots_->next;
			return result;
		}
		else {
			if (currentSlot_ >= lastSlot_)
				// ֮ǰ������ڴ������ˣ������µ� block  
				allocateBlock();
			// �ӷ���� block �л��ֳ�ȥ  
			return reinterpret_cast<pointer>(currentSlot_++);
		}
	}
	//��Ԫ�ص��ڴ淵�����ڴ�����
	template<typename T>
	inline void MyAllocator<T>::deallocate(pointer p, size_type n)
	{
		if (p != nullptr)
		{
			// ת���� slot_pointer_ ָ�룬next ָ�� freeSlots_ ����  
			reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
			// �µ� freeSlots_ ͷΪ p  
			freeSlots_ = reinterpret_cast<slot_pointer_>(p);
		}
	}
	//�������Ԫ������
	template<typename T>
	inline typename MyAllocator<T>::size_type MyAllocator<T>::max_size() const _NOEXCEPT
	{
		size_type maxBlocks = -1 / BlockSize;
		return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
	}
		// ���ѷ����ڴ��Ϲ������ 
		template<typename T>
	template<class U, class... Args>
	inline void MyAllocator<T>::construct(U* p, Args&&... args)
	{
		new (p)U(std::forward<Args>(args)...);
	}
	//���ٶ���
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
		// operator new �����Ӧ��С�ڴ棬���� void* ָ��  
		data_pointer_ newBlock = reinterpret_cast<data_pointer_>
			(operator new(BlockSize));
		// ԭ���� block ��ͷ�ӵ� newblock  
		reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
		// �µ� currentblock_  
		currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
		// Pad block body to staisfy the alignment requirements for elements  
		data_pointer_ body = newBlock + sizeof(slot_pointer_);
		// ����Ϊ�˶���Ӧ�ÿճ�����λ��  
		size_t bodyPadding = (sizeof(slot_type_) - reinterpret_cast<size_t>body) % sizeof(slot_type_); /*padPointer(body, sizeof(slot_type_));*/
		// currentslot_ Ϊ�� block ��ʼ�ĵط����� bodypadding �� char* �ռ�  
		currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
		// �������һ���ܷ��� slot_type_ ��λ��  
		lastSlot_ = reinterpret_cast<slot_pointer_>
			(newBlock + BlockSize - sizeof(slot_type_) + 1);
	}
	template <typename T>
	inline pointer MyAllocator<T>::newElement(const_reference val){
		// �����ڴ�  
		pointer result = allocate();
		// ���ڴ��Ϲ������  
		construct(result, val);
		return result;
	}
	template <typename T>
	inline void MyAllocator<T>::deleteElement(pointer p){
		if (p != 0) {
			// placement new ����Ҫ�ֶ�����Ԫ�� T ����������  
			p->~value_type();
			// �黹�ڴ�  
			deallocate(p);
		}
	}

#endif // !MY_ALLOCATOR_H