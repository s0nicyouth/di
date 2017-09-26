#include <utility>
#include <type_traits>
#include <functional>

#include "debug/debug.h"

#ifndef TYPE_HOLDER_H_
#define TYPE_HOLDER_H_

namespace di {

#define str(s) #s
#define INJECT(name, ...) explicit name(di::DiMark&& m, __VA_ARGS__)
#define INJECT_EMPTY(name) explicit name(di::DiMark&& m)

struct DiMark{};

template<typename T>static T* ConstructIfPossible();

template<typename I> struct LazyCreator {
	static bool creation_in_progress;
	static I* value;
	static std::function<I*()> creator;
	static void Register(std::function<I*()> f) {
		creator = f;
	}
	static I* Resolve() {
		DCHECK(!creation_in_progress, "Loop detected!");
		creation_in_progress = true;
		if (!value) {
			value = creator();
		}
		creation_in_progress = false;
		return value;
	}
};
template<typename I> I* LazyCreator<I>::value = reinterpret_cast<I*>(0);
template<typename I> bool LazyCreator<I>::creation_in_progress = false;
template<typename I> std::function<I*()> LazyCreator<I>::creator = []() -> I* {
	DCHECK(false, "Trying to resolve unregistered type");
	return LazyCreator<I>::value;
};

template<typename T> void Register(T* val = 0) {
	if (!val) {
		LazyCreator<T>::Register([] { return ConstructIfPossible<T>(); });
	} else {
		LazyCreator<T>::Register([val] { return val; });
	}
}


template<typename I, typename T> void RegisterInterface(T* val = 0) {
	if (!val) {
		LazyCreator<I>::Register([] { return static_cast<I*>(ConstructIfPossible<T>()); });
	} else {
		LazyCreator<I>::Register([val] { return static_cast<I*>(val); });
	}
}

template<typename T> T* Resolve() {
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
		return new T(std::forward<Args>(args)...);
	}
};

template<typename T> T* ConstructIfPossible() {
	std::cout << typeid(T).name() << std::endl;
	if (std::is_constructible<T, DiMark&&, decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&, decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver()), decltype(AnyResolver())>{},
							 T,
							 DiMark&&,
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver()),
							 decltype(AnyResolver())>::construct(DiMark{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{}, AnyResolver{});
	} else if (std::is_constructible<T, DiMark&&>{}) {
		return DiConstructor<std::is_constructible<T, DiMark&&>{},
							 T,
							 DiMark&&>::construct(DiMark{});
	} else {
		DCHECK(false, "Could not find apropriate constructor");
		return reinterpret_cast<T*>(0);
	}
}

}

#endif