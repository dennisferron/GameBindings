// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#pragma once

#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <memory>
#include <map>

#include "boost/intrusive_ptr.hpp"
#include "boost/functional/hash.hpp"
#include "boost/unordered_map.hpp"

#include "LikeMagic/Utility/TypeInfo.hpp"
#include "LikeMagic/Utility/DLLHelper.hpp"

namespace LM {

class TypeIndex;

LIKEMAGIC_API TypeInfo get_info(TypeIndex const& index);

class TypeIndex
{
private:
    std::size_t id;

public:
    TypeIndex() : id(0) {}
    explicit TypeIndex(std::size_t id_) : id(id_) {}
    TypeIndex(TypeIndex const& that) : id(that.id) {}

    inline bool is_initialized() const { return id != 0; }

    inline bool operator <(TypeIndex const& that) const
        { return this->id < that.id; }

    inline bool operator ==(TypeIndex const& that) const
        { return this->id == that.id; }

    inline std::size_t get_id() const { return id; }
    inline TypeInfo get_info() const { return LM::get_info(*this); }
    inline std::string description() const { return get_info().description(); }
};

LIKEMAGIC_API TypeIndex get_index(TypeInfo const& type);
LIKEMAGIC_API TypeIndex get_class_index(TypeIndex index);
LIKEMAGIC_API TypeIndex as_ptr_type(TypeIndex index);
LIKEMAGIC_API TypeIndex as_const_ptr_type(TypeIndex index);
LIKEMAGIC_API TypeIndex as_const_type(TypeIndex index);

typedef std::vector<TypeIndex> TypeInfoList;
LIKEMAGIC_API extern const TypeInfoList& empty_arg_list;

LIKEMAGIC_API TypeIndex create_cpp_type_index(std::type_info const* info_, bool is_const_, bool is_ptr_);
LIKEMAGIC_API TypeIndex create_bottom_ptr_type_index();
LIKEMAGIC_API TypeIndex create_namespace_type_index(std::string namespace_name);

// Value types - not const, not ptr
template <typename T>
struct TypId
{
    TypId() = delete; TypId(TypId const&) = delete; ~TypId() = delete;
    static TypeIndex get()
    {
        return create_cpp_type_index(&typeid(T), false, false);
    }
};

// Const value - is const, not ptr
template <typename T>
struct TypId<T const>
{
    TypId() = delete; TypId(TypId const&) = delete; ~TypId() = delete;
    static TypeIndex get()
    {
        return create_cpp_type_index(&typeid(T), true, false);
    }
};

// Nonconst ptr - not const, is ptr
template <typename T>
struct TypId<T*>
{
    TypId() = delete; TypId(TypId const&) = delete; ~TypId() = delete;
    static TypeIndex get()
    {
        return create_cpp_type_index(&typeid(T), false, true);
    }
};

// ptr to const - is const, is ptr
template <typename T>
struct TypId<T const*>
{
    TypId() = delete; TypId(TypId const&) = delete; ~TypId() = delete;
    static TypeIndex get()
    {
        return create_cpp_type_index(&typeid(T), true, true);
    }
};

// nonconst ref marshals as nonconst ptr
template <typename T>
struct TypId<T&>
{
    TypId() = delete; TypId(TypId const&) = delete; ~TypId() = delete;
    static TypeIndex get()
    {
        return create_cpp_type_index(&typeid(T), false, true);
    }
};

// const ref marshals as const ptr
template <typename T>
struct TypId<T const&>
{
    TypId() = delete; TypId(TypId const&) = delete; ~TypId() = delete;
    static TypeIndex get()
    {
        return create_cpp_type_index(&typeid(T), true, true);
    }
};

}
