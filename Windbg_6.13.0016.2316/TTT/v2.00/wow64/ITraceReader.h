//-----------------------------------------------------------------------------
//
// Time Travel Tracing (TTT) Reader Interface
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Module name:
//
//    ITraceReader.h
//
// Description:
//
// The TTT TREADER is the interface between the stored execution of an
// application (the trace) and the clients seeking information from the stored
// trace.  A client can open as many readers as it wants within one process.
//
// The reader API's do not support concurrent operation (i.e. two threads
// simultaneously calling on the same or two different API's).
//
//-----------------------------------------------------------------------------

#ifndef ITRACEREADER
#define ITRACEREADER

#pragma once

#include <ole2.h>

#include <windows.h>
#include "evntprov.h"

// allow unnamed structs and unions
#pragma warning(push)
#pragma warning (disable:4201)

#if !defined(_IMAGEHLP_) && !defined(_DBGHELP_)
#define DBGHELP_TRANSLATE_TCHAR
   #include <dbghelp.h>
#endif

#include "x86state.h"
#include "x64state.h"
#include "registers_a32.h"
#include "registers_x86.h"
#include "registers_x64.h"
#include "metrics.h"

#include "ITraceReaderShared.h"

//
// Constants
//

// TODO: Issue callbacks for each load/st exclusive instruction so we can lower this to something reasonable
#define TR_MAX_INSTRUCTION_LEN      TR_MAX_INSTRUCTION_LEN_V2

union TR_REGISTER_STATE
{
   Nirvana::EmulatorRegisters State; // Unified V2 state
   Nirvana::X86REGS X86State;
   Nirvana::X64REGS X64State;
   Nirvana::KX86REGS KX86State;
   Nirvana::KX64REGS KX64State;
};

#pragma warning(pop)

//
// The ITREADER interface
//

// The following API creates an ITREADER interface.
// In addition to the defined API return values, all the API's return TR_ERROR_INVALID_READER if
// an internal error in the reader has occurred.  In this case the reader is in an irrecoverable
// state.  You can create multiple readers in a single process or add multiple processes to a
// single reader, with the restrictions that if there is more than one
// reader or process you cannot commit a memory index to any of the open trace files and
// you must create all your readers and attach to all your trace files before calling on any
// other API's in any of those readers.
// See CreateMemoryIndex() AttachTraceFile() for more details.
extern "C" ITREADER* __cdecl CreateITReader();


// Function: CreateDbgCmdTree
// Description:
// Call to generate .cmdtree file for the given ITReader instance.
// Parameters:
// [in] TracePath: the trace file path cmd file will be based on.
// [in,out] CmdFilePath: the full path of Windbg Command File.
// [out]    PathLen: the buffer length of cmdFileName.
// [in]     ForceOverwrite: True for always create command tree file
//                          overwriting the exisiting file.
extern "C" HRESULT   __cdecl CreateDbgCmdTree(
   __in  ITREADER*  pTraceReader,
   __out WCHAR* CmdTreePath,
   __out size_t PathLen,
   __in  bool   ForceOverwrite);


