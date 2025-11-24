#include<bits/stdc++.h>

// Koncept na typ krotkowy, tak jak zdefiniowano w treści zadania
template<typename T>
concept Tuple_like = 
requires {
    std::tuple_size<T>::value; 
} 
&& std::same_as<typename std::tuple_size<T>, std::size_t>
&& std::derived_from<std::tuple_size<T>, std::integral_constant<std::size_t, std::tuple_size<T>::value>>;


template<typename T, std::size_t... i>
constexpr bool gettable_impl(std::index_sequence<i...>) {
    return (true && ... && requires(T a){
        std::get<i>(a);
    });
};

// Koncept na typ wyciągalny.
template<typename T>
concept Gettable = 
Tuple_like<std::remove_cvref_t<T>> // Sprawdza czy goły typ T jest krotkowy.
&& gettable_impl(std::make_index_sequence<std::tuple_size<T>::value>{});

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
    return (s == wanted_size+1)|(s == 0);
}

// Funkcja pomocnicza do wyciągnięcia i-tej pozycji z typu wyciągalnego,
// jeśli argument nie jest typu wyciągalnego, zwraca jego samego.

// Funkcja jest przeciążona dla typów wyciągalnych i niewyciągalnych,
// aby zwracany przez funkcję typ dał się wydedukować.

template<size_t i, typename T>
constexpr decltype(auto) helper_get_value(T&& argument) 
    requires(Gettable<T>) {
    return std::get<i>(std::forward<T>(argument));
}

template<size_t i, typename T>
constexpr decltype(auto) helper_get_value(T&& argument) 
    requires(!Gettable<T>) {
    return std::forward<T>(argument);
}


template<typename F, typename... Args, std::size_t... i>
constexpr auto helper_caller (F&& function, std::index_sequence<i...>, Args&&... args) {
    return std::make_tuple(std::invoke(std::forward<F>(function), helper_get_value<i>(args)...)...);
}


template<typename... Args> constexpr auto invoke_forall(Args&&... args) {
    size_t m = (helper_get_size(std::forward<Args>(args))|...);
    if(m == 0) {
        // Żaden z argumentów nie jest wyciągalny.
        return std::invoke(std::forward<Args>(args)...);
    }
    m--;
    // Sprawdzamy czy wszystkie wyciągalne argumenty mają tą samą arność.
    constexpr bool all_valid = (helper_check_validity(std::forward<Args>(args, m))|...);

    if constexpr (!all_valid) {
        // costam costam error error
    }
    for(size_t i=0; i<m; i++) {
        std::invoke(helper_get_value(args)...);
    }
}
