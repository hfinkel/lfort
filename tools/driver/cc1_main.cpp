//===-- cc1_main.cpp - LFort CC1 Compiler Frontend ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the entry point to the lfort -cc1 functionality, which implements the
// core compiler functionality along with a number of additional tools for
// demonstration and testing purposes.
//
//===----------------------------------------------------------------------===//

#include "lfort/Driver/Arg.h"
#include "lfort/Driver/ArgList.h"
#include "lfort/Driver/DriverDiagnostic.h"
#include "lfort/Driver/OptTable.h"
#include "lfort/Driver/Options.h"
#include "lfort/Frontend/CompilerInstance.h"
#include "lfort/Frontend/CompilerInvocation.h"
#include "lfort/Frontend/FrontendDiagnostic.h"
#include "lfort/Frontend/TextDiagnosticBuffer.h"
#include "lfort/Frontend/TextDiagnosticPrinter.h"
#include "lfort/FrontendTool/Utils.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/LinkAllPasses.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>
using namespace lfort;

//===----------------------------------------------------------------------===//
// Main driver
//===----------------------------------------------------------------------===//

static void LLVMErrorHandler(void *UserData, const std::string &Message) {
  DiagnosticsEngine &Diags = *static_cast<DiagnosticsEngine*>(UserData);

  Diags.Report(diag::err_fe_error_backend) << Message;

  // Run the interrupt handlers to make sure any special cleanups get done, in
  // particular that we remove files registered with RemoveFileOnSignal.
  llvm::sys::RunInterruptHandlers();

  // We cannot recover from llvm errors.  When reporting a fatal error, exit
  // with status 70.  For BSD systems this is defined as an internal software
  // error.  This notifies the driver to report diagnostics information.
  exit(70);
}

int cc1_main(const char **ArgBegin, const char **ArgEnd,
             const char *Argv0, void *MainAddr) {
  OwningPtr<CompilerInstance> LFort(new CompilerInstance());
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());

  // Initialize targets first, so that --version shows registered targets.
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();

  // Buffer diagnostics from argument parsing so that we can output them using a
  // well formed diagnostic object.
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  TextDiagnosticBuffer *DiagsBuffer = new TextDiagnosticBuffer;
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagsBuffer);
  bool Success;
  Success = CompilerInvocation::CreateFromArgs(LFort->getInvocation(),
                                               ArgBegin, ArgEnd, Diags);

  // Infer the builtin include path if unspecified.
  if (LFort->getHeaderSearchOpts().UseBuiltinIncludes &&
      LFort->getHeaderSearchOpts().ResourceDir.empty())
    LFort->getHeaderSearchOpts().ResourceDir =
      CompilerInvocation::GetResourcesPath(Argv0, MainAddr);

  // Create the actual diagnostics engine.
  LFort->createDiagnostics(ArgEnd - ArgBegin, const_cast<char**>(ArgBegin));
  if (!LFort->hasDiagnostics())
    return 1;

  // Set an error handler, so that any LLVM backend diagnostics go through our
  // error handler.
  llvm::install_fatal_error_handler(LLVMErrorHandler,
                                  static_cast<void*>(&LFort->getDiagnostics()));

  DiagsBuffer->FlushDiagnostics(LFort->getDiagnostics());
  if (!Success)
    return 1;

  // Execute the frontend actions.
  Success = ExecuteCompilerInvocation(LFort.get());

  // If any timers were active but haven't been destroyed yet, print their
  // results now.  This happens in -disable-free mode.
  llvm::TimerGroup::printAll(llvm::errs());

  // Our error handler depends on the Diagnostics object, which we're
  // potentially about to delete. Uninstall the handler now so that any
  // later errors use the default handling behavior instead.
  llvm::remove_fatal_error_handler();

  // When running with -disable-free, don't do any destruction or shutdown.
  if (LFort->getFrontendOpts().DisableFree) {
    if (llvm::AreStatisticsEnabled() || LFort->getFrontendOpts().ShowStats)
      llvm::PrintStatistics();
    LFort.take();
    return !Success;
  }

  // Managed static deconstruction. Useful for making things like
  // -time-passes usable.
  llvm::llvm_shutdown();

  return !Success;
}
