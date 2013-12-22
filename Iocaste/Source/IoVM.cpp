// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#include "Iocaste/LikeMagicAdapters/API_Io_Impl.hpp"
#include "Iocaste/LikeMagicAdapters/IoVM.hpp"
#include "Iocaste/LikeMagicAdapters/IoBlock.hpp"
#include "Iocaste/LikeMagicAdapters/IoObjectExpr.hpp"
#include "Iocaste/LikeMagicAdapters/ToIoObjectExpr.hpp"
#include "Iocaste/LikeMagicAdapters/FromIoTypeInfo.hpp"
#include "Iocaste/Exception.hpp"
#include "Iocaste/Breakpoint.hpp"
#include "Iocaste/DebugAPI.hpp"
#include "LikeMagic/TypeConv/NoChangeConv.hpp"

#include "LikeMagic/Utility/TypeDescr.hpp"
#include "LikeMagic/Utility/TypeInfo.hpp"
#include "LikeMagic/BindingMacros.hpp"
#include "LikeMagic/Exceptions/Exception.hpp"

#include "boost/lexical_cast.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <stdexcept>

#ifdef USE_DMALLOC
#include "dmalloc.h"
#endif

// Debugging (to raise breakpoint signal for gdb)
//#include <signal.h>

#include "Iocaste/CShims/IoVMCpp.h"

using namespace std;
using namespace Iocaste;
using namespace Iocaste::LMAdapters;

string IoVM::get_script_path()
{
    return scriptPath;
}

void IoVM::set_script_path(string value)
{
    scriptPath = value;
}

char const* likemagic_proto_id = "LikeMagic";
char const* likemagic_proto_data = "$$$ This is a dummy string to create a unique value for the LikeMagic proto's data pointer. $$$";


void iovm_set_pending_breakpoints(IoMessage *self)
{
    IoVM::get(self)->set_pending_breakpoints(self);
}

StepMode_t iovm_hit_breakpoint(void* bp,
    void *self, void *locals, void *m)
{
    Breakpoint& bkpt = *reinterpret_cast<Breakpoint*>(bp);

    StepMode_t mode = (StepMode_t)io_debugger_break_here(self, locals, m,
        bkpt.breakpoint_number, bkpt.file_name.c_str(),
            bkpt.line_number, bkpt.char_number);

    cout << "iovm_hit_breakpoint returns " << mode << endl;

    return mode;
}

StepMode_t iovm_step_stop(
    void *self, void *locals, void *m)
{
    IoMessageData* data = reinterpret_cast<IoMessageData*>(IoObject_dataPointer(m));

    char const* file_name = CSTRING(data->label);
    int line_number = data->lineNumber;
    int char_number = data->charNumber;

    StepMode_t mode = (StepMode_t)io_debugger_break_here(self, locals, m,
        -1,
        file_name,
        line_number,
        char_number
    );

    cout << "iovm_step_stop returns " << mode << endl;

    return mode;
}

std::string IoVM::get_path(std::string path_identifier)
{
    auto iter = paths.find(path_identifier);

    if (iter == paths.end())
        throw std::runtime_error("Invalid path key.");

    return iter->second;
}
void IoVM::set_path(std::string path_identifier, std::string path_value)
{
    paths[path_identifier] = path_value;
}


IoVM* IoVM::get(IoState* state)
{
    //return reinterpret_cast<IoVM*>(state->callbackContext);
    return static_cast<IoVM*>(state);
}

IoVM* IoVM::get(IoMessage* m)
{
    return get(get_io_state(m));
}

void IoVM::set_pending_breakpoints(IoMessage* m)
{
    IoMessageData* data = reinterpret_cast<IoMessageData*>(IoObject_dataPointer(m));

    data->breakpoint = find_pending_breakpoint(m);

	List_do_(data->args, (ListDoCallback *)iovm_set_pending_breakpoints);

	if (data->next)
	{
		set_pending_breakpoints(data->next);
	}
}

void IoVM::set_breakpoint(int breakpoint_number, const char *file_name, int line_number)
{
    breakpoints.push_back(Breakpoint(breakpoint_number, file_name, line_number, -1));
}

Breakpoint* IoVM::find_pending_breakpoint(std::string file_name, int line_number, int char_number)
{
    for (Breakpoint& bp : breakpoints)
    {
        // TODO:  Some situations may call for just lowest char number
        // TODO:  Use "m" or use separate args?
        if (bp.match(file_name, line_number, char_number))
            return &bp;
    }

    return NULL;
}

