zuse - A Fuzzy ECMAScript Implementation.
=========================================

Supported platforms
--------------------

    WINNT-i386-MSVC{7.1,8,9}    - supports JIT, COM support
    WINNT-x86_64-MSVC{7.1,8,9} 
    Cygwin-i386-gcc{3,4}        - supports JIT
    Darwin-i386-gcc{3,4}        - supports JIT
    Darwin-x86_64-gcc{3,4}
    Linux-i386-gcc{3,4}         - supports JIT
    Linux-x86_64-gcc{3,4}

INSTALL
-------

    - Cygwin, Darwin, Linux
        ./configure && make
        sudo make install

WINNT COM Support, and IActiveScriptParse Integration
-----------------------------------------------------

    In WINNT environment, This engine suports active script, 
    This feature makes you enable to call it using <script language="zuse">...</script> tag,
    at IE-component, HTA, IIS/ASP, etc..
    Then COM factory feature such as "new ActiveXObject('...')", will be also enabled.


