/* ***** BEGIN LICENSE BLOCK Version: GPL 3.0 ***** 
 * Copyright (C) 2008-2011  Hayaki Saito <user@zuse.jp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

//////////////////////////////////////////////////////////////////////////////
//
// definition of basic types and interfaces
//

#include "services.hpp"

namespace ecmascript {

    struct es_nil_t {};

    namespace VT {

        enum VT
        {
            Primitive   = 0,
            Undefined   = 1,
            Null        = 2,
            Boolean     = 3,
            Number      = 4,
            String      = 5,
            Object      = 6,
            Reference   = 7,
            Metadata    = 8,
        };

    } // namespace ecmascript::VT

    enum TypeHint 
    {
        TH_Primitive    = 0x0 << 2 & 0x3,
        TH_Undefined    = 0x1 << 2 & 0x3,
        TH_Null         = 0x2 << 2 & 0x3,
        TH_Boolean      = 0x3 << 2 & 0x3,
        TH_Number       = 0x4 << 2 & 0x3,
//        TH_SmallInt     = 0x5 << 2 & 0x3
//      TH_LargeInt       = 0x6 << 2 & 0x3
        TH_String       = 0x7 << 2 & 0x3,
//        TH_NullString   = 0x8 << 2 & 0x3,
        TH_Reference    = 0x9 << 2 & 0x3,
        TH_RegExp       = 0xa << 2 & 0x3,
        TH_Function     = 0xb << 2 & 0x3,
        TH_Object       = 0xc << 2 & 0x3,
        TH_Array        = 0xd << 2 & 0x3,
        TH_Empty        = 0xe << 2 & 0x3,
    };
    
    enum StaticState 
    {        
        SS_Immediate    = 0x0 << 16,
        SS_Variable     = 0x1 << 16,
        SS_Expression   = 0x2 << 16,
        SS_Statement    = 0x3 << 16,
    };

    enum Rule
    {
        Nop,
        And,
        Or,
        Alternative,
        If,
        IfElse,
        DoWhile,
        While,
        For,
        ForIn,
        Switch,
        Case,
        Default,
        With,
        Identifier,
        Bracket,
        Member,
        Call,
        Args,
        Arg,
        ArgEnd,
        NewArgEnd,
        String,
        SingleQuotedString,
        DoubleQuotedString,
        NullString,
        Null,
        RegExp,
        Undefined,
        Number,
        SmallInteger,
        True,
        False,
        Func,
        Parameter,
        FunctionRoot,
        TryCatch,
        TryFinally,
        Label,
        New,
        Throw,
        Var,
        VarInit,
        ForInVar,
        ForInVarInit,
        Array,
        NullArray,
        ArrayElement,
        Object,
        NullObject,
        ObjectElement,
        This,
        PostInc,
        PostDec,
        Delete,
        Void,
        TypeOf,
        Inc,
        Dec,
        UnaryPlus,
        UnaryMinus,
        Tilde,
        Not,
        Mul,
        Div,
        Mod,
        Plus,
        Minus,
        Shl,
        Sar,
        Shr,
        Lt,
        Gt,
        Le,
        Ge,
        InstanceOf,
        In,
        Eq,
        Ne,
        StrictEq,
        StrictNe,
        BitAnd,
        BitXor,
        BitOr,
        Assign,
        AssignMul,
        AssignDiv,
        AssignMod,
        AssignPlus,
        AssignMinus,
        AssignShl,
        AssignSar,
        AssignShr,
        AssignAnd,
        AssignXor,
        AssignOr,
        Continue,
        ContinueNoArg,
        Break,
        BreakNoArg,
        Return,
        ReturnNoArg,
    };

#ifdef ES_TRACE_NODE
    static wchar_t const* RuleSymbol[] = {
        L"Nop",
        L"And",
        L"Or",
        L"Alternative",
        L"DoWhile",
        L"While",
        L"For",
        L"ForIn",
        L"Switch",
        L"Case",
        L"Default",
        L"With",
        L"Identifier",
        L"Bracket",
        L"Member",
        L"Call",
        L"Args",
        L"Arg",
        L"ArgEnd",
        L"NewArgEnd",
        L"String",
        L"SingleQuotedString",
        L"DoubleQuotedString",
        L"NullString",
        L"Null",
        L"RegExp",
        L"Undefined",
        L"Number",
        L"True",
        L"False",
        L"Func",
        L"TryCatch",
        L"TryFinally",
        L"Label",
        L"New",
        L"Throw",
        L"Var",
        L"VarInit",
        L"ForInVar",
        L"ForInVarInit",
        L"Array",
        L"NullArray",
        L"ArrayElement",
        L"Object",
        L"NullObject",
        L"ObjectElement",
        L"This",
        L"PostInc",
        L"PostDec",
        L"Delete",
        L"Void",
        L"TypeOf",
        L"Inc",
        L"Dec",
        L"UnaryPlus",
        L"UnaryMinus",
        L"Tilde",
        L"Not",
        L"Mul",
        L"Div",
        L"Mod",
        L"Plus",
        L"Minus",
        L"Shl",
        L"Sar",
        L"Shr",
        L"Lt",
        L"Gt",
        L"Le",
        L"Ge",
        L"InstanceOf",
        L"In",
        L"Eq",
        L"Ne",
        L"StrictEq",
        L"StrictNe",
        L"BitAnd",
        L"BitXor",
        L"BitOr",
        L"Assign",
        L"AssignMul",
        L"AssignDiv",
        L"AssignMod",
        L"AssignPlus",
        L"AssignMinus",
        L"AssignShl",
        L"AssignSar",
        L"AssignShr",
        L"AssignAnd",
        L"AssignXor",
        L"AssignOr",
        L"Continue",
        L"ContinueNoArg",
        L"Break",
        L"BreakNoArg",
        L"Return",
        L"ReturnNoArg"
    };
#endif // ES_TRACE_NODE

} // namespace ecmascript

#if defined(__MINGW32__) && !defined(__CYGWIN__)
namespace std {
    typedef std::basic_string<wchar_t> wstring;
}
#endif // defined(__MINGW__) && !defined(__CYGWIN__)

// interface

namespace ecmascript {

    enum CallType
    {
        Cdecl = 0x000,
        Stdcall = 0x001,
    };

    enum MethodType
    {
        Method = 0x000,
        Property = 0x002,
    };

    enum Writability
    {
        ReadWrite = 0x000,
        ReadOnly = 0x004,
    };

    enum Deletability
    {
        CanDelete = 0x000,
        DontDelete = 0x008,
    };

    enum Enumerability
    {
        Enumerable = 0x000,
        NonEnumerable = 0x010,
    };

} // namespace ecmascript

//////////////////////////////////////////////////////////////////////////////
//
// internace definition
//
namespace ecmascript {

    struct IPrimitive;
    struct IReference;
    struct IUndefined;
    struct IBoolean;
    struct INumber;
    struct IString;
    struct IPrimitive;
    struct IFunction;
    struct IArray;
    struct IObject;
    struct IArguments;
    struct IActorBase;
    struct IActor;

    struct Completion
    {
        CompletionType type;
        union {
            IPrimitive *p_value;
            IActor *p_closure;
        };
    };

    struct es_init_map
    {
        wchar_t const* name;
        IPrimitive *object;
    };
  
    struct es_attributes
    {
        enum { id = VT::Metadata };
        wchar_t const* name;
        ecmascript::uint32_t offset;
        unsigned short flag;
        unsigned int argc;
        es_attributes const* parent;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IEmpty
    //
    struct IEmpty
    {
        enum { vtsize = 0 };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IManageProperty
    //
    template <typename baseT>
    struct IManageProperty
    : baseT
    {
        enum { vtsize = baseT::vtsize + 5 };
        typedef std::basic_string<wchar_t> string_t;
        virtual IPrimitive& __stdcall get_value__() = 0;
        virtual IPrimitive& __stdcall get__(const_string_t const&) = 0;
        virtual IPrimitive& __stdcall get_by_value__(IPrimitive const&) = 0;
        virtual void __stdcall put__(const_string_t const&, IPrimitive&) = 0;
        virtual void __stdcall put_by_value__(IPrimitive const&, IPrimitive&) = 0;

        enum
        {
            verb_get_value__      = baseT::vtsize + 0,
            verb_get__            = baseT::vtsize + 1,
            verb_get_by_value__   = baseT::vtsize + 2,
            verb_put__            = baseT::vtsize + 3,
            verb_put_by_value__   = baseT::vtsize + 4,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IManageScopeChain
    //
    template <typename baseT>
    struct IManageScopeChain
    : baseT
    {
        enum { vtsize = baseT::vtsize + 2 };
        virtual IPrimitive& get_scope__() = 0;
        virtual void set_scope__(IPrimitive&) = 0;

        enum
        {
            verb_get_scope__      = baseT::vtsize + 0,
            verb_set_scope__      = baseT::vtsize + 1,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IManagePrototypeChain
    //
    template <typename baseT>
    struct IManagePrototypeChain
    : baseT
    {
        enum { vtsize = baseT::vtsize + 2 };
        virtual IPrimitive& __stdcall get_prototype__() = 0;
        virtual void __stdcall set_prototype__(IPrimitive&) = 0;

        enum
        {
            verb_get_prototype__  = baseT::vtsize + 0,
            verb_set_prototype__  = baseT::vtsize + 1,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IEnumProperty
    //
    template <typename baseT>
    struct IEnumProperty
    : baseT
    {
        enum { vtsize = baseT::vtsize + 2 };
        virtual IPrimitive& __stdcall reset__() = 0;
        virtual IPrimitive& __stdcall next__() = 0;

        enum
        {
            verb_reset__          = baseT::vtsize + 0,
            verb_next__           = baseT::vtsize + 1,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IRundomAccess
    //
    template <typename baseT>
    struct IRundomAccess
    : baseT
    {
        enum { vtsize = baseT::vtsize + 4 };
        virtual void __stdcall push__(IPrimitive& value) = 0;
        virtual IPrimitive& operator [](ecmascript::uint32_t index) = 0;
        virtual ecmascript::uint32_t length__() = 0;
        virtual IPrimitive ** begin__() = 0;

        enum
        {
            verb_push__           = baseT::vtsize + 0,
            verb_index__          = baseT::vtsize + 1,
            verb_length__         = baseT::vtsize + 2,
            verb_begin__          = baseT::vtsize + 3,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IManageLifeCycle
    //
    template <typename baseT>
    struct IManageLifeCycle
    : baseT
    {
        enum { vtsize = baseT::vtsize + 2 };
        virtual void mark__(int) = 0;
        virtual void sweep__(int) = 0;

        enum
        {
            verb_mark__           = baseT::vtsize + 0,
            verb_sweep__          = baseT::vtsize + 1,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IReferenceCounting
    //
    template <typename baseT>
    struct IReferenceCounting
    : baseT
    {
        enum { vtsize = baseT::vtsize + 3 };
        virtual ecmascript::uint32_t addref__() = 0;
        virtual ecmascript::uint32_t release__() = 0;
        virtual bool collect__() = 0;

        enum
        {
            verb_addref__         = baseT::vtsize + 0,
            verb_release__        = baseT::vtsize + 1,
            verb_collect__        = baseT::vtsize + 2,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  INativeTypeConversion
    //
    template <typename baseT>
    struct INativeTypeConversion
    : baseT
    {
        enum { vtsize = baseT::vtsize + 9 };
        typedef std::basic_string<wchar_t> string_t;
        typedef es_const_string<wchar_t> const_string_t;
        virtual __stdcall operator bool() const = 0;
        virtual operator double() const = 0;
        virtual operator const_string_t const() const = 0;
        virtual operator string_t const() const = 0;
        virtual operator ecmascript::integer_t() const = 0;
        virtual operator ecmascript::int32_t() const = 0;
        virtual operator ecmascript::uint32_t() const = 0;
        virtual operator ecmascript::uint16_t() const = 0;
        virtual operator es_attributes const&() const = 0;

        enum
        {
            verb_operator_bool                 = baseT::vtsize + 0,
            verb_operator_double               = baseT::vtsize + 1,
            verb_operator_const_string_t_const = baseT::vtsize + 2,
            verb_operator_string_t_const       = baseT::vtsize + 3,
            verb_operator_integer_t            = baseT::vtsize + 4,
            verb_operator_int32_t              = baseT::vtsize + 5,
            verb_operator_uint32_t             = baseT::vtsize + 6,
            verb_operator_uint16_t             = baseT::vtsize + 7,
            verb_operator_es_attributes_const  = baseT::vtsize + 8,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  ILexicalConversion
    //
    template <typename baseT>
    struct ILexicalConversion
    : baseT
    {
        enum { vtsize = baseT::vtsize + 8 };
        
        virtual IBoolean& ToBoolean() const = 0;
        virtual INumber& ToNumber() const = 0;
        virtual IString& ToString() const = 0;
        virtual INumber const& ToInteger() const = 0;
        virtual INumber const& ToInt32() const = 0;
        virtual INumber const& ToUint32() const = 0;
        virtual IPrimitive const& ToPrimitive(unsigned char) const = 0;
        virtual IPrimitive& ToObject() = 0;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IInvoke
    //
    template <typename baseT>
    struct IInvoke
    : baseT
    {
        enum { vtsize = baseT::vtsize + 2 };
        
        virtual IPrimitive& __stdcall call__(IPrimitive&, IPrimitive&) = 0;
        virtual IPrimitive& __stdcall construct__(IPrimitive&) = 0;

        enum
        {
            verb_call__                       = baseT::vtsize + 0,
            verb_construct__                  = baseT::vtsize + 1,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  ITypeInformation
    //
    template <typename baseT>
    struct ITypeInformation
    : baseT
    {
        enum { vtsize = baseT::vtsize + 5 };
        virtual unsigned char __stdcall type__() const = 0;
        virtual const_string_t const class__() const = 0;
        virtual void const* address__() const = 0;
        virtual IBoolean& has_instance__(IPrimitive const&) const = 0;
        virtual bool has_property__(const_string_t const&) = 0;

        enum
        {
            verb_type__                       = baseT::vtsize + 0,
            verb_class__                      = baseT::vtsize + 1,
            verb_address__                    = baseT::vtsize + 2,
            verb_has_instance__               = baseT::vtsize + 3,
            verb_has_property__               = baseT::vtsize + 4,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IPostfixOperators
    //
    template <typename baseT>
    struct IPostfixOperators
    : baseT
    {
        enum { vtsize = baseT::vtsize + 2 };
        // postfix operators
        virtual IPrimitive& __stdcall postfix_inc__() = 0;
        virtual IPrimitive& __stdcall postfix_dec__() = 0;

        enum
        {
            verb_postfix_inc__                = baseT::vtsize + 0,
            verb_postfix_dec__                = baseT::vtsize + 1,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IUnaryOperators
    //
    template <typename baseT>
    struct IUnaryOperators
    : baseT
    {
        enum { vtsize = baseT::vtsize + 9 };
        // unary operators
        virtual IBoolean& __stdcall delete__() = 0;
        virtual IUndefined& __stdcall void__() const = 0;
        virtual IString& __stdcall typeof__() const = 0;
        virtual IPrimitive& __stdcall prefix_inc__() = 0;
        virtual IPrimitive& __stdcall prefix_dec__() = 0;
        virtual INumber& __stdcall unary_plus__() const = 0;
        virtual INumber& __stdcall unary_minus__() const = 0;
        virtual INumber& __stdcall tilde__() const = 0;
        virtual IBoolean& __stdcall not__() const = 0;

        enum
        {
            verb_delete__                     = baseT::vtsize + 0,
            verb_void__                       = baseT::vtsize + 1,
            verb_typeof__                     = baseT::vtsize + 2,
            verb_prefix_inc__                 = baseT::vtsize + 3,
            verb_prefix_dec__                 = baseT::vtsize + 4,
            verb_unary_plus__                 = baseT::vtsize + 5,
            verb_unary_minus__                = baseT::vtsize + 6,
            verb_tilde__                      = baseT::vtsize + 7,
            verb_not__                        = baseT::vtsize + 8,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IMultiplicativeOperators
    //
    template <typename baseT>
    struct IMultiplicativeOperators
    : baseT
    {
        enum { vtsize = baseT::vtsize + 3 };
        // multiplicative operators
        virtual INumber& __stdcall mul__(IPrimitive const&) const = 0;
        virtual INumber& __stdcall div__(IPrimitive const&) const = 0;
        virtual INumber& __stdcall mod__(IPrimitive const&) const = 0;

        enum
        {
            verb_mul__                        = baseT::vtsize + 0,
            verb_div__                        = baseT::vtsize + 1,
            verb_mod__                        = baseT::vtsize + 2,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IAdditiveOperators
    //
    template <typename baseT>
    struct IAdditiveOperators
    : baseT
    {
        enum { vtsize = baseT::vtsize + 2 };
        // additive operators
        virtual IPrimitive& __stdcall binary_plus__(IPrimitive const&) const = 0;
        virtual INumber& __stdcall binary_minus__(IPrimitive const&) const = 0;

        enum
        {
            verb_binary_plus__                = baseT::vtsize + 0,
            verb_binary_minus__               = baseT::vtsize + 1,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IShiftOperators
    //
    template <typename baseT>
    struct IShiftOperators
    : baseT
    {
        enum { vtsize = baseT::vtsize + 3 };
        // shift operators
        virtual INumber& __stdcall shl__(IPrimitive const&) const = 0;
        virtual INumber& __stdcall sar__(IPrimitive const&) const = 0;
        virtual INumber& __stdcall shr__(IPrimitive const&) const = 0;

        enum
        {
            verb_shl__                        = baseT::vtsize + 0,
            verb_sar__                        = baseT::vtsize + 1,
            verb_shr__                        = baseT::vtsize + 2,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IRelationalOperators
    //
    template <typename baseT>
    struct IRelationalOperators
    : baseT
    {
        enum { vtsize = baseT::vtsize + 6 };
        // relational operators
        virtual IBoolean& __stdcall lt__(IPrimitive const&) const = 0;
        virtual IBoolean& __stdcall gt__(IPrimitive const&) const = 0;
        virtual IBoolean& __stdcall le__(IPrimitive const&) const = 0;
        virtual IBoolean& __stdcall ge__(IPrimitive const&) const = 0;
        virtual IBoolean& __stdcall instanceof__(IPrimitive const&) const = 0;
        virtual IBoolean& __stdcall in__(IPrimitive const&) = 0;

        enum
        {
            verb_lt__                         = baseT::vtsize + 0,
            verb_gt__                         = baseT::vtsize + 1,
            verb_le__                         = baseT::vtsize + 2,
            verb_ge__                         = baseT::vtsize + 3,
            verb_instanceof__                 = baseT::vtsize + 4,
            verb_in__                         = baseT::vtsize + 5,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IEqualityOperators
    //
    template <typename baseT>
    struct IEqualityOperators
    : baseT
    {
        enum { vtsize = baseT::vtsize + 4 };
        // equality operators
        virtual IBoolean& __stdcall eq__(IPrimitive const&) const = 0;
        virtual IBoolean& __stdcall ne__(IPrimitive const&) const = 0;
        virtual IBoolean& __stdcall strict_eq__(IPrimitive const&) const = 0;
        virtual IBoolean& __stdcall strict_ne__(IPrimitive const&) const = 0;

        enum
        {
            verb_eq__                         = baseT::vtsize + 0,
            verb_ne__                         = baseT::vtsize + 1,
            verb_strict_eq__                  = baseT::vtsize + 2,
            verb_strict_ne__                  = baseT::vtsize + 3,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IBinaryBitwiseOperators
    //
    template <typename baseT>
    struct IBinaryBitwiseOperators
    : baseT
    {
        enum { vtsize = baseT::vtsize + 3 };
        // binary bitwise operators
        virtual INumber& __stdcall bitwise_and__(IPrimitive const&) const = 0;
        virtual INumber& __stdcall bitwise_xor__(IPrimitive const&) const = 0;
        virtual INumber& __stdcall bitwise_or__(IPrimitive const&) const = 0;

        enum
        {
            verb_bitwise_and__                = baseT::vtsize + 0,
            verb_bitwise_xor__                = baseT::vtsize + 1,
            verb_bitwise_or__                 = baseT::vtsize + 2,
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  IAssignmentOperators
    //
    template <typename baseT>
    struct IAssignmentOperators
    : baseT
    {
        enum { vtsize = baseT::vtsize + 12 };
        // assignment operators
        virtual IPrimitive& __stdcall assign__(IPrimitive&) = 0;
        virtual IPrimitive& __stdcall assign_mul__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_div__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_mod__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_plus__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_minus__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_shl__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_sar__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_shr__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_and__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_xor__(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall assign_or__(IPrimitive const&) = 0;

        enum
        {
            verb_assign__                     = baseT::vtsize + 0,
            verb_assign_mul__                 = baseT::vtsize + 1,
            verb_assign_div__                 = baseT::vtsize + 2,
            verb_assign_mod__                 = baseT::vtsize + 3,
            verb_assign_plus__                = baseT::vtsize + 4,
            verb_assign_minus__              = baseT::vtsize + 5,
            verb_assign_shl__                 = baseT::vtsize + 6,
            verb_assign_sar__                 = baseT::vtsize + 7,
            verb_assign_shr__                 = baseT::vtsize + 8,
            verb_assign_and__                 = baseT::vtsize + 9,
            verb_assign_xor__                 = baseT::vtsize + 10,
            verb_assign_or__                  = baseT::vtsize + 11,
        };

    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  IPrimitive
    //
    struct IPrimitive
    : IManageProperty<
      IManageScopeChain<
      IManagePrototypeChain<
      IEnumProperty<
      IRundomAccess<
      INativeTypeConversion<
      IReferenceCounting<
      IManageLifeCycle<
      ILexicalConversion<
      IInvoke<
      ITypeInformation<
      IPostfixOperators<
      IUnaryOperators<
      IMultiplicativeOperators<
      IAdditiveOperators<
      IShiftOperators<
      IRelationalOperators<
      IEqualityOperators<
      IBinaryBitwiseOperators<
      IAssignmentOperators<
      IEmpty> > > > > > > > > > > > > > > > > > > >
    {
        typedef IManageProperty<
            IManageScopeChain<
            IManagePrototypeChain<
            IEnumProperty<
            IRundomAccess<
            INativeTypeConversion<
            IReferenceCounting<
            IManageLifeCycle<
            ILexicalConversion<
            IInvoke<
            ITypeInformation<
            IPostfixOperators<
            IUnaryOperators<
            IMultiplicativeOperators<
            IAdditiveOperators<
            IShiftOperators<
            IRelationalOperators<
            IEqualityOperators<
            IBinaryBitwiseOperators<
            IAssignmentOperators<
            IEmpty> > > > > > > > > > > > > > > > > > > > base_t;
        typedef std::basic_string<wchar_t> string_t;
        enum { id = VT::Primitive };
        enum { vtsize = base_t::vtsize };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0 },
            };
            return *funcdata;
        }

    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IReference
    //  @brief  Reference object interface
    //
    struct IReference
    : IPrimitive
    {
        typedef IPrimitive base_t;

        enum { id = VT::Reference };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IUndefined
    //  @brief  Undefined object interface
    //
    struct IUndefined
    : IPrimitive
    {
        typedef IPrimitive base_t;

        enum { id = VT::Undefined };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  INull
    //  @brief  Null object interface
    //
    struct INull
    : IPrimitive
    {
        typedef IPrimitive base_t;

        enum { id = VT::Null };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IBooleanT
    //  @brief  Boolean Object interface
    //
    struct IBoolean
    : IPrimitive
    {
        typedef IPrimitive base_t;

        enum { id = VT::Boolean };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 1 };

        // dispatch interface
        virtual IReference const& __stdcall constructor() const = 0;
        virtual IString const& __stdcall toString() const = 0;
        virtual IBoolean const& __stdcall valueOf() const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"constructor", offset + 0, Stdcall| Method, 0 },
                { L"toString"   , offset + 1, Stdcall| Method, 0 },
                { L"valueOf"    , offset + 2, Stdcall| Method, 0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  INumber
    //  @brief  Number object interface
    //
    struct INumber
    : IPrimitive
    {
        typedef IPrimitive base_t;

        enum { id = VT::Number };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 6 };

        // dispatch interface
        virtual IReference const& __stdcall constructor() const = 0;
        virtual IString const& __stdcall toString(IPrimitive&) const = 0;
        virtual IString const& __stdcall toLocaleString() const = 0;
        virtual INumber const& __stdcall valueOf() const = 0;
        virtual IString const& __stdcall toFixed (IPrimitive&) const = 0;
        virtual IString const& __stdcall toExponential(IPrimitive&) const = 0;
        virtual IString const& __stdcall toPrecision(IPrimitive&) const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"constructor",    offset + 0, Stdcall| Property, 0 },
                { L"toString",       offset + 1, Stdcall| Method,   1 },
                { L"toLocaleString", offset + 2, Stdcall| Method,   0 },
                { L"valueOf",        offset + 3, Stdcall| Method,   0 },
                { L"toFixed",        offset + 4, Stdcall| Method,   1 },
                { L"toExponential",  offset + 5, Stdcall| Method,   1 },
                { L"toPrecision",    offset + 6, Stdcall| Method,   1 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IString
    //  @brief  String object interface
    //
    struct IString
    : IPrimitive
    {
        typedef IPrimitive base_t;

        enum { id = VT::String };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 19 };

        // dispatch interaface
        virtual INumber& __stdcall length() const = 0;
        virtual IString const& __stdcall toString() const = 0;
        virtual IString const& __stdcall valueOf() const = 0;
        virtual IString const& __stdcall charAt(IPrimitive&) const = 0;
        virtual INumber const& __stdcall charCodeAt(IPrimitive&) const = 0;
        virtual IString const& __stdcall concat(IPrimitive&) const = 0;
        virtual INumber const& __stdcall indexOf(IPrimitive&) const = 0;
        virtual INumber const& __stdcall lastIndexOf(IPrimitive&) const = 0;
        virtual INumber const& __stdcall localeCompare(IPrimitive&) const = 0;
        virtual IPrimitive const& __stdcall match(IPrimitive&) const = 0;
        virtual IString const& __stdcall replace(IPrimitive&) const = 0;
        virtual INumber const& __stdcall search(IPrimitive&) const = 0;
        virtual IString const& __stdcall slice(IPrimitive&, IPrimitive&) const = 0;
        virtual IArray const& __stdcall split(IPrimitive&) const = 0;
        virtual IString const& __stdcall substring(IPrimitive&, IPrimitive&) const = 0;
        virtual IString const& __stdcall toLowerCase() const = 0;
        virtual IString const& __stdcall toLocaleLowerCase() const = 0;
        virtual IString const& __stdcall toUpperCase() const = 0;
        virtual IString const& __stdcall toLocaleUpperCase() const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"length",            offset + 0 , Stdcall| Property, 0 },
                { L"toString",          offset + 1 , Stdcall| Method,   0 },
                { L"valueOf",           offset + 2 , Stdcall| Method,   0 },
                { L"charAt",            offset + 3 , Stdcall| Method,   1 },
                { L"charCodeAt",        offset + 4 , Stdcall| Method,   1 },
                { L"concat",            offset + 5 , Cdecl  | Method,   0 },
                { L"indexOf",           offset + 6 , Cdecl  | Method,   1 },
                { L"lastIndexOf",       offset + 7 , Cdecl  | Method,   1 },
                { L"localeCompare",     offset + 8 , Stdcall| Method,   1 },
                { L"match",             offset + 9 , Stdcall| Method,   1 },
                { L"replace",           offset + 10, Cdecl  | Method,   1 },
                { L"search",            offset + 11, Stdcall| Method,   1 },
                { L"slice",             offset + 12, Stdcall| Method,   2 },
                { L"split",             offset + 13, Cdecl  | Method,   2 },
                { L"substring",         offset + 14, Stdcall| Method,   2 },
                { L"toLowerCase",       offset + 15, Stdcall| Method,   0 },
                { L"toLocaleLowerCase", offset + 16, Stdcall| Method,   0 },
                { L"toUpperCase",       offset + 17, Stdcall| Method,   0 },
                { L"toLocaleUpperCase", offset + 18, Stdcall| Method,   0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IObject
    //  @brief  Object object interface
    //
    struct IObject
    : IPrimitive
    {
        typedef IPrimitive base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 7 };

        // dispatch interface
        virtual IFunction const& __stdcall constructor() const = 0;
        virtual IString const& __stdcall toString() const = 0;
        virtual IString const& __stdcall toLocaleString() const = 0;
        virtual IObject& __stdcall valueOf() = 0;
        virtual IBoolean& __stdcall hasOwnProperty(IPrimitive const&) = 0;
        virtual IBoolean& __stdcall isPrototypeOf(IPrimitive&) = 0;
        virtual IBoolean& __stdcall propertyIsEnumerable(IPrimitive const&) = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"constructor",          offset + 0, Stdcall| Property, 0 },
                { L"toString",             offset + 1, Stdcall| Method,   0 },
                { L"toLocaleString",       offset + 2, Stdcall| Method,   0 },
                { L"valueOf",              offset + 3, Stdcall| Method,   0 },
                { L"hasOwnProperty",       offset + 4, Stdcall| Method,   1 },
                { L"isPrototypeOf",        offset + 5, Stdcall| Method,   1 },
                { L"propertyIsEnumerable", offset + 6, Stdcall| Method,   1 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @interface  IInternalObject
    //  @brief  Arguments object interface
    //
    struct IInternalObject
    : IObject
    {
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IArguments
    //  @brief  Arguments object interface
    //
    struct IArguments
    : IObject
    {
        typedef IObject base_t;

        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 2 };

        virtual IPrimitive& __stdcall callee() = 0;
        virtual INumber& __stdcall length() const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"callee", offset + 0, Stdcall| Property, 0 },
                { L"length", offset + 1, Stdcall| Property, 0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };


    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IArray
    //  @brief  Array object interface
    //
    struct IArray
    : IObject
    {
        typedef IObject base_t;

        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 6 };

        // dispatch interface
        virtual IPrimitive& __stdcall pop() = 0;
        virtual INumber& __stdcall push(IPrimitive& object) = 0;
        virtual IPrimitive& __stdcall shift() = 0;
        virtual IUndefined& __stdcall unshift(IPrimitive& object) = 0;
        virtual INumber& __stdcall length() const = 0;
        virtual IString const& __stdcall toString() const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"pop",      offset + 0, Stdcall| Method,   0 },
                { L"push",     offset + 1, Stdcall| Method,   1 },
                { L"shift",    offset + 2, Stdcall| Method,   0 },
                { L"unshift",  offset + 3, Stdcall| Method,   1 },
                { L"length",   offset + 4, Stdcall| Property, 0 },
                { L"toString", offset + 5, Stdcall| Method,   0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IRegExp
    //  @brief
    //
    struct IRegExp
    : IObject
    {
        typedef IObject base_t;
        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 8 };

        virtual IPrimitive& __stdcall internal_print() const = 0;
        virtual IPrimitive& __stdcall exec(IPrimitive&) = 0;
        virtual IBoolean& __stdcall test(IPrimitive&) = 0;
        virtual IString& __stdcall source() const = 0;
        virtual IBoolean& __stdcall global() const = 0;
        virtual IBoolean& __stdcall ignoreCase() const = 0;
        virtual IBoolean& __stdcall multiline() const = 0;
        virtual INumber& __stdcall lastIndex() const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"__print",    offset + 0, Stdcall| Method,   0 },
                { L"exec",       offset + 1, Stdcall| Method,   1 },
                { L"test",       offset + 2, Stdcall| Method,   1 },
                { L"source",     offset + 3, Stdcall| Property, 0 },
                { L"global",     offset + 4, Stdcall| Property, 0 },
                { L"ignoreCase", offset + 5, Stdcall| Property, 0 },
                { L"multiline",  offset + 6, Stdcall| Property, 0 },
                { L"lastIndex",  offset + 7, Stdcall| Property, 0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IFunction
    //  @brief  Function object interface
    //
    struct IFunction
    : IObject
    {
        typedef IObject base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 4 };

        virtual IPrimitive& __stdcall apply(IPrimitive&, IPrimitive&) = 0;
        virtual IPrimitive& __stdcall call(IPrimitive&) = 0;
        virtual INumber& __stdcall length() const = 0;
        virtual IPrimitive& __stdcall prototype() = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"apply",     offset + 0, Stdcall| Method,   2 },
                { L"call",      offset + 1, Cdecl  | Method,   1 },
                { L"length",    offset + 2, Stdcall| Property, 0 },
                { L"prototype", offset + 3, Stdcall| Property, 0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  INativeError
    //  @brief
    //
    struct INativeError
    : IObject
    {
        typedef IObject base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 2 };

        virtual IPrimitive& __stdcall message() const = 0;
        virtual IPrimitive& __stdcall name() const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"message", offset + 0, Stdcall| Property, 0 },
                { L"name",    offset + 1, Stdcall| Property, 0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  INativeErrorConstructor
    //  @brief
    //
    struct INativeErrorConstructor
    : IFunction
    {
        typedef IFunction base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IBooleanConstructor
    //  @brief
    //
    struct IBooleanConstructor
    : IFunction
    {
        typedef IFunction base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  INumberConstructor
    //  @brief
    //
    struct INumberConstructor
    : IFunction
    {
        typedef IFunction base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 5 };

        virtual INumber const& __stdcall MAX_VALUE() const = 0;
        virtual INumber const& __stdcall MIN_VALUE() const = 0;
        virtual INumber const& __stdcall NaN() const = 0;
        virtual INumber const& __stdcall NEGATIVE_INFINITY() const = 0;
        virtual INumber const& __stdcall POSITIVE_INFINITY() const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"MAX_VALUE",         offset + 0, Stdcall| Property, 0 },
                { L"MIN_VALUE",         offset + 1, Stdcall| Property, 0 },
                { L"NaN",               offset + 2, Stdcall| Property, 0 },
                { L"NEGATIVE_INFINITY", offset + 3, Stdcall| Property, 0 },
                { L"POSITIVE_INFINITY", offset + 4, Stdcall| Property, 0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IStringConstructor
    //  @brief
    //
    struct IStringConstructor
    : IFunction
    {
        typedef IFunction base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 1 };

        virtual IString const& __stdcall fromCharCode(IPrimitive&) const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"fromCharCode", offset + 0, Cdecl| Method, 1 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IObjectConstructor
    //  @brief
    //
    struct IObjectConstructor
    : IFunction
    {
        typedef IFunction base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IFunctionConstructor
    //  @brief
    //
    struct IFunctionConstructor
    : IFunction
    {
        typedef IFunction base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IArrayConstructor
    //  @brief
    //
    struct IArrayConstructor
    : IFunction
    {
        typedef IFunction base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IRegExpConstructor
    //  @brief
    //
    struct IRegExpConstructor
    : IFunction
    {
        typedef IFunction base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IGlobal
    //  @brief  Global object interface
    //
    struct IGlobal
    : IFunction
    {
        typedef IFunction base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 14 };

        virtual INumber const& __stdcall NaN() const = 0;
        virtual INumber const& __stdcall Infinity() const = 0;
        virtual IPrimitive& __stdcall eval(IPrimitive&) = 0;
        virtual INumber const& __stdcall parseInt(IPrimitive const&) const = 0;
        virtual INumber const& __stdcall parseFloat(IPrimitive const&) const = 0;
        virtual IBoolean const& __stdcall isNaN(IPrimitive const&) const = 0;
        virtual IBoolean const& __stdcall isFinite(IPrimitive const&) const = 0;
        virtual IFunction const& __stdcall Object() = 0;
        virtual IFunction const& __stdcall Boolean() = 0;
        virtual IFunction const& __stdcall Number() = 0;
        virtual IFunction const& __stdcall String() = 0;
        virtual IFunction const& __stdcall Array() = 0;
        virtual IFunction const& __stdcall Function() = 0;
        virtual IFunction const& __stdcall RegExp() = 0;
        virtual IFunction const& __stdcall Date() = 0;
        virtual IObject const& __stdcall Math() = 0;
        virtual IUndefined const& __stdcall verbose(IPrimitive& value) = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"NaN",       offset + 0,  Stdcall| Property, 0 },
                { L"Infinity",  offset + 1,  Stdcall| Property, 0 },
                { L"eval",      offset + 2,  Stdcall| Method,   1 },
                { L"parseInt",  offset + 3,  Stdcall| Method,   1 },
                { L"parseFloat",offset + 4,  Stdcall| Method,   1 },
                { L"isNaN",     offset + 5,  Stdcall| Method,   1 },
                { L"isFinite",  offset + 6,  Stdcall| Method,   1 },
                { L"Object",    offset + 7,  Stdcall| Property, 0 },
                { L"Boolean",   offset + 8,  Stdcall| Property, 0 },
                { L"Number",    offset + 9,  Stdcall| Property, 0 },
                { L"String",    offset + 10, Stdcall| Property, 0 },
                { L"Array",     offset + 11, Stdcall| Property, 0 },
                { L"Function",  offset + 12, Stdcall| Property, 0 },
                { L"RegExp",    offset + 13, Stdcall| Property, 0 },
                { L"Date",      offset + 14, Stdcall| Property, 0 },
                { L"Math",      offset + 15, Stdcall| Property, 0 },
                { L"verbose",   offset + 16, Stdcall| Method,   1 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };
    
    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IActorBase
    //
    struct IActorBase
    {
        virtual void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>&) const = 0;
    };
    
    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IActor
    //
    struct IActor 
    : IActorBase
    {
        virtual es_value_or_hint<IPrimitive> const hint__() const = 0;
        virtual IActor const * operator [] (ecmascript::uint32_t) const = 0;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IParser
    //
    struct IParser
    {
        virtual IActor& parse(wchar_t const *const first, wchar_t const *const last) const = 0;
    };

} // namespace ecmascript
