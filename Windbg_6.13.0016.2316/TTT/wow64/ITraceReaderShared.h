//-----------------------------------------------------------------------------
// Copyright (C) 2013 Microsoft Corporation
//
// File name:
//      ITraceReaderShared.h
//
// Abstract:
//      Unified header for v1 and v2.
//
// Description:
//      Definitions for various types and constants which are likely to remain
//      compatible between v1 and v2.
//
// Usage:
//      When linking directly against iDNA v1 or v2, include ITraceReader.h
//      for the desired version and do not include ITraceReaderShared.h
//      directly.
//
//      Code which implements support for both v1 and v2 may include this
//      file directly, since it is not possible to include ITraceReader.h
//      for v1 and v2 simultaneously. WARNING: This file may be removed in
//      the future if and when v1 goes away forever. At that time, code which
//      depends upon this file will need to include ITraceReader.h for v2.
//
// WARNINGS FOR ANYONE EDITING THIS FILE:
//     1) If possible, please avoid changing enum values or structure layout
//        in ways that would break compatibility reading legacy v1 traces.
//     2) When it is not possible to do so, please consider moving constants
//        and type definitions back in to the respective ITraceReader.h
//        headers, and notify consumers of this header so that they can plan
//        accordingly and refactor logic to consume the version specific
//        ITraceReader.h as appropriate.
//     3) If a proposed change to this header breaks the build for any tool
//        (such as TruScan or the debugger), it's a good indication that
//        the tool needs to be updated. However, don't rely on build breaks to
//        identify all breaking changes, since reordering structures or enum
//        values could break compatibility for legacy traces without breaking a
//        build.
//
// Revision History:
//  1   Evan Tice (evant) 10/30/2013
//      Refactored out of ITraceReader.h
//-----------------------------------------------------------------------------

#ifndef ITRACEREADERSHARED
#define ITRACEREADERSHARED

#pragma once

#include <ole2.h>
#include <windows.h>
#include <evntprov.h>
#if !defined(_IMAGEHLP_) && !defined(_DBGHELP_)
#define DBGHELP_TRANSLATE_TCHAR
#include <dbghelp.h>
#endif


//SMGL - Trace Header Structure
typedef struct _TRACE_HEADER
{
    ULONG signature;
    ULONG version;
    GUID  toolID;
} TRACE_HEADER, *PTRACE_HEADER;

//Unique global file signature; it is appear in the first 128 bytes of each
//trace file.  Traces of 32 and 64 bit processes have different signatures.
const GUID TRACE_FILE_SIGNATURE_X86 = { /* aea5e609-d6dc-4afc-9487-3a042acea989 */
    0xaea5e609,
    0xd6dc,
    0x4afc,
    {0x94, 0x87, 0x3a, 0x04, 0x2a, 0xce, 0xa9, 0x89}
  };

const GUID TRACE_FILE_SIGNATURE_X64 = { /* dae571d8-5138-4c6e-8280-b2d662b73cba */
    0xdae571d8,
    0x5138,
    0x4c6e,
    {0x82, 0x80, 0xb2, 0xd6, 0x62, 0xb7, 0x3c, 0xba}
  };

const GUID TRACE_FILE_SIGNATURE_X86_20 = { /* 9466c11a-3825-4d58-94b5-2d133454a0bc */
    0x9466c11a,
    0x3825,
    0x4d58,
    {0x94, 0xb5, 0x2d, 0x13, 0x34, 0x54, 0xa0, 0xbc}
  };

const GUID TRACE_FILE_SIGNATURE_X64_20 = { /* f207615d-6e79-42a1-bf95-5765bca25b77 */
    0xf207615d,
    0x6e79,
    0x42a1,
    {0xbf, 0x95, 0x57, 0x65, 0xbc, 0xa2, 0x5b, 0x77}
  };

const GUID TRACE_FILE_SIGNATURE_KX86 = { /* 4CAAFACD-5F27-420f-8EEB-9E00208B3C62 */
    0x4caafacd,
    0x5f27,
    0x420f,
    {0x8e, 0xeb, 0x9e, 0x0, 0x20, 0x8b, 0x3c, 0x62}
  };

