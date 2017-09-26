#include <stdexcept>
#include <utility>
#include <type_traits>
#include <functional>

#include "debug/debug.h"
#include "inject_markers.h"

#ifndef TYPE_HOLDER_H_
#define TYPE_HOLDER_H_

namespace di {

enum class CreationState : int {
	NOT_REGISTERED,
	REGISTERED,
	CONSTRUCTED
};
template<class ...> using void_t = void;

template<typename, typename T, typename ...Ts> struct IsConstructable_ : std::false_type {};
template<typename T, typename... Ts> struct IsConstructable_<void_t<decltype(T(std::declval<Ts>()...))>, T, Ts...> : std::true_type {};
template<typename T, typename... Ts> using IsConstructable = IsConstructable_<void_t<>, T, Ts...>;

template<typename T>static T* ConstructIfPossible();

template<typename I> struct LazyCreator {
	static I* value;
	static std::function<I*()> function;
	static void Register(std::function<I*()> f) {
		function = f;
	}
	static I* Resolve() {
		if (!value) {
			value = function();
		}
		return value;
	}
};
template<typename I> I* LazyCreator<I>::value = reinterpret_cast<I*>(0);
template<typename I> std::function<I*()> LazyCreator<I>::function = []() -> I* {
	DCHECK(false, "Trying to resolve unregistered type");
	return LazyCreator<I>::value;
};

template<typename T> void Register(T* val = 0) {
	if (!val) {
		LazyCreator<T>::Register([]() -> T* { return ConstructIfPossible<T>(); });
	} else {
		LazyCreator<T>::Register([val]() -> T* { return val; });
	}
}


template<typename I, typename T> void RegisterInterface(T* val = 0) {
	if (!val) {
		LazyCreator<I>::Register([]() -> I* { return static_cast<I*>(ConstructIfPossible<T>()); });
	} else {
		LazyCreator<I>::Register([val] () -> I* { return static_cast<I*>(val); });
	}
}

template<typename T> T* Resolve() {
	std::cout << "Type is: " << typeid(T).name() << std::endl;
	return LazyCreator<T>::Resolve();
}

struct AnyResolver {
	template<typename T> operator T() {
		return Resolve<typename std::remove_pointer<T>::type>();
	}
};


template<bool E, typename T, typename... Args> struct DiConstructor {
	static T* construct(Args... args) {
		return reinterpret_cast<T*>(0);
	}
};

template<typename T, typename... Args> struct DiConstructor<true, T, Args...> {
	static T* construct(Args... args) {
		std::cout << "Constructing" << std::endl;
		return new T(std::forward<Args>(args)...);
	}
};

template<typename T> T* ConstructIfPossible() {
	if (IsConstructable<T, DiMark&&, decltype(AnyResolver())>{}) {
		std::cout << "1 argument" << std::endl;
		return DiConstructor<IsConstructable<T, DiMark&&, decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{});
	} else if (IsConstructable<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		std::cout << "2 arguments" << std::endl;
		return DiConstructor<IsConstructable<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{});
	} else {
		DCHECK(false, "Can't find constructor to construct");
		return reinterpret_cast<T*>(0);
	}
}

}

#endif