#undef INTERFACE
#define INTERFACE ITREADER
DECLARE_INTERFACE_(ITREADER, IUnknown)
{
   // IUnknown.
   STDMETHOD(QueryInterface)(
       __in REFIID InterfaceId,
       __out PVOID* Interface
       ) PURE;

   STDMETHOD_(ULONG, AddRef)() PURE;
   STDMETHOD_(ULONG, Release)() PURE;

   //
   // Description:
   //  Attach a trace file to the reader.  If this is successful, the
   //  position is the first instruction in the trace.  You can attach
   //  multiple trace files to a reader, where each trace file represents
   //  a single traced process.  You must attach all of your trace files
   //  before calling on any other API's in this interface.
   //
   //  NOTE: In the multi-reader scenario, you cannot call this after you
   //  call any other API's in this interface in any of the readers
   //  (i.e. you must attach to all your trace files up front).
   //
   // Parameters:
   //  TraceFile - NULL terminated file name of trace of a program's execution.
   //
   // Return Value:
   //  TR_ERROR_SUCCESS - no problem.
   //  TR_ERRCODE_BAD_FILE_FORMAT - file not a valid application trace.
   //  TR_ERRCODE_FILE_NOT_FOUND - file not found.
   //  TR_ERROR_TRACEFILE_READER_VERSION_MISMATCH - reader and log version do not match.
   //  TR_ERROR_OUTOFMEMORY - client could not be created because of memory issue.
   //  TR_ERROR_TOO_MANY_READERS - Attempt to attach a trace file to a reader after
   //  a reader in the same process has already started execution.
   //  TR_ERROR_SEQUENCE_OVERFLOW - The trace file has too many sequences to process.  The
   //  maximum number of sequences when attaching multiple trace files is less than when
   //  attaching one file, so if this is the second trace file you are attaching the problem
   //  could be with the first one.
   STDMETHOD(AttachTraceFile)(
       __in const PWCHAR TraceFile
       ) PURE;

   //
   // POSITIONING / THREAD STATE
   //   A position is the location of an instruction in the trace.  There is one unique
   //   position per instruction.
   //

   //
   // Description:
   //  The following operation compares how two positions relate in terms of execution time
   //  to each other in a trace.  It is legal to compare positions on different threads or
   //  processes, but comparing positions from two different readers or an invalid position
   //  results in an exception.
   //  To determine if two positions are the same instruction, the position thread and
   //  instruction count must be equal.
   //
   // Parameters:
   //  pos1    - position that is being compared.
   //  pos2    - position that is being compared.
   //
   // Return Value:
   //  On a single thread the results of these operations are simply based on the relative
   //  order of the instructions these position represent.  When comparing positions across
   //  two threads, it may only be possible to know the relative order of a range of instructions
   //  in each thread.  If the range of instructions the two positions are in fall into the same
   //  sequence, those two positions are considered equal.
   //  If pos1 > pos2 it returns > 0
   //  If pos1 < pos2 it returns < 0
   //  If pos1 = pos2 it returns 0
   //
   // If the positions are invalid this throws and exception.
   STDMETHOD_(LONG, ComparePositions)(
      __in const TR_POSITION_HANDLE pos1,
      __in const TR_POSITION_HANDLE pos2) const PURE;

   // Description:
   //  Each position is tied to a particular thread.
   //
   // Return Values:
   //  This either returns TR_ERROR_SUCCESS or TR_ERROR_INVALID_POSITION.
   STDMETHOD(GetThread)(
      __in const TR_POSITION_HANDLE pos,
      __out TR_THREAD_HANDLE & thread) const PURE;

   // Description
   //  This returns a copy of the code bytes at the passed in IP valid at the passed in position
   //  in the reader.  Because of delay loaded modules and self-modifying code, code bytes
   //  may be different at the same IP at different places in the trace.  Any bytes in the range
   //  that are unknown are filled in with the TR_DEFAULT_CODEBYTE opcode.
   //
   // Parameters:
   //  pos - The position in the trace.
   //  ip  - The instruction pointer address.
   //  length - Length of the code bytes buffer.
   //  codeBytes - A copy of the code bytes.
   // Return Value:
   //  Returns TR_ERROR_SUCCESS or it
   //  returns TR_ERROR_INVALID_POSITION if the passed in position is not valid.
   STDMETHOD(GetCodeBytes)(
      __in const TR_POSITION_HANDLE pos,
      __in const TR_ADDRESS         ip,
      __in const ULONG              length,
      __out_ecount(length) BYTE     codeBytes[]) const PURE;

   //
   // Description:
   //  Returns the current register state of the passed in thread.  Event callbacks
   //  (see RegisterEventCallback) should use the register state in the context
   //  whenever possible, since this operation is less efficient.  The only reason
   //  to use this operation within a callback is to get the valid eflags.
   //
   // Parameters:
   //  thread - Get current thread register values.
   //  Regs - The register values.
   //
   // Return Value:
   //  This returns TR_ERROR_SUCCESS or TR_ERROR_FAILURE if called during
   //  execution.
   STDMETHOD(GetRegisters)(
      __in const TR_THREAD_HANDLE thread,
      __out TR_REGISTER_STATE & Regs) PURE;

   //
   // GLOBAL DATA
   //

   // Description:
   //  The following two operation convert reader handles to their appropriate
   //  types.
   //
   // Parameters:
   //  <handle> - The handle.
   //  <type>   - The converted type.
   //
   // Return Values:
   //  If the handle is valid it returns the TR_ERROR_SUCCESS or it returns
   //  TR_ERROR_FAILURE.
   STDMETHOD(ConvertHandleToModule)(
      __in const TR_MODULE_HANDLE Handle,
      __out TR_MODULE & Module
   ) PURE;

   STDMETHOD(ConvertHandleToExceptionRecord)(
      __in const TR_EXCEPTION_RECORD_HANDLE Handle,
      __out EXCEPTION_RECORD64 & ExceptionRecord
   ) PURE;

   //
   // Description:
   //  Returns the first and last sequence number for those
   //  sequences of instructions in the trace(s).
   //
   // Parameters:
   //  FirstSeq  - The first sequence of instructions in the trace.
   //  LastSeq   - The last sequence of instructions in the trace.
   //
   STDMETHOD_(VOID, GetTraceBoundarySequences)(
      __out TR_SEQUENCE & FirstSeq,
      __out TR_SEQUENCE & LastSeq) const PURE;

   // Description:
   //  This operation returns a thread handle for each thread of execution
   //  in the trace associated with the current process.
   //
   // Parameters:
   //  ThreadsLen - The length of Threads[].
   //  Threads - The thread handles; there is one unique thread handle per thread in
   //            the trace.  If this is NULL and ThreadLen is 0, it just
   //            returns the ThreadCount.
   //  ThreadsCount - The number of unique threads in the trace.
   //
   // Return Values:
   //  If the passed in thread count is large enough it returns TR_ERROR_SUCCESS,
   //  the threads, and the actual thread count.  If Threads[] is not large enough
   //  it returns TR_ERROR_MORE_DATA and returns as many thread handles as will fit.
   //  The returned ThreadCount is the length needed in Threads[].
   STDMETHOD(GetThreads)(
      __in const ULONG  ThreadsLen,
      __out_ecount_opt(ThreadsLen) TR_THREAD_HANDLE Threads[],
      __out ULONG      &ThreadsCount
   ) PURE;

   // Description:
   //  This operation returns all the modules loaded into the trace at the time of
   //  the passed in position.  If there are overlapping modules it returns the
   //  module loaded closest to the current position.
   //
   // Parameters:
   //  pos - A reader position.
   //  Length - The length of the passed in module array.
   //  Modules - The module array.
   //  Count - The actual number of modules.  This can be greater than Length if
   //          TR_ERROR_MORE_DATA is returned.
   //
   // Return Values:
   //  If the passed in module count is large enough it returns TR_ERROR_SUCCESS,
   //  the modules, and the actual module count.  If Modules[] is not large enough
   //  it returns TR_ERROR_MORE_DATA and returns as many modules as will fit, and
   //  it returns the size of the array that is needed in Count.  The modules are
   //  returned in ascending address range order.
   STDMETHOD(GetModules)(
      __in const TR_POSITION_HANDLE pos,
      __in const ULONG         Length,
      __out_ecount_opt(Length) TR_MODULE    Modules[],
      __out ULONG          &Count
   ) PURE;

   //
   // Description:
   //  Returns the name of the trace file associated with the currently executing
   //  process.
   //
   // Parameters:
   //
   // Return Values:
   //  Returns the name of the trace file this reader is associated with.
   STDMETHOD(GetTraceFileName)(__out WCHAR file[MAX_PATH]) const PURE;

   //
   // Description:
   //  Retrieves information about the trace file, process, and system
   //  the trace was taken on.  This returns the system information for
   //  the currently executing process.
   //
   // Parameters:
   //  sysinfo - holds all the information.
   STDMETHOD(GetTraceSystemInfo)(
      __out TR_SYSTEM_INFO & sysinfo) const PURE;

   //
   // Description:
   //  Retrieves the unique identifier for the trace file of the currently
   //  executing process.
   //
   // Parameters:
   //  id - the unique guid
   STDMETHOD(GetTraceIdentifier)(
      __out GUID & id) const PURE;

   //
   // THREAD DATA
   //

   // Description:
   //  Returns the threads system id.
   //
   // Parameters:
   //  Thread - the thread.
   //  id     - the thread's system ID.
   STDMETHOD(GetThreadId)(
      __in const TR_THREAD_HANDLE thread,
      __out DWORD & id) const PURE;

   // Description:
   //  Returns the position of the first instruction to execute in the
   //  passed in thread.
   //
   // Parameters:
   //  Thread - the thread.
   //  Pos    - the first position to execute in the thread.
   STDMETHOD(GetThreadStartPosition)(
      __in const TR_THREAD_HANDLE Thread,
      __out TR_POSITION_HANDLE & Pos) const PURE;

   //
   // NAVIGATION
   //

   //
   // Description:
   //  Returns the current position of the reader in the trace.  The current position
   //  after the reader is created is the first instruction in the trace to execute.
   //  The current position is updated when the client executes (see EXECUTION below) forwards
   //  or backwards.
   //
   // Parameters:
   //  Pos    - Returned position.
   //
   // Return Value:
   //  This return TR_ERROR_FAILURE or TR_ERROR_SUCCESS.
   STDMETHOD(GetCurrentPosition)(
      __out TR_POSITION_HANDLE & Pos) const PURE;

   //
   // Description:
   //  Returns the current position of the reader in the trace on a
   //  thread.  The current position of each thread after the reader
   //  is created is the first instruction of each thread.  The current position
   //  is updated when the client executes (see EXECUTION below) forwards or
   //  backwards.
   //
   // Parameters:
   //  thread - Position in this thread.
   //  Pos    - Returned position.
   //
   // Return Value:
   //  This return TR_ERROR_FAILURE or TR_ERROR_SUCCESS.
   STDMETHOD(GetCurrentPosition)(
      __in  const TR_THREAD_HANDLE thread,
      __out TR_POSITION_HANDLE & Pos) const PURE;

   //
   // Description:
   //  Returns the instruction sequence the passed in position is in.
   //
   // Parameters:
   //  Pos    - A trace file position.
   //  Seq    - The sequence that position is in.
   //
   // Return Value:
   //  This return TR_ERROR_FAILURE or TR_ERROR_SUCCESS.
   STDMETHOD(GetPositionSequence)(
      __in const TR_POSITION_HANDLE Pos,
      __out TR_SEQUENCE & Seq) const PURE;

   //
   // EXECUTION (REPLAY)
   //

   //
   // Description:
   //  The execute operations simulate execution from the current position in the trace until
   //  a breakpoint is hit or the trace ends.  If a breakpoint is hit, that breakpoint
   //  is returned and the current position in the trace is the next instruction to execute.
   //
   //  Instructions are executed in the order of their sequences.  For example, all the
   //  instructions in sequence one are executed, then all the instructions in sequence
   //  two are executed, then sequence three, etc...  This is not necessarily the same
   //  order the instructions are executed at recording time, however, if two adjoining
   //  sequences overlap.  The search operations, as opposed to breakpoints, should be used
   //  determine memory access ording, since the search operations indicate whether or
   //  not multiple access happened in overlapping sequences.
   //
   //  The reader maintains the current state of each thread.  When execution ends the state
   //  (or position) is the next instruction to execute on each thread.
   //
   //  When the step argument is used, the reader steps forward the specified number of
   //  instructions in the current thread.  If it hits a sequence of instructions in
   //  another thread, it executes through those instruction until it gets back to the
   //  current thread.  If it hits a breakpoint on the current or a different thread
   //  before it finishes executing the specified number of instructions, it stops
   //  at the breakpoint.  It will only stop at breakpoints in the process owning the
   //  current thread.
   //
   //  The ExecuteForward operation supports registering Nirvana style callbacks.  When a
   //  callback generating event is hit, all the registered callbacks for that event are
   //  called.
   //
   // Parameters:
   //  step        - Number of instructions to execute.  If this is zero it executes to the
   //                next breakpoint.
   //  bphit       - If non-NULL this returns a pointer to the breakpoint from the
   //                breakpoint array that is hit.
   //
   // Return Value:
   //  TR_ERROR_SUCCESS if it executes the number of steps given.
   //  TR_ERROR_BREAKPOINT_HIT if execution stops at a breakpoint.
   //  TR_ERROR_ENDOFTRACE if it executes to the end/start of the trace.
   //  TR_ERROR_INVALID_POSITION if the step goes past the first instruction of the trace (backwards exec).
   STDMETHOD(ExecuteForward)(
      __in  const ULONGLONG        step,
      __out TR_BREAKPOINT        & bphit) PURE;

   // Backwards execution does not support replay callbacks.
   STDMETHOD(ExecuteBackwards)(
      __in  const ULONGLONG        step,
      __out TR_BREAKPOINT        & bphit) PURE;

   // Description:
   //  Sets the execution state of the reader to the passed in position.  No callbacks
   //  are called or breakpoints hit when this operation is used.
   //  If the position passed in is 0 to 100 it treats the position as a percent through
   //  the trace.  For example if you pass in 50, it jumps to a position approximately
   //  50% through the trace.  If you pass in 0 it jumps to the first position in the
   //  trace.
   //
   // Parameters:
   //  position - position to jump to or the percent through the trace.
   //
   // Return Values:
   //  TR_ERROR_SUCCESS if it jumps to the position.
   //  TR_ERROR_INVALID_POSITION if the position not a valid position in this trace.
   STDMETHOD(JumpToPosition)(
      __in const TR_POSITION_HANDLE position) PURE;

   // Description:
   //  Registers a Nirvana style event callback.  These events only occur during
   //  forward execution.  To change a callback simply call this function
   //  again with a different callback operation, and to remove a callback, pass NULL
   //  in as the function pointer.
   //
   //  The reader client can stop forward execution by calling on StopExecution() from any
   //  of these callbacks.  All of the supported events, except for TranslateEvent, occur
   //  at the same point in execution as they occurred during recording.  The translation
   //  event happens when an instruction is JIT'd to the Nirvana code cache at replay time.
   //
   //  The event callback is registered for all processes contained in the reader.
   //
   // NOTE: Replay clients cannot dereference address and IP pointers and expect to get to
   //       anything meaningful.
   //
   // Parameters:
   //  e - The callback event to be registered.
   //  callback - TTT client's callback
   //
   // Return Value:
   //  Returns TR_ERROR_SUCCESS if the callback is registered or removed.
   //  Returns TR_ERROR_FAILURE if the event is unsupported.
   STDMETHOD(RegisterEventCallback)(
      __in TR_CALLBACK_TYPE e,
      __in TR_EXECUTION_CALLBACK callback) PURE;

   // Description:
   //  The following operation sets a breakpoint that is used to stop both
    //  forward and backwards execution at the specified position.
   //
   // Parameters:
   //  pos - position in the trace(s).
   //  bp - breakpoint handle (only set if a new breakpoint is created).
   //
   // Return Value:
   //  Return either TR_ERROR_SUCCESS or TR_ERROR_INVALID_POSITION.  Returns
   //  TR_ERROR_BREAKPOINT_EXISTS if the breakpoint already exists.
   //  The current instruction when the breakpoint is hit (next instruction to execute)
   //  is the instruction at the position in the breakpoint.
   STDMETHOD(SetPositionBreakPoint)(
      __in const TR_POSITION_HANDLE pos,
      __out TR_BREAKPOINT_HANDLE & bp
      ) PURE;

   // Description:
   //  The following operation sets a breakpoint that is used to stop both
   //  forward and backwards execution.  The breakpoint is set in the current
   //  process.
   //
   // Parameters:
   //  eip - instruction in the trace.
   //  bp - breakpoint handle (only set if a new breakpoint is created).
   //
   // Return Value:
   //  Returns TR_ERROR_SUCCESS or TR_ERROR_BREAKPOINT_EXISTS
   //  if the breakpoint already exists.  The current instruction
   //  when the breakpoint is hit (next instruction to execute) is the
   //  instruction at the IP in the breakpoint.
   STDMETHOD(SetExecutionBreakPoint)(
      __in const TR_ADDRESS eip,
      __out TR_BREAKPOINT_HANDLE & bp
      ) PURE;

   // Description:
   //  The following operation sets a breakpoint that is used to stop both
   //  forward and backwards execution.  The breakpoint is set in the current
   //  process.
   //
   // Parameters:
   //  type - read/write/readwrite BP
   //  addr - data address referenced in the trace
   //  range - the reader will break on any address from [address,address+range)
   //  bp - breakpoint handle (only set if a new breakpoint is created).
   //
   // Return Value:
   //  This operation returns TR_ERROR_SUCCESS if it succeeds, TR_ERROR_FAILURE, or
   //  TR_ERROR_BREAKPOINT_EXISTS if exact watchpoint already exists.  The
   //  current instruction when the breakpoint is hit is the instruction following the
   //  instruction that referenced the watched memory.
   STDMETHOD(SetMemoryBreakPoint)(
      __in const TR_BREAKPOINT_TYPE  type,
      __in const TR_ADDRESS          addr,
      __in const ULONG               range,
      __out TR_BREAKPOINT_HANDLE & bp
      ) PURE;

   // Description:
   //  The following operation sets the reader to break on certain
   //  events that occur during execution.  The breakpoint is set
   //  in the current process.
   //
   // Parameters:
   //  type - the supported types are TR_ModuleLoad, TR_Exception (and filters),
   //         TR_Marker, TR_CreateThread, and TR_DeleteThread.
   //  bp - breakpoint handle (only set if a new breakpoint is created).
   //
   // Return Value:
   //  Returns TR_ERROR_SUCCESS or TR_ERROR_BREAKPOINT_EXISTS
   //  if the breakpoint already exists.  It returns TR_ERROR_FAILURE
   //  if an unsupported type is given.
   STDMETHOD(SetEventBreakPoint)(
      __in const TR_BREAKPOINT_TYPE type,
      __out TR_BREAKPOINT_HANDLE & bp
      ) PURE;

   // Description:
   // Query the Memory Breakpoint with the specified type from the current
   // process.
   //
   // Parameters:
   //  type - read/write/readwrite BP
   //  addr - data address referenced in the trace
   //  range - the reader will break on any address from [address,address+range)
   //  bp    - validate breakpoint handle if successful, otherwise NULL.
   //
   // Return:
   // TR_ERROR_SUCCESS if successful.
   STDMETHOD(GetMemoryBreakPoint)(
      __in const TR_BREAKPOINT_TYPE type,
      __in const TR_ADDRESS         addr,
      __in const ULONG              range,
      __out TR_BREAKPOINT_HANDLE &bp
      ) PURE;


   // Description:
   // Query the Execution Breakpoint with the specified type from the current
   // process.
   //
   // Parameter:
   //  eip - instruction in the trace.
   //  bp    - validate breakpoint handle if successful, otherwise NULL.
   //
   // Return:
   // TR_ERROR_SUCCESS if successful.
   STDMETHOD(GetExecutionBreakPoint)(
       __in const TR_ADDRESS eip,
       __out TR_BREAKPOINT_HANDLE &bp
      ) PURE;

   // Description:
   // Query the Position Breakpoint with the specified type.
   //
   // Parameter:
   //  pos - position in the trace.
   //  bp    - validate breakpoint handle if successful, otherwise NULL.
   //
   // Return:
   // TR_ERROR_SUCCESS if successful.
   STDMETHOD(GetPositionBreakPoint)(
       __in const TR_POSITION_HANDLE pos,
       __out TR_BREAKPOINT_HANDLE &bp
      ) PURE;

   // Description:
   // Query the Event Breakpoint with the specified type from the
   // current process.
   //
   // Parameter:
   //  type - event type
   //  bpHandle - returned handle
   //
   // Return:
   // TR_ERROR_SUCCESS if successful.
   STDMETHOD(GetEventBreakPoint)(
      __in const TR_BREAKPOINT_TYPE type,
      __out TR_BREAKPOINT_HANDLE &bpHandle
      ) PURE;

   // Description:
   //  Clears the breakpoint represented by the handle from the process
   //  it exists in.
   //
   // Parameters:
   //  bp - breakpoint handle.
   //
   // Return Value:
   //  Return TR_ERROR_SUCCESS or TR_ERROR_FAILURE if the breakpoint doesn't exist.
   //  After this returns the handle is no longer valid.
   STDMETHOD(ClearBreakPoint)(
      __in const TR_BREAKPOINT_HANDLE bp
      ) PURE;

   // Description:
   //  Clears any active breakpoints from all processes.
   STDMETHOD_(void, ClearAllBreakpoints)() PURE;

   //
   // SEARCH OPERATIONS
   //

   // NOTE: Memory search operations cannot be called from within a registered
   //       callback.  They all return memory values from the current process.

   //
   // Description:
   //  Get the memory data value(s) at the passed in address in the passed in range from
   //  the current position.  Each data value is from 1 to 8 bytes, where the valid parts
   //  of that value are indicated in the TR_MEMORY_DATA structure.  In a
   //  multi-threaded application it may not be possible to know what the one
   //  dereferenced value of an address is, since the true ordering of the instructions
   //  during execution is only partially recorded in the trace.  If this is the case,
   //  the API returns mutliple possible values.
   //
   //  This operation works by searching for accesses to the range back from the
   //  current position.  It is possible that the first access found by the search
   //  will yield only a partial result.  In this case this operation continues
   //  to resolve the unknown bytes in the range by searching back from the initial
   //  access.  The position returned to the caller is the position of the closest
   //  access to any byte in the range, however.  If the search cannot resolve the
   //  full range of bytes, it indicates which bytes are unknown using the mask in
   //  the return data.
   //
   //  There are some cases where this operation can get the memory value, but it cannot
   //  get the location of that value.  In this case it sets the position of the value
   //  to 0.  This happens when the address range is within the code space or the reader
   //  cannot efficiently look up where the value come from.
   //
   //  NOTE: The client of the reader cannot assume that the trace holds a
   //  snapshot of all of memory at every instruction.  Because of this, the
   //  dereferenced values at some address's are unknown.  Typically the known
   //  values are values that are read or written by the traced process.  Also, if
   //  a secondary process modifies memory in the traced process, the only value
   //  that is guaranteed to be correct is the value read by the current
   //  instruction.
   //
   //  NOTE: The reader only searches back a limited number of instructions when getting
   //  a memory value.  In order to get the value of any memory address accessed before
   //  the current location, the user must create index of the trace file.  See
   //  CreateMemoryIndex() for more details.
   //
   // Parameters:
   //  Address   - Address to start the search from
   //  Range     - Range to search (1 to 8 bytes).
   //  DataCmpProc - Optional (can be NULL) comparison procedure to enable the client
   //                to reject a found value, so that the search can continue
   //                to the next one.  You can set these operations up as iterators by
   //                always returning TRUE.  After the callback returns, the data passed
   //                into that callback is undefined, so save it if you want to keep it.
   //                CALLBACKS ARE CURRENTLY NOT IMPLEMENTED
   //  DataCount - Length of passed in Data array.
   //  Data      - Array of data values.  This can be NULL if the caller just wants
   //              the DataCount.
   //  DataCount - Number of Data items.  This can be greater than DataLen (see Return Values).
   //
   // Return Value:
   //  TR_ERROR_SUCCESS if it succeeds.  DataCount contains the number of returned
   //  data items.
   //  TR_ERROR_MORE_DATA if the Data array is not large enough to hold all the potential
   //  returned values.  In this case DataCount holds the length of the Data array that
   //  it needs to hold all the values, and the Data[] array holds as much as it can.  This
   //  is a potentially expensive operation, so sizing the Data[] array correctly the first
   //  time is recommended.
   //  TR_ERROR_ADDR_NOT_FOUND if the value of this address plus range is unknown by
   //  the reader.
   //  TR_ERROR_FAILURE if the range is not from 1 to 8 bytes.
   STDMETHOD(GetMemoryValue)(
      __in const TR_ADDRESS         Address,
      __in const ULONG              Range,
      __in_opt TR_DATA_CALLBACK     DataCmpProc,
      __in const ULONG              DataLen,
      __out_ecount(DataLen) TR_MEMORY_DATA   Data[],
      __out ULONG                  &DataCount
      ) PURE;

   //
   // Description:
   //  The next four find operations are similar to GetMemoryValue(), except for
   //  that they search for values accessed in a more specific way.  They also
   //  return the value at the first position that hits within the specified
   //  address and range, which may only contain part of the value in the data
   //  range specified in the call.  Use the mask to know which bytes within
   //  the range are valid.

   // Address writes.
   STDMETHOD(FindPrevWrite)(
      __in const TR_ADDRESS        Address,
      __in const ULONG             Range,
      __in_opt TR_DATA_CALLBACK    DataCmpProc,
      __in const ULONG             DataLen,
      __out_ecount(DataLen) TR_MEMORY_DATA   Data[],
      __out ULONG                 &DataCount
   ) PURE;
   STDMETHOD(FindNextWrite)(
      __in const TR_ADDRESS        Address,
      __in const ULONG             Range,
      __in_opt TR_DATA_CALLBACK    DataCmpProc,
      __in const ULONG             DataLen,
      __out_ecount(DataLen) TR_MEMORY_DATA   Data[],
      __out ULONG                 &DataCount
   ) PURE;

   // Address reads
   STDMETHOD(FindPrevRead)(
      __in const TR_ADDRESS        Address,
      __in const ULONG             Range,
      __in_opt TR_DATA_CALLBACK DataCmpProc,
      __in const ULONG             DataLen,
      __out_ecount(DataLen) TR_MEMORY_DATA   Data[],
      __out ULONG              &DataCount
   ) PURE;
   STDMETHOD(FindNextRead)(
      __in const TR_ADDRESS        Address,
      __in const ULONG             Range,
      __in_opt TR_DATA_CALLBACK    DataCmpProc,
      __in const ULONG             DataLen,
      __out_ecount(DataLen) TR_MEMORY_DATA   Data[],
      __out ULONG                 &DataCount
   ) PURE;

   //
   // Description:
   //  The following operation starts up an event enumerator.  The events
   //  that are enumerated are the same set of events that the client
   //  can set an event breakpoint for (see SetEventBreakPoint()).  The events
   //  for all processes are enumerated.
   //
   // Parameters:
   //  EventCallback - The event callback.
   //  UserContext   - Value passed into the callback.
   //
   // Return Value:
   //  The search stops if FALSE is returned or continues if TRUE is returned.
   STDMETHOD(EnumerateEvents)(
      __in TR_EVENT_CALLBACK EventCallback,
      __in PVOID UserContext
   );

   //
   // Description:
   //  Inserts a positional breakpoint at the next instruction to execute, which
   //  effectively stops forward execution at that instruction.  This is called from an
   //  event callback to halt execution.  After this is called the current instruction
   //  is the next instruction to execute.
   //
   // Parameters:
   //
   // Return Value:
   //  TR_ERROR_SUCCESS if it succeeds.
   STDMETHOD(StopExecution)() PURE;

   //
   // INDEXING
   //

   //
   // Description:
   //  To ensure the memory search operations are responsive, the reader only looks back a
   //  limited number of instruction for memory values.  The following operation provides
   //  reader clients with a way to efficiently get all memory values accessed within the
   //  trace file.
   //
   //  This operation crawls through the trace, creating memory snapshots at various points
   //  in time.  After the index is created, the reader uses it to return memory values to
   //  the client that are not within the limited range of the search operations.
   //  TR_MEMORY_DATA positions from values resolved from the index are always 0, and only
   //  a single value is returned based on the sequence execution ordering.
   //
   //  If "commit" is specified, the index is written back into the trace log file, and the
   //  next reader session will use the stored index.
   //
   //  This operation creates the index in the current executing process.
   //
   // Parameters:
   //  commit  - After creating the index, store it away for future reader sessions on
   //            current trace file.  This functionality is disabled if there is more
   //            than one active reader in a single process.
   //  Wait    - If true this operation blocks until the index is created.  If it is
   //            false the indexing is run in a background thread.
   //
   //  LowPri  - If this is true and the indexing is being run in the background,
   //            the thread will run at below normal priority.
   //
   // Return Value:
   //  TR_ERROR_SUCCESS if it finished creating the index.
   //  TR_ERROR_PENDING if it returns before indexing is complete (indexing continues in
   //                   the background).
   //  TR_ERROR_COMMIT_FAILED if it built the index but failed to commit it for another session.
   //  TR_ERROR_FAILURE if it fails to start index creation.
   //  TR_ERROR_OUTOFMEMORY if it fails during a memory allocation.
   STDMETHOD(CreateMemoryIndex)(
      __in const bool Commit,
      __in const bool Wait,
      __in const bool LowPri
   ) PURE;

   //
   // Description:
   //  This operation gets the result of a pending background index.  The user can optionally
   //  specify a period of time to wait for the reader to finish creating the index.
   //
   // Parameters:
   //  Milliseconds - The number of milliseconds to block for the indexing service to complete.
   //                 If INFINITE is specified the time out never elapses.
   //  CurrentSequence - The current sequence being indexed.
   //
   // Return Values:
   //  TR_ERROR_SUCCESS if it successfully created the index.
   //  TR_ERROR_PENDING if it returns before the index is built.
   //  TR_ERROR_COMMIT_FAILED if it built the index but failed to commit it for another session.
   //  TR_ERROR_NO_INDEX if a memory index was never created or committed to this log file.
   //  TR_ERROR_FAILURE or TR_ERROR_OUTOFMEMORY for other types of failure.
   STDMETHOD(GetMemoryIndexResult)(
      __in const DWORD    Milliseconds,
      __out TR_SEQUENCE & CurrentSequence
   ) PURE;

   //
   // Description:
   //  This call queries the index of sample runs for the run numbers of the first and
   //  last runs found in the trace.  This can only be used in single reader single
   //  process scenarios.
   //
   // Parameters:
   //  firstRunNumber - the number of the first sample run found in the trace
   //  lastRunNumber - the number of the last sample run found in the trace
   //
   // Return Values:
   //  TR_ERROR_SUCCESS if the values were found.
   //  TR_ERROR_NO_INDEX if no sample index was created from the log file.
   //  TR_ERROR_FAILURE if no samples were found.
   STDMETHOD(GetSampleRangeFromIndex)(
      __out ULONG & firstRunNumber,
      __out ULONG & lastRunNumber
   ) PURE;

   //
   // Description:
   //  This call queries the index of sample runs for details of a particular run.
   //  This can only be used in single reader single process scenarios.
   //
   // Parameters:
   //  sampleRunNumber - the sample run number to lookup in the index
   //  isValidRun - this run is complete and all sequence numbers are provided
   //    if a run is invalid, then some sequences might be zero or out of order.
   //  firstStartSequence - the sequence number of the first sample start event
   //  lastStartSequence - the sequence number of the last sample start event
   //  firstFinishSequence - the sequence number of the first sample finish event
   //  lastFinishSequence - the sequence number of the last sample finish event
   //
   // Return Values:
   //  TR_ERROR_SUCCESS if the values were found.
   //  TR_ERROR_NO_INDEX if no sample index was created from the log file.
   //  TR_ERROR_FAILURE if the sample run was not found.
   STDMETHOD(GetSampleRunFromIndex)(
      __in const ULONG sampleRunNumber,
      __out bool & isValidRun,
      __out TR_SEQUENCE & firstStartSequence,
      __out TR_SEQUENCE & lastStartSequence,
      __out TR_SEQUENCE & firstFinishSequence,
      __out TR_SEQUENCE & lastFinishSequence
   ) PURE;

   //
   // Description:
   //  Returns the QueryPerformanceFrequence() value from the guest process's
   //  system during record.
   //
   // Parameters:
   //  PerfFreq - The frequency used by the performance counter.
   //
   // Return Values:
   //  TR_ERROR_SUCCESS if the frequency is found.
   //  TR_ERROR_FAILURE if no frequency was recorded.
   STDMETHOD(QueryPerformanceFrequency)(
      __out LARGE_INTEGER & PerfFreq
   ) PURE;

   //
   // Description:
   //  Returns information about how the trace was captured.  See
   //  TR_TRACE_CAPTURE_FLAGS for flags that are set.  The returns
   //  the information for the current process.
   //
   // Parameters:
   //  TraceFlags - Bit mask of TR_TRACE_CAPTURE_FLAGS
   //
   // Return Values:
   //  TR_ERROR_SUCCESS
   STDMETHOD(GetTraceCaptureInfo)(
      __out ULONG & TraceFlags
   ) PURE;

   // Description:
   //  The following operation converts a reader handles to a message.
   //
   // Parameters:
   //  Handle   - The handle.
   //  Message  - The marker string message.
   //
   // Return Values:
   //  If the handle is valid it returns the TR_ERROR_SUCCESS or it returns
   //  TR_ERROR_FAILURE.
   STDMETHOD(ConvertHandleToMarker)(
      __in const TR_MARKER_HANDLE Handle,
      __out WCHAR Message[TR_MAX_MARKER_MESSAGE]
   ) PURE;

   // Description:
   //  The following operation retrieves guest process image bytes from
   //  the trace file.  The parameters to this operation are returned from
   //  TR_MemoryBlock callback.  See TR_MemoryBlock for more details.  This
   //  returns the image bytes from the current process.
   //
   // Parameters:
   //  pos      - The position at which to search for the image bytes.
   //  addr     - Address to start the copy from.
   //  length   - Amount of data to copy.
   //  imageBytes - Passed in buffer to hold the image data.
   //
   // Return Values:
   //  Returns TR_ERROR_SUCCESS if it gets the image bytes.
   //  Return TR_ERROR_ADDR_NOT_FOUND if the data isn't available.
   STDMETHOD(GetImageBytes)(
      __in const TR_POSITION_HANDLE hpos,
      __in const TR_ADDRESS addr,
      __in const ULONG length,
      __out_ecount(length) BYTE imageBytes[]) PURE;

   //
   // Description:
   //  Retrieves the unique trace group identifier.  The group identifier is logged
   //  into the trace file at record time.  If multiple traces have the same group id
   //  than those trace files are synchronized in a way that allows them to be
   //  attached to and replayed in a single reader as one process.
   //
   // Parameters:
   //  id - the unique guid from the currently executing process.
   //
   // Return Value:
   //  This always returns a valid GUID.
   STDMETHOD(GetTraceGroupIdentifier)(
      GUID & id
      ) const PURE;

   //
   // Description:
   //  Retrieves the process handle that owns the thread represented by
   //  the passed in thread handle.
   //
   // Parameters:
   //  thread - thread handle we want the parent process for.
   //  process - the handle of the process owning the thread.
   //
   // Return Value:
   //  Returns TR_ERROR_SUCCESS or TR_ERROR_INVALID_HANDLE.
   STDMETHOD(ConvertThreadToProcess)(
      __in const TR_THREAD_HANDLE thread,
      __out TR_PROCESS_HANDLE & process) const PURE;

   // Description:
   //  The current process is the current process at the currently executing
   //  position.
   //
   // Return Values:
   //  This either returns TR_ERROR_SUCCESS or invalid reader.
   STDMETHOD(GetCurrentProcess)(
      __out TR_PROCESS_HANDLE & process) const PURE;

   // Description:
   //  Each position is tied to a particular process.
   //
   // Return Values:
   //  This either returns TR_ERROR_SUCCESS or TR_ERROR_INVALID_POSITION.
   STDMETHOD(GetProcess)(
      __in const TR_POSITION_HANDLE pos,
      __out TR_PROCESS_HANDLE & process) const PURE;

   //
   // Description:
   //  Returns the first and last sequence number for those
   //  sequences of instructions in the process.
   //
   // Parameters:
   //  process   - The process we want the boundaries for.
   //  FirstSeq  - The first sequence of instructions in the trace.
   //  LastSeq   - The last sequence of instructions in the trace.
   //
   STDMETHOD(GetProcessTraceBoundarySequences)(
      __in  const TR_PROCESS_HANDLE process,
      __out TR_SEQUENCE & FirstSeq,
      __out TR_SEQUENCE & LastSeq) const PURE;

   //
   // Description:
   //  Returns the current position of the process in the trace.
   //
   // Parameters:
   //  process - Position in this process.
   //  Pos    - Returned position.
   //
   // Return Value:
   //  This return TR_ERROR_FAILURE or TR_ERROR_SUCCESS.
   STDMETHOD(GetProcessCurrentPosition)(
      __in  const TR_PROCESS_HANDLE process,
      __out TR_POSITION_HANDLE & Pos) const PURE;

   // Description:
   //  This operation returns a process handle for each process attached to
   //  the reader.
   //
   // Parameters:
   //  ProcessesLen - The length of Processes[].
   //  Processes - The process handles; there is one unique process handle per process in
   //            in the reader.  If this is NULL and ProcessesLen is 0, it just
   //            returns the ProcessesCount.
   //  ProcessesCount - The number of unique processes in the reader.
   //
   // Return Values:
   //  If the passed in process count is large enough it returns TR_ERROR_SUCCESS,
   //  the processes, and the actual process count.  If Processes[] is not large enough
   //  it returns TR_ERROR_MORE_DATA and returns as many process handles as will fit.
   //  The returned ProcessesCount is the length needed in Processes[].
   STDMETHOD(GetProcesses)(
      __in const ULONG  ProcessesLen,
      __out_ecount_opt(ProcessesLen) TR_PROCESS_HANDLE Processes[],
      __out ULONG      &ProcessesCount
   ) PURE;

   // Description:
   //  This operation returns a thread handle for each thread of execution
   //  in the passed in process.
   //
   // Parameters:
   //  Process - Process to retrieve the threads for.
   //  ThreadsLen - The length of Threads[].
   //  Threads - The thread handles; there is one unique thread handle per thread in
   //            the trace.  If this is NULL and ThreadLen is 0, it just
   //            returns the ThreadCount.
   //  ThreadsCount - The number of unique threads in the trace.
   //
   // Return Values:
   //  If the passed in thread count is large enough it returns TR_ERROR_SUCCESS,
   //  the threads, and the actual thread count.  If Threads[] is not large enough
   //  it returns TR_ERROR_MORE_DATA and returns as many thread handles as will fit.
   //  The returned ThreadCount is the length needed in Threads[].
   STDMETHOD(GetProcessThreads)(
      __in const TR_PROCESS_HANDLE Process,
      __in const ULONG  ThreadsLen,
      __out_ecount_opt(ThreadsLen) TR_THREAD_HANDLE Threads[],
      __out ULONG      &ThreadsCount
   ) PURE;

   //
   // Description:
   //  Returns the name of the trace file associated with the currently executing
   //  process.
   //
   // Parameters:
   //  Process - The trace file name for this process.
   //  file - The trace file name.
   //
   // Return Values:
   //  Returns the name of the trace file this reader is associated with.
   STDMETHOD(GetProcessTraceFileName)(
      __in  const TR_PROCESS_HANDLE Process,
      __out WCHAR file[MAX_PATH]) const PURE;

   //
   // Description:
   //  Retrieves information about the trace file, process, and system
   //  the trace was taken on.  This returns the system information for
   //  the currently executing process.
   //
   // Parameters:
   //  Process - The system information about this process.
   //  sysinfo - holds all the information.
   STDMETHOD(GetProcessTraceSystemInfo)(
      __in  const TR_PROCESS_HANDLE Process,
      __out TR_SYSTEM_INFO & sysinfo) const PURE;

   //
   // Description:
   //  Retrieves the unique identifier for the trace file of the currently
   //  executing process.
   //
   // Parameters:
   //  Process - the unique guid for this process.
   //  id - the unique guid
   STDMETHOD(GetProcessTraceIdentifier)(
      __in  const TR_PROCESS_HANDLE Process,
      __out GUID & id) const PURE;

   //
   // Description:
   //  Returns information about how the trace was captured.  See
   //  TR_TRACE_CAPTURE_FLAGS for flags that are set.  The returns
   //  the information for the current process.
   //
   // Parameters:
   //  Process - The flags for this process.
   //  TraceFlags - Bit mask of TR_TRACE_CAPTURE_FLAGS
   //
   // Return Values:
   //  TR_ERROR_SUCCESS
   STDMETHOD(GetProcessTraceCaptureInfo)(
      __in  const TR_PROCESS_HANDLE Process,
      __out ULONG & TraceFlags
   ) PURE;

   // Description:
   //  The following operation sets the reader to break on certain
   //  events that occur during execution within any process.
   //
   // Parameters:
   //  type - the supported types are TR_ModuleLoad, TR_Exception,
   //         TR_Marker, TR_CreateThread, and TR_DeleteThread.
   //
   // Return Value:
   //  Returns TR_ERROR_SUCCESS if the breakpoint is set in every process
   //  even if it already exists.  It returns TR_ERROR_FAILURE
   //  if an unsupported type is given.
   STDMETHOD(SetGlobalEventBreakPoint)(
      __in  const TR_BREAKPOINT_TYPE type
      ) PURE;

   // Description:
   //  See description of SetExecutionBreakPoint() above.
   STDMETHOD(SetProcessExecutionBreakPoint)(
      __in const TR_PROCESS_HANDLE Process,
      __in const TR_ADDRESS eip,
      __out TR_BREAKPOINT_HANDLE & bp
      ) PURE;

   // Description:
   //  See description of SetMemoryBreakPoint() above.
   STDMETHOD(SetProcessMemoryBreakPoint)(
      __in const TR_PROCESS_HANDLE Process,
      __in const TR_BREAKPOINT_TYPE  type,
      __in const TR_ADDRESS          addr,
      __in const ULONG               range,
      __out TR_BREAKPOINT_HANDLE & bp
      ) PURE;

   // Description:
   //  See description of SetEventBreakPoint() above.
   STDMETHOD(SetProcessEventBreakPoint)(
      __in const TR_PROCESS_HANDLE Process,
      __in const TR_BREAKPOINT_TYPE type,
      __out TR_BREAKPOINT_HANDLE & bp
      ) PURE;

   // Description:
   //  See description of GetExecutionBreakPoint() above.
   STDMETHOD(GetProcessExecutionBreakPoint)(
      __in const TR_PROCESS_HANDLE Process,
      __in const TR_ADDRESS eip,
      __out TR_BREAKPOINT_HANDLE &bp
      ) PURE;

   // Description:
   //  See description of GetMemoryBreakPoint() above.
   STDMETHOD(GetProcessMemoryBreakPoint)(
      __in const TR_PROCESS_HANDLE Process,
      __in const TR_BREAKPOINT_TYPE type,
      __in const TR_ADDRESS         addr,
      __in const ULONG              range,
      __out TR_BREAKPOINT_HANDLE &bp
      ) PURE;

   // Description:
   //  See description of GetEventBreakPoint() above.
   STDMETHOD(GetProcessEventBreakPoint)(
      __in const TR_PROCESS_HANDLE Process,
      __in const TR_BREAKPOINT_TYPE type,
      __out TR_BREAKPOINT_HANDLE &bpHandle
      ) PURE;

   // Description:
   //  See description of GetMemoryValue() above.
   STDMETHOD(GetProcessMemoryValue)(
      __in const TR_PROCESS_HANDLE  Process,
      __in const TR_ADDRESS         Address,
      __in const ULONG              Range,
      __in const ULONG              DataLen,
      __out_ecount(DataLen) TR_MEMORY_DATA   Data[],
      __out ULONG                  &DataCount
      ) PURE;

   //
   // Description:
   //  This utility converts a unique thread handle to a unique index, where all the threads
   //  in all the processes are numbered sequentially from 0 to total number of threads minus
   //  1.
   //
   // Parameters:
   //  Thread - Thread to convert.
   //  Index  - Unique sequential index.
   //
   // Return Values:
   //  TR_ERROR_SUCCESS if it successfully created the index.
   //  It returns a failure code if it cannot convert the thread.
   STDMETHOD(GetUniqueThreadIndex)(
      __in const TR_THREAD_HANDLE  Thread,
      __out ULONG                  &Index
      ) PURE;

   //
   // Description:
   //  Searches for the position where the expression evaluates to true.  The position
   //  returned is not necessarily the exact instruction where the change in state
   //  resulting in the expression evaluating to true occurs.  For example if the change
   //  occurred outside the bounds of the trace, such as in kernel mode in a user mode
   //  trace, the position returned is after the point in execution that the indirect
   //  write happened.  The search will stop when it sees an out of bounds change.
   //
   //  The expression is of the following form with the listed constraints:
   //
   //     * You can only have one $source operator per predicate.
   //     * You can only have one $ip operator per predicate.
   //
   //   search-string := <direction> <predicate>
   //   predicate := ["("] <expression> [ <loper> ["("]<predicate>[")"] ]* [")"]
   //     direction := "[j]+" | "[j]-"   # forwards and backwards, respectively
   //                                    # with optional jump to that position.
   //     loper := "&&" | "||"
   //     expression := <variable> <oper>[(<range|rrange>)] <value> | <special> | <ip>
   //       variable := <register> | <pseudo-register> | <address>
   //         register := @reg            # Ex. rip, eip, eax, etc...
   //         pseudo-register := @gle |   # Resolves to the address of the TEB GetLastError
   //                            @inst    # Resolve to the code bytes of the current instruction
   //                                     # value for every thread.
   //         address := <hex>
   //       value := <decimal> | <hex> | <code-bytes>
   //                                     # All number are read in as 64bit, except code bytes
   //                                     # and registers > 8 bytes.
   //                                     # Hex numbers are assumed unsigned and decimals
   //                                     # are signed.
   //         hex := 0xNNNNNNNNNNNNNNNN |
   //                NNNNNNNN`NNNNNNNN    # DWORD pair
   //                NNNNNNNNNNNNNNNN`NNNNNNNNNNNNNNNN # QWORD pair for numbers > 8 bytes
   //         decimal := [-]N | 0nN
   //         code-bytes := <xx>          # Up to <rrange> number of hex pairs (i.e. 6a0c for push 0c)
   //       oper  := "==" | "!=" | ">" | "<" | ">=" | "<="
   //       special := "$modify"[<range>](<variable>) | # Range ignored for registers
   //                  "$source"[<range>](<variable>) |
   //                  "$write"[<range>](<address> | @gle | <\"value\">)
   //                  "$read"[<range>](<address> | @gle | <\"value\">)
   //       ip := "$ip"[(<address>)]  # If the address is left off it uses the current ip.
   //         range := "1" | "2" | "4" | "8"   # Optional range defaults to 4
   //                                          # Variable is an address
   //         trange := "1" >= <rrange> <= "16" # Optional range defaults to the register size
   //                                           # @inst default size is 1 byte
   //                                           # Variable is a register
   //
   //  Examples:
   //   "+ MyVar >= 3"
   //      Searches for when the value of the address of MyVar is >= 3.
   //   "+ poi(MyVar) ==(2) 0x5"
   //      Searches for when the 2 bytes at the dereferenced address of MyVar equals 0x5.
   //   "- (@eax != 0x3) && (@esp < 0x325ab32a)"
   //      Searches for when the values of eax and esp meet the condition.  For 64bit use
   //      @rsp and @rax.
   //   "+ @gle != 0"
   //      Searches for when the last error value does not equal 0.
   //   "- $modify4(MyVar)"
   //      Searches for when value of MyVar was modified via a direct or indirect
   //      write (4 byte range).
   //   "- $write4(MyVar)"
   //      Searches for the previous direct write to MyVar.
   //   "- $source4(MyVar)"
   //      Searches for the source of the value of MyVar (4 byte range).
   //         mov MyVar eax <--- @eax == 0 This is the $modify
   //         mov MyVar ecx <--- @ecx == 3 This is the $source
   //         push MyVar <--- MyVar == 3
   //      If the value of MyVar is modified indirectly, the position returned
   //      is at the first traced instruction after the change.  This behavior
   //      is similar to $modify.
   //   "j+ ($ip(mydll!MyFunc)) && (@eax == 3)"
   //      Searches for when the execution breakpoint hits mydll!MyFunc and eax equals 3
   //      and jumps to that position when the condition is met.
   //   "j+ @inst ==(1) 0xc2"
   //      Searches for the next near RET instruction.
   //
   // Parameters:
   //  Process - Process is set the breakpoint in.
   //  SearchString - The search-string expression.
   //  Pos - Position where expression evaluates to true.
   //
   // Return Values:
   //  TR_ERROR_SUCCESS if it finds a position where the expression evaluates to
   //  true.
   // TR_ERROR_ADDR_NOT_FOUND if it does not find a position.
   // These next error codes return a FormatErrorIndex.
   //  TR_ERROR_BAD_INPUT if the expression format is bad.
   //  TR_ERROR_NOT_IMPLEMENTED if the legal expression contains a not-yet-implemented feature.
   //  TR_ERROR_NO_VALUE if $modify is used and it cannot evaluate the variable to the
   //  current value.
   STDMETHOD(Search)(
      __in const TR_PROCESS_HANDLE  Process,
      __in const PWCHAR SearchString,
      __out ULONG & FormatErrorIndex,
      __out TR_POSITION_HANDLE & Pos
      ) PURE;

   // Description:
   //  The following operation convert an ETW event handle to an ETW
   //  event record.  See comments above TR_ETW_EVENT for more information.
   //
   // Parameters:
   //  Handle   - The handle.
   //  EtwEvent - The converted type.
   //
   // Return Values:
   //  If the handle is valid it returns the TR_ERROR_SUCCESS or it returns
   //  TR_ERROR_FAILURE.
   STDMETHOD(ConvertHandleToEtwEvent)(
      __in const TR_ETW_EVENT_HANDLE Handle,
      __out TR_ETW_EVENT & EtwEvent
   ) PURE;

   // Description:
   //  Finds the closest position at the passed in timestamp (system time)
   //  on the specified thread.  If the timestamp does
   //  not exactly match a timed event, it returns the closest timed event
   //  before the passed in time.  If more than one event has the same time
   //  stamp, it uses the optional opcode and level arguments to break the
   //  tie.
   //
   //  NOTE: Use this to search for logged ETW events.
   //
   //  No callbacks are called or breakpoints hit when this operation is used.
   //
   // Parameters:
   //  ProcessId - Process to search for the timestamp in.
   //  ThreadId - Thread in the search process.
   //  Opcode - ETW event opcode.  Use (ULONG)-1 to ignore.
   //  Level - ETW event level.  Use (ULONG)-1 to ignore.
   //  TimeStamp - The system time time stamp.
   //  Pos - The position of the timed event.
   //
   // Return Values:
   //  TR_ERROR_SUCCESS if it finds the position.
   //  TR_ERROR_INVALID_POSITION if no event matches the timestamp.
   STDMETHOD(FindTimedEvent)(
      __in const ULONG         ProcessId,
      __in const ULONG         ThreadId,
      __in const ULONG         Opcode,
      __in const ULONG         Level,
      __in const LARGE_INTEGER TimeStamp,
      __out TR_POSITION_HANDLE & Pos
      ) PURE;


   // Description:
   //  Create a command tree in a file that windbg can use to nativigate
   //  through from the current activity to other correlated activities.
   //
   // Parameters:
   //  ActivityGuid - The guid of the activity to build the tree around.
   //                 It uses the activity at the current position if this
   //                 is NULL.  If the value is set to L"all" it dumps the
   //                 whole activity tree.
   //  TreePath - Path to the command file if the operation succeeds.
   //
   // Return Values:
   //  TR_ERROR_SUCCESS if it builds the command tree.
   //  TR_ERROR_INVALID_POSITION if the activity is bad or the position is not within
   //  an activity.
   //  TR_ERROR_FAILURE for all other failures.
   STDMETHOD(CreateActivityCommandTree)(
      __in_opt PWCHAR ActivityGuid,
      __out    WCHAR TreePath[MAX_PATH]
      ) PURE;

};