const GUID TRACE_FILE_SIGNATURE_KX64 = { /* 82A4CFB8-7B3A-40ba-A5CE-93711F74612A */
    0x82a4cfb8,
    0x7b3a,
    0x40ba,
    {0xa5, 0xce, 0x93, 0x71, 0x1f, 0x74, 0x61, 0x2a}
  };
  
const GUID TRACE_FILE_SIGNATURE_A32_20 = { /* F41F55BB-3C1F-4D29-A173-08B6FAD2D4B9 */
    0xf41f55bb, 
    0x3c1f, 
    0x4d29, 
    { 0xa1, 0x73, 0x8, 0xb6, 0xfa, 0xd2, 0xd4, 0xb9 } 
  };
  
const GUID TRACE_FILE_SIGNATURE_KA32_20 = { /* C6AF32EA-97C9-4944-8C5E-F99DFC5C5070 */
    0xc6af32ea, 
    0x97c9, 
    0x4944, 
    { 0x8c, 0x5e, 0xf9, 0x9d, 0xfc, 0x5c, 0x50, 0x70 } 
  };

/* 04737ff3-83c3-4368-86c8-993e4df32fd9 */
DEFINE_GUID(IID_ITREADER,
            0x04737ff3,
            0x83c3,
            0x4368,
            0x86, 0xc8, 0x99, 0x3e, 0x4d, 0xf3, 0x2f, 0xd9);

typedef interface DECLSPEC_UUID("04737ff3-83c3-4368-86c8-993e4df32fd9") ITREADER* PITREADER;


/* {93F82E2F-3A8F-4D4C-9BBE-78575EB7F1B9} */
DEFINE_GUID(IID_ITREADER2,
            0x93f82e2f,
            0x3a8f,
            0x4d4c,
            0x9b, 0xbe, 0x78, 0x57, 0x5e, 0xb7, 0xf1, 0xb9);

typedef interface DECLSPEC_UUID("93F82E2F-3A8F-4D4C-9BBE-78575EB7F1B9") ITREADER2* PITREADER2;


//
// Return code for IREADER interface
//
const HRESULT TR_ERROR_SUCCESS                    = NO_ERROR;
const HRESULT TR_ERROR_FAILURE                    = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x1);
const HRESULT TR_ERROR_NOT_IMPLEMENTED            = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x2);
const HRESULT TR_ERROR_INVALID_POSITION           = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x3);
const HRESULT TR_ERROR_ADDR_NOT_FOUND             = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x4);
const HRESULT TR_ERROR_BAD_ALIGNMENT              = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x5);
const HRESULT TR_ERROR_MODULE_UNKNOWN             = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x6);
const HRESULT TR_ERROR_MORE_DATA                  = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x7);
const HRESULT TR_ERROR_FILE_NOT_FOUND             = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x8);
const HRESULT TR_ERROR_BAD_FILE_FORMAT            = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x9);
const HRESULT TR_ERROR_TRACEFILE_VERSION_MISMATCH = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xa);
const HRESULT TR_ERROR_BREAKPOINT_HIT             = MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_NULL, 0xb);
const HRESULT TR_ERROR_OUTOFMEMORY                = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xc);
// COM ref count 0 or initialization failed.
const HRESULT TR_ERROR_INVALID_READER             = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xd);
const HRESULT TR_ERROR_REPLAY_FAILURE             = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xe);
// This error occurs if you try to attach to a trace file and there is another reader
// in the same process that has already started executing.  See CreateITReader() for
// details.
const HRESULT TR_ERROR_TOO_MANY_READERS           = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xf);
const HRESULT TR_ERROR_BAD_CALLBACK_TYPE          = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x10);
const HRESULT TR_ERROR_ENDOFTRACE                 = MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_NULL, 0x11);
const HRESULT TR_ERROR_BREAKPOINT_EXISTS          = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x12);
const HRESULT TR_ERROR_SEQUENCE_END               = MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_NULL, 0x13);
const HRESULT TR_ERROR_MEMORY_VALUE_EXISTS        = MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_NULL, 0x14);
const HRESULT TR_ERROR_PENDING                    = MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_NULL, 0x15);
const HRESULT TR_ERROR_COMMIT_FAILED              = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x16);
const HRESULT TR_ERROR_NO_INDEX                   = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x17);
const HRESULT TR_ERROR_MEMORY_VALUE_OVERWRITE     = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x18);
const HRESULT TR_ERROR_BAD_INPUT                  = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x19);
const HRESULT TR_ERROR_BREAKPOINT_ENDOFTRACE      = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x1A);
const HRESULT TR_ERROR_INCOMPATIBLE_READERS       = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x1B);
// The reader cannot replay a trace file on a system with a lower processor level
// than the machine that the guest process was recorded on.
const HRESULT TR_ERROR_PROCESSOR_COMPATIBILITY    = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x1C);
// Log file has no sequence or hard sequence data (i.e. no fixed state to start replay from).
const HRESULT TR_ERROR_NO_SEQUENCE                = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x1D);
// The trace file is not a TTT trace or it is of the wrong type.  For example you cannot
// open a trace of a 32 bit guest process with the 64 bit reader and visa versa.
const HRESULT TR_ERROR_INCOMPATIBLE_TRACE_FILE    = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x1E);
// The trace file added to the group reader is not in the correct group.
const HRESULT TR_ERROR_NOT_IN_GROUP               = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x1F);
// The passed in handle is not valid.
const HRESULT TR_ERROR_INVALID_HANDLE             = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x20);
// The number of handles needed for threads, module loads, exceptions, or breakpoints exceeds
// the limit.
const HRESULT TR_ERROR_HANDLE_OVERFLOW            = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x21);
// Cannot evaluate a memory address to a value needed by the Search() API.
const HRESULT TR_ERROR_NO_VALUE                   = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x22);
// Semantic error in the Search() API expression.
const HRESULT TR_ERROR_EXPRESSION_SEMANTICS       = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x23);
// The number of sequences the trace holds are too many to process the trace.  The sequence limit
// for each trace when attaching multiple traces to the reader is less than if only reading one
// trace, so just load in one trace to work around this.
const HRESULT TR_ERROR_SEQUENCE_OVERFLOW          = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x24);
// Not enough privileges to complete the operation.
const HRESULT TR_ERROR_PRIVILEGE                  = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x25);
// Trying to connect to a debug session that is already active.
const HRESULT TR_ERROR_ALREADY_CONNECTED          = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, 0x26);
// Result of the operation did not result in a change of state.
const HRESULT TR_ERROR_VALUE_UNCHANGED            = MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_NULL, 0x27);


