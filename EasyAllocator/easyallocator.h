#pragma once
#include "mempool.h"
template <typename _Ty>
class EasyAllocator {
public:
	// Member types :

	typedef void _Not_user_specialized;
	typedef _Ty value_type;
	typedef value_type *pointer;
	typedef const value_type *const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef true_type propagate_on_container_move_assignment;
	typedef true_type is_always_equal;

	// Member function :
	pointer address(reference _Val) const _NOEXCEPT;
	const_pointer address(const_reference _Val) const _NOEXCEPT;
	void deallocate(pointer _Ptr, size_type _Count);
	_DECLSPEC_ALLOCATOR pointer allocate(size_type _Count) {
		mpool.malloc(_Count);
	}
	template<class _Uty> void destroy(_Uty *_Ptr);
	template<class _Objty,
		class... _Types>
		void construct(_Objty *_Ptr, _Types&&... _Args);
private:
	static MemPool mpool();


};