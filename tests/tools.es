

if (typeof __debug == 'undefined')
{
    __debug = {
	    alert: function(message) { WScript.Echo(message) }
    };
}


var Buffer = function() {
    this.__class__ = arguments.callee;
    this.initialize.apply(this, arguments);
};

Buffer.prototype = {
    initialize: function() {
        this.vec = [];
        this.length = 0;
        this.anchor = 0;
    },

    seek: function(anchor) {
        this.anchor = anchor;
    },

    U8: function(v) {
        if (this.anchor != this.vec.length) {
            if (this.vec[this.anchor].length != 1)
                throw new this.Exception();
        } else {
            this.length++;
        }
        this.vec[this.anchor++] = String.fromCharCode(v);
    },

    U16LE: function(v) {
        if (this.anchor != this.vec.length) {
            if (this.vec[this.anchor].length != 2)
                throw new this.Exception();
        } else {
            this.length += 2;
        }
        this.vec[this.anchor++] = String.fromCharCode(v & 255)
                      + String.fromCharCode((v >> 8) & 255);
    },

    U32LE: function(v) {
        if (this.anchor != this.vec.length) {
            if (this.vec[this.anchor].length != 4)
                throw new this.Exception();
        } else {
            this.length += 4;
        }
        this.vec[this.anchor++] = String.fromCharCode(v & 255)
                      + String.fromCharCode((v >> 8) & 255)
                      + String.fromCharCode((v >> 16) & 255)
                      + String.fromCharCode(v >> 24);
    },

    S: function(v) {
        if (this.anchor != this.vec.length) {
            if (this.vec[this.anchor].length != v.length)
                throw new this.Exception();
        } else {
            this.length += v.length;
        }
        this.vec[this.anchor++] = v;
    },

    toString: function() {
        return this.vec.join('');
    }
};

var Assembler = function() {
    this.__class__ = arguments.callee;
    this.initialize.apply(this, arguments);
};

Assembler.Exception = function(message) {
    this.name = "AssemblerException";
    this.message = typeof message == 'undefined' ? null: message;
    this.toString = function() { return this.message; };
};