//
// Constants
//


//
// TR_MAX_INSTRUCTION_LEN changed between v1 and v2. Code which includes ITraceReader.h
// will pick up the appropriate values from the constants defined below, and code which
// can not include a specific version if ITraceReader.h can instead consume the constants
// below
//

#define TR_MAX_INSTRUCTION_LEN_V1   15
#define TR_MAX_INSTRUCTION_LEN_V2   48

#define TR_MAX_USER_NAME            64
#define TR_MAX_SYSTEM_NAME          64
#define TR_MAX_MARKER_MESSAGE       32

// See GetCodeBytes()
#define TR_DEFAULT_CODEBYTE         0xcc

//
// Forward declare TR_REGISTER_STATE.
// Definitions provided in ITraceReader.h.
//
union TR_REGISTER_STATE;

// Description - SEQUENCES (see the design document for more details):
//  Threads are synchronized using sequence numbers.  If one event has a higher sequence
//  number than another event, that implies that the higher sequence numbered event came later.
//  Sequence numbers cannot be directly used to determine whether one instruction
//  follows another.  Use the ComparePositions method for that.
//
//  A sequence of instructions is the set of instructions on one thread
//  between two sequencing events.
typedef LONGLONG  TR_SEQUENCE;

// All addresses converted to 64bit; 32-bit addresses are NOT sign extended.
// Note on SAFE_TR_ADDRESS: It is designed for dbgeng.  The reason is that
// dbgeng always sign extends 32-bit addresses; defining SAFE_TR_ADDRESS
// enforces proper code to convert between TR_ADDRESSes (not signe-extended)
// and dbgeng's addresses (sign-extended).
#ifdef SAFE_TR_ADDRESS
struct TR_ADDRESS
{
   ULONGLONG Value;
};
#else
typedef ULONGLONG TR_ADDRESS;
#endif

//
// There is one unique position handle for every instruction in the reader.  Positions map instructions.
//
typedef ULONGLONG TR_POSITION_HANDLE;

