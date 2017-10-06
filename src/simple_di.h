#include <utility>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <memory>
#include <tuple>

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

template<size_t, class T>
using T_ = T;

template<class T, size_t... Is>
auto gen(std::index_sequence<Is...>) { return std::tuple<T_<Is, T>...>{}; }

template<class T, size_t N>
auto gen() { return gen<T>(std::make_index_sequence<N>{}); }

template<typename, typename T, typename... Args> struct is_cons : public std::is_constructible<T, Args...> {};


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
		return injector_->template Resolve<T>();
	}

	template<typename T> operator T() {
		return injector_->template ResolveValue<T>();
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
	void Register(std::function<I*()> f) {
		creator = f;
	}
	std::shared_ptr<I> Resolve() {
		DCHECK(!creation_in_progress, "Loop detected!");
		creation_in_progress = true;
		if (!value.get()) {
			value.reset(creator());
		}
		creation_in_progress = false;
		return value;
	}

	I ResolveValue() {
		DCHECK(!creation_in_progress, "Loop detected!");
		creation_in_progress = true;
		if (!value.get()) {
			value.reset(creator());
		}
		creation_in_progress = false;
		return *value;
	}
};

template<typename> void TypeId() {};
using type_id_type = void(*)();

class Injector {
public:
	template<typename T> void Register(T* val = 0) {
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

	template<typename I, typename T> void RegisterInterface(T* val = 0) {
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

	template<typename T> std::shared_ptr<T> Resolve() {
		DCHECK(registered_creators_.find(TypeId<T>) != registered_creators_.end(),
		       "Can not resolve unregistered type");
		LazyCreator<T>* creator = static_cast<LazyCreator<T>*>(registered_creators_[TypeId<T>]);
		return creator->Resolve();
	}

	template<typename T> T ResolveValue() {
		DCHECK(registered_creators_.find(TypeId<T>) != registered_creators_.end(),
		       "Can not resolve unregistered type");
		LazyCreator<T>* creator = static_cast<LazyCreator<T>*>(registered_creators_[TypeId<T>]);
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
};

}

#endif
