#pragma once
#include <cstddef>
#include <utility>
template <size_t... Indices> struct IndexSequence {
  using type = IndexSequence<Indices...>;
};
template <size_t I, typename Sequence> struct CatIndexSequence;
template <size_t I, size_t... Indices>
struct CatIndexSequence<I, IndexSequence<Indices...>>
    : IndexSequence<Indices..., I> {};
template <size_t N>
struct MakeIndexSequence
    : CatIndexSequence<N - 1, typename MakeIndexSequence<N - 1>::type>::type {};
template <> struct MakeIndexSequence<1> : IndexSequence<0> {};
template <size_t, typename T> struct TupleElement {
  T value_;
  template <typename U>
  explicit TupleElement(U &&value_) : value_(std::forward<U>(value_)) {}
};
template <typename Sequences, typename... Types> struct TupleImpl;
template <typename> struct IsTupleImpl : std::false_type {};
template <size_t... Indices, typename... Types>
struct IsTupleImpl<TupleImpl<IndexSequence<Indices...>, Types...>>
    : std::true_type {};
template <template <class> class> bool IsAnyOf() { return false; }
template <template <class> class Obj, typename Head, typename... Tail>
bool IsAnyOf() {
  return Obj<Head>::value || IsAnyOf<Obj, Tail...>();
}
template <size_t... Indices, typename... Types>
struct TupleImpl<IndexSequence<Indices...>, Types...>
    : TupleElement<Indices, Types>... {
  template <typename... OtherTypes,
            typename std::enable_if_t<
                !IsAnyOf<IsTupleImpl, std::decay_t<OtherTypes>...>>>
  explicit TupleImpl(OtherTypes &&... values)
      : TupleElement<Indices, Types>(std::forward<OtherTypes>(values))... {}
};
template <typename... Types>
class Tuple
    : public TupleImpl<typename MakeIndexSequence<sizeof...(Types)>::type,
                       Types...> {
  using Base_t =
      TupleImpl<typename MakeIndexSequence<sizeof...(Types)>::type, Types...>;
  Tuple() = default;
  Tuple(const Tuple &) = default;
  Tuple(Tuple &&) = default;
  Tuple &operator=(const Tuple &) = default;
  Tuple &operator=(Tuple &&) = default;
  template <typename... OtherTypes>
  explicit Tuple(OtherTypes &&... values)
      : Base_t(std::forward<OtherTypes>(values)...) {}
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
