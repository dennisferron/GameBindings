// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)


#pragma once

#include <utility> // std::pair
#include <map>
#include <bitset>

#include "boost/graph/graph_traits.hpp"
#include "boost/graph/adjacency_list.hpp"

#include "boost/unordered_map.hpp"

#include "LikeMagic/TypeConv/AbstractTypeConverter.hpp"
#include "LikeMagic/Exprs/Expr.hpp"

#include "LikeMagic/Utility/TypeInfo.hpp"

namespace LM {

using namespace boost::graph;

struct FindType;
struct EdgePropertyWriter;

class TypeConvGraph
{
private:
    // To give it acess to our private typedefs.
    friend struct FindType;
    friend struct EdgePropertyWriter;

    typedef boost::intrusive_ptr<AbstractTypeConverter const> p_conv_t;
    typedef boost::shared_ptr<std::vector<p_conv_t>> p_chain_t;

    struct edge_info
    {
        p_conv_t conv;
        float cost;
    };

    struct vertex_info
    {
        TypeIndex type;

        vertex_info() : type(TypId<void>::liberal()) { }
    };

    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, vertex_info, edge_info> graph_t;
    typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
    typedef boost::graph_traits<graph_t>::edge_descriptor edge_t;
    typedef std::vector<vertex_t> vertex_map_t;

    graph_t graph;
    vertex_map_t vertex_map;
    std::vector<bool> has_vertex;

    // Mutable is for when the object is logically const, but technically a member needs
    // to sometimes change in a way that's not visible from outside the class.
    // Caching is a perfect example of this.
    mutable boost::unordered_map<std::pair<std::size_t, std::size_t>, p_chain_t> conv_cache;

    ExprPtr build_conv_chain(ExprPtr from_expr, p_chain_t const& chain) const;
    p_chain_t const& search_for_conv(TypeIndex from, TypeIndex to) const;
    p_conv_t get_conv(TypeIndex from, TypeIndex to) const;

    // Don't allow TypeConvGraph to be copied accidently.
    TypeConvGraph(TypeConvGraph const&)=delete;
    TypeConvGraph& operator =(TypeConvGraph const&)=delete;

    vertex_t const no_vertex;

    p_conv_t bot_conv;

    /*
    Testing for a conversion can record a nullptr in the cache if the conversion fails.
    However if we later add a converter it may enable new conversions so we need to re-check.
    On the other hand we don't want to re-test every conversion to-script every time we
    return a value to Io.  So as a performance optimization we may want to turn this
    variable from true to false only after all type converters have been added.
    */
    bool recheck_failed_convs;

public:
    TypeConvGraph();
    ~TypeConvGraph();
    vertex_t  add_type(TypeIndex type);
    void add_conv(TypeIndex from, TypeIndex to, p_conv_t conv);
    ExprPtr wrap_expr(ExprPtr from_expr, TypeIndex from, TypeIndex to) const;
    bool has_conv(TypeIndex from_type, TypeIndex to_type) const;
    bool has_direct_conv(TypeIndex from_type, TypeIndex to_type) const;
    void print_graph() const;
    void print_conv_chain(TypeIndex from, TypeIndex to) const;
    void print_conv_chain(p_chain_t const& chain) const;
    bool has_type(TypeIndex type) const;
};

}
