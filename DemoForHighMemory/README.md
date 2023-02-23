(Check my onenote for more details) 

Download and install Debug Diagnostic Tool v2 Update 3. 

Run "DebugDiag 2 Collection". 

Follow https://knowledge.informatica.com/s/article/158308?language=en_US to create a performance rule when "memory usage is below 1GB" then start capturing "full dump" against "executable" powershell process (https://github.com/Chunlong101/SharePointScripts/blob/master/GetYourMemoryHighMemoryLeak.ps1). 

Run "DebugDiag 2 Analysis": 

Analyze the report from #4, you will find top contributors under "DotNetMemoryAnalysis": 

You will see the high cpu thread is doing "mscorlib_ni!System.Collections.Generic.List`1[[System.__Canon, mscorlib]].ToArray()+3f": 

Run Windbg to open that dump file, config the environment: 

.loadby sos clrjit (.load C:\Users\chunlonl\Desktop\Tools\Windbg_6.13.0016.2316\sym\SOS.dll)

.sympath cache*C:\Users\chunlonl\Desktop\Tools\Windbg_6.13.00s16.2316\sym;srv*https://msdl.microsoft.com/download/symbols

Go to that outstanding thread (View >> Processes and Threads) then perform some commands: 

0:012> k

  Child-SP          RetAddr           Call Site
00 00000048`3858e180 00007ff9`9577ef3f clr!ArrayNative::ArrayCopy+0x1ac
01 00000048`3858e330 00007ff9`38013b24 mscorlib_ni!System.Collections.Generic.List`1[System.__Canon].ToArray()$##6003A10+0x3f
02 00000048`3858e390 00007ff9`5eaf1b20 0x00007ff9`38013b24
03 00000048`3858e430 00007ff9`5eaf1b20 System_Management_Automation_ni+0x1561b20
…
1e 00000048`3858fb20 00000000`00000000 ntdll!RtlUserThreadStart+0x21

0:012> !dumpstack

OS Thread Id: 0x46ac (12)
Current frame: clr!ArrayNative::ArrayCopy+0x1ac
Child-SP         RetAddr          Caller, Callee
…
000000483858fb10 00007ff9b9582651 ntdll!RtlUserThreadStart+0x21, calling ntdll!guard_dispatch_icall_nop

0:012> !clrstack -p

OS Thread Id: 0x46ac (12)
        Child SP               IP Call Site
000000483858e208 00007ff9970dd9fc [HelperMethodFrame: 000000483858e208] 
000000483858e330 00007ff99577ef3f System.Collections.Generic.List`1[[System.__Canon, mscorlib]].ToArray()
    PARAMETERS:
        this (<CLR reg>) = 0x0000017b6114b648

000000483858e390 00007ff938013b24 DynamicClass.lambda_method(System.Runtime.CompilerServices.Closure, System.Object[], System.Runtime.CompilerServices.StrongBox`1[], System.Management.Automation.Interpreter.InterpretedFrame)
    PARAMETERS:
        <no data>
        <no data>
        <no data>
        <no data>
…

0:012> !dumpstackobjects

OS Thread Id: 0x46ac (12)
RSP/REG          Object           Name
rax              0000017c3f761020 System.String    <String is invalid or too large to print>

000000483858E198 0000017b6049ffb0 System.Management.Automation.Language.VariableExpressionAst
…
000000483858EDA0 0000017b54d8fa60 System.Threading.ThreadStart

0:012> !do 0x17c3f761020

Name:        System.String
MethodTable: 00007ff9952359c0
EEClass:     00007ff995212ec0
Size:        419430426(0x1900001a) bytes
File:        C:\WINDOWS\Microsoft.Net\assembly\GAC_64\mscorlib\v4.0_4.0.0.0__b77a5c561934e089\mscorlib.dll
String:      <String is invalid or too large to print>

Fields:
              MT    Field   Offset                 Type VT     Attr            Value Name
00007ff9952385a0  4000283        8         System.Int32  1 instance        209715200 m_stringLength
00007ff995236838  4000284        c          System.Char  1 instance               61 m_firstChar
00007ff9952359c0  4000288       e0        System.String  0   shared           static Empty
                                 >> Domain:Value  0000017b52758030:NotInit  <<
0:012> db 0000017c3f761020
0000017c`3f761020  c0 59 23 95 f9 7f 00 00-00 00 80 0c 61 00 61 00  .Y#.........a.a.
0000017c`3f761030  61 00 61 00 61 00 61 00-61 00 61 00 61 00 61 00  a.a.a.a.a.a.a.a.
0000017c`3f761040  61 00 61 00 61 00 61 00-61 00 61 00 61 00 61 00  a.a.a.a.a.a.a.a.
0000017c`3f761050  61 00 61 00 61 00 61 00-61 00 61 00 61 00 61 00  a.a.a.a.a.a.a.a.
0000017c`3f761060  61 00 61 00 61 00 61 00-61 00 61 00 61 00 61 00  a.a.a.a.a.a.a.a.
0000017c`3f761070  61 00 61 00 61 00 61 00-61 00 61 00 61 00 61 00  a.a.a.a.a.a.a.a.
0000017c`3f761080  61 00 61 00 61 00 61 00-61 00 61 00 61 00 61 00  a.a.a.a.a.a.a.a.
0000017c`3f761090  61 00 61 00 61 00 61 00-61 00 61 00 61 00 61 00  a.a.a.a.a.a.a.a.

We can see those "a" there, which means #12 thread is doing certain array and string operations that consumes a lot of memory, "0x17c3f761020" is on the list (top contributors) of report as well. 

We can also use below logic to identify out who consumes the most of memory: 

In the report we can see above, it has the memory address there for example "0x17b604e0bd0", above only shows the first one, we can search (!do) them one by one: 

0:012> !do 0x17b646e8d80

Name:        System.Object[]
MethodTable: 00007ff995235e70
EEClass:     00007ff9953a56b0
Size:        545560(0x85318) bytes
Array:       Rank 1, Number of elements 68192, Type CLASS (Print Array)
Fields:
None

0:012> !do 0x17b643cf5c8

Name:        System.Object[]
MethodTable: 00007ff995235e70
EEClass:     00007ff9953a56b0
Size:        32664(0x7f98) bytes
Array:       Rank 1, Number of elements 4080, Type CLASS (Print Array)
Fields:
None
(Above two are very large array, but sizes are okay) 

0:012> !do 0x17c3f761020

Name:        System.String
MethodTable: 00007ff9952359c0
EEClass:     00007ff995212ec0
Size:        419430426(0x1900001a) bytes
File:        C:\WINDOWS\Microsoft.Net\assembly\GAC_64\mscorlib\v4.0_4.0.0.0__b77a5c561934e089\mscorlib.dll
String:      <String is invalid or too large to print>

Fields:
              MT    Field   Offset                 Type VT     Attr            Value Name
00007ff9952385a0  4000283        8         System.Int32  1 instance        209715200 m_stringLength
00007ff995236838  4000284        c          System.Char  1 instance               61 m_firstChar
00007ff9952359c0  4000288       e0        System.String  0   shared           static Empty
                                 >> Domain:Value  0000017b52758030:NotInit  <<
