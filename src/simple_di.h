#include <utility>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <memory>
#include <tuple>
#include <mutex>

#include "di_debug/debug.h"

#ifndef TYPE_HOLDER_H_
#define TYPE_HOLDER_H_

namespace di {

#define str(s) #s
#define INJECT(name, ...) explicit name(di::DiMark&& m, __VA_ARGS__)
#define INJECT_EMPTY(name) explicit name(di::DiMark&& m)

namespace {
constexpr size_t MAX_SIZE = 3;
constexpr size_t NO_CTOR = MAX_SIZE + 1;
}


struct DiMark;
template<size_t> struct AnyType;
template<typename T, size_t N, size_t Sz = 0, size_t... S> struct CtorSz;
class Injector;
template<typename T> class LazyPtr;

template<size_t, class T>
using T_ = T;

template<bool, typename T, size_t N, size_t Sz, size_t... S> struct CtorSize : public CtorSz<T, N, S...> {};
template<typename T, size_t N, size_t Sz, size_t... S> struct CtorSize<true, T, N, Sz, S...> { static constexpr size_t sz = N;};

template<typename T, size_t N, size_t Sz, size_t... S> struct CtorSz : CtorSize<std::is_constructible<T,
																									  DiMark&&, 
																									  decltype(AnyType<S>{})...>{}, 
																				T, 
																				N-1, 
																				Sz, 
																				S...> {};
template<typename T> struct CtorSz<T, 0, 0> { static constexpr size_t sz = NO_CTOR; };


template<typename T, size_t... S> constexpr size_t GetConstructorSize(std::index_sequence<S...>) noexcept {																		
	return CtorSz<T, sizeof...(S), S...>::sz;
}

struct DiMark{};

template<size_t> struct AnyType {
	template<typename T> operator T();
};

template<typename C, size_t... Dummy> struct AnyResolver {
	C* injector_;
	constexpr AnyResolver(C* injector) : injector_(injector) {}
	template<typename T> operator std::shared_ptr<T>() {
		return injector_->template resolve<T>();
	}

	template<typename T> operator LazyPtr<T>() {
		return std::move(LazyPtr<T>(injector_));
	}

	template<typename T> operator T() {
		return injector_->template resolveValue<T>();
	}

	template<typename T> operator T*() {
		static_assert(!std::is_pointer<T*>{}, "Resoling by raw pointer is forbidden");
	}

	template<typename T> operator const T*() {
		static_assert(!std::is_pointer<const T*>{}, "Resoling by raw pointer is forbidden");
	}
};

template<bool E, typename T, typename... Args> struct DiConstructor {
	template<typename... FakeArgs> static T* construct(FakeArgs... args) {
		return reinterpret_cast<T*>(0);
	}
};

template<typename T, typename... Args> struct DiConstructor<true, T, Args...> {
	static T* construct(Args... args) {
		return new T(std::forward<Args>(args)...);
	}
};

template<typename I> struct LazyCreator {
	bool creation_in_progress = false;
	std::shared_ptr<I> value;
	std::function<I*()> creator;
	std::function<std::shared_ptr<I>()> creator_shared;
	void Register(std::function<I*()> f) {
		creator = f;
	}
	void Register(std::function<std::shared_ptr<I>()> f) {
		creator_shared = f;
	}
	std::shared_ptr<I> Resolve() {
		DCHECK(!creation_in_progress, "Loop detected!");
		creation_in_progress = true;
		if (!value.get()) {
			if (creator) {
				value.reset(creator());
			} else {
				value = creator_shared();
			}
		}	
		creation_in_progress = false;
		return value;
	}

	I ResolveValue() {
		DCHECK(!creation_in_progress, "Loop detected!");
		creation_in_progress = true;
		if (!value.get()) {
			if (creator) {
				value.reset(creator());
			} else {
				value = creator_shared();
			}
		}
		creation_in_progress = false;
		return *value;
	}
};

template<typename> void TypeId() {};
using type_id_type = void(*)();

class Injector {
public:
	template<typename T> void registrate(T* val = 0) {
		std::lock_guard<std::recursive_mutex> guard(di_mutex_);
		DCHECK(registered_creators_.find(TypeId<T>) == registered_creators_.end(),
		       "Can not register already registered type");
		auto creator = new LazyCreator<T>;
		if (!val) {
			creator->Register([this] { return this->ConstructIfPossible<T>(); });
		} else {
			creator->Register([val] { return val; });
		}
		registered_creators_.emplace(TypeId<T>, creator);
	}

