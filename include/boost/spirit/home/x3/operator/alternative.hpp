/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_ALTERNATIVE_JAN_07_2013_1131AM)
#define BOOST_SPIRIT_X3_ALTERNATIVE_JAN_07_2013_1131AM

#include <boost/spirit/home/x3/core/parser.hpp>
#include <boost/spirit/home/x3/operator/detail/alternative.hpp>
#include <boost/spirit/home/x3/support/meta.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <type_traits>

namespace boost { namespace spirit { namespace x3
{
    template <typename Left, typename Right>
    struct alternative : binary_parser<Left, Right, alternative<Left, Right>>
    {
        typedef binary_parser<Left, Right, alternative<Left, Right>> base_type;

        constexpr alternative(Left const& left, Right const& right)
            : base_type(left, right) {}

        template <typename Iterator, typename Context, typename RContext>
        bool parse(
            Iterator& first, Iterator const& last
          , Context const& context, RContext& rcontext, unused_type) const
        {
            return this->left.parse(first, last, context, rcontext, unused)
               || this->right.parse(first, last, context, rcontext, unused);
        }

        template <typename Iterator, typename Context
          , typename RContext, typename Attribute>
        bool parse(
            Iterator& first, Iterator const& last
          , Context const& context, RContext& rcontext, Attribute& attr) const
        {
            return detail::parse_alternative(this->left, first, last, context, rcontext, attr)
               || detail::parse_alternative(this->right, first, last, context, rcontext, attr);
        }
    };

    template <typename Left, typename Right>
    constexpr alternative<
        typename extension::as_parser<Left>::value_type
      , typename extension::as_parser<Right>::value_type>
    operator|(Left const& left, Right const& right)
    {
        return { as_parser(left), as_parser(right) };
    }
}}}


namespace boost { namespace spirit { namespace x3 { namespace detail
{
    template <typename Seq, typename... Ts>
    struct add_alternative_types_impl;

    template <template<class...> typename Seq, typename... Ts>
    struct add_alternative_types_impl<Seq<Ts...>>
    {
        using type = Seq<Ts...>;
    };

    template <template<class...> typename Seq, typename... Ts, typename U, typename... Us>
    struct add_alternative_types_impl<Seq<Ts...>, U, Us...>
    {
        using next_sequence = conditional_t<Seq<Ts...>::template contains<U>,
            Seq<Ts...>,
            conditional_t<std::is_same_v<std::remove_const_t<U>, unused_type>,
                typename Seq<Ts...>::template prepend<U>,
                typename Seq<Ts...>::template append<U>
            >
        >;

        using type = typename add_alternative_types_impl<next_sequence, Us...>::type;
    };

    template <typename Seq, typename... Ts>
    using add_alternative_types = typename add_alternative_types_impl<Seq, Ts...>::type;

    template <typename... Seqs>
    struct merge_types_of_alternative_impl;

    template <template <class...> typename Seq1, typename... T1s, template <class...> typename Seq2, typename... T2s>
    struct merge_types_of_alternative_impl<Seq1<T1s...>, Seq2<T2s...>>
    {
        using type = add_alternative_types<Seq1<T1s...>, T2s...>;
    };

    template <typename... Seqs>
    using merge_types_of_alternative = typename merge_types_of_alternative_impl<Seqs...>::type;

    template <typename P, typename C>
    struct get_types_of_alternative
    {
        using type = type_sequence<typename traits::attribute_of<P, C>::type>;
    };

    template <typename L, typename R, typename C>
    struct get_types_of_alternative<alternative<L, R>, C>
    {
        using type = merge_types_of_alternative<
            typename get_types_of_alternative<L, C>::type,
            typename get_types_of_alternative<R, C>::type
        >;
    };

    template <template <typename...> typename A, typename Seq>
    struct type_sequence_to_alternative_attribute;

    template <template <typename...> typename A, template <typename...> typename Seq>
    struct type_sequence_to_alternative_attribute<A, Seq<>>
    {
        using type = unused_type;
    };

    template <template <typename...> typename A, template <typename...> typename Seq, typename T, typename... Ts>
    struct type_sequence_to_alternative_attribute<A, Seq<T, Ts...>>
    {
        using type = conditional_t<sizeof...(Ts) == 0,
            T,
            A<T, Ts...>
        >;
    };

    template <template <typename...> typename A, template <typename...> typename Seq, typename T, typename... Ts>
    struct type_sequence_to_alternative_attribute<A, Seq<unused_type, T, Ts...>>
    {
       using type = conditional_t<sizeof...(Ts) == 0,
           boost::optional<T>,
           boost::optional<A<T, Ts...>>
       >;
    };

    template <template <typename...> class A, typename P, typename C>
    using attribute_of_alternative = type_sequence_to_alternative_attribute<A,
        typename get_types_of_alternative<P, C>::type>;
}}}}

namespace boost { namespace spirit { namespace x3 { namespace traits
{
    template <typename Left, typename Right, typename Context>
    struct attribute_of<x3::alternative<Left, Right>, Context>
        : x3::detail::attribute_of_alternative<boost::variant, x3::alternative<Left, Right>, Context> {};
}}}}

#endif