Assembler.Expr = {
    al: { type: 'reg', name: 'al', code: 0x00 },
    cl: { type: 'reg', name: 'cl', code: 0x01 },
    dl: { type: 'reg', name: 'dl', code: 0x02 },
    bl: { type: 'reg', name: 'bl', code: 0x03 },
    ah: { type: 'reg', name: 'ah', code: 0x04 },
    ch: { type: 'reg', name: 'ch', code: 0x05 },
    dh: { type: 'reg', name: 'dh', code: 0x06 },
    bh: { type: 'reg', name: 'bh', code: 0x07 },
    ax: { type: 'reg', name: 'ax', code: 0x30 },
    cx: { type: 'reg', name: 'cx', code: 0x31 },
    dx: { type: 'reg', name: 'dx', code: 0x32 },
    bx: { type: 'reg', name: 'bx', code: 0x33 },
    sp: { type: 'reg', name: 'sp', code: 0x34 },
    bp: { type: 'reg', name: 'bp', code: 0x35 },
    si: { type: 'reg', name: 'si', code: 0x36 },
    di: { type: 'reg', name: 'di', code: 0x37 },
    eax: { type: 'reg', name:'eax', code: 0x10 },
    ecx: { type: 'reg', name:'ecx', code: 0x11 },
    edx: { type: 'reg', name:'edx', code: 0x12 },
    ebx: { type: 'reg', name:'ebx', code: 0x13 },
    esp: { type: 'reg', name:'esp', code: 0x14 },
    ebp: { type: 'reg', name:'ebp', code: 0x15 },
    esi: { type: 'reg', name:'esi', code: 0x16 },
    edi: { type: 'reg', name:'edi', code: 0x17 },

    precedence: { '+': 0, '-': 0, '*': 1, '/': 1 },

    parse: function(expr) {
        var m;
        var re = /(\s+)|([,\(\)+*\/-])|([A-Za-z][0-9A-Za-z_]*)|(0x[0-9a-fA-F]+)|([1-9][0-9]*|0)|[^\s]+/g;
        var o = null, p = null, s = [], col = 1;
        var raiseSyntaxError = function(msg) {
            throw new Assembler.Exception('Syntax error at column ' + col
                    + (typeof msg != 'undefined' ? ' (' + msg + ')': ''));
        };
//re.__print();
        while ((m = re.exec(expr))) {
            if (typeof m[2] != 'undefined') {
                var c = m[2];
                if (c == '(') {
                    if (o != null) {
                        s.push(p);
                        p = { type: 'function', operands: [o] };
                        o = null;
                        s.push(p);
                        p = null;
                    } else {
                        s.push(p);
                        p = null;
                    }
                } else if (c == ')') {
                    if (p != null) {
                        if (o == null)
                            raiseSyntaxError();
                        p.operands.push(o);
                        o = p;
                    }
                    p = s.pop();
                    if (p != null && p.type == 'function') {
                        p.operands.push(o);
                        o = p;
                        p = s.pop();
                    }
                } else if (c == ',') {
                    if (p != null) {
                        if (o == null)
                            raiseSyntaxError();
                        p.operands.push(o);
                        o = p;
                    }
                    if (s.length > 0 && s[s.length - 1].p.type == 'function') {
                        s[s.length - 1].p.operands.push(o);
                    } else {
                        raiseSyntaxError();
                    }
                    p = null, o = null;
                } else {
                    if (o == null)
                        raiseSyntaxError();
                    var n = null;
                    if (p != null) {
                        var p1 = this.precedence[p.type],
                            p2 = this.precedence[c];
                        if (p1 < p2) {
                            s.push(p);
                        } else if (p1 > p2) {
                            p.operands.push(o);
                            o = p;
                            p = s.pop();
                            if (p != null) {
                                p.operands.push(o);
                                o = p;
                            }
                        } else {
                            p.operands.push(o);
                            o = p;
                        }
                    }
                    p = { type: c, operands: [o] };
                    o = null;
                }
            } else if (typeof m[3] != 'undefined') {
                if (o != null)
                    raiseSyntaxError();
                o = { type: 'label', name: m[3] };
            } else if (typeof m[4] != 'undefined') {
                o = { type: 'number', value: parseInt(m[4]) };
            } else if (typeof m[5] != 'undefined') {
                o = { type: 'number', value: parseInt(m[5]) };
            } else if (typeof m[0] != 'undefined') {
                // do nothing
            } else {
                raiseSyntaxError("invalid token: " + m[0]);
            }
            col += m[0].length;
        }
//for(;;)
        while (true) {
            if (p != null) {
                if (o == null)
                    raiseSyntaxError();
                p.operands.push(o);
                o = p;
            }
            if (!s.length)
                break;
            p = s.pop();
        }
        return o;
    },

    etaConvert: function(n) {
        var convertible = true;
        if (typeof n.operands != 'undefined') {
            var operands = n.operands;
            for (var i = 0; i < operands.length; ++i)
                convertible = convertible && this.etaConvert(operands[i]);
        }
        if (!convertible)
            return false;
        switch (n.type) {
        default:
            return false;
        case 'number':
        case 'mem':
            break;
        case '+':
            n.type = 'number';
            n.value = n.operands[0].value + n.operands[1].value;
            delete n.operands;
            break;
        case '-':
            n.type = 'number';
            n.value = n.operands[0].value - n.operands[1].value;
            delete n.operands;
            break;
        case '*':
            n.type = 'number';
            n.value = n.operands[0].value * n.operands[1].value;
            delete n.operands;
            break;
        case '/':
            n.type = 'number';
            n.value = n.operands[0].value / n.operands[1].value;
            delete n.operands;
            break;
        }
        return true;
    }
};

Assembler.Core = function() {
    __debug.alert(-1);
    this.__class__ = arguments.callee;
    __debug.alert(0)
    this.initialize.apply(this, arguments);
};

