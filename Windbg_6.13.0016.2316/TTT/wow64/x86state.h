//-----------------------------------------------------------------------------
//
// Nirvana
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Nirvana processor state definitions for 32-bit X86 user mode
//
// Remarks:
//
//    This is a common header file for Nirvana clients and components
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef X86STATE
#define X86STATE

// allow unnamed structs and unions
#pragma warning (push)
#pragma warning (disable:4201)

#ifdef __cplusplus
namespace Nirvana
{
#endif


#pragma pack(4)

// x87/SSE/SSE2 multimedia registers are store in 512-byte FXSAVE format.
// This if for use on hosts that support SSE/SSE2, since FXSAVE is needed.

typedef struct XMMSTATE32
{
    unsigned short  _FCW;
    unsigned short  _FSW;
    unsigned short  _FTW;
    unsigned short  _FOP;
    unsigned long   _EIP;
    unsigned long   _CS;
    unsigned long   _DP;
    unsigned long   _DS;
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

    unsigned char   reserved[224];
} XMMSTATE32;


// The x87 floating point registers are store in 108-byte FNSAVE format.
// This is for hosts that only support x87 and MMX, but not SSE/SSE2.

typedef struct X87STATE32
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
} X87STATE32;


// x86 descriptor
typedef struct _Descriptor32
{
    unsigned short pad[1];
    unsigned short limit;
    unsigned long base;
} Descriptor32;

// registers/state availabe only in kernel mode (eg CRs)
typedef struct _KernelRegs32
{
    unsigned long _CR0;
    unsigned long _CR2;
    unsigned long _CR3;
    unsigned long _CR4;

    unsigned long _DR0;
    unsigned long _DR1;
    unsigned long _DR2;
    unsigned long _DR3;
    union
    {
        unsigned long _DR4;
        unsigned long _DR6;
    };
    union
    {
        unsigned long _DR5;
        unsigned long _DR7;
    };

    Descriptor32 _GDTR;
    Descriptor32 _IDTR;

    unsigned short _TR;
    unsigned short _LDTR;

    unsigned long _SYSENTER_CS;
    void* _SYSENTER_ESP;
    void* _SYSENTER_EIP;

    char rsvd[8];       // for padding
} KernelRegs32;

// info about interrupts (TENTATIVE)
typedef struct _InterruptInfo32
{
    unsigned char vectorNo;
    unsigned char lengthINTn;       // instruction length for software "INT n"
                                    // 0 otherwise
    unsigned char iiPadding;
    unsigned char isException;      // non-zero value for exceptions
    unsigned long errorCode;
    unsigned long savedIP;
    unsigned long savedCS;
    unsigned long savedEFLAGS;
} InterruptInfo32;


// This is the user-mode x86 register state including integer, float, and SIMD regs

typedef struct X86REGS
{
    // the integer registers are in the same layout as a 486 task state segment

    union
        {
        unsigned long       _EIP;
        };

    union
        {
        unsigned long       _EFLAGS;
        unsigned short      _FLAGS;
        unsigned char       _FLAGS8;
        };

    union
        {
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
        unsigned long       _ESP;
        unsigned short      _SP;
        };

    union
        {
        unsigned long       _EBP;
        unsigned short      _BP;
        };

    union
        {
        unsigned long       _ESI;
        unsigned short      _SI;
        };

    union
        {
        unsigned long       _EDI;
        unsigned short      _DI;
        };

    // WARNING: _CSBase is invalid in KNirvana!!
    unsigned long           _CSBase;
    unsigned long           _FSBase;
    unsigned long           _GSBase;

    unsigned short          _ES;
    unsigned short          _CS;
    unsigned short          _SS;
    unsigned short          _DS;
    unsigned short          _FS;
    unsigned short          _GS;

    // end of 486 task state segment layout

    union
        {
        XMMSTATE32 _XMMREGS;
        X87STATE32 _X87REGS;
        };
} X86REGS;

// This is the kernel-mode x86 register state

typedef struct KX86REGS
{
    X86REGS      _UserRegs;
    KernelRegs32 _KernRegs;
} KX86REGS;


#pragma pack()

#ifdef __cplusplus
} // namespace
#endif

// allow unnamed structs and unions
#pragma warning (pop)

#endif // X86STATE