//
// A ITREADER may contain multiple processes (see AttachTraceFile()).  Each process has a
// unique handle.  The current process is the active process at the current position in
// the trace.
//
typedef ULONG TR_PROCESS_HANDLE;
const TR_PROCESS_HANDLE TR_INVALID_PROCESS_HANDLE = (TR_PROCESS_HANDLE)-1;

//
// There is one unique thread handle per active thread in the reader.
//
typedef ULONG TR_THREAD_HANDLE;

//
// This is one module handle for every unique module loaded in the reader.
//
typedef ULONG TR_MODULE_HANDLE;

//
// There is a unique breakpoint handle for every breakpoint set.
//
typedef ULONG TR_BREAKPOINT_HANDLE;

//
// This handle maps to an exception record corresponding to an
// exception sequencing event.
//
typedef ULONG TR_EXCEPTION_RECORD_HANDLE;

//
// This handle maps a user string marker handle to the
// string.
//
typedef ULONG TR_MARKER_HANDLE;

//
// This handle maps to a logged ETW event.
//
typedef ULONG TR_ETW_EVENT_HANDLE;


typedef struct TR_TIMING_INFO
{
   // GetSystemTime() function timing information.
   FILETIME               SystemTime;
   // GetProcessTimes() function timing information.
   FILETIME               ProcessCreateTime;
   FILETIME               ProcessUserTime;
   FILETIME               ProcessKernelTime;
   // Currently undefined.
   FILETIME               SystemUpTime;
} *PTR_TIMING_INFO;

// NB: For backward compatibility, this structure must NOT be larger than
// MINIDUMP_SYSTEM_INFO, which is 0x38 bytes.
typedef struct KTR_SYSTEM_INFO
{
   USHORT  _ProcessorArchitecture; // 0x02
   USHORT  _ProcessorLevel;        // 0x04

   ULONG   _NtBuildNumber;         // 0x08

   TR_ADDRESS _KernBase;           // 0x10
   TR_ADDRESS _PsLoadedModuleList; // 0x18
   TR_ADDRESS _DebuggerDataList;   // 0x20
} *PKTR_SYSTEM_INFO;

typedef struct TR_SYSTEM_INFO
{
   ULONG                  MajorVersion;   // Log major version
   ULONG                  MinorVersion;   // Log minor version
   ULONG                  BuildNumber;    // Log build version
   // ProcessId is always 0 in kernel-mode.
   ULONG                  ProcessId;      // System Process Id
   TR_TIMING_INFO         Time;
   union
   {
      MINIDUMP_SYSTEM_INFO SystemInfo;    // For user-mode traces.
      KTR_SYSTEM_INFO      KSystemInfo;   // For kernel-mode traces.
   };
   // Name of person that ran the guest process.
   WCHAR                  UserName[TR_MAX_USER_NAME];
   WCHAR                  SystemName[TR_MAX_SYSTEM_NAME];
} *PTR_SYSTEM_INFO;


enum TR_IMAGE_RANGE_TYPE
{
   TR_IMAGE_RANGE_DOS_HEADER,        // Saved IMAGE_DOS_HEADER
   TR_IMAGE_RANGE_FILE_HEADER,       // Saved IMAGE_FILE_HEADER
   TR_IMAGE_RANGE_OPTIONAL_HEADER32, // Saved IMAGE_OPTIONAL_HEADER32
   TR_IMAGE_RANGE_OPTIONAL_HEADER64, // Saved IMAGE_OPTIONAL_HEADER64
   TR_IMAGE_RANGE_DEBUG_DIRECTORY,   // Saved IMAGE_DEBUG_DIRECTORY
   TR_IMAGE_RANGE_DEBUG_DATA,        // Saved debug information from debug directory
   TR_IMAGE_RANGE_VERSION_RESOURCE,  // Saved resource information
};

typedef struct TR_IMAGE_ADDRESS_RANGE
{
   TR_IMAGE_RANGE_TYPE RangeType; // Type of range that has been captured.
   ULONG StartRva;                // The RVA of the range in the original image.
   ULONG Size;                    // Size of the range.
} *PTR_IMAGE_ADDRESS_RANGE;


// When an image is loaded, we capture information from the image that
// is useful to uniquely identify the image and enables loading of
// the matching PDB. This information is the image header (with optional header),
// the debug directories, and the data that is pointed to by the
// debug directories. The HeaderRanges array will describe the type
// of range it is as well as the size and original RVA (offset in the binary).
// The actual data is packed in the HeaderData buffer of the Module.

