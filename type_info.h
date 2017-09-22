#include <type_traits>

#ifndef TYPE_INFO_H_
#define TYPE_INFO_H_

namespace di {
template<int n, typename T, typename ...Ts> struct NthType {using type = typename NthType<n - 1, Ts...>::type;};
template<typename T, typename ...Ts> struct NthType<0, T, Ts...> { using type = T;} ;

template<int n, typename R, typename ...Args> typename NthType<n, Args...>::type GetNthType(R (*)(Args...));
template<int n, typename Func> using NthFuncArgType = decltype(GetNthType<n - 1>(std::declval<Func>()));
}
#endif  // TYPE_INFO_H_