Breakpoint* IoVM::find_pending_breakpoint(IoMessage* m)
{
    IoMessageData* data = reinterpret_cast<IoMessageData*>(IoObject_dataPointer(m));
    return find_pending_breakpoint(
        string(CSTRING(data->label)),
        data->lineNumber,
        data->charNumber);
}

// The difference between this and a no-change or implicit conv is this evals in context to return IoObject* directly.
struct PtrToIoObjectConv : public LM::AbstractTypeConverter
{
    static IoObject* eval_in_context(IoObject *self, IoObject *locals, IoMessage *m, IoObject* value)
    {
        return value;
    }

    virtual ExprPtr wrap_expr(ExprPtr expr) const
    {
        return ToIoObjectExpr<IoObject*, PtrToIoObjectConv>::create(expr);
    }

    virtual std::string description() const { return "PtrToIoObjectConv"; }

    virtual float cost() const { return 5.0f; }
};

void IoVM::setShowAllMessages(bool value)
{
    state->showAllMessages = value;
}

IoVM::IoVM(std::string bootstrap_path) : last_exception(0)
{
    set_path("language", bootstrap_path);

    // It's very important you static_cast here, if you use
    // reinterpret_cast or allow conversion to void* which
    // is the function arg, you get a different pointer!
    state = static_cast<IoState*>(this);

    // IoState_new_atAddress assumes the allocator zeroed the memory already,
    // but this is a C++ class derived from IoState, and the C++ allocator didn't zero us.
    // We have to manually zero-out the IoState part of ourself.
    memset(state, 0, sizeof(IoState));

    // "Constructs" the IoState part of us.
    IoState_new_atAddress(state, bootstrap_path.c_str());

    state->bindingsInitCallback = NULL;
    IoState_init(state);
    state->callbackContext = reinterpret_cast<void*>(this);

    io_debugger_init(state);

    IoState_exceptionCallback_(state, &io_exception);

    IoObject* lobby = state->lobby;

    LM_Proxy = create_likemagic_proto();
    IoState_retain_(state, LM_Proxy);
    IoObject_setSlot_to_(lobby, IoState_symbolWithCString_(state, "LikeMagicProxy"),
        LM_Proxy);

    // We want forward to work so that we can suggest method fixes when a method lookup fails.
    IoSymbol* method_symbol = IoState_symbolWithCString_(state, "forward");
    IoObject_addMethod_(LM_Proxy, method_symbol, &API_io_forward);

    LM_Protos = IoObject_new(state);
    IoObject_setSlot_to_(lobby, IoState_symbolWithCString_(state, "LikeMagic"),
        LM_Protos);

    add_convs_from_script(this);
    add_convs_to_script(this);

    auto& global_ns = type_system->global_namespace();

    // Register this vm
    LM_CLASS(global_ns, IoVM)

    LM_FUNC(IoVM, (run_cli)(do_string)(castToIoObjectPointer)(expr_to_io_obj)(setShowAllMessages))

    LM_CLASS(global_ns, TypeSystem)

    LM_CLASS(global_ns, IoObject)

    // To convert an Io object to a void*
    type_system->add_converter_simple(FromIoTypeInfo::create_index("Object"), TypId<void*>::get(), new LM::StaticCastConv<IoObject*, void*>);

    // Make general Io objects convertible with IoObject*.
    type_system->add_converter_simple(FromIoTypeInfo::create_index("Object"), TypId<IoObject*>::get(), new LM::NoChangeConv);
    type_system->add_converter_simple(TypId<IoObject*>::get(), ToIoTypeInfo::create_index("Object"), new PtrToIoObjectConv);
    type_system->add_converter_simple(ToIoTypeInfo::create_index("Object"), ToIoTypeInfo::create_index(), new LM::NoChangeConv);

    // Allow LikeMagic proxy objects to be converted to the C/C++ type IoObject*
    type_system->add_converter_simple(FromIoTypeInfo::create_index("LikeMagic"), TypId<IoObject*>::get(), new LM::NoChangeConv);

    LM_CLASS(global_ns, IoBlock)

    // Allow conversion of Io blocks to IoObject*
    type_system->add_converter_simple(FromIoTypeInfo::create_index("Block"), TypId<IoObject*>::get(), new LM::NoChangeConv);

    // You have to have registered the types before you can add protos for them.

    // Make this vm accessible in the bootstrap environment
    Iocaste::LMAdapters::add_proto<IoVM&>(*this, "io_vm", *this);

    // Also make the abstract type system available by pointer.
    IoObject* ts_io_obj = Iocaste::LMAdapters::add_proto<TypeSystem*>(*this, "type_system", type_system);

    ExprPtr global_ns_expr = create_expr(nullptr, global_ns.get_class_type());

    // The object that represents the global namespace.
    add_value(lobby, "namespace", global_ns_expr, false);

    return;
}