typedef struct TR_MODULE
{
   TR_SEQUENCE LoadTime;            // 0 indicates no module
   TR_ADDRESS  ModuleBase;
   ULONG       ModuleSize;
   WCHAR       ModuleName[MAX_PATH];
   ULONG       HeaderRangeCount;
   const TR_IMAGE_ADDRESS_RANGE *  HeaderRanges;
   const BYTE * HeaderData;
} *PTR_MODULE;

// On Vista+ systems TTT records an ETW event for all calls to
// EventWrite, EventWriteString, and EventWriteTransfer.  The
// timestamp does not exactly match the timestamp for the
// corresponding event, so given a real ETW timestamp you want
// to find the closest traced ETW timestamp.
typedef struct TR_ETW_EVENT
{
   // QueryPerformanceCounter() and GetSystemTimeAsFileTime().
   LARGE_INTEGER    PerfCounter;
   FILETIME         SystemTime;
   EVENT_DESCRIPTOR Descriptor;
   GUID             ActivityID;
   GUID             RelatedID;
} *PTR_ETW_EVENT;

typedef struct TR_CALLSTACK_POS
{
   TR_POSITION_HANDLE Position; // Trace position
   TR_ADDRESS         ip;       // Call address
} *PTR_CALLSTACK_POS;

//
// A memory value is up to 8 bytes worth of data.  In the case where not all 8
// bytes of data are validly known by the reader, the mask distinguishes
// known bits from unknown ones.  Bits where the mask is set to zero are unknown.
// Values are set on a byte granularity.
typedef struct TR_MEMORY_DATA
{
   TR_POSITION_HANDLE Position;  // Instruction that either read or wrote the value
   TR_ADDRESS         Eip;       // Instruction that references the memory.
   ULONG64            Mask;      // Valid bits within the returned data.
   union {
      ULONG64 Data;         // Value of memory at an address
      BYTE    DataBytes[sizeof(ULONG64)];
   };
} *PTR_MEMORY_DATA;

//
// Breakpoint are used by the TREADER execute operations to stop
// execution when the specified address or position is accessed
// in the specified way.
//
enum TR_BREAKPOINT_TYPE {
   TR_InvalidBP       = 0,
   TR_ExecutionBP     = 1,    // Break at an instruction.
   TR_MemReadBP       = 2,    // Break at next read of a data address.
   TR_MemWriteBP      = 3,    // Break at the next write a data address.
   TR_MemReadWriteBP  = 4,    // Break at the next read or write of a data address.
   TR_PositionBP      = 5,    // Break at an ITraceReader position.
   TR_ExceptionBP     = 6,    // Break at the next exception (any kind).
   TR_ModuleLoadBP    = 7,    // Break at the next module load.
   TR_CreateThreadBP  = 8,    // Start of execution of a thread in the trace.
   TR_DeleteThreadBP  = 9,    // Thread exited.
   TR_MarkerBP        = 10,   // User trace file mark.
   // Note that these events do not necessarily correspond thread creation and
   // deletion events.  They indicate when tracing started and stopped.
   TR_ProcessStartBP  = 11,   // Start of process trace.
   TR_ProcessEndBP    = 12,   // End of process trace.

   // Using these breakpoints, instead of TR_ExceptionBP, allows for more fine grained
   // control of what exceptions the debugger stops at.
   //
   // These breakpoints act to filter out certain types of exceptions.  When a
   // breakpoint is hit, the returned breakpoint type is always TR_ExceptionBP.
   //
   // You can only have one of these breakpoints, including TR_ExceptionBP, defined
   // at once.  SetEventBreakpoint() will return the breakpoint exists error if you
   // define more than one exception breakpoint type at once.
   TR_HardwareExceptionBP = 13, // Break at the next hardware exception.  These types
                                // of exceptions are usually considered debugger
                                // first-chance exceptions.  TTT will not break on
                                // second-chance exceptions, such as C++ EH software
                                // exceptions.
   TR_Reserved1ExceptionBP = 14,
   TR_Reserved2ExceptionBP = 15,
   TR_Reserved3ExceptionBP = 16,
   TR_Reserved4ExceptionBP = 17,
   TR_Reserved5ExceptionBP = 18,

   TR_EtwEventBP           = 19  // ETW Event
};

