#pragma once

#include <type_traits>
#include <concepts>

namespace MyHttpd::Meta {
    using ASCIIOctet = char;
    using RawOctet = unsigned char;

    template <typename T>
    constexpr bool is_buffer_item_v = std::is_same_v<T, ASCIIOctet> or std::is_same_v<T, RawOctet>;

    template <typename T>
    constexpr bool has_unique_ownership_v = (not std::is_copy_constructible_v<T> and not std::is_copy_assignable_v<T>) and (std::is_move_constructible_v<T>
        and std::is_move_assignable_v<T>);

    template <template <typename> typename BufferT, typename ItemT>
    concept BufferKind = requires(BufferT<ItemT>&& arg) {
        {arg.isFull()} -> std::same_as<bool>;
        {arg.isEmpty()} -> std::same_as<bool>;
        {arg.markLength()} -> std::same_as<void>;
        {arg.getPtr()} -> std::same_as<ItemT*>;
    }
    and has_unique_ownership_v<BufferT<ItemT>>
    and is_buffer_item_v<BufferT<ItemT>>;
}