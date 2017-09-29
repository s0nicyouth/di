#include <utility>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <memory>

#include "debug/debug.h"

#ifndef TYPE_HOLDER_H_
#define TYPE_HOLDER_H_

namespace di {

#define str(s) #s
#define INJECT(name, ...) explicit name(di::DiMark&& m, __VA_ARGS__)
#define INJECT_EMPTY(name) explicit name(di::DiMark&& m)

struct DiMark{};

template<typename C> struct AnyResolver {
	C* injector_;
	AnyResolver(C* injector) : injector_(injector) {}
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
	static T* construct(Args... args) {
		return reinterpret_cast<T*>(0);
	}
};

template<typename T, typename... Args> struct DiConstructor<true, T, Args...> {
	static T* construct(Args... args) {
		return new T(std::forward<Args>(args)...);
	}
};

template<typename I> struct LazyCreator {
	bool creation_in_progress;
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
	template<typename T> T* ConstructIfPossible() {
		AnyResolver<Injector> resolver(this);
		if (std::is_constructible<T, DiMark&&, decltype(resolver)>{}) {
			return DiConstructor<std::is_constructible<T, DiMark&&, decltype(resolver)>{},
								 T,
								 DiMark&&,
								 decltype(resolver)>::construct(DiMark{}, resolver);
		} else if (std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver)>{}) {
			return DiConstructor<std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver)>{},
								 T,
								 DiMark&&,
								 decltype(resolver),
								 decltype(resolver)>::construct(DiMark{}, resolver, resolver);
		} else if (std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver)>{}) {
			return DiConstructor<std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver)>{},
								 T,
								 DiMark&&,
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver)>::construct(DiMark{}, resolver, resolver, resolver);
		} else if (std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{}) {
			return DiConstructor<std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{},
								 T,
								 DiMark&&,
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver)>::construct(DiMark{}, resolver, resolver, resolver, resolver);
		} else if (std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{}) {
			return DiConstructor<std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{},
								 T,
								 DiMark&&,
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver)>::construct(DiMark{}, resolver, resolver, resolver, resolver, resolver);
		} else if (std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{}) {
			return DiConstructor<std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{},
								 T,
								 DiMark&&,
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver)>::construct(DiMark{}, resolver, resolver, resolver, resolver, resolver, resolver);
		} else if (std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{}) {
			return DiConstructor<std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{},
								 T,
								 DiMark&&,
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver)>::construct(DiMark{}, resolver, resolver, resolver, resolver, resolver, resolver, resolver);
		} else if (std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{}) {
			return DiConstructor<std::is_constructible<T, DiMark&&, decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver), decltype(resolver)>{},
								 T,
								 DiMark&&,
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver),
								 decltype(resolver)>::construct(DiMark{}, resolver, resolver, resolver, resolver, resolver, resolver, resolver, resolver);
		} else {
			DCHECK(false, "Can not find appropriate constructor");
			return reinterpret_cast<T*>(0);
		}
	}

	std::unordered_map<type_id_type, void*> registered_creators_;
};

}

#endif