typedef struct TR_BREAKPOINT
{
   TR_BREAKPOINT_HANDLE Handle;
   TR_BREAKPOINT_TYPE   Type;
   union
   {
      // TR_PosBP
      TR_POSITION_HANDLE Position;
      // TR_ExecutionBP
      TR_ADDRESS         Eip;
      // TR_MemReadBP, TR_MemWriteBP, TR_MemReadWriteBP

#pragma warning(push)
#pragma warning(disable:4201) // allow nameless struct below for compat
      struct {
         TR_ADDRESS  Address;
         ULONG       Range;  // Number of bytes starting at address to watch.
      };
#pragma warning(pop)
      // TR_CreateThreadBP, TR_DeleteThreadBP
      TR_THREAD_HANDLE Thread;
      // TR_ModuleLoadBP
      TR_MODULE_HANDLE Module;
      // TR_ExceptionBP
      TR_EXCEPTION_RECORD_HANDLE ExceptionRecord;
      // TR_MarkerBP
      TR_MARKER_HANDLE Marker;
      // TR_ProcessStartBP, TR_ProcessEndBP
      TR_PROCESS_HANDLE Process;
      // TR_EtwEventBP
      TR_ETW_EVENT_HANDLE EtwEvent;
   };
} *PTR_BREAKPOINT;

typedef struct TR_VERSION_INFO
{
   ULONG32 major;
   ULONG32 minor;
   ULONG32 build;
   ULONG32 log;
} *PTR_VERSION_INFO;

//
// Description (EVENT CALLBACK):
//  The following callback is defined in conjunction with EnumerateEvents() in
//  the ITREADER class.
//
// Parameters:
//  IReader - the reader being enumerated.
//  type    - the event type.
//  pos     - the position of the event.
//  ContextData - data specific to the type of event.  The life time of the
//                context data is until this call returns.
//              - TR_ExceptionBP:    TR_EXCEPTION_RECORD_HANDLE
//              - TR_ModuleLoadBP:   TR_MODULE_HANDLE
//              - TR_CreateThreadBP: TR_THREAD_HANDLE
//              - TR_DeleteThreadBP: TR_THREAD_HANDLE
//              - TR_MarkerBP:       TR_MARKER_HANDLE
//              - TR_EtwEventBP:     TR_ETW_EVENT_HANDLE
//  ClientData - the user value passed in from EnumerateEvents().
//
// Return Value:
//  The search stops if FALSE is returned or continues if TRUE is returned.
typedef BOOL (CALLBACK *TR_EVENT_CALLBACK)(PITREADER IReader,
                                           const TR_BREAKPOINT_TYPE Type,
                                           const TR_POSITION_HANDLE Position,
                                           const ULONG ContextData,
                                           PVOID ClientData);

//
// Description (DATA CALLBACK):
//  The following callback can be defined in conjunction with the search operations in
//  the ITREADER class.  If the callback returns TRUE the search continues or else
//  the search stops and the data is returned to the caller.  You cannot make any assumption
//  about the state of the arguments after the callback exits.
typedef BOOL (CALLBACK *TR_DATA_CALLBACK)(PITREADER IReader,
                                          const TR_ADDRESS Addr,
                                          const ULONG Count,
                                          const TR_MEMORY_DATA Data[],
                                          PVOID UserContext);

//
// Description (EXECUTION CALLBACKS):
//
// The TR_CONTEXT structure contains the state of execution.  It is read-only, except *ClientData.
// *ClientData is valid across multiple execution operations.
//
typedef struct _TR_CONTEXT
{
    PVOID    *ClientData;        // per thread pointer that can be used by the client
    TR_REGISTER_STATE *CpuRegs;  // pointer to the CPU state (see x86state.h/x64state.h)
                                 //  all registers, except eflags are valid.
    PITREADER IReader;
    PVOID     ContextData;  // Event specific data passed back by the reader (Ex. read data in read event)
} TR_CONTEXT, *PTR_CONTEXT;