Assembler.Core.prototype = {
    Exception: Assembler.Exception,
    Expr: Assembler.Expr,

    initialize: function(buf) {
    __debug.alert(1)
    this._labels = {};
        this._buf = buf;
        this._lastLabel = null;
        this._backpatches = [];
    },

    eval: function(n) {
        switch (n.type) {
        default:
            throw new this.Exception("Unsupported node: " + n.type);
        case 'function':
            switch (n.operands[0].name) {
            default:
                throw new this.Exception("Undefined function: " + n.operands[0].name);
            case 'sizeof':
                var lbl = n.operands[1];
                if (lbl.type != 'label')
                    throw new this.Exception("Usage: sizeof(label)");
                if (!(lbl.name in this._labels))
                    throw new this.Exception("Undefined label: " + lbl.name);
                var size = this._labels[lbl.name].size;
                if (size < 0)
                    throw new this.Exception("Size cannot be determined for the section of label " + lbl.name);
                return size;
            };
        case 'number':
            return n.value;
        case 'label':
            if (!(n.name in this._labels))
                throw new this.Exception("Unresolved label: " + n.name);
            return this._labels[n.name].offset;
        case '+':
            return this.eval(n.operands[0]) + this.eval(n.operands[1]);
        case '-':
            return this.eval(n.operands[0]) - this.eval(n.operands[1]);
        case '*':
            return this.eval(n.operands[0]) - this.eval(n.operands[1]);
        case '/':
            return this.eval(n.operands[0]) - this.eval(n.operands[1]);
        }
    },

    putU32LE: function(a) {
        if (typeof a.value == 'undefined')
            this.deferResolution('U32LE', a);
        else
            this._buf.U32LE(a.value);
    },

    putU16LE: function(a) {
        if (typeof a.value == 'undefined')
            this.deferResolution('U16LE', a);
        else
            this._buf.U16LE(a.value);
    },

    putU8: function(a) {
        if (typeof a.value == 'undefined')
            this.deferResolution('U8', a);
        else
            this._buf.U8(a.value);
    },

    putImmediate: function(c, a) {
        switch (c & 0x30) {
        default:
            throw new this.Exception("Unsupported operand size");
        case 0x00:
            this.putU8(a);
            break;
        case 0x10:
            this.putU32LE(a);
            break;
        case 0x30:
            this.putU16LE(a);
            break;
        }
    },

    deferResolution: function(type, value) {
        var offset = this._buf.offset, anchor = this._buf.anchor;
        this._buf[type].call(this._buf, value);
        this._backpatches.push({ offset: offset, anchor: anchor, type: type, value: value });
    },

    backpatch: function() {
        var backpatches = this._backpatches;
        for (var i = backpatches.length; --i >= 0; ) {
            var bp = backpatches[i];
            this._buf.seek(bp.anchor);
            var v = this.eval(bp.value);
            this._buf[bp.type].call(this._buf, v);
        }
    },

    expandString: function(a) {
        var offset = this._buf.length;
        this._buf.S(a);
        if (this._last_label && this._last_label.offset == offset) {
            this._last_label.size = this._buf.length - offset;
        }
    },

    makeLabel: function(a){
        this._last_label = this._labels[a] = {
            name: a,
            offset: this._buf.length,
            size: -1
        };
    },

    createExpr: function(expr) {
        var n = this.Expr.parse(expr);
        this.Expr.etaConvert(n);
        return n;
    },

    emitMov: function(a, b) {
        if (b.type == 'reg') {
            if (a.type == 'reg') {
                if ((a.code & 0x30) != (b.code & 0x30))
                    throw new this.Exception("Operand size mismatch");
                if (b.code & 0x20)
                    this._buf.U8(0x66);
                this._buf.U8(0x88 | (b.code >> 4) & 1);
                this._buf.U8(0xc0 | ((a.code & 7) << 3) | (b.code & 7));
            } else if (a.type == 'mem') {
                throw new this.Exception("Not implemented");
            } else {
                if (b.code & 0x20)
                    this._buf.U8(0x66);
                this._buf.U8(0xb0 | ((b.code >> 1) & 8) | (b.code & 7));
                this.putImmediate(b.code, a);
            }
        } else {
            throw new this.Exception("Unsupported combination of operands (" + a.type + ":" + b.type + ")");
        }
    },

    emitXor: function(a, b) {
        if (b.type == 'reg') {
            if (a.type == 'reg') {
                if ((a.code & 0x30) != (b.code & 0x30))
                    throw new this.Exception("Operand size mismatch");
                if (b.code & 0x20)
                    this._buf.U8(0x66);
                this._buf.U8(3 | (b.code >> 4) & 1);
                this._buf.U8(0xc0 | ((a.code & 7) << 3) | (b.code & 7));
            } else if (a.type == 'mem') {
                throw new this.Exception("Not implemented");
            } else {
                if (b.code & 0x20)
                    this._buf.U8(0x66);
                this._buf.U8(0x80 | ((b.code >> 4) & 1));
                this._buf.U8(0xf0 | b.code & 7);
                this.putImmediate(b.code, a);
            }
        } else {
            throw new this.Exception("Unsupported combination of operands (" + a.type + ":" + b.type + ")");
        }
    },

    emitAdd: function(a, b) {
        if (b.type == 'reg') {
            if (a.type == 'reg') {
                if ((a.code & 0x30) != (b.code & 0x30))
                    throw new this.Exception("Operand size mismatch");
                if (b.code & 0x20)
                    this._buf.U8(0x66);
                this._buf.U8((b.code >> 4) & 1);
                this._buf.U8(0xc0 | ((a.code & 7) << 3) | (b.code & 7));
            } else if (a.type == 'mem') {
                throw new this.Exception("Not implemented");
            } else {
                if (b.code & 0x20)
                    this._buf.U8(0x66);
                this._buf.U8(0x80 | ((b.code >> 4) & 1));
                this._buf.U8(0xc0 | b.code & 7);
                this.putImmediate(b.code, a);
            }
        } else {
            throw new this.Exception("Unsupported combination of operands (" + a.type + ":" + b.type + ")");
        }
    },

    emitPop: function(a) {
        if (a.type != 'reg' || !(a.code & 0x10))
            throw new this.Exception("Unsupported operand");
        if (a.code & 0x20)
            this._buf.U8(0x66);
        this._buf.U8(0x58 | a.code & 7);
    },

    emitCall: function(a) {

        this._buf.U8(0xe8);
        a = {
            type: '-',
            operands: [ a, { type: 'number', value: this._buf.length + 4 } ]
        };
        this.Expr.etaConvert(a);
        this.putU32LE(a);
    },

    emitInt: function(a) {
        if (typeof a.value == 'undefined') {
            throw new this.Exception('Invalid operand');
        }
        if (a.value == 3) {
            this._buf.U8(0xcc);
        } else {
            this._buf.U8(0xcd);
            this._buf.U8(a.value);
        }
    }
};