	template<typename T> void registrateShared(std::shared_ptr<T> val) {
		std::lock_guard<std::recursive_mutex> guard(di_mutex_);
		DCHECK(registered_creators_.find(TypeId<T>) == registered_creators_.end(),
		       "Can not register already registered type");
		auto creator = new LazyCreator<T>;
		creator->Register([val] { return val; });
		registered_creators_.emplace(TypeId<T>, creator);
	}

	template<typename I, typename T> void registrateInterface(T* val = 0) {
		std::lock_guard<std::recursive_mutex> guard(di_mutex_);
		DCHECK(registered_creators_.find(TypeId<I>) == registered_creators_.end(),
		   	   "Can not register already registered type");
		auto creator = new LazyCreator<I>;
		if (!val) {
			creator->Register([this] { return static_cast<I*>(this->ConstructIfPossible<T>()); });
		} else {
			creator->Register([val] { return static_cast<I*>(val); });
		}
		registered_creators_.emplace(TypeId<I>, creator);
	}

	template<typename I, typename T> void registrateSharedInterface(std::shared_ptr<T> val) {
		std::lock_guard<std::recursive_mutex> guard(di_mutex_);
		DCHECK(registered_creators_.find(TypeId<I>) == registered_creators_.end(),
		   	   "Can not register already registered type");
		auto creator = new LazyCreator<I>;
		creator->Register([val] { return static_cast<std::shared_ptr<I> >(val); });
		registered_creators_.emplace(TypeId<I>, creator);
	}

	template<typename T> std::shared_ptr<T> resolve() const {
		std::lock_guard<std::recursive_mutex> guard(di_mutex_);
		DCHECK(registered_creators_.find(TypeId<T>) != registered_creators_.end(),
		       "Can not resolve unregistered type");
		LazyCreator<T>* creator = static_cast<LazyCreator<T>*>(registered_creators_.at(TypeId<T>));
		return creator->Resolve();
	}

	template<typename T> T resolveValue() const {
		std::lock_guard<std::recursive_mutex> guard(di_mutex_);
		DCHECK(registered_creators_.find(TypeId<T>) != registered_creators_.end(),
		       "Can not resolve unregistered type");
		LazyCreator<T>* creator = static_cast<LazyCreator<T>*>(registered_creators_.at(TypeId<T>));
		return creator->ResolveValue();
	}

private:
	template<size_t... S> constexpr auto CreateTupleOfResolversOfSize(std::index_sequence<S...>) {
		return std::tuple<AnyResolver<Injector, S>...>(AnyResolver<Injector, S>(this)...);
	}

	template<typename T, typename Tuple, size_t... TupleSize> constexpr T* ConstructType(const Tuple& t, std::index_sequence<TupleSize...>) {
		return DiConstructor<std::is_constructible<T,
												   DiMark&&, 
												   decltype(AnyType<TupleSize>{})...>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver<Injector, TupleSize>(this))...>::construct(std::move(DiMark{}), std::get<TupleSize>(t)...);
	}

	template<typename T> constexpr T* ConstructIfPossible() {
		AnyResolver<Injector> resolver(this);
		constexpr size_t ctor_size = GetConstructorSize<T>(std::make_index_sequence<MAX_SIZE>{});
		auto tuple = CreateTupleOfResolversOfSize(std::make_index_sequence<ctor_size>{});
		return ConstructType<T>(tuple, std::make_index_sequence<ctor_size>{});	
	}

	std::unordered_map<type_id_type, void*> registered_creators_;
	mutable std::recursive_mutex di_mutex_;
};

template<typename T> class LazyPtr {
public:
	LazyPtr() = delete;
	template<typename A> LazyPtr(const LazyPtr<A>&) = delete;
	template<typename A> LazyPtr(LazyPtr<A>&) = delete;
	LazyPtr(const LazyPtr<T>&) = delete;
	LazyPtr(LazyPtr<T>&) = delete;

	LazyPtr(LazyPtr<T>&& a) {
		injector_ = a.injector_;
	}

	LazyPtr(Injector* i) : injector_(i) {}
	std::shared_ptr<T> get() {
		return injector_->resolve<T>();
	}

private:
	Injector* injector_;
};

}

#endif