IoObject* IoVM::create_likemagic_proto()
{
    //std::cout << "Creating LikeMagic proto" <<std::endl;

    IoObject *self = IoObject_new(state);

    IoTag* tag = IoTag_newWithName_("LikeMagic");
    IoTag_state_(tag, state);
    IoTag_freeFunc_(tag, (IoTagFreeFunc*)API_io_free_expr);
    IoTag_cloneFunc_(tag, (IoTagCloneFunc*)API_io_rawClone);
    IoTag_markFunc_(tag, (IoTagMarkFunc*)API_io_mark);

    // Added this so that instead of forward, the user func will be called directly.
    // This should be faster because we don't have to wait for proto lookup, and has the advantage that
    // it will work even for methods defined in Object, such as "==", so operator overloads will work.
    IoTag_performFunc_(tag, (IoTagPerformFunc*)API_io_perform);

    IoObject_tag_(self, tag);

    IoObject_setDataPointer_(self, likemagic_proto_data);

	IoState_registerProtoWithId_(state, self, likemagic_proto_id);

    return self;
}

IoState* IoVM::iovm_get_io_state() const
{
    return state;
}

IoVM::~IoVM()
{
    IoState_done(state);

    // Don't free; we're derived from it!
    //IoState_free(state);
}


void IoVM::check_tracking_info(Expr* expr, IoObject* io_obj, IoObject* m) const
{
    auto iter = debug_tracking.find(expr);

    if (iter == debug_tracking.end())
        throw IoStateError(io_obj, "Expr not found in tracking info.", m);
    else
    {
        ExprTrackingInfo info = iter->second;
        //cout << "debug tracking info found for expr=" << info.expr << " data=" << info.data << " name='" << info.name << "'" << endl;
    }
}

void IoVM::set_tracking_info(Expr* expr, std::string name) const
{
    auto iter = debug_tracking.find(expr);

    if (iter == debug_tracking.end())
    {
        debug_tracking[expr] = ExprTrackingInfo(expr, name);
    }
    else
    {
        ExprTrackingInfo info = iter->second;
        //cout << "debug tracking info for expr=" << expr << " as '" << name << "' already exists as expr=" << info.expr << " data=" << info.data << " name='" << info.name << "'" << endl;
    }
}

IoObject* IoVM::castToIoObjectPointer(void* p)
{
    return reinterpret_cast<IoObject*>(p);
}

IoObject* IoVM::add_proto(std::string name, ExprPtr expr, string ns, bool conv_to_script) const
{
    return add_value(LM_Protos, name, expr, conv_to_script);
}

IoObject* IoVM::add_value(IoObject* slot_holder, std::string slot_name, ExprPtr expr, bool conv_to_script) const
{
    IoObject* clone;
    if (conv_to_script)
    {
        clone = to_script(state->lobby, state->lobby, NULL, expr);
    }
    else
    {
        IoObject* proto = LM_Proxy;
        clone = API_io_rawClone(proto);
        Expr* ptr = expr.get();
        IoObject_setDataPointer_(clone, ptr);
        intrusive_ptr_add_ref(ptr);
        set_tracking_info(expr.get(), slot_name.c_str());
    }

    IoObject_setSlot_to_(slot_holder, IoState_symbolWithCString_(state, slot_name.c_str()), clone);

    return clone;
}

IoObject* IoVM::do_string(std::string io_code) const
{
    //std::cout << "IoVM::do_string(" << io_code << ")" << std::endl;
    std::cout << io_code << std::endl;

    IoObject* result = IoState_doCString_(state, io_code.c_str());

    // If we throw the exception here, can't check it in other IoVM code.
    // If we do not throw the exception here, could miss it in user code.
    // TODO:  Use try-catch instead of last_exception check in other IoVM functions.
    //if (last_exception)
    //{
    //    last_exception = 0;
    //    throw std::logic_error("Caught Io exception while running Io code.");
    //}

    return result;
}