#undef INTERFACE
#define INTERFACE ITREADER2
DECLARE_INTERFACE_(ITREADER2, ITREADER)
{
    // Description:
    //  Copies the metrics info to the struct specified by the caller
    //
    // Parameters:
    //  pMetricsInfo - A pointer to the TR_METRICS_INFO structure.
    //                 It copies the contents of TR_METRICS_INFO struct
    //                 to the caller-supplied destination.
    //
    // Return Values:
    //  TR_ERROR_SUCCESS.
    STDMETHOD(GetMetricsInfo)(
        __out TR_METRICS_INFO * pMetricsInfo 
    ) PURE;

    // Description:
    //  Copies the trace file version info to the struct specified by the caller
    //
    // Parameters:
    //  pVersionInfo - A pointer to the TR_VERSION_INFO structure.
    //                 It copies the contents of TR_VERSION_INFO struct
    //                 to the caller-supplied destination.
    //
    // Return Values:
    //  TR_ERROR_SUCCESS.
    STDMETHOD(GetTraceVersionInfo)(
        __out  TR_VERSION_INFO * pVersionInfo
    ) PURE;

    // Description:
    //  Copies the reader version info to the struct specified by the caller
    //
    // Parameters:
    //  pVersionInfo - A pointer to the TR_VERSION_INFO structure.
    //                 It copies the contents of TR_VERSION_INFO struct
    //                 to the caller-supplied destination.
    //
    // Return Values:
    //  TR_ERROR_SUCCESS.
    STDMETHOD(GetReaderVersionInfo)(
        __out  TR_VERSION_INFO * pVersionInfo
    ) PURE;

};