Assembler.makeDelegate = function(proc) {
    return function() {
        if (arguments.length > proc.length) {
            throw new this.__class__.Exception('Too many operands');
        }
        for (var i = arguments.length; --i >= 0;) {
            if (typeof arguments[i] == 'number') {
                arguments[i] = { type: 'number', value: arguments[i] };
            } else if (typeof arguments[i] == 'object') {
                if (arguments[i] instanceof Array)
                    arguments[i] = { type: 'mem', operands: [ arguments[i] ] };
            } else {
                throw new this.__class__.Exception('Invalid operand');
            }
        }
        proc.apply(this.core, arguments);
    };
};

Assembler.prototype = {
    al: Assembler.Expr.al,
    cl: Assembler.Expr.cl,
    dl: Assembler.Expr.dl,
    bl: Assembler.Expr.bl,
    ah: Assembler.Expr.ah,
    ch: Assembler.Expr.ch,
    dh: Assembler.Expr.dh,
    bh: Assembler.Expr.bh,
    ax: Assembler.Expr.ax,
    cx: Assembler.Expr.cx,
    dx: Assembler.Expr.dx,
    bx: Assembler.Expr.bx,
    sp: Assembler.Expr.sp,
    bp: Assembler.Expr.bp,
    si: Assembler.Expr.si,
    di: Assembler.Expr.di,
    eax: Assembler.Expr.eax,
    ecx: Assembler.Expr.ecx,
    edx: Assembler.Expr.edx,
    ebx: Assembler.Expr.ebx,
    esp: Assembler.Expr.esp,
    ebp: Assembler.Expr.ebp,
    esi: Assembler.Expr.esi,
    edi: Assembler.Expr.edi,

    initialize: function(buf) {
        throw alert;
        this.core = new Assembler.Core(buf);
    },

    $: function(expr) {
        return this.core.createExpr(expr);
    },

    $string: function(a) {
        this.core.expandString(a);
    },

    $$: function(lbl) {
        this.core.makeLabel(lbl);
    },

    $end: function() {
        this.core.backpatch();
    },

    mov: Assembler.makeDelegate(Assembler.Core.prototype.emitMov),
    add: Assembler.makeDelegate(Assembler.Core.prototype.emitAdd),
    xor: Assembler.makeDelegate(Assembler.Core.prototype.emitXor),
    pop: Assembler.makeDelegate(Assembler.Core.prototype.emitPop),
    call: Assembler.makeDelegate(Assembler.Core.prototype.emitCall),
    int_: Assembler.makeDelegate(Assembler.Core.prototype.emitInt)
};

var buildELF = function(data) {
    var buf = new Buffer();
    var o = data.length + 0x7c;
    buf.S("\x7fELF\1\1\1\0\0\0\0\0\0\0\0\0\2\0\3\0\1\0\0\0\x7c\0\0\7\x34\0\0\0\x54\0\0\0\0\0\0\0\x34\0\x20\0\1\0\x28\0\1\0\0\0\1\0\0\0\0\0\0\0\0\0\0\7\0\0\0\7");
    buf.U32LE(o);
    buf.U32LE(o);
    buf.S("\5\0\0\0\0\x10\0\0\0\0\0\0\1\0\0\0\6\0\0\0\0\0\0\7\0\0\0\0");
    buf.U32LE(o);
    buf.S("\0\0\0\0\0\0\0\0\4\0\0\0\0\0\0\0");
    buf.S(data);
    return buf;
};


try {

    var buf = new Buffer();
with (new Assembler(buf)) {
    call($('get_addr'));
$$('get_addr');
    pop(ecx);
    add($('str - get_addr'), ecx);
    mov($('sizeof(str)'), edx);
    xor(ebx, ebx);
    mov(4, eax);
    aita(80);
    mov(1, eax);
    aita(80);
$$('str');
    $string('Hello, world!\x0d\x0a');
$end();
}

} catch(e) {
   __debug.print(e.message)
}

//location.href = 'data:application/octet-stream,' + escape(buildELF(buf));


