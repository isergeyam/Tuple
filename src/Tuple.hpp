#pragma once
#include <cstddef>
#include <utility>
template <size_t... Indices> struct IndexSequence {
  using type = IndexSequence<Indices...>;
};
template <typename... Types> struct TypeSequence {
  using type = TypeSequence<Types...>;
};
template <size_t I, typename Sequence> struct CatIndexSequence;
template <size_t I, size_t... Indices>
struct CatIndexSequence<I, IndexSequence<Indices...>>
    : IndexSequence<Indices..., I> {};
template <size_t N>
struct MakeIndexSequence
    : CatIndexSequence<N - 1, typename MakeIndexSequence<N - 1>::type>::type {};
template <> struct MakeIndexSequence<1> : IndexSequence<0> {};
template <size_t N>
using MakeIndexSequence_t = typename MakeIndexSequence<N>::type;
template <size_t, typename T> struct TupleElement {
  T value_;
  TupleElement() = default;
  template <typename U>
  explicit TupleElement(U &&value_) : value_(std::forward<U>(value_)) {}
};
template <typename Sequences, typename... Types> struct TupleImpl;
template <typename> struct IsTupleImpl : std::false_type {};
template <size_t... Indices, typename... Types>
struct IsTupleImpl<TupleImpl<IndexSequence<Indices...>, Types...>>
    : std::true_type {};
template <template <class> class> constexpr bool IsAnyOf() { return false; }
template <template <class> class Obj, typename Head, typename... Tail>
constexpr bool IsAnyOf() {
  return Obj<Head>::value || IsAnyOf<Obj, Tail...>();
}
template <size_t... Indices, typename... Types>
struct TupleImpl<IndexSequence<Indices...>, Types...>
    : TupleElement<Indices, Types>... {
  TupleImpl() = default;
  template <typename... OtherTypes,
            typename = std::enable_if_t<
                !IsAnyOf<IsTupleImpl, std::decay_t<OtherTypes>...>()>>
  explicit TupleImpl(OtherTypes &&... values)
      : TupleElement<Indices, Types>(std::forward<OtherTypes>(values))... {}
};
template <typename... Types>
class Tuple
    : public TupleImpl<MakeIndexSequence_t<sizeof...(Types)>, Types...> {
  using Base_t = TupleImpl<MakeIndexSequence_t<sizeof...(Types)>, Types...>;
  using InnerTypes = TypeSequence<Types...>;

public:
  Tuple() = default;
  Tuple(const Tuple &) = default;
  Tuple(Tuple &&) = default;
  Tuple &operator=(const Tuple &) = default;
  Tuple &operator=(Tuple &&) = default;
  template <typename... OtherTypes>
  explicit Tuple(OtherTypes &&... values)
      : Base_t(std::forward<OtherTypes>(values)...) {}
  void swap(Tuple &other) {
    Tuple tmp = other;
    other = std::move(*this);
    *this = std::move(tmp);
  }
};
template <class T> struct unwrap_refwrapper { using type = T; };

template <class T> struct unwrap_refwrapper<std::reference_wrapper<T>> {
  using type = T &;
};

template <class T>
using special_decay_t =
    typename unwrap_refwrapper<typename std::decay_t<T>>::type;
template <typename... Types> constexpr auto MakeTuple(Types &&... args) {
  return Tuple<special_decay_t<Types>...>(std::forward<Types>(args)...);
}
template <size_t I, typename Head, typename... Tail> struct TypeAtIndex;
template <typename Head, typename... Tail>
struct TypeAtIndex<0, Head, Tail...> {
  using type = Head;
};
template <size_t I, typename Head, typename... Tail> struct TypeAtIndex {
  using type = typename TypeAtIndex<I - 1, Tail...>::type;
};
template <size_t I, typename... Types>
using TypeAtIndex_t = typename TypeAtIndex<I, Types...>::type;
template <size_t I, typename... Types>
constexpr TypeAtIndex_t<I, Types...> &get(Tuple<Types...> &tup) noexcept {
  TupleElement<I, TypeAtIndex_t<I, Types...>> &base = tup;
  return base.value_;
}
template <size_t I, typename... Types>
constexpr TypeAtIndex_t<I, Types...> const &
get(Tuple<Types...> const &tup) noexcept {
  const TupleElement<I, TypeAtIndex_t<I, Types...>> &base = tup;
  return base.value_;
}
template <size_t I, typename... Types>
constexpr std::remove_reference_t<TypeAtIndex_t<I, Types...>> &&
get(Tuple<Types...> &&tup) noexcept {
  std::remove_reference_t<TypeAtIndex_t<I, Types...>> &&base = std::move(tup);
  return base.value_;
}
template <typename> constexpr int CountType() { return 0; }
template <typename T, typename Head, typename... Tail>
constexpr int CountType() {
  return std::is_same<T, Head>::value + CountType<T, Tail...>();
}
template <typename> constexpr int FindType(int) { return -1; }
template <typename T, typename Head, typename... Tail>
constexpr int FindType(int cur_index = 0) noexcept {
  return (std::is_same<T, Head>::value) ? cur_index
                                        : FindType<T, Tail...>(cur_index + 1);
}
template <typename T, typename... Types>
constexpr T &get(Tuple<Types...> &t) noexcept {
  static_assert(CountType<T, Types...>() == 1,
                "Exactly one type expected in Tuple types.");
  return get<FindType<T, Types...>()>(t);
}
template <typename T, typename... Types>
constexpr T const &get(Tuple<Types...> const &t) noexcept {
  static_assert(CountType<T, Types...>() == 1,
                "Exactly one type expected in Tuple types.");
  return get<FindType<T, Types...>()>(t);
}
template <typename T, typename... Types>
constexpr T &&get(Tuple<Types...> &&t) noexcept {
  static_assert(CountType<T, Types...>() == 1,
                "Exactly one type expected in Tuple types.");
  return get<FindType<T, Types...>()>(std::move(t));
}
