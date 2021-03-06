
#include "Iocaste/Stack.hpp"
#include "Iocaste/CShims/Stack.h"
#include "Iocaste/IoCallStack.hpp"

#include "IoObject_inline.h"

using namespace Iocaste;

#include "List.h"

#include <stdexcept>
#include <vector>
#include <iostream>
using namespace std;

#include "LikeMagic/Utility/TraceDb.hpp"
using namespace LM;

void IoCallStack::retain_data(IoObject* io_obj)
{
    items.push_back(io_obj);
    IF_TRACE_DB(trace_db->IoStack_push_object(io_obj));
}

void IoCallStack::retain_call(IoObject* io_call)
{
    items.push_back(io_call);
    IF_TRACE_DB(trace_db->IoStack_push_object(io_call));
}

void IoCallStack::mark() const
{
    for (auto& obj : this->items)
        IoObject_shouldMark(obj);
}

IoCallStack::mark_type IoCallStack::push_mark(MarkReason reason)
{
    marks.push_back({items.size(), reason});
    IF_TRACE_DB(trace_db->IoStack_pop_mark(items.size()));
    return marks.size()-1;
}

void IoCallStack::pop_to_mark_point(mark_type mark)
{
    items.erase(items.begin() + marks[mark].index, items.end());
    marks.erase(marks.begin() + mark, marks.end());
    IF_TRACE_DB(trace_db->IoStack_pop_mark(mark));
}

void IoCallStack::pop_mark()
{
    pop_to_mark_point(marks.size()-1);
}

IoObject* IoCallStack::top() const
{
    return items.back();
}

std::size_t IoCallStack::size()
{
    return items.size();
}

IoCallStack::const_reverse_iterator IoCallStack::rbegin() const
{
    return items.rbegin();
}

IoCallStack::const_reverse_iterator IoCallStack::rend() const
{
    return items.rend();
}

IoCallStack::const_iterator IoCallStack::begin() const
{
    return items.begin();
}

IoCallStack::const_iterator IoCallStack::end() const
{
    return items.end();
}

void IoCallStack::clear()
{
    items.clear();
    marks.clear();
}

// TODO:  Possibly require mark value to verify correct mark.
void IoCallStack::clear_to_last_mark()
{
    items.erase(items.begin() + marks.back().index, items.end());
}