// ITREADER supported callbacks.
enum TR_CALLBACK_TYPE {
   TR_InvalidEvent = 0,
   TR_TranslationEvent,
      // callback(context, type, eip, nir) Use GetCodeBytes(eip) to get the code.
   TR_RunInstructionStartEvent,
      // callback(context, type, eip, nir)
   TR_RunSequencingEvent,
      // callback(context, type, [optional] , TR_SEQUENCE_TYPE) ContextData points to the sequence number
   TR_RunMemRefEvent,
      // callback(context, type, addr,  )
   TR_RunMemReadEvent,
   TR_RunMemWriteEvent,
   TR_RunMemImplicitReadEvent,
   TR_RunMemImplicitWriteEvent,
      // callback(context, type, addr, datasize) ContextData points to the value at addr
   TR_RunAllFlowChangeEvent,
      // callback(context, type, target-addr, fallthrough-addr)
   TR_RunCallRetsEvent,
      // callback(context, type, target-addr, fallthrough on call AND null on return)
   TR_DllLoadEvent, // Registers module unload event also.
      // callback(context, type,  ,  ) ContextData points to TR_MODULE.
   TR_ThreadEvent,
      // callback(context, type, , TRUE|FALSE) - TRUE is create, FALSE is terminate
      //                                    ContextData points to TR_THREAD_HANDLE.
   TR_RandomSampleEvent,
      // callback(context, type, sample-run-number, TRUE|FALSE)
      //    sample-run-number = unique ULONG for each sample run, 0 for inter-sample
      //    TRUE|FALSE = TRUE for sample start (thread about to enter),
      //                 FALSE for sample finish (thread just exited)
      // Each thread will generate callbacks as a sample run starts, so only after
      // the last one associated with a sample-run-number have all threads been
      // acquired by Nirvana.  All threads that exit at the end of a sample run
      // will also generate callbacks, so as soon as the first finish event is
      // received Nirvana is no longer in control of all threads.
   TR_DllUnloadEvent, // Registers module load event also.
      // callback(context, type,  ,  ) ContextData points to TR_MODULE
   TR_MarkerEvent, // User inserted string marker
      // callback(context, type,  ,  ) ContextData points to the string.

   // While recording a guest process, TTTracer may log parts of the memory state at
   // certain sequences.  While replaying the trace a client can retrieve that memory
   // by registering for this callback.  The memory returned is the current memory
   // state of the guest process at the point in time the callback is made.  Translation
   // and memory event callbacks may update this memory subsequently.  Use
   // GetImageBytes(sequence, addr, size, ...) to retrieve the memory.
   // If you jump backwards in the trace to any position but the first, the behavior
   // of the callback is undefined.  It assumes you start executing at the start of
   // the trace and move forward.
   TR_MemoryBlockEvent,
      // callback(context, type, addr, size) ContextData points to the sequence number
   TR_ProcessEvent,
      // callback(context, type, , TRUE|FALSE) - TRUE is start tracing, FALSE is stop tracing
      //                                    ContextData points to TR_PROCESS_HANDLE.
};

enum TR_SEQUENCE_TYPE {
   TR_InvalidSequence = 0,

   // User-mode sequence types
   TR_EnterThread,           // we are starting simulation at a new thread state;
                             // the [optional] third parameter is a LONGLONG
                             // representing the performance counter value at this
                             // point during recording.  See QueryPerformanceFrequency
                             // in this interface to get the frequency.
   TR_ExitThread,            // we are exiting simulation
   TR_AtomicOp,              // instruction just executed an atomic operation
   TR_RegStateChanged,       // OBSOLETE: non-deterministic register change
   TR_InvalidInstr,          // invalid (or unrecognized) instruction (exits simulation)
   TR_TrapToNative,          // we are about to trap (exits simulation)
   TR_CodeCacheFlush,        // code cache flushed
   TR_EndOfThread,           // Reached the last instruction in the thread in the trace
   TR_Exception,             // Hit an exception.  The [optional] third parameter is the
                             // handle for the exception record.
   TR_Abort,                 // Tracing aborted
   TR_SequenceStart,         // About to execute a new sequence of instructions
   TR_DebugBreak,            // Hit a debug breakpoint (int 3). (exits simulation)
   TR_OpaqueData,            // system-dependent data (CPUID/RDTSC/SGDT/...)
   TR_SequenceEnd,           // Finished executing a sequence of instructions
   TR_LastUserModeSequenceType,

