#pragma once
#include <cstddef>
#include <utility>
template <typename... Types> class Tuple;
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
  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  explicit TupleElement(U &&value_) : value_(std::forward<U>(value_)) {}
};
template <typename... Types> struct IsTuple : std::false_type {};
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
template <class...> struct conjunction : std::true_type {};
template <class B1> struct conjunction<B1> : B1 {};
template <class B1, class... Bn>
struct conjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};
template <class...> struct disjunction : std::false_type {};
template <class B1> struct disjunction<B1> : B1 {};
template <class B1, class... Bn>
struct disjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};
template <size_t Val1, size_t Val2> struct IsEqual : std::false_type {};
template <size_t Val1> struct IsEqual<Val1, Val1> : std::true_type {};
template <size_t... Indices, typename... Types>
struct TupleImpl<IndexSequence<Indices...>, Types...>
    : TupleElement<Indices, Types>... {
  TupleImpl() = default;
  template <
      typename... OtherTypes,
      typename = std::enable_if_t<sizeof...(Types) == sizeof...(OtherTypes)>>
  explicit TupleImpl(OtherTypes &&... values)
      : TupleElement<Indices, Types>(std::forward<OtherTypes>(values))... {}
  /*template <
      typename... OtherTypes,
      typename = std::enable_if_t<!IsEqual<sizeof...(OtherTypes), 1>::value>>
  explicit TupleImpl(OtherTypes &&... values)
      : TupleElement<Indices, Types>(std::forward<OtherTypes>(values))... {}*/
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
  static constexpr size_t Size = sizeof...(Types);
  template <typename... OtherTypes>
  explicit Tuple(OtherTypes &&... values)
      : Base_t(std::forward<OtherTypes>(values)...) {}
  void swap(Tuple &other) {
    Tuple tmp = other;
    other = std::move(*this);
    *this = std::move(tmp);
  }
  /*template <typename... OtherTypes, size_t... Indices>
  explicit Tuple(Tuple<OtherTypes...> &&other, IndexSequence<Indices...>)
      : Base_t(std::forward<TypeAtIndex_t<Indices, OtherTypes...>(get<Indices>(
                   std::forward<Tuple<OtherTypes...>>(other)))...>) {}
  template <typename... OtherTypes>
  explicit Tuple(Tuple<OtherTypes...> &&other)
      : Tuple(std::forward<Tuple<OtherTypes...>>(other),
              MakeIndexSequence<Size>()) {}
  template <typename... OtherTypes>
  constexpr bool operator<(const Tuple<OtherTypes...> &other) {
    for (size_t i = 0; i < Size; ++i) {
      if (get<i>(*this) < get<i>(other))
        return true;
      else if (get<i>(*this) > get<i>(other))
        return false;
    }
    return false;
  }
  template <typename... OtherTypes>
  constexpr bool operator==(const Tuple<OtherTypes...> &other) {
    for (size_t i = 0; i < Size; ++i) {
      if (get<i>(*this) != get<i>(other))
        return false;
    }
    return true;
  }*/
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
template <typename Tuple1, size_t... Indices1, typename Tuple2,
          size_t... Indices2>
auto TwoTuplesCatHelper(Tuple1 &&t1, IndexSequence<Indices1...>, Tuple2 &&t2,
                        IndexSequence<Indices2...>) {
  return MakeTuple(get<Indices1>(std::forward<Tuple1>(t1))...,
                   get<Indices2>(std::forward<Tuple2>(t2))...);
}
template <typename Tuple1, typename Tuple2>
constexpr auto TwoTuplesCat(Tuple1 &&t1, Tuple2 &&t2) {
  return TwoTuplesCatHelper(
      std::forward<Tuple1>(t1),
      MakeIndexSequence<std::remove_reference_t<Tuple1>::Size>(),
      std::forward<Tuple2>(t2),
      MakeIndexSequence<std::remove_reference_t<Tuple2>::Size>());
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
