#ifndef INVOKE_FORALL_H
#define INVOKE_FORALL_H
#include<bits/stdc++.h>
namespace detail {
    // Koncept na typ krotkowy, tak jak zdefiniowano w treści zadania.
    template<typename T>
    concept Tuple_like =
        requires { typename std::tuple_size<T>; }
    && std::derived_from<std::tuple_size<T>,
        std::integral_constant<std::size_t, std::tuple_size_v<T>>>;

    // Koncept na typ wyciągalny.
    template<typename T, std::size_t I>
    constexpr bool is_gettable_index() {
        return requires(T t) { std::get<I>(t); };
    }

    template<typename T, std::size_t... I>
    constexpr bool gettable_impl(std::index_sequence<I...>) {
        return (true && ... && is_gettable_index<T, I>());
    }

    template<typename T>
    concept Gettable =
        Tuple_like<std::remove_cvref_t<T>> &&
        gettable_impl<T>(
            std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{}
        );


    // Zwraca 0 gdy argument nie jest wyciągalny.
    // W przeciwnym wypadku zwraca jego tuple_size + 1 > 0.
    template<typename T>
    constexpr size_t helper_get_size(T&& argument) {
        if constexpr (Gettable<T>) {
            return std::tuple_size<T>::value+1;
        }
        return 0;
    }

    // Sprawdza czy argument ma arność wanted_size lub nie jest wyciągalny.
    template<typename T>
    constexpr bool helper_check_validity(T&& argument, size_t wanted_size) {
        size_t s = helper_get_size(argument);
        return (s == wanted_size+1)||(s == 0);
    }

    // Funkcja pomocnicza do wyciągnięcia i-tej pozycji z typu wyciągalnego,
    // jeśli argument nie jest typu wyciągalnego, zwraca jego samego.

    // Funkcja jest przeciążona dla typów wyciągalnych i niewyciągalnych,
    // aby zwracany przez funkcję typ dał się wydedukować.
}
template<typename T>
    struct Protect {
    T&& value;  // przechowuje referencję/r-wartość
    constexpr explicit Protect(T&& v) : value(std::forward<T>(v)) {}
};
template<typename T>
constexpr auto protect_arg(T&& v) {
    return Protect<T>(std::forward<T>(v));
}

namespace detail {
    template<size_t i, typename T>
    constexpr decltype(auto) helper_get_value(T&& argument)
        requires(detail::Gettable<T>) {
        return std::get<i>(std::forward<T>(argument));
        }

    template<size_t i, typename T>
    constexpr decltype(auto) helper_get_value(T&& argument)
        requires(!detail::Gettable<T>) {
        return std::forward<T>(argument);
        }
    template<size_t i, typename T>
    constexpr decltype(auto) helper_get_value(Protect<T>&& arg)
    {
        return std::forward<T>(arg.value);
    }

    template<std::size_t I, typename... Args>
    constexpr auto invoke_for_index(Args&&... args) {
        return std::invoke(helper_get_value<I>(std::forward<Args>(args))...);
    }

    template<typename... Args, std::size_t... I>
    constexpr auto helper_caller(std::index_sequence<I...>, Args&&... args) {
        return std::make_tuple(invoke_for_index<I>(std::forward<Args>(args)...)...);
    }
}

template<typename... Args>
constexpr auto invoke_forall(Args&&... args) {
    // Or na arnościach wszystkich elementów. Jeśli są takie same to daje dobrą
    // wartość, w.p.p. kolejne linijki to wyłapią.
    constexpr size_t m = (detail::helper_get_size(std::forward<Args>(args))|...);
    if(m == 0) {
        // Żaden z argumentów nie jest wyciągalny.
        return std::invoke(std::forward<Args>(args)...);
    }

    // Sprawdzamy czy wszystkie wyciągalne argumenty mają tą samą arność.
    constexpr bool all_valid = (detail::helper_check_validity(std::forward<Args>(args), m)&...);
    if constexpr (!all_valid)
        throw std::runtime_error("Nie wszystkie tuple mają tę samą arność");
    // Wywołujemy dla wszystkich
    for(size_t i = 0; i < m; i++) {

    }
    return detail::helper_caller(std::make_index_sequence<m>{},std::forward<Args>(args)...);
}


#endif