   // Kernel-mode sequence types
   KTR_SequenceStart = TR_SequenceStart,
   KTR_SequenceEnd = TR_SequenceEnd,
   // 1. events that indicate beginning simulation at a new state
   KTR_Interrupt = 0x41,     // interrupt
   KTR_Syscall = 0x42,       // syscall
   // 2. events that occur during simulation
   KTR_AtomicOp = TR_AtomicOp,
   KTR_OpaqueData = TR_OpaqueData,
   KTR_CodeCacheFlush = TR_CodeCacheFlush,
   // 3. events that indicate ending simulation
   KTR_InvalidInstr = TR_InvalidInstr,
   KTR_SwitchToUserMode = 0x61,
   KTR_SwitchToNative = 0x62,// due to change of mode (e.g., into real mode)
   KTR_Halt = 0x63           // Hlt instruction
};

inline bool TrIsUserModeSequenceType(TR_SEQUENCE_TYPE seq)
{
   return (seq > TR_InvalidSequence) && (seq < TR_LastUserModeSequenceType);
}

inline bool TrIsKernelModeSequenceTypeBeginSimulation(TR_SEQUENCE_TYPE seq)
{
   return (seq == KTR_Interrupt) || (seq == KTR_Syscall);
}

inline bool TrIsKernelModeSequenceTypeContinueSimulation(TR_SEQUENCE_TYPE seq)
{
   return (seq == KTR_AtomicOp) ||
          (seq == KTR_OpaqueData) ||
          (seq == KTR_CodeCacheFlush);
}

inline bool TrIsKernelModeSequenceTypeEndSimulation(TR_SEQUENCE_TYPE seq)
{
   return (seq == KTR_InvalidInstr) ||
          (seq == KTR_SwitchToUserMode) ||
          (seq == KTR_SwitchToNative) ||
          (seq == KTR_Halt);
}

inline bool TrIsKernelModeSequenceType(TR_SEQUENCE_TYPE seq)
{
   return (seq == KTR_SequenceStart) ||
          (seq == KTR_SequenceEnd) ||
          TrIsKernelModeSequenceTypeBeginSimulation(seq) ||
          TrIsKernelModeSequenceTypeContinueSimulation(seq) ||
          TrIsKernelModeSequenceTypeEndSimulation(seq);
}

typedef void (__fastcall *TR_EXECUTION_CALLBACK)(struct _TR_CONTEXT const *, TR_CALLBACK_TYPE ,void *,void *);

// Information about how the trace was captured.
enum TR_TRACE_CAPTURE_FLAGS
{
   TR_CaptureRingBuffer        = 0x1, // Full trace if not set.

   // If neither of the next two flags are set, then how tracing
   // attached to the process is unknown.  This is possible with
   // older trace files where this information is not available.
   TR_CaptureAttachRunning     = 0x2, // Tracing started on an already running process.
   TR_CaptureAttachStartup     = 0x4, // Tracing started right when the guest process
                                      // was launched.

   TR_CaptureTraceChildren     = 0x8, // Children of this process are traced too.

   // When you trace managed code a full dump is captured at trace time even if the
   // -DumpFull option isn't used.  If the option is used then the full dump occurs
   // before the first instruction is traced.  If the full dump is implicit, then the
   // dump occurs later in the trace so that memory won't be available until
   // after the CLR (mscorwks) is loaded.  If the full dump is implicit,
   // TR_CaptureDumpModules is not set, while if it is explicit it is set.
   TR_CaptureDumpModules       = 0x10, // Entire binaries are included in the trace.
   TR_CaptureDumpFull          = 0x20, // The snapshot of the process is included in
                                       // this trace (this also implies Peb/Teb and
                                       // module data).
   TR_CapturePebTeb            = 0x40, // Peb and Teb data is in the trace.

   // This trace was captured using group sequencing.  This means that the trace has a
   // group identifier and multiple traces with the same group ID were recorded in a
   // synchronized fashion  (i.e. Their sequence events are interleaved as if the traces'
   // threads were running in a single process).
   TR_CaptureGrouped           = 0x80
};

/* de137e92-952c-4cf7-8836-d9b473c6cafe */
DEFINE_GUID(IID_ITREADER_GROUP,
            0xde137e92,
            0x952c,
            0x4cf7,
            0x88, 0x36, 0xd9, 0xb4, 0x73, 0xc6, 0xca, 0xfe);

typedef interface DECLSPEC_UUID("de137e92-952c-4cf7-8836-d9b473c6cafe") ITREADER_GROUP* PITREADER_GROUP;

#endif // ITRACEREADERSHARED