///////////////////////////////////////////////////////////////////
//
// The ITREADER_GROUP interface
//

// The following API creates an ITREADER_GROUP interface.
// In addition to the defined API return values, all the API's return TR_ERROR_INVALID_READER if
// an internal error in the group reader has occurred.  In this case the reader is in an irrecoverable
// state.  The group reader can only be used to ExecuteForward() from the start of the trace.
extern "C" ITREADER_GROUP* __cdecl CreateITReaderGroup();

// Reads the trace guid tag from the given trace file.
// May be used to determine the architecture of the trace prior to opening the trace.
extern "C" HRESULT __cdecl GetTraceGUIDTag(
    _In_z_ LPCWSTR pwcszFileName,
    _Out_ GUID * pGUID
    );

#undef INTERFACE
#define INTERFACE ITREADER_GROUP
DECLARE_INTERFACE_(ITREADER_GROUP, IUnknown)
{
   // IUnknown.
   STDMETHOD(QueryInterface)(
       __in REFIID InterfaceId,
       __out PVOID* Interface
       ) PURE;

   STDMETHOD_(ULONG, AddRef)() PURE;
   STDMETHOD_(ULONG, Release)() PURE;

   //
   // Description:
   //  Add a trace file to the group reader.  All trace files must be from the same
   //  group, and all trace files must be added before execution begins.
   //
   //  You can use the reader returned to set callbacks and breakpoints.  Note that
   //  the registered callbacks cannot be modified after PrepareForExecution() has
   //  been called.
   //
   // Parameters:
   //  TraceFile - NULL terminated file name of trace of a program's execution.
   //  reader    - The reader added to the group.
   //
   // Return Value:
   //  See TREADER::AttachTraceFile for full list of return values.
   //  TR_ERROR_NOT_IN_GROUP - The trace file is not within the same group as the previous
   //  trace files added to the group.
   //  TR_ERROR_TOO_MANY_READERS - You tried to add a trace file after already starting
   //  execution.
   STDMETHOD(AddTraceFile)(
       __in const PWCHAR TraceFile,
       __out PITREADER & reader
       ) PURE;


   // Description:
   //  This operation returns an array containing all the readers in the group.
   //
   // Parameters:
   //  ReadersLen - The length of Readers[].
   //  Readers - The readers.  If this is NULL then ReadersLen is ignored, and it
   //            returns the ReadersCount.
   //  ReadersCount - The number of readers in the group; this can be more than
   //                 ReadersLen.
   //
   // Return Values:
   //  If the passed in reader count is large enough it returns TR_ERROR_SUCCESS,
   //  the readers, and the actual reader count.  If Readers[] is not large enough
   //  it returns TR_ERROR_MORE_DATA and returns as many readers as will fit.
   //  The returned ReaderCount is the length needed in Readers[].
   STDMETHOD(GetReaders)(
      __in const ULONG  ReadersLen,
      __out_ecount_opt(ReadersLen) PITREADER Readers[],
      __out ULONG      &ReadersCount
   ) PURE;

   //
   // Description:
   //  The following operation compares how two positions relate in terms of execution time
   //  to each other in a trace.  This operation is similar to the ITREADER version, except
   //  that it works cross process.
   //  To determine if two positions are the same instruction, the position thread and
   //  instruction count must be equal.
   //
   // Parameters:
   //  pos1    - position that is being compared.
   //  pos2    - position that is being compared.
   //
   // Return Value:
   //  On a single thread the results of these operations are simply based on the relative
   //  order of the instructions these position represent.  When comparing positions across
   //  two threads, it may only be possible to know the relative order of a range of instructions
   //  in each thread.  If the range of instructions the two positions are in fall into the same
   //  sequence, those two positions are considered equal.
   //  If pos1 > pos2 it returns > 0
   //  If pos1 < pos2 it returns < 0
   //  If pos1 = pos2 it returns 0
   //
   // If the positions are invalid this throws and exception.
   STDMETHOD_(LONG, ComparePositions)(
      __in const TR_POSITION_HANDLE pos1,
      __in const TR_POSITION_HANDLE pos2) const PURE;

   //
   // Description:
   //  This must be called before the first call to ExecuteForward() and after all
   //  the trace files are added to the group.  You can also call this at anytime
   //  during execution to reset the group state back to the first instruction to
   //  execute.
   //
   // Parameters:
   //  reader - The reader containing the first sequence of instruction to execute.
   //
   // Return Value:
   //  TR_ERROR_SUCCESS if it succeeds.
   //  TR_ERROR_INVALID_READER if there are no readers in the group.
   STDMETHOD(PrepareForExecution)(
       __out PITREADER & reader
       ) PURE;

   //
   // Description:
   //  Returns the current reader in execution.
   //
   // Parameters:
   //  reader - The reader the is going to execute the next instruction.
   //
   // Return Value:
   //  TR_ERROR_SUCCESS if it succeeds.
   //  TR_ERROR_ENDOFTRACE if there is no reader because execution has finished.
   //  TR_ERROR_INVALID_READER if called in error.
   STDMETHOD(GetCurrentReader)(
       __out PITREADER & reader
       ) PURE;

   //
   // Description:
   //  The group reader's execute forward operation is similar to a reader's execute
   //  forward operation in that it executes forward from the current position to
   //  the next breakpoint, the end of the trace, or the specified number of steps.
   //  The difference is that it uses multiple readers in its execution.  The first
   //  sequence it executes is the sequence of instructions with the lowest sequence
   //  number among all the added trace files.  It gets to the end of the trace when
   //  it has finished executing through all the sequences of all the added trace
   //  files.
   //
   //  In addition it stops execution when it the next instruction to execute
   //  is in a different reader that the last sequence of instructions it was
   //  executing.  In this case it returns the reader owning that next sequence
   //  of instructions to execute.  The caller can then continue execution by
   //  re-calling ExecuteForward.
   //
   //  See ITREADER::ExecuteForward for a more details on forward execution.
   //
   // Parameters:
   //  step        - Number of instructions to execute.  If this is zero it executes to
   //                the next breakpoint.
   //  bphit       - If non-NULL this returns a pointer to the breakpoint from the
   //                breakpoint array that is hit.
   //  reader      - The reader containing the next sequence of instructions to execute.
   //                This reader is released when this ITREADER_GROUP object is release.
   //                Undefined behavior will result if you release a reader returned
   //                by this operation.
   //
   // Return Value:
   //  TR_ERROR_SUCCESS if it executes the number of steps given or returning with the
   //  first reader.
   //  TR_ERROR_BREAKPOINT_HIT if execution stops at a breakpoint.
   //  TR_ERROR_ENDOFTRACE if it executes to the end of the group of traces.
   //  TR_ERROR_SEQUENCE_END if it runs through to the next sequence.
   STDMETHOD(ExecuteForward)(
      __in  const ULONGLONG        step,
      __out TR_BREAKPOINT        & bphit,
      __out PITREADER & reader) PURE;

   //
   // Description:
   //  Returns the first and last sequence number for those
   //  sequences of instructions in the group of traces.
   //
   // Parameters:
   //  FirstSeq  - The first sequence of instructions in the group.
   //  LastSeq   - The last sequence of instructions in the group.
   //
   STDMETHOD(GetGroupBoundarySequences)(
      __out TR_SEQUENCE & FirstSeq,
      __out TR_SEQUENCE & LastSeq) const PURE;

   //
   // Description:
   //  Retrieves the unique group identifier for the trace file.  A trace file
   //  has a group identifier if it has the TR_CaptureGrouped property.
   //
   //  NOTE: This is a utility that can be used on any ITREADER instance.
   //
   // Parameters:
   //  id - the group guid
   //
   // Return Values:
   //  Returns TR_ERROR_SUCCESS if the trace has a group ID.
   //  Returns TR_ERROR_NOT_IN_GROUP if the trace does not have a group ID.
   STDMETHOD(GetTraceGroupIdentifier)(
      __in const PITREADER reader,
      __out GUID & id) const PURE;

};

#endif // ITRACEREADER
