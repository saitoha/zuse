
d1 = {d:111};
data = {
    a: {
        b: {
            c: function() {
                return {
                    d: {
                        e: function() {
                            return {
                                f: function() {
                                    return 456;
                                }
                            };
                        }
                    }
                };
            }
        }
    }
};

//__debug.alert(data.a.b.c)
//__debug.alert(data.a.b.c())
//__debug.alert(data.a.b.c().d)
//__debug.alert(data.a.b.c().d.e)
//__debug.alert(data.a.b.c().d.e())
//__debug.alert(data.a.b.c().d.e().f)
__debug.print(data.a.b.c().d.e().f() == 456)
