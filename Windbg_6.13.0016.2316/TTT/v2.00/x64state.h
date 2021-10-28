//-----------------------------------------------------------------------------
//
// Nirvana
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Nirvana processor state definitions for 64-bit X64 user mode
//
// Remarks:
//
//    This is a common header file for Nirvana clients and components
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef X64STATE
#define X64STATE
#undef _SP
#undef _DI
#ifdef __cplusplus
namespace Nirvana
{
#endif

#pragma warning (push)
#pragma warning (disable:4201)
#pragma pack(4)

// x87/SSE/SSE2 multimedia registers are store in 512-byte FXSAVE format.
// This if for use on hosts that support SSE/SSE2, since FXSAVE is needed.

typedef struct XMMSTATE64
{
    unsigned short  _FCW;
    unsigned short  _FSW;
    unsigned short  _FTW;
    unsigned short  _FOP;
    union
    {
        ULONG64     _RIP;
        struct
        {
            unsigned long   _EIP;
            unsigned long   _CS;
        };
    };
    union
    {
        ULONG64     _DP64;
        struct
        {
            unsigned long   _DP;
            unsigned long   _DS;
        };
    };
    unsigned long   _MXCSR;
    unsigned long   _MXCSR_MASK;

    unsigned char   _ST0[16];
    unsigned char   _ST1[16];
    unsigned char   _ST2[16];
    unsigned char   _ST3[16];
    unsigned char   _ST4[16];
    unsigned char   _ST5[16];
    unsigned char   _ST6[16];
    unsigned char   _ST7[16];

    unsigned char   _XMM0[16];
    unsigned char   _XMM1[16];
    unsigned char   _XMM2[16];
    unsigned char   _XMM3[16];
    unsigned char   _XMM4[16];
    unsigned char   _XMM5[16];
    unsigned char   _XMM6[16];
    unsigned char   _XMM7[16];
    unsigned char   _XMM8[16];
    unsigned char   _XMM9[16];
    unsigned char   _XMM10[16];
    unsigned char   _XMM11[16];
    unsigned char   _XMM12[16];
    unsigned char   _XMM13[16];
    unsigned char   _XMM14[16];
    unsigned char   _XMM15[16];

    unsigned char   reserved[96];
} XMMSTATE64;


// The x87 floating point registers are store in 108-byte FNSAVE format.
// This is for hosts that only support x87 and MMX, but not SSE/SSE2.

typedef struct X87STATE64
{
    unsigned long   _FCW;
    unsigned long   _FSW;
    unsigned long   _FTW;
    unsigned long   _FIP;
    unsigned short  _CS;
    unsigned short  _OP;
    unsigned long   _DP;
    unsigned long   _DS;

    union
        {
        unsigned char       _sFPR[8*10];

        struct
            {
            unsigned char   _ST0[10];
            unsigned char   _ST1[10];
            unsigned char   _ST2[10];
            unsigned char   _ST3[10];
            unsigned char   _ST4[10];
            unsigned char   _ST5[10];
            unsigned char   _ST6[10];
            unsigned char   _ST7[10];
            };
        };
} X87STATE64;


// x64 descriptor
typedef struct _Descriptor64
{
    unsigned short pad[3];
    unsigned short limit;
    ULONG64 base;
} Descriptor64;

// registers/state availabe only in kernel mode (eg CRs)
typedef struct _KernelRegs64
{
    ULONG64 _CR0;
    ULONG64 _CR2;
    ULONG64 _CR3;
    ULONG64 _CR4;

    ULONG64 _DR0;
    ULONG64 _DR1;
    ULONG64 _DR2;
    ULONG64 _DR3;
    union
    {
        ULONG64 _DR4;
        ULONG64 _DR6;
    };
    union
    {
        ULONG64 _DR5;
        ULONG64 _DR7;
    };

    Descriptor64 _GDTR;
    Descriptor64 _IDTR;

    unsigned short _TR;
    unsigned short _LDTR;

    char rsvd[4];       // for padding

    ULONG64 _EFER;                  // Extended Feature Enable Register
    ULONG64 _STAR;                  // Syscal/sysret Target Address Register
    void*   _LSTAR;                 // Long-mode STAR
    void*   _CSTAR;                 // Compatibility STAR
    ULONG64 _SFMASK;                // Syscall Flags Mask

    ULONG64 _GsBaseSwap;            // from MSR_KernelGsBase

    ULONG64 _CR8;
} KernelRegs64;

// info about interrupts (TENTATIVE)
typedef struct _InterruptInfo64
{
    unsigned long errorCode;
    unsigned char vectorNo;
    unsigned char lengthINTn;       // instruction length for software "INT n"
                                    // 0 otherwise
    unsigned char iiPadding;
    unsigned char isException;      // non-zero value for exceptions
    ULONG64 savedIP;
    ULONG64 savedCS;
    ULONG64 savedEFLAGS;
    ULONG64 savedSP;
} InterruptInfo64;


// This is the user-mode x64 register state including integer, float, and SIMD regs

typedef struct X64REGS
{
    // the integer registers are in the same layout as a 486 task state segment

    union
        {
        ULONG64             _RIP;
        unsigned long       _EIP;
        };

    union
        {
        ULONG64             _RFLAGS;
        unsigned long       _EFLAGS;
        unsigned short      _FLAGS;
        unsigned char       _FLAGS8;
        };

    union
        {
        ULONG64             _RAX;
        unsigned long       _EAX;
        unsigned short      _AX;

        struct
            {
            unsigned char   _AL;
            unsigned char   _AH;
            };
        };

    union
        {
        ULONG64             _RCX;
        unsigned long       _ECX;
        unsigned short      _CX;

        struct
            {
            unsigned char   _CL;
            unsigned char   _CH;
            };
        };

    union
        {
        ULONG64             _RDX;
        unsigned long       _EDX;
        unsigned short      _DX;

        struct
            {
            unsigned char   _DL;
            unsigned char   _DH;
            };
        };

    union
        {
        ULONG64             _RBX;
        unsigned long       _EBX;
        unsigned short      _BX;

        struct
            {
            unsigned char   _BL;
            unsigned char   _BH;
            };
        };

    union
        {
        ULONG64             _RSP;
        unsigned long       _ESP;
        unsigned short      _SP;
        unsigned char       _SPL;
       };

    union
        {
        ULONG64             _RBP;
        unsigned long       _EBP;
        unsigned short      _BP;
        unsigned char       _BPL;
        };

    union
        {
        ULONG64             _RSI;
        unsigned long       _ESI;
        unsigned short      _SI;
        unsigned char       _SIL;
        };

    union
        {
        ULONG64             _RDI;
        unsigned long       _EDI;
        unsigned short      _DI;
        unsigned char       _DIL;
        };

    union
        {
        ULONG64             _R8;
        unsigned long       _R8D;
        unsigned short      _R8W;
        unsigned char       _R8B;
        };

    union
        {
        ULONG64             _R9;
        unsigned long       _R9D;
        unsigned short      _R9W;
        unsigned char       _R9B;
        };

    union
        {
        ULONG64             _R10;
        unsigned long       _R10D;
        unsigned short      _R10W;
        unsigned char       _R10B;
        };

    union
        {
        ULONG64             _R11;
        unsigned long       _R11D;
        unsigned short      _R11W;
        unsigned char       _R11B;
        };

    union
        {
        ULONG64             _R12;
        unsigned long       _R12D;
        unsigned short      _R12W;
        unsigned char       _R12B;
        };

    union
        {
        ULONG64             _R13;
        unsigned long       _R13D;
        unsigned short      _R13W;
        unsigned char       _R13B;
        };

    union
        {
        ULONG64             _R14;
        unsigned long       _R14D;
        unsigned short      _R14W;
        unsigned char       _R14B;
        };

    union
        {
        ULONG64             _R15;
        unsigned long       _R15D;
        unsigned short      _R15W;
        unsigned char       _R15B;
        };

    ULONG64                 _FSBase;
    ULONG64                 _GSBase;

    unsigned short          _ES;
    unsigned short          _CS;
    unsigned short          _SS;
    unsigned short          _DS;
    unsigned short          _FS;
    unsigned short          _GS;

    unsigned char           reserved[4];    // end with 16-byte alignment

    // end of 486 task state segment layout

    union
        {
        XMMSTATE64 _XMMREGS;
        X87STATE64 _X87REGS;
        };
} X64REGS;


// This is the kernel-mode x64 register state
typedef struct KX64REGS
{
    X64REGS      _UserRegs;
    KernelRegs64 _KernRegs;
} KX64REGS;

#pragma pack()
#pragma warning (pop)

#ifdef __cplusplus
} // namespace
#endif

#endif // X64STATE

