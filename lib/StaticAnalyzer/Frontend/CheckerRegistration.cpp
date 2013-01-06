//===--- CheckerRegistration.cpp - Registration for the Analyzer Checkers -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Defines the registration function for the analyzer checkers.
//
//===----------------------------------------------------------------------===//

#include "lfort/StaticAnalyzer/Frontend/CheckerRegistration.h"
#include "lfort/Basic/Diagnostic.h"
#include "lfort/Frontend/FrontendDiagnostic.h"
#include "lfort/StaticAnalyzer/Checkers/LFortCheckers.h"
#include "lfort/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "lfort/StaticAnalyzer/Core/CheckerManager.h"
#include "lfort/StaticAnalyzer/Core/CheckerOptInfo.h"
#include "lfort/StaticAnalyzer/Core/CheckerRegistry.h"
#include "lfort/StaticAnalyzer/Frontend/FrontendActions.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

using namespace lfort;
using namespace ento;
using llvm::sys::DynamicLibrary;

namespace {
class LFortCheckerRegistry : public CheckerRegistry {
  typedef void (*RegisterCheckersFn)(CheckerRegistry &);

  static bool isCompatibleAPIVersion(const char *versionString);
  static void warnIncompatible(DiagnosticsEngine *diags, StringRef pluginPath,
                               const char *pluginAPIVersion);

public:
  LFortCheckerRegistry(ArrayRef<std::string> plugins,
                       DiagnosticsEngine *diags = 0);
};
  
} // end anonymous namespace

LFortCheckerRegistry::LFortCheckerRegistry(ArrayRef<std::string> plugins,
                                           DiagnosticsEngine *diags) {
  registerBuiltinCheckers(*this);

  for (ArrayRef<std::string>::iterator i = plugins.begin(), e = plugins.end();
       i != e; ++i) {
    // Get access to the plugin.
    DynamicLibrary lib = DynamicLibrary::getPermanentLibrary(i->c_str());

    // See if it's compatible with this build of lfort.
    const char *pluginAPIVersion =
      (const char *) lib.getAddressOfSymbol("lfort_analyzerAPIVersionString");
    if (!isCompatibleAPIVersion(pluginAPIVersion)) {
      warnIncompatible(diags, *i, pluginAPIVersion);
      continue;
    }

    // Register its checkers.
    RegisterCheckersFn registerPluginCheckers =
      (RegisterCheckersFn) (intptr_t) lib.getAddressOfSymbol(
                                                      "lfort_registerCheckers");
    if (registerPluginCheckers)
      registerPluginCheckers(*this);
  }
}

bool LFortCheckerRegistry::isCompatibleAPIVersion(const char *versionString) {
  // If the version string is null, it's not an analyzer plugin.
  if (versionString == 0)
    return false;

  // For now, none of the static analyzer API is considered stable.
  // Versions must match exactly.
  if (strcmp(versionString, LFORT_ANALYZER_API_VERSION_STRING) == 0)
    return true;

  return false;
}

void LFortCheckerRegistry::warnIncompatible(DiagnosticsEngine *diags,
                                            StringRef pluginPath,
                                            const char *pluginAPIVersion) {
  if (!diags)
    return;
  if (!pluginAPIVersion)
    return;

  diags->Report(diag::warn_incompatible_analyzer_plugin_api)
      << llvm::sys::path::filename(pluginPath);
  diags->Report(diag::note_incompatible_analyzer_plugin_api)
      << LFORT_ANALYZER_API_VERSION_STRING
      << pluginAPIVersion;
}


CheckerManager *ento::createCheckerManager(const AnalyzerOptions &opts,
                                           const LangOptions &langOpts,
                                           ArrayRef<std::string> plugins,
                                           DiagnosticsEngine &diags) {
  OwningPtr<CheckerManager> checkerMgr(new CheckerManager(langOpts));

  SmallVector<CheckerOptInfo, 8> checkerOpts;
  for (unsigned i = 0, e = opts.CheckersControlList.size(); i != e; ++i) {
    const std::pair<std::string, bool> &opt = opts.CheckersControlList[i];
    checkerOpts.push_back(CheckerOptInfo(opt.first.c_str(), opt.second));
  }

  LFortCheckerRegistry allCheckers(plugins, &diags);
  allCheckers.initializeManager(*checkerMgr, checkerOpts);
  checkerMgr->finishedCheckerRegistration();

  for (unsigned i = 0, e = checkerOpts.size(); i != e; ++i) {
    if (checkerOpts[i].isUnclaimed())
      diags.Report(diag::err_unknown_analyzer_checker)
          << checkerOpts[i].getName();
  }

  return checkerMgr.take();
}

void ento::printCheckerHelp(raw_ostream &out, ArrayRef<std::string> plugins) {
  out << "OVERVIEW: LFort Static Analyzer Checkers List\n\n";
  out << "USAGE: -analyzer-checker <CHECKER or PACKAGE,...>\n\n";

  LFortCheckerRegistry(plugins).printHelp(out);
}
