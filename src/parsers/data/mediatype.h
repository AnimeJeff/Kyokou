
#ifndef MEDIATYPE_H
#define MEDIATYPE_H

#include <type_traits>


template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

enum class MediaType {
    Movie,
    TvSeries,
    Reality,
    Anime,
    Documentary
};
#endif // MEDIATYPE_H