void IoVM::run_cli() const
{
    IoState_runCLI(state);
}

ExprPtr IoVM::get_abs_expr(std::string io_code) const
{
    auto io_obj = do_string(io_code);
    static TypeIndex unspec_type = TypeIndex();  // id = -1, type not specified
    ExprPtr result = from_script(state->lobby, io_obj, unspec_type);
    set_tracking_info(result.get(), io_code);
    return result;
}

void IoVM::io_exception(void* context, IoObject* coroutine)
{
    IoCoroutine_rawPrintBackTrace(coroutine);

    IoVM* vm = reinterpret_cast<IoVM*>(context);
	IoObject* e = IoCoroutine_rawException(coroutine);

    cout << "Caught Io exception: ";

    IoObject* self = coroutine;
    IoSymbol* error = IoObject_rawGetSlot_(e, IOSYMBOL("error"));

    cout << CSTRING(error) << endl;

    vm->last_exception = e;
}

void IoVM::mark() const
{
    // If you don't call base mark, child objects don't get marked!
    MarkableObjGraph::mark();

    if (!is_just_testing())
    {
        IoObject_shouldMarkIfNonNull(last_exception);
        IoObject_shouldMarkIfNonNull(LM_Proxy);
    }
}

IoObject* IoVM::expr_to_io_obj(ExprPtr expr)
{
    if (!expr)
        throw logic_error("IoVM::expr_to_io_obj: expr argument was NULL.");

    IoObject* proto = LM_Proxy;
    IoObject* clone = API_io_rawClone(proto);
    IoObject_setDataPointer_(clone, expr.get());
    return clone;
}

void build_arg_exception(string method_name, TypeInfoList arg_types, int i, int arg_count, std::exception const& e)
{
    throw std::logic_error(
           std::string() + "In call of method " + method_name +
           ", error converting argument " + boost::lexical_cast<std::string>(i) + " of " + boost::lexical_cast<std::string>(arg_count) +
           " to type " + arg_types[i].description() + " exception was: " + e.what());
}

IoObject* IoVM::perform(IoObject *self, IoObject *locals, IoMessage *m)
{
 	IoVM* iovm = 0;

    //std::cout << " (type " << IoObject_tag(self)->name << ") perform "  << CSTRING(IoMessage_name(m)) << std::endl << std::flush;

    if (!is_Exprs_obj(self))
    {
        //std::cout << " object is not exprs object, calling IoObject_perform instead." << std::endl;
        return IoObject_perform(self, locals, m);
    }

    try
    {
        if (!IOSTATE->callbackContext)
            throw std::logic_error("The IoState does not have a callbackContext (supposed to contain pointer to IoVM object).");

        iovm = reinterpret_cast<IoVM*>(IOSTATE->callbackContext);

        if (iovm->state != IOSTATE)
            throw std::logic_error("Failed to retrieve IoVM object from IoState callback context.");

        std::string method_name = CSTRING(IoMessage_name(m));

        void* data_ptr = IoObject_dataPointer(self);
        Expr* expr = reinterpret_cast<Expr*>(data_ptr);

        iovm->check_tracking_info(expr, self, m);

        int arg_count = IoMessage_argCount(m);
        TypeIndex exprType = expr->get_type();
        TypeMirror* type_mirror = type_system->get_class(exprType);
        auto* method = type_mirror->get_method(method_name, arg_count);

        // If it's not a C++ method, maybe it is an Io method.  If it is neither,
        // it will end up coming back to us via IoVM::forward where we can throw the method not found exception.
        if (!method)
            return IoObject_perform(self, locals, m);

        std::vector<ExprPtr> args;
        TypeInfoList arg_types = method->get_arg_types();

        for (size_t i=0; i<arg_types.size(); i++)
        {
            try
            {
                ExprPtr expr = get_expr_arg_at(self, locals, m, i, arg_types[i]);
                args.push_back(expr);
            }
            catch (ScriptException const&)
            {
                throw;
            }
            catch (std::exception const& e)
            {
                build_arg_exception(method_name, arg_types, i, arg_count, e);
            }
        }

        auto result = method->call(expr, &args[0]);
        IoObject* result_obj = iovm->to_script(self, locals, m, result);

        return result_obj;
    }
    catch (LM::Exception const& ex)
    {
        cerr << "Caught LikeMagic exception: " << ex.what() << endl << flush;
        throw;
    }
    catch (Iocaste::Exception& e)
    {
        std::cout << "Caught script exception: " << e.what() << std::endl;
        throw;
    }
    catch (std::logic_error const& le)
    {
        //std::cout << "Caught exception: " << le.what() << std::endl;
        IoState_error_(IOSTATE,  m, "C++ %s, %s", LM::demangle_name(typeid(le).name()).c_str(), le.what());
        return IONIL(self);
    }
    catch (std::exception const& e)
    {
        std::cout << "Caught exception: " << e.what() << std::endl;
        IoState_error_(IOSTATE,  m, "C++ %s, %s", LM::demangle_name(typeid(e).name()).c_str(), e.what());
        return IONIL(self);
    }
    catch (...)
    {
        cout << "LikeMagic:  Unknown C++ exception, aborting." << endl;
        abort();
    }
}


