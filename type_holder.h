#include <stdexcept>
#include <utility>
#include <type_traits>

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

template<typename T> struct ValueHolder {
	using type = T;
	static T* value;
	static CreationState creation_state;
};
template<typename T> T* ValueHolder<T>::value = reinterpret_cast<T*>(0);
template<typename T>CreationState ValueHolder<T>::creation_state = CreationState::NOT_REGISTERED;

template<typename T> void Register(T* val = 0) {
	ValueHolder<T>::value = val;
	if (!val) {
		ValueHolder<T>::creation_state = CreationState::REGISTERED;
	} else {
		ValueHolder<T>::creation_state = CreationState::CONSTRUCTED;
	}
}

template<typename T> T* Resolve() {
	std::cout << "Type is: " << typeid(T).name() << std::endl;
	DCHECK(ValueHolder<T>::creation_state != CreationState::NOT_REGISTERED,
		   "Not registered!");
	if (ValueHolder<T>::creation_state == CreationState::REGISTERED) {
		std::cout << "Creating" << std::endl;
		ValueHolder<T>::value = ConstructIfPossible<typename ValueHolder<T>::type>();
		ValueHolder<T>::creation_state = CreationState::CONSTRUCTED;
	} else {
		std::cout << "Already created" << std::endl;
	}
	return ValueHolder<T>::value;
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