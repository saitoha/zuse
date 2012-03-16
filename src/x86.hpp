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
 * ***** END LICENSE BLOCK ***** 
 *
 * $Revision: 8 $
 * $Author: user $
 * $Date: 2010-06-05 15:49:50 +0900 (土, 05  6 2010) $
 * $HeadURL: svn+ssh://zuse.jp/svn/zuse/tags/zuse-0.1.4/src/x86.hpp $
 */


#if ES_TEST_JIT

namespace ecmascript { namespace generator { namespace x86 {

#pragma pack(push, 1)

    enum Register
    {
        reg_al  = 0,
        reg_ax  = 0,
        reg_eax = 0,
        reg_cl  = 1,
        reg_cx  = 1,
        reg_ecx = 1,
        reg_dl  = 2,
        reg_dx  = 2,
        reg_edx = 2,
        reg_bl  = 3,
        reg_bx  = 3,
        reg_ebx = 3,
        reg_esp = 4,
        reg_ebp = 5,
        reg_esi = 6,
        reg_edi = 7,
    };

    enum Mode
    {
        mod_base         = 0x0,
        mod_disp32       = 0xff,
        mod_base_disp8   = 0x1,
        mod_base_disp32  = 0x2,
        mod_reg          = 0x3,
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct register_disp8__
    //
    struct register_disp8__
    {
        explicit register_disp8__(
            Register const reg,
            int const disp8
            ) throw()
        : reg_(reg)
        , disp8_(disp8)
        {
        }

        Register const reg_;
        int const disp8_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct register__
    //
    struct register__
    {
        typedef register__ self_t;

        explicit register__(Register const reg) throw()
        : reg_(reg)
        {
        }

        template <typename rhsT>
        register_disp8__ const operator +(rhsT disp8) const
        {
            return register_disp8__(*this, disp8);
        }

        template <typename rhsT>
        register_disp8__ const operator -(rhsT disp8) const
        {
            return register_disp8__(*this, - disp8);
        }

        template <typename rhsT>
        rhsT operator << (rhsT rhs) const
        {
            return reg_ << rhs;
        }

        operator Register const () const
        {
            return reg_;
        }