IoObject* IoVM::forward(IoObject *self, IoObject *locals, IoMessage *m)
{
    //TODO: Handle cases where forward is called before perform; don't always assert.
    // When you access an object slot via bare "name" rather than "self name"
    // from within an Io method attached to a LikeMagic object,
    // locals forward forwards to this forward; we should look it up and perform it.

    std::string method_name = CSTRING(IoMessage_name(m));
    int arg_count = IoMessage_argCount(m);

    //std::string error_msg = "No C++ object on IoVM::forward " + method_name;
    //IOASSERT(IoObject_dataPointer(self), error_msg.c_str());

    // This causes infinite recursion, so just assert instead
    //if (!IoObject_dataPointer(self))
        //return IoObject_forward(self, locals, m);

    if (!is_Exprs_obj(self))
    {
        string msg = "Slot lookup failed for method " + method_name + " with " + boost::lexical_cast<string>(arg_count) + " arguments, and there is no LikeMagic.";
        throw IoStateError(self, msg, m);
    }

    try
    {
        std::cout << "forward "  << method_name << std::endl;

        auto expr = reinterpret_cast<Expr*>(IoObject_dataPointer(self));

        TypeMirror* type_mirror = type_system->get_class(expr->get_type());
        type_mirror->suggest_method(method_name, arg_count);

        // Never get here; suggest_method always throws.
        return IONIL(self);
    }
    catch (std::logic_error le)
    {
        //std::cout << "Caught exception: " << le.what() << std::endl;
        IoState_error_(IOSTATE,  m, "C++ %s, %s", LM::demangle_name(typeid(le).name()).c_str(), le.what());
        return IONIL(self);
    }
    catch (std::exception e)
    {
        //std::cout << "Caught exception: " << e.what() << std::endl;
        IoState_error_(IOSTATE,  m, "C++ %s, %s", LM::demangle_name(typeid(e).name()).c_str(), e.what());
        return IONIL(self);
    }
    catch (...)
    {
        cout << "LikeMagic:  Unknown C++ exception, aborting." << endl;
        abort();
    }
}


IoObject* IoVM::to_script(IoObject *self, IoObject *locals, IoMessage *m, ExprPtr from_expr) const
{
    static TypeIndex to_io_type = ToIoTypeInfo::create_index();

    if (!from_expr)
        return IOSTATE->ioNil;

    bool is_terminal = from_expr->is_terminal();
    bool disable_to_script = from_expr->disable_to_script_conv();
    bool expr_has_conv = is_terminal && !disable_to_script
        && type_system->has_conv(from_expr->get_type(), to_io_type);

    if (!disable_to_script && from_expr->get_value_ptr().as_const == NULL)
    {
        return IOSTATE->ioNil;
    }
    else if (expr_has_conv)
    {
        ExprPtr to_expr = type_system->try_conv(from_expr, LM::TypId<AbstractToIoObjectExpr*>::get());
        AbstractToIoObjectExpr* io_expr =
            reinterpret_cast<AbstractToIoObjectExpr*>(to_expr->get_value_ptr().as_nonconst);
        IoObject* io_obj = io_expr->eval_in_context(self, locals, m);
        return io_obj;
    }
    else
    {
        IoObject* proto;

        proto = LM_Proxy;
        IoObject* clone = IOCLONE(proto);
        IoObject_setDataPointer_(clone, from_expr.get());

        return clone;
    }
}
