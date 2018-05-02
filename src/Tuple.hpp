#pragma once
#include <cstddef>
#include <utility>
template <typename... Types> class Tuple;
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
template <size_t N>
using MakeIndexSequence_t = typename MakeIndexSequence<N>::type;
template <size_t, typename T> struct TupleElement {
  T value_;
  TupleElement() = default;
  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  explicit TupleElement(U &&value_) : value_(std::forward<U>(value_)) {}
};
template <typename Sequences, typename... Types> struct TupleImpl;
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
constexpr TypeAtIndex_t<I, Types...> &get(Tuple<Types...> &tup) noexcept;
template <typename T, typename... Types>
constexpr T const &get(Tuple<Types...> const &t) noexcept;
template <typename T, typename... Types>
constexpr T &&get(Tuple<Types...> &&t) noexcept;
template <size_t... Indices, typename... Types>
struct TupleImpl<IndexSequence<Indices...>, Types...>
    : TupleElement<Indices, Types>... {
  TupleImpl() = default;
  template <
      typename... OtherTypes,
      typename = std::enable_if_t<sizeof...(Types) == sizeof...(OtherTypes)>>
  explicit TupleImpl(OtherTypes &&... values)
      : TupleElement<Indices, Types>(std::forward<OtherTypes>(values))... {}
};
template <typename... Types>
class Tuple
    : public TupleImpl<MakeIndexSequence_t<sizeof...(Types)>, Types...> {
  using Base_t = TupleImpl<MakeIndexSequence_t<sizeof...(Types)>, Types...>;

public:
  Tuple() = default;
  Tuple(const Tuple &) = default;
  Tuple(Tuple &&) = default;
  Tuple &operator=(const Tuple &) = default;
  Tuple &operator=(Tuple &&) = default;
  static constexpr size_t Size = sizeof...(Types);
  template <typename... OtherTypes>
  explicit Tuple(OtherTypes &&... values)
      : Base_t(std::forward<OtherTypes>(values)...) {}
  void swap(Tuple &other) {
    Tuple tmp = other;
    other = std::move(*this);
    *this = std::move(tmp);
  }
};
template <typename... Types> constexpr size_t Tuple<Types...>::Size;
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
  TupleElement<I, std::remove_reference_t<TypeAtIndex_t<I, Types...>>> &&base =
      std::move(tup);
  return std::move(base.value_);
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
template <typename Tuple1, size_t... Indices1, typename Tuple2,
          size_t... Indices2>
auto __TwoTuplesCatHelper(Tuple1 &&t1, IndexSequence<Indices1...>, Tuple2 &&t2,
                          IndexSequence<Indices2...>) {
  return MakeTuple(get<Indices1>(std::forward<Tuple1>(t1))...,
                   get<Indices2>(std::forward<Tuple2>(t2))...);
}
template <typename Tuple1, typename Tuple2>
constexpr auto TwoTuplesCat(Tuple1 &&t1, Tuple2 &&t2) {
  return __TwoTuplesCatHelper(
      std::forward<Tuple1>(t1),
      MakeIndexSequence<std::remove_reference_t<Tuple1>::Size>(),
      std::forward<Tuple2>(t2),
      MakeIndexSequence<std::remove_reference_t<Tuple2>::Size>());
}
template <typename... Tuples> constexpr auto TupleCat(Tuples &&...);
template <typename HeadTuple1, typename HeadTuple2, typename... TailTuples>
constexpr auto TupleCat(HeadTuple1 &&t1, HeadTuple2 &&t2,
                        TailTuples &&... OtherTuples) {
  return TwoTuplesCat(
      TwoTuplesCat(std::forward<HeadTuple1>(t1), std::forward<HeadTuple2>(t2)),
      TupleCat(OtherTuples...));
}
template <typename HeadTuple1, typename HeadTuple2>
constexpr auto TupleCat(HeadTuple1 &&t1, HeadTuple2 &&t2) {
  return TwoTuplesCat(std::forward<HeadTuple1>(t1),
                      std::forward<HeadTuple2>(t2));
}
template <typename HeadTuple> constexpr auto TupleCat(HeadTuple &&t) {
  return std::forward<HeadTuple>(t);
}
template <typename _Tp, typename _Up, size_t __i, size_t __size>
struct __TupleCompare {
  static constexpr bool __eq(const _Tp &__t, const _Up &__u) {
    return bool(get<__i>(__t) == get<__i>(__u)) &&
           __TupleCompare<_Tp, _Up, __i + 1, __size>::__eq(__t, __u);
  }

  static constexpr bool __less(const _Tp &__t, const _Up &__u) {
    return bool(get<__i>(__t) < get<__i>(__u)) ||
           (!bool(get<__i>(__u) < get<__i>(__t)) &&
            __TupleCompare<_Tp, _Up, __i + 1, __size>::__less(__t, __u));
  }
};

template <typename _Tp, typename _Up, size_t __size>
struct __TupleCompare<_Tp, _Up, __size, __size> {
  static constexpr bool __eq(const _Tp &, const _Up &) { return true; }

  static constexpr bool __less(const _Tp &, const _Up &) { return false; }
};
template <typename... Types1, typename... Types2>
constexpr bool operator==(const Tuple<Types1...> &t1,
                          const Tuple<Types2...> &t2) {
  static_assert(sizeof...(Types1) == sizeof...(Types2),
                "Only tuples of same length can be compared");
  return __TupleCompare<Tuple<Types1...>, Tuple<Types2...>, 0,
                        sizeof...(Types1)>::__eq(t1, t2);
}
template <typename... Types1, typename... Types2>
constexpr bool operator<(const Tuple<Types1...> &t1,
                         const Tuple<Types2...> &t2) {
  static_assert(sizeof...(Types1) == sizeof...(Types2),
                "Only tuples of same length can be compared");
  return __TupleCompare<Tuple<Types1...>, Tuple<Types2...>, 0,
                        sizeof...(Types1)>::__less(t1, t2);
}
