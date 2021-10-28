//-----------------------------------------------------------------------------
//
// Time Travel Tracing (TTT) Writer Interface
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Module name:
//
//    ITraceWriter.h
//
// Description:
//
//  The trace writer interface is used to control the recording process of an
//  application under TTT tracing control (guest process).  This interface can
//  be used by the guest process itself or another process communicating with
//  the guest.  There can be only one writer per process.
//
//-----------------------------------------------------------------------------

#ifndef ITRACEWRITER
#define ITRACEWRITER

#pragma once

#include <windows.h>
#include "ole2.h"

/* 49736b3e-55cb-43ce-bdbc-ded8f57b9e5b */
DEFINE_GUID(IID_ITWRITER,
    0x49736b3e,
    0x55cb,
    0x43ce,
    0xbd, 0xbc, 0xde, 0xd8, 0xf5, 0x7b, 0x9e, 0x5b
    );

// See StartTracing()
const UUID TW_NULL_GUID = { /* 00000000-0000-0000-0000-000000000000 */
    0x0,
    0x0,
    0x0,
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
  };

typedef interface DECLSPEC_UUID("49736b3e-55cb-43ce-bdbc-ded8f57b9e5b") ITWRITER* PITWRITER;

//
// Return code for IWRITER interface
//
const HRESULT TW_ERROR_SUCCESS                    = NO_ERROR;
const HRESULT TW_ERROR_FAILURE                    = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x1);
const HRESULT TW_ERROR_NOT_IMPLEMENTED            = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x2);
const HRESULT TW_ERROR_PROCESS_NOT_FOUND          = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x3);
// The process is not a TTT guest process (i.e. not under TTT control)
const HRESULT TW_ERROR_NO_GUEST                   = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x4);
// Do not have the proper security privileges
const HRESULT TW_ERROR_PRIVILEGE                  = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x6);
// Trying to attach to a guest process that already has TTT loaded in it.
const HRESULT TW_ERROR_ALREADY_ATTACHED           = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x7);
// Problem attaching to a given guest process ID.
const HRESULT TW_ERROR_ATTACH     = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x8);
// Problem with the arguments to an API.
const HRESULT TW_ERROR_ARGUMENT   = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x9);
// Could not open the trace file to log to.
const HRESULT TW_ERROR_RUNFILE    = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xa);
// Error setting up the client <=> guest process communication
const HRESULT TW_ERROR_COMMUNICATION = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xb);
// Error finding the path where the client DLL is located.
const HRESULT TW_ERROR_CLIENTPATH = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xc);
// Ran out of memory allocating something.
const HRESULT TW_ERROR_OUTOFMEMORY = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xd);
// Could not open a stream to the user specified error output file.
const HRESULT TW_ERROR_OUTPUTFILE = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xe);
// Failed to create a resource, such as a create event.
const HRESULT TW_ERROR_RESOURCE   = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0xf);
// Unsupported OS version
const HRESULT TW_ERROR_OSVERSION  = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x10);
// Trace session is not enabled.
const HRESULT TW_ERROR_NOT_ENABLED  = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x11);
// Binding two different versions of the interface to the same guest process.
const HRESULT TW_ERROR_VERSION_MISMATCH = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x12);
// Timed out waiting for operation to finish.
const HRESULT TW_ERROR_TIMEOUT = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x13);
// A failure occurred during initialization in the guest process after TTT was loaded into
// it or a trace session was started.
const HRESULT TW_ERROR_GUEST_INITIALIZATION = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x14);
// E2E not initialized.
const HRESULT TW_ERROR_NOT_INITIALIZED  = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x15);
// Cannot load the interface needed to complete an operation.
const HRESULT TW_ERROR_NO_INTERFACE  = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_NULL, 0x16);

const ULONG TW_CURRENT_PROCESS = (ULONG)-1;

struct TW_SESSION_INFO
{
   GUID   TraceId;
   WCHAR  LogFile[MAX_PATH];
};

//
// Description (EVENT CALLBACK):
//  The following callback is invoked when a trace session ends.  It is not safe
//  to wait on synchronization events inside this callback.
//
// Parameters:
//  SessionInfo  - Information about the session that just ended.
//  Status - How the session fared.
//  CallbackContext - Context defined during the bind.
//
// Return Values:
typedef VOID (CALLBACK *TW_SESSION_CALLBACK)(
   const TW_SESSION_INFO & SessionInfo,
   const HRESULT Status,
   __in_opt PVOID CallbackContext);


