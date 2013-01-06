#include "lfort-c/CXCompilationDatabase.h"
#include "CXString.h"
#include "lfort/Tooling/CompilationDatabase.h"

using namespace lfort;
using namespace lfort::tooling;
using namespace lfort::cxstring;

extern "C" {

// FIXME: do something more usefull with the error message
CXCompilationDatabase
lfort_CompilationDatabase_fromDirectory(const char *BuildDir,
                                        CXCompilationDatabase_Error *ErrorCode)
{
  std::string ErrorMsg;
  CXCompilationDatabase_Error Err = CXCompilationDatabase_NoError;

  CompilationDatabase *db = CompilationDatabase::loadFromDirectory(BuildDir,
                                                                   ErrorMsg);

  if (!db) {
    fprintf(stderr, "LIBLFORT TOOLING ERROR: %s\n", ErrorMsg.c_str());
    Err = CXCompilationDatabase_CanNotLoadDatabase;
  }

  if (ErrorCode)
    *ErrorCode = Err;

  return db;
}

void
lfort_CompilationDatabase_dispose(CXCompilationDatabase CDb)
{
  delete static_cast<CompilationDatabase *>(CDb);
}

struct AllocatedCXCompileCommands
{
  std::vector<CompileCommand> CCmd;

  AllocatedCXCompileCommands(const std::vector<CompileCommand>& Cmd)
    : CCmd(Cmd)
  { }
};

CXCompileCommands
lfort_CompilationDatabase_getCompileCommands(CXCompilationDatabase CDb,
                                             const char *CompleteFileName)
{
  if (CompilationDatabase *db = static_cast<CompilationDatabase *>(CDb)) {
    const std::vector<CompileCommand>
      CCmd(db->getCompileCommands(CompleteFileName));
    if (!CCmd.empty())
      return new AllocatedCXCompileCommands( CCmd );
  }

  return 0;
}

CXCompileCommands
lfort_CompilationDatabase_getAllCompileCommands(CXCompilationDatabase CDb) {
  if (CompilationDatabase *db = static_cast<CompilationDatabase *>(CDb)) {
    const std::vector<CompileCommand> CCmd(db->getAllCompileCommands());
    if (!CCmd.empty())
      return new AllocatedCXCompileCommands( CCmd );
  }

  return 0;
}

void
lfort_CompileCommands_dispose(CXCompileCommands Cmds)
{
  delete static_cast<AllocatedCXCompileCommands *>(Cmds);
}

unsigned
lfort_CompileCommands_getSize(CXCompileCommands Cmds)
{
  if (!Cmds)
    return 0;

  AllocatedCXCompileCommands *ACC =
    static_cast<AllocatedCXCompileCommands *>(Cmds);

  return ACC->CCmd.size();
}

CXCompileCommand
lfort_CompileCommands_getCommand(CXCompileCommands Cmds, unsigned I)
{
  if (!Cmds)
    return 0;

  AllocatedCXCompileCommands *ACC =
    static_cast<AllocatedCXCompileCommands *>(Cmds);

  if (I >= ACC->CCmd.size())
    return 0;

  return &ACC->CCmd[I];
}

CXString
lfort_CompileCommand_getDirectory(CXCompileCommand CCmd)
{
  if (!CCmd)
    return createCXString((const char*)NULL);

  CompileCommand *cmd = static_cast<CompileCommand *>(CCmd);
  return createCXString(cmd->Directory.c_str(), /*DupString=*/false);
}

unsigned
lfort_CompileCommand_getNumArgs(CXCompileCommand CCmd)
{
  if (!CCmd)
    return 0;

  return static_cast<CompileCommand *>(CCmd)->CommandLine.size();
}

CXString
lfort_CompileCommand_getArg(CXCompileCommand CCmd, unsigned Arg)
{
  if (!CCmd)
    return createCXString((const char*)NULL);

  CompileCommand *Cmd = static_cast<CompileCommand *>(CCmd);

  if (Arg >= Cmd->CommandLine.size())
    return createCXString((const char*)NULL);

  return createCXString(Cmd->CommandLine[Arg].c_str(), /*DupString=*/false);
}


} // end: extern "C"