        Register const reg_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct r8
    //
    struct r8__
    : register__
    {
        explicit r8__(Register const reg) throw()
        : register__(reg)
        {
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct r16
    //
    struct r16__
    : register__
    {
        explicit r16__(Register const reg) throw()
        : register__(reg)
        {
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct register_al__
    //
    struct register_al__
    : r8__
    {
        register_al__() throw()
        : r8__(reg_al)
        {
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct register_ax__
    //
    struct register_ax__
    : r16__
    {
        register_ax__() throw()
        : r16__(reg_ax)
        {
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct register_eax__
    //
    struct register_eax__
    : register__
    {
        register_eax__() throw()
        : register__(reg_eax)
        {
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct register_esp__
    //
    struct register_esp__
    : register__
    {
        register_esp__()
        : register__(reg_esp)
        {
        }
    };

    register_al__ const al;
    register_ax__ const ax;
    register_eax__ const eax;
    register__ const ecx(reg_ecx);
    register__ const edx(reg_edx);
    register__ const ebx(reg_ebx);
    register_esp__ const esp;
    register__ const ebp(reg_ebp);
    register__ const esi(reg_esi);
    register__ const edi(reg_edi);


    //////////////////////////////////////////////////////////////////////////
    //
    // @struct ptr__
    //
    struct ptr__
    {
        typedef ptr__ self_t;
    friend struct mov__;
        explicit ptr__(register__ const reg)
        : reg_(reg)
        {
        }

        Register const reg_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct imm32_ptr__
    //
    struct imm32_ptr__
    {
        typedef imm32_ptr__ self_t;
    friend struct mov__;
        explicit imm32_ptr__(void const * addr)
        : imm32_(unsigned(addr))
        {
        }

        unsigned int const imm32_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct qword_ptr__
    //
    struct qword_ptr__
    {
        typedef qword_ptr__ self_t;
    friend struct mov__;
        explicit qword_ptr__(register__ const reg)
        : reg_(reg)
        {
        }

        Register const reg_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct ptr_disp8__
    //
    struct ptr_disp8__
    {
        typedef ptr_disp8__ self_t;

        explicit ptr_disp8__(register_disp8__ const reg)
        : reg_(reg.reg_)
        , disp8_(reg.disp8_)
        {
        }

        Register const reg_;
        unsigned int const disp8_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct qword_ptr_disp8__
    //
    struct qword_ptr_disp8__
    {
        typedef qword_ptr_disp8__ self_t;

        explicit qword_ptr_disp8__(register_disp8__ const reg)
        : reg_(reg.reg_)
        , disp8_(reg.disp8_)
        {
        }

        Register const reg_;
        unsigned int const disp8_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct ptr_constructor__
    //
    struct ptr_constructor__
    {
        ptr__ operator [](register__ const reg) const
        {
            return ptr__(reg);
        }

        imm32_ptr__ operator [](void const * address) const
        {
            return imm32_ptr__(address);
        }

        ptr_disp8__ const operator [](register_disp8__ const reg) const
        {
            return ptr_disp8__(reg);
        }        
    };
    ptr_constructor__ const ptr = ptr_constructor__();

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct qword_ptr_constructor__
    //
    struct qword_ptr_constructor__
    {
        qword_ptr__ operator [](register__ const reg) const
        {
            return qword_ptr__(reg);
        }

        qword_ptr_disp8__ const operator [](register_disp8__ const reg) const
        {
            return qword_ptr_disp8__(reg);
        }
    };
    qword_ptr_constructor__ const qword_ptr = qword_ptr_constructor__();

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct long_relative_label__
    //
    struct long_relative_label__
    {
        typedef long_relative_label__ self_t;

        long_relative_label__()
        : address_(0)
        {
        }
        
        ~long_relative_label__()
        {
            if (0 == address_)
                return;
            int *p;
            unsigned int next;
        loop:
            p = reinterpret_cast<int *>(address_) - 1; 
            next = *p;                       
            *p = unsigned(it_) - unsigned(address_);
            address_ = next;
            if (0 != address_)
                goto loop;
        }
        
        unsigned int const get() const
        {
            return address_;
        }
        
        void set(unsigned int rel32)
        {
            *(reinterpret_cast<unsigned int *>(rel32) - 1) = address_;
            address_ = rel32;
        }
        
        operator void const *() const
        {
            return reinterpret_cast<void const *>(get());
        }
        
        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            it_ = code.end();
        }

    private:
        unsigned int address_;
        mutable char *it_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct long_absolute_label__
    //
    struct long_absolute_label__
    {
        typedef long_absolute_label__ self_t;

        long_absolute_label__()
        : address_(0)
        {
        }
        
        unsigned int const get() const
        {
            return address_;
        }
        
        void set(unsigned int rel32)
        {
            *(reinterpret_cast<unsigned int *>(rel32) - 1) = address_;
            address_ = rel32;
        }
        
        operator void const *() const
        {
            return reinterpret_cast<void const *>(get());
        }
        
        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            if (0 != address_)
                code.absolute_patch(address_);
            else
                const_cast<unsigned int&>(address_) = unsigned(code.end());
        }

    private:
        unsigned int address_;
    };
    
    //////////////////////////////////////////////////////////////////////////
    //
    // @struct mov__
    //
    struct mov__
    {
        typedef mov__ self_t;

        explicit mov__(register__ const reg1, register__ const reg2)
        : op_(0x8b)
        , mod_r_m_(mod_reg << 6 | reg1 << 3 | reg2)
        , length_(2)
        {
        }

        explicit mov__(register__ const reg, ptr__ const ptr)
        : op_(0x8b)
        {
            mod_r_m_ = mod_base << 6 | reg << 3 | ptr.reg_;
            length_ = 2;
        }
        
        explicit mov__(register__ const reg, ptr_disp8__ const ptr)
        : op_(0x8b)
        {
            if (ptr.disp8_ < 0x80)
            {
                mod_r_m_ = mod_base_disp8 << 6 | reg << 3 | ptr.reg_;
                if (reg_esp == ptr.reg_)
                {
                    sib_ = reg_esp << 3 | reg_esp;
                    disp8_ = ptr.disp8_;
                    length_ = 4;
                }
                else
                {
                    imm8_ = ptr.disp8_;
                    length_ = 3;
                }
            }
            else
            {
                mod_r_m_ = mod_base_disp32 << 6 | reg << 3 | ptr.reg_;
                disp32_ = ptr.disp8_;
                length_ = 6;
            }
        }

        explicit mov__(ptr__ const ptr, register__ const reg)
        : op_(0x89)
        , mod_r_m_(mod_base << 6 | reg << 3 | ptr.reg_)
        , length_(2)
        {
        }
        
        explicit mov__(ptr_disp8__ const ptr, register__ const reg)
        : op_(0x89)
        , mod_r_m_(mod_base_disp32 << 6 | reg << 3 | ptr.reg_)
        , disp32_(ptr.disp8_)
        , length_(6)
        {
        }

        explicit mov__(register__ const reg, unsigned int const imm32)
        : op_(0xb8 + reg)
        , imm32_(imm32)
        , length_(5)
        {
        }
        
        explicit mov__(register__ const reg, void const *addr)
        : op_(0x8b)
        , mod_r_m_(mod_base_disp32 << 6 | reg << 3 | 5)
        , disp32_(unsigned(addr))
        , length_(6)
        {
        }

        explicit mov__(ptr__ const ptr, void const *addr)
        : op_(0xc7)
        , mod_r_m_(mod_base << 6 | 0 << 3 | ptr.reg_)
        , disp32_(unsigned(addr))
        , length_(6)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        union {
            struct {
                unsigned char mod_r_m_;
                union {
                    unsigned char imm8_;
                    struct {
                        unsigned char sib_;
                        unsigned char disp8_;
                    };
                    unsigned int disp32_;
                };
            };
            unsigned int imm32_;
        };
        unsigned char length_;
    };

    inline mov__ const mov(register__ const reg1, register__ const reg2)
    {
        return mov__(reg1, reg2);
    }

    inline mov__ const mov(register__ const reg, ptr__ const ptr)
    {
        return mov__(reg, ptr);
    }

    inline mov__ const mov(register__ const reg, ptr_disp8__ const ptr)
    {
        return mov__(reg, ptr);
    }

    inline mov__ const mov(ptr__ const ptr, register__ const reg)
    {
        return mov__(ptr, reg);
    }

    inline mov__ const mov(ptr_disp8__ const ptr, register__ const reg)
    {
        return mov__(ptr, reg);
    }
    
    inline mov__ const mov(register__ const reg, int const imm32)
    {
        return mov__(reg, imm32);
    }

    inline mov__ const mov(register__ const reg, void const *addr)
    {
        return mov__(reg, reinterpret_cast<unsigned int>(addr));
    }

    inline mov__ const mov(ptr__ const ptr, void const *addr)
    {
        return mov__(ptr, addr);
    }
    
    //////////////////////////////////////////////////////////////////////////
    //
    // @struct labeled_mov__
    //
    struct labeled_mov__
    {
        typedef labeled_mov__ self_t;

        explicit labeled_mov__(ptr__ const ptr, long_absolute_label__& label)
        : op_(0xc7)
        , mod_r_m_(mod_base << 6 | 0 << 3 | ptr.reg_)
        , disp32_(label.get())
        , length_(6)
        , label_(label)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
            if (0 == disp32_)
                label_.set(unsigned(code.end()));
        }

    private:
        unsigned char const op_;
        unsigned char const mod_r_m_;
        unsigned long disp32_;
        unsigned char const length_;
        long_absolute_label__& label_;
    };
    
    inline labeled_mov__ const push(
        ptr__ const ptr, long_absolute_label__& label)
    {
        return labeled_mov__(ptr, label);
    }



    //////////////////////////////////////////////////////////////////////////
    //
    // @struct add__
    //
    struct add__
    {
        typedef add__ self_t;

        explicit add__(register__ const reg, unsigned int const imm)
        : op_(0x83)
        , mod_r_m_(mod_reg << 6 | 0 << 3 | reg)
        , imm_(imm)
        , length_(3)
        {
        }
        
        explicit add__(imm32_ptr__ const imm32_ptr, char const imm8)
        : op_(0x83)
        , mod_r_m_(mod_base << 6 | 0 << 3 | 5)
        , abs32_(imm32_ptr.imm32_)
        , imm8_(imm8)
        , length_(7)
        {
        }

        explicit add__(ptr__ const ptr, char const imm8)
        : op_(0x83)
        , mod_r_m_(mod_base << 6 | 0 << 3 | ptr.reg_)
        , imm_(imm8)
        , length_(3)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        unsigned char const mod_r_m_;
        union {
            unsigned char imm_;
            struct {
                unsigned int abs32_;
                unsigned int imm8_;
            };
        };
        unsigned char const length_;
    };

    inline add__ const add(register__ const reg, char const imm8)
    {
        return add__(reg, imm8);
    }

    inline add__ const add(ptr__ const ptr, char const imm8)
    {
        return add__(ptr, imm8);
    }

    inline add__ const add(imm32_ptr__ const imm32_ptr, char const imm8)
    {
        return add__(imm32_ptr, imm8);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct sub__
    //
    struct sub__
    {
        typedef sub__ self_t;

        explicit sub__(register__ const reg, unsigned char const imm8)
        : op_(0x83)
        , mod_r_m_(mod_reg << 6 | 5 << 3 | reg)
        , imm_(imm8)
        , length_(3)
        {
        }

        explicit sub__(imm32_ptr__ const imm32_ptr, char const imm8)
        : op_(0x83)
        , mod_r_m_(mod_base << 6 | 5 << 3 | 5)
        , abs32_(imm32_ptr.imm32_)
        , imm8_(imm8)
        , length_(7)
        {
        }

        explicit sub__(ptr__ const ptr, char const imm8)
        : op_(0x83)
        , mod_r_m_(mod_base << 6 | 5 << 3 | ptr.reg_)
        , imm_(imm8)
        , length_(3)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        unsigned char const mod_r_m_;
        union {
            unsigned char imm_;
            struct {
                unsigned int abs32_;
                unsigned int imm8_;
            };
        };
        unsigned char const length_;
    };

    inline sub__ const sub(register__ const reg, unsigned char const disp8)
    {
        return sub__(reg, disp8);
    }

    inline sub__ const sub(ptr__ const ptr, char const imm8)
    {
        return sub__(ptr, imm8);
    }

    inline sub__ const sub(imm32_ptr__ const imm32_ptr, char const imm8)
    {
        return sub__(imm32_ptr, imm8);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct fld__
    //
    struct fld__
    {
        typedef fld__ self_t;

        explicit fld__(qword_ptr__ const qword_ptr)
        : op_(0xdd)
        , mod_r_m_(mod_base << 6 | 0 << 3 | qword_ptr.reg_)
        , length_(2)
        {
        }

        explicit fld__(qword_ptr_disp8__ const qword_ptr_disp8)
        : op_(0xdd)
        , mod_r_m_(mod_base_disp8 << 6 | 0 << 3 | qword_ptr_disp8.reg_)
        , disp8_(qword_ptr_disp8.disp8_)
        , length_(3)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        unsigned char const mod_r_m_;
        unsigned char disp8_;
        unsigned char const length_;
    };

    inline fld__ const fld(qword_ptr__ const qword_ptr)
    {
        return fld__(qword_ptr);
    }

    inline fld__ const fld(qword_ptr_disp8__ const qword_ptr_disp8)
    {
        return fld__(qword_ptr_disp8);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct fstp__
    //
    struct fstp__
    {
        typedef fstp__ self_t;

        explicit fstp__(qword_ptr__ const qword_ptr)
        : op_(0xdd)
        , mod_r_m_(mod_base << 6 | 3 << 3 | qword_ptr.reg_)
        , sib_(reg_esp << 3 | reg_esp)
        , length_(3)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        unsigned char const mod_r_m_;
        unsigned char const sib_;
        unsigned char const length_;
    };

    inline fstp__ const fstp(qword_ptr__ const qword_ptr)
    {
        return fstp__(qword_ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct push__
    //
    struct push__
    {
        typedef push__ self_t;

        explicit push__(register__ const reg)
        : op_(0x50 + reg)
        , length_(1)
        {
        }

        explicit push__(ptr__ const ptr)
        : op_(0xff)
        , mod_r_m_(mod_base << 6 | 6 << 3 | ptr.reg_)
        , length_(2)
        {
        }

        explicit push__(void const *addr)
        : op_(0x68)
        , imm32_(addr)
        , length_(5)
        {
        }

        explicit push__(bool const imm_boolean)
        : op_(0x6a)
        , imm_boolean_(imm_boolean)
        , length_(2)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        union {
            unsigned char mod_r_m_;
            bool imm_boolean_;
            void const *imm32_;
        };
        unsigned char length_;
    };

    inline push__ const push(register__ const reg)
    {
        return push__(reg);
    }

    inline push__ const push(ptr__ const ptr)
    {
        return push__(ptr);
    }
    
    inline push__ const push(void const *addr)
    {
        return push__(addr);
    }

    inline push__ const push(bool const imm_boolean)
    {
        return push__(imm_boolean);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct labeled_push__
    //
    struct labeled_push__
    {
        typedef labeled_push__ self_t;

        explicit labeled_push__(long_absolute_label__& label) throw()
        : op_(0x68)
        , abs32_(label.get())
        , length_(5)
        , label_(label)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
            if (0 == abs32_)
                label_.set(unsigned(code.end()));
        }

    private:
        unsigned char const op_;
        unsigned long abs32_;
        unsigned char const length_;
        long_absolute_label__& label_;
    };
    
    inline labeled_push__ const push(long_absolute_label__& label)
    {
        return labeled_push__(label);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct call__
    //
    struct call__
    {
        typedef call__ self_t;

        explicit call__(unsigned int const addr)
        : op_(0xe8)
        , imm32_(addr)
        , length_(5)
        {
        }

        explicit call__(register__ const reg)
        : op_(0xff)
        , mod_r_m_(mod_reg << 6 | 2 << 3 | reg)
        , length_(2)
        {
        }

        explicit call__(ptr_disp8__ const ptr)
        : op_(0xff)
        , mod_r_m_(mod_base_disp32 << 6 | 2 << 3 | ptr.reg_)
        , disp32_(ptr.disp8_)
        , length_(6)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            if (0xe8 == op_)
            {
                code.write(this, 1);
                unsigned int imm32
                    = imm32_ - reinterpret_cast<unsigned int>(code.end() + sizeof(imm32));
                code.write(&imm32, sizeof(imm32));
            }
            else
            {
                code.write(this, length_);
            }
        }

    private:
        unsigned char const op_;
        union {
            struct {
                unsigned char mod_r_m_;
                unsigned int disp32_;
            };
            unsigned int imm32_;
        };
        unsigned char const length_;
    };

    template <typename T>
    inline call__ const call(T const function_pointer)
    {
        union {
            T function_pointer_;
            unsigned int address_;
        } box;
        box.function_pointer_ = function_pointer;
        return call__(box.address_);
    }

    inline call__ const call(unsigned int const addr)
    {
        return call__(addr);
    }

    inline call__ const call(register__ const reg)
    {
        return call__(reg);
    }

    inline call__ const call(ptr_disp8__ const ptr)
    {
        return call__(ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct ret__
    //
    struct ret__
    {
        typedef ret__ self_t;

        explicit ret__(unsigned short const imm16)
        : op_(0xc2)
        , imm16_(imm16)
        , length_(3)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        unsigned short const imm16_;
        unsigned char const length_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct ret_constructor__
    //
    struct ret_constructor__
    {
        ret_constructor__()
        : op_(0xc3)
        , length_(1)
        {
        }

        ret__ const operator ()(unsigned short const imm16) const
        {
            return ret__(imm16);
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

        private:
            unsigned char const op_;
            unsigned char const length_;
    };
    ret_constructor__ const ret = ret_constructor__();

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct test__
    //
    struct test__
    {
        typedef test__ self_t;

        test__(
            r8__ const reg1,
            r8__ const reg2
            )
        : op_(0x84)
        , mod_r_m_(mod_reg << 6 | reg1 << 3 | reg2)
        , length_(2)
        {
        }
        
        test__(
            register_al__ const reg1,
            r8__ const reg2
            )
        : op_(0x84)
        , mod_r_m_(mod_reg << 6 | reg1 << 3 | reg2)
        , length_(2)
        {
        }
        
        explicit test__(
            register_al__ const,
            unsigned char const imm8
            )
        : op_(0xa8)
        , imm8_(imm8)
        , length_(2)
        {
        }
        
        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        union {
            unsigned char mod_r_m_;
            unsigned char imm8_;
        };
        unsigned char const length_;
    };

    inline test__ const test(r8__ const reg1, r8__ const reg2)
    {
        return test__(reg1, reg2);
    }

    inline test__ const test(register_al__ const reg1, r8__ const reg2)
    {
        return test__(reg1, reg2);
    }

    inline test__ const test(register_al__ const reg1, unsigned char const imm8)
    {
        return test__(reg1, imm8);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct cmp__
    //
    struct cmp__
    {
        typedef cmp__ self_t;

        explicit cmp__(
            register_al__ const,
            unsigned char const imm8
            )
        : op_(0x3c)
        , imm8_(imm8)
        , length_(2)
        {
        }
        
        explicit cmp__(
            register_eax__ const,
            int const imm32
            )
        : op_(0x3d)
        , imm32_(imm32)
        , length_(5)
        {
        }
        
        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        union {
            unsigned char mod_r_m_;
            unsigned char imm8_;
            int imm32_;
        };
        unsigned char const length_;
    };

    inline cmp__ const cmp(register_al__ const reg1, unsigned char const imm8)
    {
        return cmp__(reg1, imm8);
    }

    inline cmp__ const cmp(register_eax__ const reg1, int const imm32)
    {
        return cmp__(reg1, imm32);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct int3__
    //
    struct int3__
    {
        typedef int3__ self_t;

        explicit int3__()
        : op_(0xcc)
        , length_(1)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        unsigned char const length_;
    };

    inline int3__ const int3()
    {
        return int3__();
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct je__
    //
    struct je__
    {
        typedef je__ self_t;

        explicit je__(unsigned char const rel8)
        : op_(0x74)
        , rel8_(rel8)
        , length_(2)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        unsigned char const rel8_;
        unsigned char const length_;
    };

    inline je__ const je(unsigned char const rel8)
    {
        return je__(rel8);
    }
    
    //////////////////////////////////////////////////////////////////////////
    //
    // @struct labeled_je__
    //
    struct labeled_je__
    {
        typedef labeled_je__ self_t;

        explicit labeled_je__(long_relative_label__& label) throw()
        : prefix_(0x0f)
        , op_(0x84)
        , rel32_(label.get())
        , length_(6)
        , label_(label)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
            label_.set(unsigned(code.end()));
        }

    private:
        unsigned char const prefix_;
        unsigned char const op_;
        mutable int rel32_;
        unsigned char const length_;
        long_relative_label__& label_;
    };
    
    inline labeled_je__ const je(long_relative_label__& label)
    {
        return labeled_je__(label);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct jne__
    //
    struct jne__
    {
        typedef jne__ self_t;

        explicit jne__(unsigned char const rel8)
        : op_(0x75)
        , rel8_(rel8)
        , length_(2)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        unsigned char const rel8_;
        unsigned char const length_;
    };

    inline jne__ const jne(unsigned char const rel8)
    {
        return jne__(rel8);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct labeled_jne__
    //
    struct labeled_jne__
    {
    private:
        typedef labeled_jne__ self_t;

    public:
        explicit labeled_jne__(long_relative_label__& label) throw()
        : prefix_(0x0f)
        , op_(0x85)
        , rel32_()
        , length_(6)
        , label_(label)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
            label_.set(unsigned(code.end()));
        }

    private:
        unsigned char const prefix_;
        unsigned char const op_;
        mutable unsigned int rel32_;
        unsigned char const length_;
        long_relative_label__& label_;
    };
    
    inline labeled_jne__ const jne(long_relative_label__& label)
    {
        return labeled_jne__(label);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct jmp__
    //
    struct jmp__
    {
        typedef jmp__ self_t;

        explicit jmp__(unsigned char const rel8)
        : op_(0xeb)
        , rel8_(rel8)
        , length_(2)
        {
        }

        explicit jmp__(register__ const reg)
        : op_(0xff)
        , mod_r_m_(mod_reg << 6 | 4 << 3 | reg)
        , length_(2)
        {
        }
        
        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        unsigned char const op_;
        union {
            unsigned char rel8_;
            unsigned char mod_r_m_;
        };
        unsigned char const length_;
    };

    inline jmp__ const jmp(unsigned const rel8)
    {
        ES_ASSERT(0x100 > rel8);
        return jmp__(rel8);
    }

    inline jmp__ const jmp(register__ const reg)
    {
        return jmp__(reg);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct labeled_jmp__
    //
    struct labeled_jmp__
    {
    private:
        typedef labeled_jmp__ self_t;

    public:
        explicit labeled_jmp__(long_relative_label__& label) throw()
        : op_(0xe9)
        , rel32_(label.get())
        , length_(5)
        , label_(label)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
            if (0 == rel32_)
                label_.set(unsigned(code.end()));
        }

    private:
        unsigned char const op_;
        unsigned int rel32_;
        unsigned char const length_;
        long_relative_label__& label_;
    };
    
    inline labeled_jmp__ const jmp(long_relative_label__& label)
    {
        return labeled_jmp__(label);
    }
    
    //////////////////////////////////////////////////////////////////////////
    //
    // @struct pop__
    //
    struct pop__
    {
        typedef pop__ self_t;

        explicit pop__(register__ const reg)
        : op_(0x58 + reg)
        , length_(1)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code.write(this, length_);
        }

    private:
        char const op_;
        char length_;
    };

    inline pop__ const pop(register__ const reg)
    {
        return pop__(reg);
    }
    
    
    //////////////////////////////////////////////////////////////////////////
    //
    // @struct call_vfunc__
    //
    template <typename registerT>
    struct call_vfunc__
    {
    private:
        typedef call_vfunc__<registerT> self_t;

    public:
        explicit call_vfunc__(registerT const reg, unsigned int verb)
        : reg_(reg)
        , verb_(verb)
        {
        }

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            code
                << push(reg_)
                << mov(reg_, ptr[reg_])
                << call(ptr[reg_ + verb_ * sizeof(void *)])
                ;
        }

    private:
        registerT const reg_;
        unsigned int verb_;
    };

    template <typename registerT>
    inline call_vfunc__<registerT> const 
    call_vfunc(registerT const reg, unsigned int verb)
    {
        return call_vfunc__<registerT>(reg, verb);
    }    

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct breakpoint
    //
    struct breakpoint__
    {
        typedef breakpoint__ self_t;

        template <typename codestreamT>
        void assemble(codestreamT& code) const throw()
        {
            struct debug_ { static void break_() {} };
#ifdef _DEBUG
            code
#if 1             
                << call(&debug_::break_)
#else
                << int3()
#endif // 0 or 1

#endif // _DEBUG
            ;

        }
    };
    
    breakpoint__ const breakpoint = breakpoint__();

#pragma pack(pop)

} } } // namespace ecmascript::generator::x86

#endif // ES_TEST_JIT