// The ITWRITER interface
//

// The following API retrieves the ITWRITER interface for this process.  Each ITWRITER
// can manage one session.  See AttachToProcess below for more details.
extern "C" ITWRITER* GetITWriter();


#undef INTERFACE
#define INTERFACE ITWRITER
DECLARE_INTERFACE(ITWRITER)
{
   //
   // Description:
   //  Checks to see if the passed in process is an active trace
   //  session.  This interface does not have to be bound to a session
   //  to use this API.
   //
   // Parameters:
   //  Pid - Process to check to see if the trace session is active.
   //
   // Return Values:
   //  TW_ERROR_SUCCESS - if the trace session is active.
   //  TW_ERROR_NOT_ENABLED - if it is not.
   STDMETHOD(IsActiveTraceSession)(
       __in  ULONG Pid
       ) PURE;

   //
   // Description:
   //  Binds this interface to a guest process by loading the TTT writer
   //  into it.  Tracing is not active after the load.  If TTT is already
   //  loaded into the guest process it just returns TW_ERROR_ALREADY_ATTACHED.
   //
   // NOTE: Reddog OS allows clients to bind to themselves (TW_CURRENT_PROCESS)
   //       without admin privileges.
   //
   // Parameters:
   //  Pid - The process to load the writer into.  Use
   //        TW_CURRENT_PROCESS for the current process.
   //  Flags - reserved
   //  Path - Directory to hold the trace file output.  The trace file
   //         is <Path>\<traceID>.run.
   //  OutputProviderGuid - Event tracing provider guid used to log error and
   //                       informational messages.  This is only supported in
   //                       Vista+ systems.  This overrides any information that
   //                       normally would go to a TTT output file.
   //
   // Return Values:
   //  TW_ERROR_SUCCESS if it binds to the process or another TW_ERROR_* code for
   //  failure.  It returns TW_ERROR_ALREADY_ATTACHED if the session is already
   //  bound.
   STDMETHOD(BindTraceSession)(
       __in  const ULONG Pid,
       __in  const ULONG Flags,
       __in  const WCHAR Path[MAX_PATH],
       __in_opt LPCGUID  OutputProviderGuid
       ) PURE;

   //
   // Description:
   //  Starts active tracing in a process
   //
   // Parameters:
   //  TraceId - Unique ID to identify the trace session.  We'll generate one if
   //            this is a TW_NULL_GUID.
   //  MaxFileSize - Tracing ends after the trace file gets to this
   //                size in MB.
   //  Flags - set to 0
   //  Timer - Tracing ends automatically after this many
   //          ms.  Use 0 for infinite.
   //  SessionCallback - Notifies the client that a trace session has finished.
   //  CallbackContext - User context for the callback.
   //  TraceFile - If not null, this returns with the target trace file name.
   //
   // Return Values:
   //  TW_ERROR_NO_GUEST - session not bound to a guest process.
   //  TW_ERROR_ALREADY_ATTACHED - guest is already being traced.
   STDMETHOD(StartTracing)(
       __in const GUID & TraceId,
       __in const ULONG MaxFileSize,
       __in const ULONG Flags,
       __in const ULONG Timer,
       __in_opt TW_SESSION_CALLBACK SessionCallback,
       __in_opt  PVOID CallbackContext,
       __out_opt WCHAR TraceFile[MAX_PATH]
       ) PURE;

   //
   // Description:
   //  End an active tracing session.  The tracing session also
   //  ends if the guest process exits.
   //
   // Parameters:
   //
   // Return Values:
   //  TW_ERROR_NO_GUEST - session not bound to a guest process.
   //  TW_ERROR_TIMEOUT - did not stop the trace in the default
   //                     alloted time.
   //  If successful the session result is returned in the SessionCallback
   //  passed in with tracing was started.
   STDMETHOD(StopTracing)(
       ) PURE;

   //
   // Description:
   //  Returns the ID of the trace session for the passed in process.
   //  This fails if the process is not being traced.
   //
   // Parameters:
   //  Pid - process ID of the session you want to get the ID for.
   //  TraceId - The ID for the current active session.
   //
   // Return Values:
   //  TW_ERROR_NOT_ENABLED - not actively tracing.
   //  TW_ERROR_SUCCESS  - returned the ID.
   STDMETHOD(GetSessionTraceId)(
       __in  ULONG Pid,
       __out GUID & TraceId
       ) PURE;

};

#endif // ITRACEWRITER
