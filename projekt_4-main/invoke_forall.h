#ifndef INVOKE_FORALL_H
#define INVOKE_FORALL_H
#include<bits/stdc++.h>
namespace detail {
    using std::remove_cvref_t;
    using std::size_t;
    using std::tuple_size;
    using std::tuple_size_v;
    using std::derived_from;
    using std::integral_constant;
    using std::get;
    using std::index_sequence;
    using std::make_index_sequence;
    using std::forward;
    // Struct do chowania Tupli.
    template<typename T>
    struct Protect {
        T&& value;  // przechowuje referencję/r-wartość
        constexpr explicit Protect(T&& v) : value(std::forward<T>(v)) {}
    };


    // Koncept na typ krotkowy, tak jak zdefiniowano w treści zadania.
    template <typename T>
concept Tuple_like =
    requires {
        typename std::tuple_size<std::remove_cvref_t<T>>;
        tuple_size<std::remove_cvref_t<T>>::value;
    }
    &&
    // tu już wymuszamy pełną definicję i ::value
    std::derived_from<
        std::tuple_size<std::remove_cvref_t<T>>,
        std::integral_constant<
            std::size_t,
            std::tuple_size<std::remove_cvref_t<T>>::value
        >
    >;
    // Koncept na typ wyciągalny.
    template<typename T, size_t I>
    constexpr bool is_gettable_index() {
        return requires {
            std::get<I>(std::declval<T>());
        };
    }

    template<typename T, std::size_t... I>
    constexpr bool gettable_impl(index_sequence<I...>) {
        return (true && ... && is_gettable_index<remove_cvref_t<T>, I>() );
    }

    template<typename T>
    concept Gettable =
        Tuple_like<remove_cvref_t<T>> &&
        gettable_impl<remove_cvref_t<T>>(
            make_index_sequence<tuple_size_v<remove_cvref_t<T>>>{}
        );


    // Zwraca 0 gdy typ nie jest wyciągalny.
    // W przeciwnym wypadku zwraca jego tuple_size + 1 > 0.
    template<typename T>
    constexpr size_t helper_get_size_type() {
        if constexpr (Gettable<remove_cvref_t<T>>) {
            return tuple_size_v<remove_cvref_t<T>> + 1;
        }
        return 0;
    }

    // Sprawdza czy argument ma arność wanted_size lub nie jest wyciągalny.
    template<typename T>
    constexpr bool helper_check_validity(size_t wanted_size) {
        size_t s = helper_get_size_type<remove_cvref_t<T>>();
        return (s == wanted_size+1)||(s == 0);
    }

    // Funkcja pomocnicza do wyciągnięcia i-tej pozycji z typu wyciągalnego,
    // jeśli argument nie jest typu wyciągalnego, zwraca jego samego.

    // Funkcja jest przeciążona dla typów wyciągalnych i niewyciągalnych,
    // aby zwracany przez funkcję typ dał się wydedukować.
    template<size_t I, typename T>
    constexpr decltype(auto) helper_get_value(T&& argument)
        requires(Gettable<remove_cvref_t<T>>) {
        return std::get<I>(forward<T>(argument));
        }

    template<size_t I, typename T>
    constexpr decltype(auto) helper_get_value(T&& argument)
        requires(!Gettable<remove_cvref_t<T>>) {
        return forward<T>(argument);
        }
    template<size_t I, typename T>
    constexpr decltype(auto) helper_get_value(Protect<T>&& arg)
    {
        return forward<T>(arg.value);
    }

    template<std::size_t I, typename... Args>
    constexpr auto invoke_for_index(Args&&... args) {
        return std::invoke(
            helper_get_value<I>(forward<Args>(args))...);
    }

    template<typename... Args, std::size_t... I>
    constexpr auto helper_caller(index_sequence<I...>, Args&&... args) {
        return std::make_tuple(invoke_for_index<I>(forward<Args>(args)...)...);
    }
    template<typename tuple_t>
    constexpr auto get_array_from_tuple(tuple_t&& tuple)
    {
        constexpr auto get_array = [](auto&& ... x){ return std::array{std::forward<decltype(x)>(x) ... }; };
        return std::apply(get_array, std::forward<tuple_t>(tuple));
    }

    template <typename First, typename... T>
    struct all_same_type {
        constexpr static bool value = std::is_same_v< std::tuple<First,T...>,
                                                      std::tuple<T...,First>>;
    };

    template <typename... T>
    struct all_same_type<std::tuple<T...>> : all_same_type<T...> {};

    template <typename... T>
    constexpr bool all_same_type_v = all_same_type<T...>::value;


}
template<typename T>
constexpr auto protect_arg(T&& v) {
    return detail::Protect<T>(std::forward<T>(v));
}

template<typename... Args>
constexpr auto invoke_forall(Args&&... args) {
    // Or na arnościach wszystkich elementów. Jeśli są takie same to daje dobrą
    // wartość, w.p.p. kolejne linijki to wyłapią.
    constexpr std::size_t m = (0 |...| detail::helper_get_size_type<Args>() );

    // Sprawdzamy czy wszystkie wyciągalne argumenty mają tą samą arność.
    constexpr bool all_valid = (detail::helper_check_validity<Args>(m)&...);
    if constexpr (!all_valid)
        throw std::runtime_error("Nie wszystkie tuple mają tę samą arność");
    // Wywołujemy dla wszystkich
    if constexpr (m == 0) {
        // Żaden z argumentów nie jest wyciągalny. Rozpakowujemy też protected.
        return std::invoke(detail::helper_get_value<m>(std::forward<Args>(args))...);
    } else {
        constexpr std::size_t n = m - 1;
        auto tuple = detail::helper_caller(std::make_index_sequence<n>{},std::forward<Args>(args)...);
        if(detail::all_same_type_v<decltype(tuple)>) {
            return detail::get_array_from_tuple(tuple);
        } else {
            return tuple;
        }
    }
}


#endif