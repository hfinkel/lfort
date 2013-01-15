/* c-index-test.c */

#include "lfort-c/Index.h"
#include "lfort-c/CXCompilationDatabase.h"
#include "llvm/Config/config.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef LFORT_HAVE_LIBXML
#include <libxml/parser.h>
#include <libxml/relaxng.h>
#include <libxml/xmlerror.h>
#endif

#ifdef _WIN32
#  include <direct.h>
#else
#  include <unistd.h>
#endif

/******************************************************************************/
/* Utility functions.                                                         */
/******************************************************************************/

#ifdef _MSC_VER
char *basename(const char* path)
{
    char* base1 = (char*)strrchr(path, '/');
    char* base2 = (char*)strrchr(path, '\\');
    if (base1 && base2)
        return((base1 > base2) ? base1 + 1 : base2 + 1);
    else if (base1)
        return(base1 + 1);
    else if (base2)
        return(base2 + 1);

    return((char*)path);
}
char *dirname(char* path)
{
    char* base1 = (char*)strrchr(path, '/');
    char* base2 = (char*)strrchr(path, '\\');
    if (base1 && base2)
        if (base1 > base2)
          *base1 = 0;
        else
          *base2 = 0;
    else if (base1)
        *base1 = 0;
    else if (base2)
        *base2 = 0;

    return path;
}
#else
extern char *basename(const char *);
extern char *dirname(char *);
#endif

/** \brief Return the default parsing options. */
static unsigned getDefaultParsingOptions() {
  unsigned options = CXProgram_DetailedPreprocessingRecord;

  if (getenv("CINDEXTEST_EDITING"))
    options |= lfort_defaultEditingProgramOptions();
  if (getenv("CINDEXTEST_COMPLETION_CACHING"))
    options |= CXProgram_CacheCompletionResults;
  if (getenv("CINDEXTEST_COMPLETION_NO_CACHING"))
    options &= ~CXProgram_CacheCompletionResults;
  if (getenv("CINDEXTEST_SKIP_FUNCTION_BODIES"))
    options |= CXProgram_SkipSubprogramBodies;
  if (getenv("CINDEXTEST_COMPLETION_BRIEF_COMMENTS"))
    options |= CXProgram_IncludeBriefCommentsInCodeCompletion;
  
  return options;
}

static int checkForErrors(CXProgram Pgm);

static void PrintExtent(FILE *out, unsigned begin_line, unsigned begin_column,
                        unsigned end_line, unsigned end_column) {
  fprintf(out, "[%d:%d - %d:%d]", begin_line, begin_column,
          end_line, end_column);
}

static unsigned CreateProgram(CXIndex Idx, const char *file,
                                      CXProgram *Pgm) {

  *Pgm = lfort_createProgram(Idx, file);
  if (!*Pgm) {
    fprintf(stderr, "Unable to load translation unit from '%s'!\n", file);
    return 0;
  }
  return 1;
}

void free_remapped_files(struct CXUnsavedFile *unsaved_files,
                         int num_unsaved_files) {
  int i;
  for (i = 0; i != num_unsaved_files; ++i) {
    free((char *)unsaved_files[i].Filename);
    free((char *)unsaved_files[i].Contents);
  }
  free(unsaved_files);
}

int parse_remapped_files(int argc, const char **argv, int start_arg,
                         struct CXUnsavedFile **unsaved_files,
                         int *num_unsaved_files) {
  int i;
  int arg;
  int prefix_len = strlen("-remap-file=");
  *unsaved_files = 0;
  *num_unsaved_files = 0;

  /* Count the number of remapped files. */
  for (arg = start_arg; arg < argc; ++arg) {
    if (strncmp(argv[arg], "-remap-file=", prefix_len))
      break;

    ++*num_unsaved_files;
  }

  if (*num_unsaved_files == 0)
    return 0;

  *unsaved_files
    = (struct CXUnsavedFile *)malloc(sizeof(struct CXUnsavedFile) *
                                     *num_unsaved_files);
  for (arg = start_arg, i = 0; i != *num_unsaved_files; ++i, ++arg) {
    struct CXUnsavedFile *unsaved = *unsaved_files + i;
    const char *arg_string = argv[arg] + prefix_len;
    int filename_len;
    char *filename;
    char *contents;
    FILE *to_file;
    const char *semi = strchr(arg_string, ';');
    if (!semi) {
      fprintf(stderr,
              "error: -remap-file=from;to argument is missing semicolon\n");
      free_remapped_files(*unsaved_files, i);
      *unsaved_files = 0;
      *num_unsaved_files = 0;
      return -1;
    }

    /* Open the file that we're remapping to. */
    to_file = fopen(semi + 1, "rb");
    if (!to_file) {
      fprintf(stderr, "error: cannot open file %s that we are remapping to\n",
              semi + 1);
      free_remapped_files(*unsaved_files, i);
      *unsaved_files = 0;
      *num_unsaved_files = 0;
      return -1;
    }

    /* Determine the length of the file we're remapping to. */
    fseek(to_file, 0, SEEK_END);
    unsaved->Length = ftell(to_file);
    fseek(to_file, 0, SEEK_SET);

    /* Read the contents of the file we're remapping to. */
    contents = (char *)malloc(unsaved->Length + 1);
    if (fread(contents, 1, unsaved->Length, to_file) != unsaved->Length) {
      fprintf(stderr, "error: unexpected %s reading 'to' file %s\n",
              (feof(to_file) ? "EOF" : "error"), semi + 1);
      fclose(to_file);
      free_remapped_files(*unsaved_files, i);
      free(contents);
      *unsaved_files = 0;
      *num_unsaved_files = 0;
      return -1;
    }
    contents[unsaved->Length] = 0;
    unsaved->Contents = contents;

    /* Close the file. */
    fclose(to_file);

    /* Copy the file name that we're remapping from. */
    filename_len = semi - arg_string;
    filename = (char *)malloc(filename_len + 1);
    memcpy(filename, arg_string, filename_len);
    filename[filename_len] = 0;
    unsaved->Filename = filename;
  }

  return 0;
}

static const char *parse_comments_schema(int argc, const char **argv) {
  const char *CommentsSchemaArg = "-comments-xml-schema=";
  const char *CommentSchemaFile = NULL;

  if (argc == 0)
    return CommentSchemaFile;

  if (!strncmp(argv[0], CommentsSchemaArg, strlen(CommentsSchemaArg)))
    CommentSchemaFile = argv[0] + strlen(CommentsSchemaArg);

  return CommentSchemaFile;
}

/******************************************************************************/
/* Pretty-printing.                                                           */
/******************************************************************************/

static const char *FileCheckPrefix = "CHECK";

static void PrintCString(const char *CStr) {
  if (CStr != NULL && CStr[0] != '\0') {
    for ( ; *CStr; ++CStr) {
      const char C = *CStr;
      switch (C) {
        case '\n': printf("\\n"); break;
        case '\r': printf("\\r"); break;
        case '\t': printf("\\t"); break;
        case '\v': printf("\\v"); break;
        case '\f': printf("\\f"); break;
        default:   putchar(C);    break;
      }
    }
  }
}

static void PrintCStringWithPrefix(const char *Prefix, const char *CStr) {
  printf(" %s=[", Prefix);
  PrintCString(CStr);
  printf("]");
}

static void PrintCXStringAndDispose(CXString Str) {
  PrintCString(lfort_getCString(Str));
  lfort_disposeString(Str);
}

static void PrintCXStringWithPrefix(const char *Prefix, CXString Str) {
  PrintCStringWithPrefix(Prefix, lfort_getCString(Str));
}

static void PrintCXStringWithPrefixAndDispose(const char *Prefix,
                                              CXString Str) {
  PrintCStringWithPrefix(Prefix, lfort_getCString(Str));
  lfort_disposeString(Str);
}

static void PrintRange(CXSourceRange R, const char *str) {
  CXFile begin_file, end_file;
  unsigned begin_line, begin_column, end_line, end_column;

  lfort_getSpellingLocation(lfort_getRangeStart(R),
                            &begin_file, &begin_line, &begin_column, 0);
  lfort_getSpellingLocation(lfort_getRangeEnd(R),
                            &end_file, &end_line, &end_column, 0);
  if (!begin_file || !end_file)
    return;

  if (str)
    printf(" %s=", str);
  PrintExtent(stdout, begin_line, begin_column, end_line, end_column);
}

int want_display_name = 0;

static void printVersion(const char *Prefix, CXVersion Version) {
  if (Version.Major < 0)
    return;
  printf("%s%d", Prefix, Version.Major);
  
  if (Version.Minor < 0)
    return;
  printf(".%d", Version.Minor);

  if (Version.Subminor < 0)
    return;
  printf(".%d", Version.Subminor);
}

struct CommentASTDumpingContext {
  int IndentLevel;
};

static void DumpCXCommentInternal(struct CommentASTDumpingContext *Ctx,
                                  CXComment Comment) {
  unsigned i;
  unsigned e;
  enum CXCommentKind Kind = lfort_Comment_getKind(Comment);

  Ctx->IndentLevel++;
  for (i = 0, e = Ctx->IndentLevel; i != e; ++i)
    printf("  ");

  printf("(");
  switch (Kind) {
  case CXComment_Null:
    printf("CXComment_Null");
    break;
  case CXComment_Text:
    printf("CXComment_Text");
    PrintCXStringWithPrefixAndDispose("Text",
                                      lfort_TextComment_getText(Comment));
    if (lfort_Comment_isWhitespace(Comment))
      printf(" IsWhitespace");
    if (lfort_InlineContentComment_hasTrailingNewline(Comment))
      printf(" HasTrailingNewline");
    break;
  case CXComment_InlineCommand:
    printf("CXComment_InlineCommand");
    PrintCXStringWithPrefixAndDispose(
        "CommandName",
        lfort_InlineCommandComment_getCommandName(Comment));
    switch (lfort_InlineCommandComment_getRenderKind(Comment)) {
    case CXCommentInlineCommandRenderKind_Normal:
      printf(" RenderNormal");
      break;
    case CXCommentInlineCommandRenderKind_Bold:
      printf(" RenderBold");
      break;
    case CXCommentInlineCommandRenderKind_Monospaced:
      printf(" RenderMonospaced");
      break;
    case CXCommentInlineCommandRenderKind_Emphasized:
      printf(" RenderEmphasized");
      break;
    }
    for (i = 0, e = lfort_InlineCommandComment_getNumArgs(Comment);
         i != e; ++i) {
      printf(" Arg[%u]=", i);
      PrintCXStringAndDispose(
          lfort_InlineCommandComment_getArgText(Comment, i));
    }
    if (lfort_InlineContentComment_hasTrailingNewline(Comment))
      printf(" HasTrailingNewline");
    break;
  case CXComment_HTMLStartTag: {
    unsigned NumAttrs;
    printf("CXComment_HTMLStartTag");
    PrintCXStringWithPrefixAndDispose(
        "Name",
        lfort_HTMLTagComment_getTagName(Comment));
    NumAttrs = lfort_HTMLStartTag_getNumAttrs(Comment);
    if (NumAttrs != 0) {
      printf(" Attrs:");
      for (i = 0; i != NumAttrs; ++i) {
        printf(" ");
        PrintCXStringAndDispose(lfort_HTMLStartTag_getAttrName(Comment, i));
        printf("=");
        PrintCXStringAndDispose(lfort_HTMLStartTag_getAttrValue(Comment, i));
      }
    }
    if (lfort_HTMLStartTagComment_isSelfClosing(Comment))
      printf(" SelfClosing");
    if (lfort_InlineContentComment_hasTrailingNewline(Comment))
      printf(" HasTrailingNewline");
    break;
  }
  case CXComment_HTMLEndTag:
    printf("CXComment_HTMLEndTag");
    PrintCXStringWithPrefixAndDispose(
        "Name",
        lfort_HTMLTagComment_getTagName(Comment));
    if (lfort_InlineContentComment_hasTrailingNewline(Comment))
      printf(" HasTrailingNewline");
    break;
  case CXComment_Paragraph:
    printf("CXComment_Paragraph");
    if (lfort_Comment_isWhitespace(Comment))
      printf(" IsWhitespace");
    break;
  case CXComment_BlockCommand:
    printf("CXComment_BlockCommand");
    PrintCXStringWithPrefixAndDispose(
        "CommandName",
        lfort_BlockCommandComment_getCommandName(Comment));
    for (i = 0, e = lfort_BlockCommandComment_getNumArgs(Comment);
         i != e; ++i) {
      printf(" Arg[%u]=", i);
      PrintCXStringAndDispose(
          lfort_BlockCommandComment_getArgText(Comment, i));
    }
    break;
  case CXComment_ParamCommand:
    printf("CXComment_ParamCommand");
    switch (lfort_ParamCommandComment_getDirection(Comment)) {
    case CXCommentParamPassDirection_In:
      printf(" in");
      break;
    case CXCommentParamPassDirection_Out:
      printf(" out");
      break;
    case CXCommentParamPassDirection_InOut:
      printf(" in,out");
      break;
    }
    if (lfort_ParamCommandComment_isDirectionExplicit(Comment))
      printf(" explicitly");
    else
      printf(" implicitly");
    PrintCXStringWithPrefixAndDispose(
        "ParamName",
        lfort_ParamCommandComment_getParamName(Comment));
    if (lfort_ParamCommandComment_isParamIndexValid(Comment))
      printf(" ParamIndex=%u", lfort_ParamCommandComment_getParamIndex(Comment));
    else
      printf(" ParamIndex=Invalid");
    break;
  case CXComment_TParamCommand:
    printf("CXComment_TParamCommand");
    PrintCXStringWithPrefixAndDispose(
        "ParamName",
        lfort_TParamCommandComment_getParamName(Comment));
    if (lfort_TParamCommandComment_isParamPositionValid(Comment)) {
      printf(" ParamPosition={");
      for (i = 0, e = lfort_TParamCommandComment_getDepth(Comment);
           i != e; ++i) {
        printf("%u", lfort_TParamCommandComment_getIndex(Comment, i));
        if (i != e - 1)
          printf(", ");
      }
      printf("}");
    } else
      printf(" ParamPosition=Invalid");
    break;
  case CXComment_VerbatimBlockCommand:
    printf("CXComment_VerbatimBlockCommand");
    PrintCXStringWithPrefixAndDispose(
        "CommandName",
        lfort_BlockCommandComment_getCommandName(Comment));
    break;
  case CXComment_VerbatimBlockLine:
    printf("CXComment_VerbatimBlockLine");
    PrintCXStringWithPrefixAndDispose(
        "Text",
        lfort_VerbatimBlockLineComment_getText(Comment));
    break;
  case CXComment_VerbatimLine:
    printf("CXComment_VerbatimLine");
    PrintCXStringWithPrefixAndDispose(
        "Text",
        lfort_VerbatimLineComment_getText(Comment));
    break;
  case CXComment_FullComment:
    printf("CXComment_FullComment");
    break;
  }
  if (Kind != CXComment_Null) {
    const unsigned NumChildren = lfort_Comment_getNumChildren(Comment);
    unsigned i;
    for (i = 0; i != NumChildren; ++i) {
      printf("\n// %s: ", FileCheckPrefix);
      DumpCXCommentInternal(Ctx, lfort_Comment_getChild(Comment, i));
    }
  }
  printf(")");
  Ctx->IndentLevel--;
}

static void DumpCXComment(CXComment Comment) {
  struct CommentASTDumpingContext Ctx;
  Ctx.IndentLevel = 1;
  printf("\n// %s:  CommentAST=[\n// %s:", FileCheckPrefix, FileCheckPrefix);
  DumpCXCommentInternal(&Ctx, Comment);
  printf("]");
}

typedef struct {
  const char *CommentSchemaFile;
#ifdef LFORT_HAVE_LIBXML
  xmlRelaxNGParserCtxtPtr RNGParser;
  xmlRelaxNGPtr Schema;
#endif
} CommentXMLValidationData;

static void ValidateCommentXML(const char *Str,
                               CommentXMLValidationData *ValidationData) {
#ifdef LFORT_HAVE_LIBXML
  xmlDocPtr Doc;
  xmlRelaxNGValidCtxtPtr ValidationCtxt;
  int status;

  if (!ValidationData || !ValidationData->CommentSchemaFile)
    return;

  if (!ValidationData->RNGParser) {
    ValidationData->RNGParser =
        xmlRelaxNGNewParserCtxt(ValidationData->CommentSchemaFile);
    ValidationData->Schema = xmlRelaxNGParse(ValidationData->RNGParser);
  }
  if (!ValidationData->RNGParser) {
    printf(" libXMLError");
    return;
  }

  Doc = xmlParseDoc((const xmlChar *) Str);

  if (!Doc) {
    xmlErrorPtr Error = xmlGetLastError();
    printf(" CommentXMLInvalid [not well-formed XML: %s]", Error->message);
    return;
  }

  ValidationCtxt = xmlRelaxNGNewValidCtxt(ValidationData->Schema);
  status = xmlRelaxNGValidateDoc(ValidationCtxt, Doc);
  if (!status)
    printf(" CommentXMLValid");
  else if (status > 0) {
    xmlErrorPtr Error = xmlGetLastError();
    printf(" CommentXMLInvalid [not vaild XML: %s]", Error->message);
  } else
    printf(" libXMLError");

  xmlRelaxNGFreeValidCtxt(ValidationCtxt);
  xmlFreeDoc(Doc);
#endif
}

static void PrintCursorComments(CXCursor Cursor,
                                CommentXMLValidationData *ValidationData) {
  {
    CXString RawComment;
    const char *RawCommentCString;
    CXString BriefComment;
    const char *BriefCommentCString;

    RawComment = lfort_Cursor_getRawCommentText(Cursor);
    RawCommentCString = lfort_getCString(RawComment);
    if (RawCommentCString != NULL && RawCommentCString[0] != '\0') {
      PrintCStringWithPrefix("RawComment", RawCommentCString);
      PrintRange(lfort_Cursor_getCommentRange(Cursor), "RawCommentRange");

      BriefComment = lfort_Cursor_getBriefCommentText(Cursor);
      BriefCommentCString = lfort_getCString(BriefComment);
      if (BriefCommentCString != NULL && BriefCommentCString[0] != '\0')
        PrintCStringWithPrefix("BriefComment", BriefCommentCString);
      lfort_disposeString(BriefComment);
    }
    lfort_disposeString(RawComment);
  }

  {
    CXComment Comment = lfort_Cursor_getParsedComment(Cursor);
    if (lfort_Comment_getKind(Comment) != CXComment_Null) {
      PrintCXStringWithPrefixAndDispose("FullCommentAsHTML",
                                        lfort_FullComment_getAsHTML(Comment));
      {
        CXString XML;
        XML = lfort_FullComment_getAsXML(Comment);
        PrintCXStringWithPrefix("FullCommentAsXML", XML);
        ValidateCommentXML(lfort_getCString(XML), ValidationData);
        lfort_disposeString(XML);
      }

      DumpCXComment(Comment);
    }
  }
}

typedef struct {
  unsigned line;
  unsigned col;
} LineCol;

static int lineCol_cmp(const void *p1, const void *p2) {
  const LineCol *lhs = p1;
  const LineCol *rhs = p2;
  if (lhs->line != rhs->line)
    return (int)lhs->line - (int)rhs->line;
  return (int)lhs->col - (int)rhs->col;
}

static void PrintCursor(CXCursor Cursor,
                        CommentXMLValidationData *ValidationData) {
  CXProgram Pgm = lfort_Cursor_getProgram(Cursor);
  if (lfort_isInvalid(Cursor.kind)) {
    CXString ks = lfort_getCursorKindSpelling(Cursor.kind);
    printf("Invalid Cursor => %s", lfort_getCString(ks));
    lfort_disposeString(ks);
  }
  else {
    CXString string, ks;
    CXCursor Referenced;
    unsigned line, column;
    CXCursor SpecializationOf;
    CXCursor *overridden;
    unsigned num_overridden;
    unsigned RefNameRangeNr;
    CXSourceRange CursorExtent;
    CXSourceRange RefNameRange;
    int AlwaysUnavailable;
    int AlwaysDeprecated;
    CXString UnavailableMessage;
    CXString DeprecatedMessage;
    CXPlatformAvailability PlatformAvailability[2];
    int NumPlatformAvailability;
    int I;

    ks = lfort_getCursorKindSpelling(Cursor.kind);
    string = want_display_name? lfort_getCursorDisplayName(Cursor) 
                              : lfort_getCursorSpelling(Cursor);
    printf("%s=%s", lfort_getCString(ks),
                    lfort_getCString(string));
    lfort_disposeString(ks);
    lfort_disposeString(string);

    Referenced = lfort_getCursorReferenced(Cursor);
    if (!lfort_equalCursors(Referenced, lfort_getNullCursor())) {
      if (lfort_getCursorKind(Referenced) == CXCursor_OverloadedDeclRef) {
        unsigned I, N = lfort_getNumOverloadedDecls(Referenced);
        printf("[");
        for (I = 0; I != N; ++I) {
          CXCursor Ovl = lfort_getOverloadedDecl(Referenced, I);
          CXSourceLocation Loc;
          if (I)
            printf(", ");
          
          Loc = lfort_getCursorLocation(Ovl);
          lfort_getSpellingLocation(Loc, 0, &line, &column, 0);
          printf("%d:%d", line, column);          
        }
        printf("]");
      } else {
        CXSourceLocation Loc = lfort_getCursorLocation(Referenced);
        lfort_getSpellingLocation(Loc, 0, &line, &column, 0);
        printf(":%d:%d", line, column);
      }
    }

    if (lfort_isCursorDefinition(Cursor))
      printf(" (Definition)");
    
    switch (lfort_getCursorAvailability(Cursor)) {
      case CXAvailability_Available:
        break;
        
      case CXAvailability_Deprecated:
        printf(" (deprecated)");
        break;
        
      case CXAvailability_NotAvailable:
        printf(" (unavailable)");
        break;

      case CXAvailability_NotAccessible:
        printf(" (inaccessible)");
        break;
    }
    
    NumPlatformAvailability
      = lfort_getCursorPlatformAvailability(Cursor,
                                            &AlwaysDeprecated,
                                            &DeprecatedMessage,
                                            &AlwaysUnavailable,
                                            &UnavailableMessage,
                                            PlatformAvailability, 2);
    if (AlwaysUnavailable) {
      printf("  (always unavailable: \"%s\")",
             lfort_getCString(UnavailableMessage));
    } else if (AlwaysDeprecated) {
      printf("  (always deprecated: \"%s\")",
             lfort_getCString(DeprecatedMessage));
    } else {
      for (I = 0; I != NumPlatformAvailability; ++I) {
        if (I >= 2)
          break;
        
        printf("  (%s", lfort_getCString(PlatformAvailability[I].Platform));
        if (PlatformAvailability[I].Unavailable)
          printf(", unavailable");
        else {
          printVersion(", introduced=", PlatformAvailability[I].Introduced);
          printVersion(", deprecated=", PlatformAvailability[I].Deprecated);
          printVersion(", obsoleted=", PlatformAvailability[I].Obsoleted);
        }
        if (lfort_getCString(PlatformAvailability[I].Message)[0])
          printf(", message=\"%s\"",
                 lfort_getCString(PlatformAvailability[I].Message));
        printf(")");
      }
    }
    for (I = 0; I != NumPlatformAvailability; ++I) {
      if (I >= 2)
        break;
      lfort_disposeCXPlatformAvailability(PlatformAvailability + I);
    }
    
    lfort_disposeString(DeprecatedMessage);
    lfort_disposeString(UnavailableMessage);
    
    if (lfort_CXXMethod_isStatic(Cursor))
      printf(" (static)");
    if (lfort_CXXMethod_isVirtual(Cursor))
      printf(" (virtual)");
    
    if (Cursor.kind == CXCursor_IBOutletCollectionAttr) {
      CXType T =
        lfort_getCanonicalType(lfort_getIBOutletCollectionType(Cursor));
      CXString S = lfort_getTypeKindSpelling(T.kind);
      printf(" [IBOutletCollection=%s]", lfort_getCString(S));
      lfort_disposeString(S);
    }
    
    if (Cursor.kind == CXCursor_CXXBaseSpecifier) {
      enum CX_CXXAccessSpecifier access = lfort_getCXXAccessSpecifier(Cursor);
      unsigned isVirtual = lfort_isVirtualBase(Cursor);
      const char *accessStr = 0;

      switch (access) {
        case CX_CXXInvalidAccessSpecifier:
          accessStr = "invalid"; break;
        case CX_CXXPublic:
          accessStr = "public"; break;
        case CX_CXXProtected:
          accessStr = "protected"; break;
        case CX_CXXPrivate:
          accessStr = "private"; break;
      }      
      
      printf(" [access=%s isVirtual=%s]", accessStr,
             isVirtual ? "true" : "false");
    }
    
    SpecializationOf = lfort_getSpecializedCursorTemplate(Cursor);
    if (!lfort_equalCursors(SpecializationOf, lfort_getNullCursor())) {
      CXSourceLocation Loc = lfort_getCursorLocation(SpecializationOf);
      CXString Name = lfort_getCursorSpelling(SpecializationOf);
      lfort_getSpellingLocation(Loc, 0, &line, &column, 0);
      printf(" [Specialization of %s:%d:%d]", 
             lfort_getCString(Name), line, column);
      lfort_disposeString(Name);
    }

    lfort_getOverriddenCursors(Cursor, &overridden, &num_overridden);
    if (num_overridden) {      
      unsigned I;
      LineCol lineCols[50];
      assert(num_overridden <= 50);
      printf(" [Overrides ");
      for (I = 0; I != num_overridden; ++I) {
        CXSourceLocation Loc = lfort_getCursorLocation(overridden[I]);
        lfort_getSpellingLocation(Loc, 0, &line, &column, 0);
        lineCols[I].line = line;
        lineCols[I].col = column;
      }
      /* Make the order of the override list deterministic. */
      qsort(lineCols, num_overridden, sizeof(LineCol), lineCol_cmp);
      for (I = 0; I != num_overridden; ++I) {
        if (I)
          printf(", ");
        printf("@%d:%d", lineCols[I].line, lineCols[I].col);
      }
      printf("]");
      lfort_disposeOverriddenCursors(overridden);
    }
    
    if (Cursor.kind == CXCursor_InclusionDirective) {
      CXFile File = lfort_getIncludedFile(Cursor);
      CXString Included = lfort_getFileName(File);
      printf(" (%s)", lfort_getCString(Included));
      lfort_disposeString(Included);
      
      if (lfort_isFileMultipleIncludeGuarded(Pgm, File))
        printf("  [multi-include guarded]");
    }
    
    CursorExtent = lfort_getCursorExtent(Cursor);
    RefNameRange = lfort_getCursorReferenceNameRange(Cursor, 
                                                   CXNameRange_WantQualifier
                                                 | CXNameRange_WantSinglePiece
                                                 | CXNameRange_WantTemplateArgs,
                                                     0);
    if (!lfort_equalRanges(CursorExtent, RefNameRange))
      PrintRange(RefNameRange, "SingleRefName");
    
    for (RefNameRangeNr = 0; 1; RefNameRangeNr++) {
      RefNameRange = lfort_getCursorReferenceNameRange(Cursor, 
                                                   CXNameRange_WantQualifier
                                                 | CXNameRange_WantTemplateArgs,
                                                       RefNameRangeNr);
      if (lfort_equalRanges(lfort_getNullRange(), RefNameRange))
        break;
      if (!lfort_equalRanges(CursorExtent, RefNameRange))
        PrintRange(RefNameRange, "RefName");
    }

    PrintCursorComments(Cursor, ValidationData);
  }
}

static const char* GetCursorSource(CXCursor Cursor) {
  CXSourceLocation Loc = lfort_getCursorLocation(Cursor);
  CXString source;
  CXFile file;
  lfort_getExpansionLocation(Loc, &file, 0, 0, 0);
  source = lfort_getFileName(file);
  if (!lfort_getCString(source)) {
    lfort_disposeString(source);
    return "<invalid loc>";
  }
  else {
    const char *b = basename(lfort_getCString(source));
    lfort_disposeString(source);
    return b;
  }
}

/******************************************************************************/
/* Callbacks.                                                                 */
/******************************************************************************/

typedef void (*PostVisitPgm)(CXProgram);

void PrintDiagnostic(CXDiagnostic Diagnostic) {
  FILE *out = stderr;
  CXFile file;
  CXString Msg;
  unsigned display_opts = CXDiagnostic_DisplaySourceLocation
    | CXDiagnostic_DisplayColumn | CXDiagnostic_DisplaySourceRanges
    | CXDiagnostic_DisplayOption;
  unsigned i, num_fixits;

  if (lfort_getDiagnosticSeverity(Diagnostic) == CXDiagnostic_Ignored)
    return;

  Msg = lfort_formatDiagnostic(Diagnostic, display_opts);
  fprintf(stderr, "%s\n", lfort_getCString(Msg));
  lfort_disposeString(Msg);

  lfort_getSpellingLocation(lfort_getDiagnosticLocation(Diagnostic),
                            &file, 0, 0, 0);
  if (!file)
    return;

  num_fixits = lfort_getDiagnosticNumFixIts(Diagnostic);
  fprintf(stderr, "Number FIX-ITs = %d\n", num_fixits);
  for (i = 0; i != num_fixits; ++i) {
    CXSourceRange range;
    CXString insertion_text = lfort_getDiagnosticFixIt(Diagnostic, i, &range);
    CXSourceLocation start = lfort_getRangeStart(range);
    CXSourceLocation end = lfort_getRangeEnd(range);
    unsigned start_line, start_column, end_line, end_column;
    CXFile start_file, end_file;
    lfort_getSpellingLocation(start, &start_file, &start_line,
                              &start_column, 0);
    lfort_getSpellingLocation(end, &end_file, &end_line, &end_column, 0);
    if (lfort_equalLocations(start, end)) {
      /* Insertion. */
      if (start_file == file)
        fprintf(out, "FIX-IT: Insert \"%s\" at %d:%d\n",
                lfort_getCString(insertion_text), start_line, start_column);
    } else if (strcmp(lfort_getCString(insertion_text), "") == 0) {
      /* Removal. */
      if (start_file == file && end_file == file) {
        fprintf(out, "FIX-IT: Remove ");
        PrintExtent(out, start_line, start_column, end_line, end_column);
        fprintf(out, "\n");
      }
    } else {
      /* Replacement. */
      if (start_file == end_file) {
        fprintf(out, "FIX-IT: Replace ");
        PrintExtent(out, start_line, start_column, end_line, end_column);
        fprintf(out, " with \"%s\"\n", lfort_getCString(insertion_text));
      }
      break;
    }
    lfort_disposeString(insertion_text);
  }
}

void PrintDiagnosticSet(CXDiagnosticSet Set) {
  int i = 0, n = lfort_getNumDiagnosticsInSet(Set);
  for ( ; i != n ; ++i) {
    CXDiagnostic Diag = lfort_getDiagnosticInSet(Set, i);
    CXDiagnosticSet ChildDiags = lfort_getChildDiagnostics(Diag);
    PrintDiagnostic(Diag);
    if (ChildDiags)
      PrintDiagnosticSet(ChildDiags);
  }  
}

void PrintDiagnostics(CXProgram Pgm) {
  CXDiagnosticSet PgmSet = lfort_getDiagnosticSetFromPgm(Pgm);
  PrintDiagnosticSet(PgmSet);
  lfort_disposeDiagnosticSet(PgmSet);
}

void PrintMemoryUsage(CXProgram Pgm) {
  unsigned long total = 0;
  unsigned i = 0;
  CXPgmResourceUsage usage = lfort_getCXPgmResourceUsage(Pgm);
  fprintf(stderr, "Memory usage:\n");
  for (i = 0 ; i != usage.numEntries; ++i) {
    const char *name = lfort_getPgmResourceUsageName(usage.entries[i].kind);
    unsigned long amount = usage.entries[i].amount;
    total += amount;
    fprintf(stderr, "  %s : %ld bytes (%f MBytes)\n", name, amount,
            ((double) amount)/(1024*1024));
  }
  fprintf(stderr, "  TOTAL = %ld bytes (%f MBytes)\n", total,
          ((double) total)/(1024*1024));
  lfort_disposeCXPgmResourceUsage(usage);  
}

/******************************************************************************/
/* Logic for testing traversal.                                               */
/******************************************************************************/

static void PrintCursorExtent(CXCursor C) {
  CXSourceRange extent = lfort_getCursorExtent(C);
  PrintRange(extent, "Extent");
}

/* Data used by the visitors. */
typedef struct {
  CXProgram Pgm;
  enum CXCursorKind *Filter;
  CommentXMLValidationData ValidationData;
} VisitorData;


enum CXChildVisitResult FilteredPrintingVisitor(CXCursor Cursor,
                                                CXCursor Parent,
                                                CXClientData ClientData) {
  VisitorData *Data = (VisitorData *)ClientData;
  if (!Data->Filter || (Cursor.kind == *(enum CXCursorKind *)Data->Filter)) {
    CXSourceLocation Loc = lfort_getCursorLocation(Cursor);
    unsigned line, column;
    lfort_getSpellingLocation(Loc, 0, &line, &column, 0);
    printf("// %s: %s:%d:%d: ", FileCheckPrefix,
           GetCursorSource(Cursor), line, column);
    PrintCursor(Cursor, &Data->ValidationData);
    PrintCursorExtent(Cursor);
    printf("\n");
    return CXChildVisit_Recurse;
  }

  return CXChildVisit_Continue;
}

static enum CXChildVisitResult SubprogramScanVisitor(CXCursor Cursor,
                                                   CXCursor Parent,
                                                   CXClientData ClientData) {
  const char *startBuf, *endBuf;
  unsigned startLine, startColumn, endLine, endColumn, curLine, curColumn;
  CXCursor Ref;
  VisitorData *Data = (VisitorData *)ClientData;

  if (Cursor.kind != CXCursor_SubprogramDecl ||
      !lfort_isCursorDefinition(Cursor))
    return CXChildVisit_Continue;

  lfort_getDefinitionSpellingAndExtent(Cursor, &startBuf, &endBuf,
                                       &startLine, &startColumn,
                                       &endLine, &endColumn);
  /* Probe the entire body, looking for both decls and refs. */
  curLine = startLine;
  curColumn = startColumn;

  while (startBuf < endBuf) {
    CXSourceLocation Loc;
    CXFile file;
    CXString source;

    if (*startBuf == '\n') {
      startBuf++;
      curLine++;
      curColumn = 1;
    } else if (*startBuf != '\t')
      curColumn++;

    Loc = lfort_getCursorLocation(Cursor);
    lfort_getSpellingLocation(Loc, &file, 0, 0, 0);

    source = lfort_getFileName(file);
    if (lfort_getCString(source)) {
      CXSourceLocation RefLoc
        = lfort_getLocation(Data->Pgm, file, curLine, curColumn);
      Ref = lfort_getCursor(Data->Pgm, RefLoc);
      if (Ref.kind == CXCursor_NoDeclFound) {
        /* Nothing found here; that's fine. */
      } else if (Ref.kind != CXCursor_SubprogramDecl) {
        printf("// %s: %s:%d:%d: ", FileCheckPrefix, GetCursorSource(Ref),
               curLine, curColumn);
        PrintCursor(Ref, &Data->ValidationData);
        printf("\n");
      }
    }
    lfort_disposeString(source);
    startBuf++;
  }

  return CXChildVisit_Continue;
}

/******************************************************************************/
/* USR testing.                                                               */
/******************************************************************************/

enum CXChildVisitResult USRVisitor(CXCursor C, CXCursor parent,
                                   CXClientData ClientData) {
  VisitorData *Data = (VisitorData *)ClientData;
  if (!Data->Filter || (C.kind == *(enum CXCursorKind *)Data->Filter)) {
    CXString USR = lfort_getCursorUSR(C);
    const char *cstr = lfort_getCString(USR);
    if (!cstr || cstr[0] == '\0') {
      lfort_disposeString(USR);
      return CXChildVisit_Recurse;
    }
    printf("// %s: %s %s", FileCheckPrefix, GetCursorSource(C), cstr);

    PrintCursorExtent(C);
    printf("\n");
    lfort_disposeString(USR);

    return CXChildVisit_Recurse;
  }

  return CXChildVisit_Continue;
}

/******************************************************************************/
/* Inclusion stack testing.                                                   */
/******************************************************************************/

void InclusionVisitor(CXFile includedFile, CXSourceLocation *includeStack,
                      unsigned includeStackLen, CXClientData data) {

  unsigned i;
  CXString fname;

  fname = lfort_getFileName(includedFile);
  printf("file: %s\nincluded by:\n", lfort_getCString(fname));
  lfort_disposeString(fname);

  for (i = 0; i < includeStackLen; ++i) {
    CXFile includingFile;
    unsigned line, column;
    lfort_getSpellingLocation(includeStack[i], &includingFile, &line,
                              &column, 0);
    fname = lfort_getFileName(includingFile);
    printf("  %s:%d:%d\n", lfort_getCString(fname), line, column);
    lfort_disposeString(fname);
  }
  printf("\n");
}

void PrintInclusionStack(CXProgram Pgm) {
  lfort_getInclusions(Pgm, InclusionVisitor, NULL);
}

/******************************************************************************/
/* Linkage testing.                                                           */
/******************************************************************************/

static enum CXChildVisitResult PrintLinkage(CXCursor cursor, CXCursor p,
                                            CXClientData d) {
  const char *linkage = 0;

  if (lfort_isInvalid(lfort_getCursorKind(cursor)))
    return CXChildVisit_Recurse;

  switch (lfort_getCursorLinkage(cursor)) {
    case CXLinkage_Invalid: break;
    case CXLinkage_NoLinkage: linkage = "NoLinkage"; break;
    case CXLinkage_Internal: linkage = "Internal"; break;
    case CXLinkage_UniqueExternal: linkage = "UniqueExternal"; break;
    case CXLinkage_External: linkage = "External"; break;
  }

  if (linkage) {
    PrintCursor(cursor, NULL);
    printf("linkage=%s\n", linkage);
  }

  return CXChildVisit_Recurse;
}

/******************************************************************************/
/* Typekind testing.                                                          */
/******************************************************************************/

static enum CXChildVisitResult PrintTypeKind(CXCursor cursor, CXCursor p,
                                             CXClientData d) {
  if (!lfort_isInvalid(lfort_getCursorKind(cursor))) {
    CXType T = lfort_getCursorType(cursor);
    CXString S = lfort_getTypeKindSpelling(T.kind);
    PrintCursor(cursor, NULL);
    printf(" typekind=%s", lfort_getCString(S));
    if (lfort_isConstQualifiedType(T))
      printf(" const");
    if (lfort_isVolatileQualifiedType(T))
      printf(" volatile");
    if (lfort_isRestrictQualifiedType(T))
      printf(" restrict");
    lfort_disposeString(S);
    /* Print the canonical type if it is different. */
    {
      CXType CT = lfort_getCanonicalType(T);
      if (!lfort_equalTypes(T, CT)) {
        CXString CS = lfort_getTypeKindSpelling(CT.kind);
        printf(" [canonical=%s]", lfort_getCString(CS));
        lfort_disposeString(CS);
      }
    }
    /* Print the return type if it exists. */
    {
      CXType RT = lfort_getCursorResultType(cursor);
      if (RT.kind != CXType_Invalid) {
        CXString RS = lfort_getTypeKindSpelling(RT.kind);
        printf(" [result=%s]", lfort_getCString(RS));
        lfort_disposeString(RS);
      }
    }
    /* Print the argument types if they exist. */
    {
      int numArgs = lfort_Cursor_getNumArguments(cursor);
      if (numArgs != -1 && numArgs != 0) {
        int i;
        printf(" [args=");
        for (i = 0; i < numArgs; ++i) {
          CXType T = lfort_getCursorType(lfort_Cursor_getArgument(cursor, i));
          if (T.kind != CXType_Invalid) {
            CXString S = lfort_getTypeKindSpelling(T.kind);
            printf(" %s", lfort_getCString(S));
            lfort_disposeString(S);
          }
        }
        printf("]");
      }
    }
    /* Print if this is a non-POD type. */
    printf(" [isPOD=%d]", lfort_isPODType(T));

    printf("\n");
  }
  return CXChildVisit_Recurse;
}

/******************************************************************************/
/* Bitwidth testing.                                                          */
/******************************************************************************/

static enum CXChildVisitResult PrintBitWidth(CXCursor cursor, CXCursor p,
                                             CXClientData d) {
  int Bitwidth;
  if (lfort_getCursorKind(cursor) != CXCursor_FieldDecl)
    return CXChildVisit_Recurse;

  Bitwidth = lfort_getFieldDeclBitWidth(cursor);
  if (Bitwidth >= 0) {
    PrintCursor(cursor, NULL);
    printf(" bitwidth=%d\n", Bitwidth);
  }

  return CXChildVisit_Recurse;
}

/******************************************************************************/
/* Loading ASTs/source.                                                       */
/******************************************************************************/

static int perform_test_load(CXIndex Idx, CXProgram Pgm,
                             const char *filter, const char *prefix,
                             CXCursorVisitor Visitor,
                             PostVisitPgm PV,
                             const char *CommentSchemaFile) {

  if (prefix)
    FileCheckPrefix = prefix;

  if (Visitor) {
    enum CXCursorKind K = CXCursor_NotImplemented;
    enum CXCursorKind *ck = &K;
    VisitorData Data;

    /* Perform some simple filtering. */
    if (!strcmp(filter, "all") || !strcmp(filter, "local")) ck = NULL;
    else if (!strcmp(filter, "all-display") || 
             !strcmp(filter, "local-display")) {
      ck = NULL;
      want_display_name = 1;
    }
    else if (!strcmp(filter, "none")) K = (enum CXCursorKind) ~0;
    else if (!strcmp(filter, "category")) K = CXCursor_ObjCCategoryDecl;
    else if (!strcmp(filter, "interface")) K = CXCursor_ObjCInterfaceDecl;
    else if (!strcmp(filter, "protocol")) K = CXCursor_ObjCProtocolDecl;
    else if (!strcmp(filter, "function")) K = CXCursor_SubprogramDecl;
    else if (!strcmp(filter, "typedef")) K = CXCursor_TypedefDecl;
    else if (!strcmp(filter, "scan-function")) Visitor = SubprogramScanVisitor;
    else {
      fprintf(stderr, "Unknown filter for -test-load-tu: %s\n", filter);
      return 1;
    }

    Data.Pgm = Pgm;
    Data.Filter = ck;
    Data.ValidationData.CommentSchemaFile = CommentSchemaFile;
#ifdef LFORT_HAVE_LIBXML
    Data.ValidationData.RNGParser = NULL;
    Data.ValidationData.Schema = NULL;
#endif
    lfort_visitChildren(lfort_getProgramCursor(Pgm), Visitor, &Data);
  }

  if (PV)
    PV(Pgm);

  PrintDiagnostics(Pgm);
  if (checkForErrors(Pgm) != 0) {
    lfort_disposeProgram(Pgm);
    return -1;
  }

  lfort_disposeProgram(Pgm);
  return 0;
}

int perform_test_load_tu(const char *file, const char *filter,
                         const char *prefix, CXCursorVisitor Visitor,
                         PostVisitPgm PV) {
  CXIndex Idx;
  CXProgram Pgm;
  int result;
  Idx = lfort_createIndex(/* excludeDeclsFromPCH */
                          !strcmp(filter, "local") ? 1 : 0,
                          /* displayDiagnosics=*/1);

  if (!CreateProgram(Idx, file, &Pgm)) {
    lfort_disposeIndex(Idx);
    return 1;
  }

  result = perform_test_load(Idx, Pgm, filter, prefix, Visitor, PV, NULL);
  lfort_disposeIndex(Idx);
  return result;
}

int perform_test_load_source(int argc, const char **argv,
                             const char *filter, CXCursorVisitor Visitor,
                             PostVisitPgm PV) {
  CXIndex Idx;
  CXProgram Pgm;
  const char *CommentSchemaFile;
  struct CXUnsavedFile *unsaved_files = 0;
  int num_unsaved_files = 0;
  int result;
  
  Idx = lfort_createIndex(/* excludeDeclsFromPCH */
                          (!strcmp(filter, "local") || 
                           !strcmp(filter, "local-display"))? 1 : 0,
                          /* displayDiagnosics=*/0);

  if ((CommentSchemaFile = parse_comments_schema(argc, argv))) {
    argc--;
    argv++;
  }

  if (parse_remapped_files(argc, argv, 0, &unsaved_files, &num_unsaved_files)) {
    lfort_disposeIndex(Idx);
    return -1;
  }

  Pgm = lfort_parseProgram(Idx, 0,
                                  argv + num_unsaved_files,
                                  argc - num_unsaved_files,
                                  unsaved_files, num_unsaved_files, 
                                  getDefaultParsingOptions());
  if (!Pgm) {
    fprintf(stderr, "Unable to load translation unit!\n");
    free_remapped_files(unsaved_files, num_unsaved_files);
    lfort_disposeIndex(Idx);
    return 1;
  }

  result = perform_test_load(Idx, Pgm, filter, NULL, Visitor, PV,
                             CommentSchemaFile);
  free_remapped_files(unsaved_files, num_unsaved_files);
  lfort_disposeIndex(Idx);
  return result;
}

int perform_test_reparse_source(int argc, const char **argv, int trials,
                                const char *filter, CXCursorVisitor Visitor,
                                PostVisitPgm PV) {
  CXIndex Idx;
  CXProgram Pgm;
  struct CXUnsavedFile *unsaved_files = 0;
  int num_unsaved_files = 0;
  int result;
  int trial;
  int remap_after_trial = 0;
  char *endptr = 0;
  
  Idx = lfort_createIndex(/* excludeDeclsFromPCH */
                          !strcmp(filter, "local") ? 1 : 0,
                          /* displayDiagnosics=*/0);
  
  if (parse_remapped_files(argc, argv, 0, &unsaved_files, &num_unsaved_files)) {
    lfort_disposeIndex(Idx);
    return -1;
  }
  
  /* Load the initial translation unit -- we do this without honoring remapped
   * files, so that we have a way to test results after changing the source. */
  Pgm = lfort_parseProgram(Idx, 0,
                                  argv + num_unsaved_files,
                                  argc - num_unsaved_files,
                                  0, 0, getDefaultParsingOptions());
  if (!Pgm) {
    fprintf(stderr, "Unable to load translation unit!\n");
    free_remapped_files(unsaved_files, num_unsaved_files);
    lfort_disposeIndex(Idx);
    return 1;
  }
  
  if (checkForErrors(Pgm) != 0)
    return -1;

  if (getenv("CINDEXTEST_REMAP_AFTER_TRIAL")) {
    remap_after_trial =
        strtol(getenv("CINDEXTEST_REMAP_AFTER_TRIAL"), &endptr, 10);
  }

  for (trial = 0; trial < trials; ++trial) {
    if (lfort_reparseProgram(Pgm,
                             trial >= remap_after_trial ? num_unsaved_files : 0,
                             trial >= remap_after_trial ? unsaved_files : 0,
                                     lfort_defaultReparseOptions(Pgm))) {
      fprintf(stderr, "Unable to reparse translation unit!\n");
      lfort_disposeProgram(Pgm);
      free_remapped_files(unsaved_files, num_unsaved_files);
      lfort_disposeIndex(Idx);
      return -1;      
    }

    if (checkForErrors(Pgm) != 0)
      return -1;
  }
  
  result = perform_test_load(Idx, Pgm, filter, NULL, Visitor, PV, NULL);

  free_remapped_files(unsaved_files, num_unsaved_files);
  lfort_disposeIndex(Idx);
  return result;
}

/******************************************************************************/
/* Logic for testing lfort_getCursor().                                       */
/******************************************************************************/

static void print_cursor_file_scan(CXProgram Pgm, CXCursor cursor,
                                   unsigned start_line, unsigned start_col,
                                   unsigned end_line, unsigned end_col,
                                   const char *prefix) {
  printf("// %s: ", FileCheckPrefix);
  if (prefix)
    printf("-%s", prefix);
  PrintExtent(stdout, start_line, start_col, end_line, end_col);
  printf(" ");
  PrintCursor(cursor, NULL);
  printf("\n");
}

static int perform_file_scan(const char *ast_file, const char *source_file,
                             const char *prefix) {
  CXIndex Idx;
  CXProgram Pgm;
  FILE *fp;
  CXCursor prevCursor = lfort_getNullCursor();
  CXFile file;
  unsigned line = 1, col = 1;
  unsigned start_line = 1, start_col = 1;

  if (!(Idx = lfort_createIndex(/* excludeDeclsFromPCH */ 1,
                                /* displayDiagnosics=*/1))) {
    fprintf(stderr, "Could not create Index\n");
    return 1;
  }

  if (!CreateProgram(Idx, ast_file, &Pgm))
    return 1;

  if ((fp = fopen(source_file, "r")) == NULL) {
    fprintf(stderr, "Could not open '%s'\n", source_file);
    return 1;
  }

  file = lfort_getFile(Pgm, source_file);
  for (;;) {
    CXCursor cursor;
    int c = fgetc(fp);

    if (c == '\n') {
      ++line;
      col = 1;
    } else
      ++col;

    /* Check the cursor at this position, and dump the previous one if we have
     * found something new.
     */
    cursor = lfort_getCursor(Pgm, lfort_getLocation(Pgm, file, line, col));
    if ((c == EOF || !lfort_equalCursors(cursor, prevCursor)) &&
        prevCursor.kind != CXCursor_InvalidFile) {
      print_cursor_file_scan(Pgm, prevCursor, start_line, start_col,
                             line, col, prefix);
      start_line = line;
      start_col = col;
    }
    if (c == EOF)
      break;

    prevCursor = cursor;
  }

  fclose(fp);
  lfort_disposeProgram(Pgm);
  lfort_disposeIndex(Idx);
  return 0;
}

/******************************************************************************/
/* Logic for testing lfort code completion.                                   */
/******************************************************************************/

/* Parse file:line:column from the input string. Returns 0 on success, non-zero
   on failure. If successful, the pointer *filename will contain newly-allocated
   memory (that will be owned by the caller) to store the file name. */
int parse_file_line_column(const char *input, char **filename, unsigned *line,
                           unsigned *column, unsigned *second_line,
                           unsigned *second_column) {
  /* Find the second colon. */
  const char *last_colon = strrchr(input, ':');
  unsigned values[4], i;
  unsigned num_values = (second_line && second_column)? 4 : 2;

  char *endptr = 0;
  if (!last_colon || last_colon == input) {
    if (num_values == 4)
      fprintf(stderr, "could not parse filename:line:column:line:column in "
              "'%s'\n", input);
    else
      fprintf(stderr, "could not parse filename:line:column in '%s'\n", input);
    return 1;
  }

  for (i = 0; i != num_values; ++i) {
    const char *prev_colon;

    /* Parse the next line or column. */
    values[num_values - i - 1] = strtol(last_colon + 1, &endptr, 10);
    if (*endptr != 0 && *endptr != ':') {
      fprintf(stderr, "could not parse %s in '%s'\n",
              (i % 2 ? "column" : "line"), input);
      return 1;
    }

    if (i + 1 == num_values)
      break;

    /* Find the previous colon. */
    prev_colon = last_colon - 1;
    while (prev_colon != input && *prev_colon != ':')
      --prev_colon;
    if (prev_colon == input) {
      fprintf(stderr, "could not parse %s in '%s'\n",
              (i % 2 == 0? "column" : "line"), input);
      return 1;
    }

    last_colon = prev_colon;
  }

  *line = values[0];
  *column = values[1];

  if (second_line && second_column) {
    *second_line = values[2];
    *second_column = values[3];
  }

  /* Copy the file name. */
  *filename = (char*)malloc(last_colon - input + 1);
  memcpy(*filename, input, last_colon - input);
  (*filename)[last_colon - input] = 0;
  return 0;
}

const char *
lfort_getCompletionChunkKindSpelling(enum CXCompletionChunkKind Kind) {
  switch (Kind) {
  case CXCompletionChunk_Optional: return "Optional";
  case CXCompletionChunk_TypedText: return "TypedText";
  case CXCompletionChunk_Text: return "Text";
  case CXCompletionChunk_Placeholder: return "Placeholder";
  case CXCompletionChunk_Informative: return "Informative";
  case CXCompletionChunk_CurrentParameter: return "CurrentParameter";
  case CXCompletionChunk_LeftParen: return "LeftParen";
  case CXCompletionChunk_RightParen: return "RightParen";
  case CXCompletionChunk_LeftBracket: return "LeftBracket";
  case CXCompletionChunk_RightBracket: return "RightBracket";
  case CXCompletionChunk_LeftBrace: return "LeftBrace";
  case CXCompletionChunk_RightBrace: return "RightBrace";
  case CXCompletionChunk_LeftAngle: return "LeftAngle";
  case CXCompletionChunk_RightAngle: return "RightAngle";
  case CXCompletionChunk_Comma: return "Comma";
  case CXCompletionChunk_ResultType: return "ResultType";
  case CXCompletionChunk_Colon: return "Colon";
  case CXCompletionChunk_SemiColon: return "SemiColon";
  case CXCompletionChunk_Equal: return "Equal";
  case CXCompletionChunk_HorizontalSpace: return "HorizontalSpace";
  case CXCompletionChunk_VerticalSpace: return "VerticalSpace";
  }

  return "Unknown";
}

static int checkForErrors(CXProgram Pgm) {
  unsigned Num, i;
  CXDiagnostic Diag;
  CXString DiagStr;

  if (!getenv("CINDEXTEST_FAILONERROR"))
    return 0;

  Num = lfort_getNumDiagnostics(Pgm);
  for (i = 0; i != Num; ++i) {
    Diag = lfort_getDiagnostic(Pgm, i);
    if (lfort_getDiagnosticSeverity(Diag) >= CXDiagnostic_Error) {
      DiagStr = lfort_formatDiagnostic(Diag,
                                       lfort_defaultDiagnosticDisplayOptions());
      fprintf(stderr, "%s\n", lfort_getCString(DiagStr));
      lfort_disposeString(DiagStr);
      lfort_disposeDiagnostic(Diag);
      return -1;
    }
    lfort_disposeDiagnostic(Diag);
  }

  return 0;
}

void print_completion_string(CXCompletionString completion_string, FILE *file) {
  int I, N;

  N = lfort_getNumCompletionChunks(completion_string);
  for (I = 0; I != N; ++I) {
    CXString text;
    const char *cstr;
    enum CXCompletionChunkKind Kind
      = lfort_getCompletionChunkKind(completion_string, I);

    if (Kind == CXCompletionChunk_Optional) {
      fprintf(file, "{Optional ");
      print_completion_string(
                lfort_getCompletionChunkCompletionString(completion_string, I),
                              file);
      fprintf(file, "}");
      continue;
    } 

    if (Kind == CXCompletionChunk_VerticalSpace) {
      fprintf(file, "{VerticalSpace  }");
      continue;
    }

    text = lfort_getCompletionChunkText(completion_string, I);
    cstr = lfort_getCString(text);
    fprintf(file, "{%s %s}",
            lfort_getCompletionChunkKindSpelling(Kind),
            cstr ? cstr : "");
    lfort_disposeString(text);
  }

}

void print_completion_result(CXCompletionResult *completion_result,
                             CXClientData client_data) {
  FILE *file = (FILE *)client_data;
  CXString ks = lfort_getCursorKindSpelling(completion_result->CursorKind);
  unsigned annotationCount;
  enum CXCursorKind ParentKind;
  CXString ParentName;
  CXString BriefComment;
  const char *BriefCommentCString;
  
  fprintf(file, "%s:", lfort_getCString(ks));
  lfort_disposeString(ks);

  print_completion_string(completion_result->CompletionString, file);
  fprintf(file, " (%u)", 
          lfort_getCompletionPriority(completion_result->CompletionString));
  switch (lfort_getCompletionAvailability(completion_result->CompletionString)){
  case CXAvailability_Available:
    break;
    
  case CXAvailability_Deprecated:
    fprintf(file, " (deprecated)");
    break;
    
  case CXAvailability_NotAvailable:
    fprintf(file, " (unavailable)");
    break;

  case CXAvailability_NotAccessible:
    fprintf(file, " (inaccessible)");
    break;
  }

  annotationCount = lfort_getCompletionNumAnnotations(
        completion_result->CompletionString);
  if (annotationCount) {
    unsigned i;
    fprintf(file, " (");
    for (i = 0; i < annotationCount; ++i) {
      if (i != 0)
        fprintf(file, ", ");
      fprintf(file, "\"%s\"",
              lfort_getCString(lfort_getCompletionAnnotation(
                                 completion_result->CompletionString, i)));
    }
    fprintf(file, ")");
  }

  if (!getenv("CINDEXTEST_NO_COMPLETION_PARENTS")) {
    ParentName = lfort_getCompletionParent(completion_result->CompletionString,
                                           &ParentKind);
    if (ParentKind != CXCursor_NotImplemented) {
      CXString KindSpelling = lfort_getCursorKindSpelling(ParentKind);
      fprintf(file, " (parent: %s '%s')",
              lfort_getCString(KindSpelling),
              lfort_getCString(ParentName));
      lfort_disposeString(KindSpelling);
    }
    lfort_disposeString(ParentName);
  }

  BriefComment = lfort_getCompletionBriefComment(
                                        completion_result->CompletionString);
  BriefCommentCString = lfort_getCString(BriefComment);
  if (BriefCommentCString && *BriefCommentCString != '\0') {
    fprintf(file, "(brief comment: %s)", BriefCommentCString);
  }
  lfort_disposeString(BriefComment);
  
  fprintf(file, "\n");
}

void print_completion_contexts(unsigned long long contexts, FILE *file) {
  fprintf(file, "Completion contexts:\n");
  if (contexts == CXCompletionContext_Unknown) {
    fprintf(file, "Unknown\n");
  }
  if (contexts & CXCompletionContext_AnyType) {
    fprintf(file, "Any type\n");
  }
  if (contexts & CXCompletionContext_AnyValue) {
    fprintf(file, "Any value\n");
  }
  if (contexts & CXCompletionContext_ObjCObjectValue) {
    fprintf(file, "Objective-C object value\n");
  }
  if (contexts & CXCompletionContext_ObjCSelectorValue) {
    fprintf(file, "Objective-C selector value\n");
  }
  if (contexts & CXCompletionContext_CXXClassTypeValue) {
    fprintf(file, "C++ class type value\n");
  }
  if (contexts & CXCompletionContext_DotMemberAccess) {
    fprintf(file, "Dot member access\n");
  }
  if (contexts & CXCompletionContext_ArrowMemberAccess) {
    fprintf(file, "Arrow member access\n");
  }
  if (contexts & CXCompletionContext_ObjCPropertyAccess) {
    fprintf(file, "Objective-C property access\n");
  }
  if (contexts & CXCompletionContext_EnumTag) {
    fprintf(file, "Enum tag\n");
  }
  if (contexts & CXCompletionContext_UnionTag) {
    fprintf(file, "Union tag\n");
  }
  if (contexts & CXCompletionContext_StructTag) {
    fprintf(file, "Struct tag\n");
  }
  if (contexts & CXCompletionContext_ClassTag) {
    fprintf(file, "Class name\n");
  }
  if (contexts & CXCompletionContext_Namespace) {
    fprintf(file, "Namespace or namespace alias\n");
  }
  if (contexts & CXCompletionContext_NestedNameSpecifier) {
    fprintf(file, "Nested name specifier\n");
  }
  if (contexts & CXCompletionContext_ObjCInterface) {
    fprintf(file, "Objective-C interface\n");
  }
  if (contexts & CXCompletionContext_ObjCProtocol) {
    fprintf(file, "Objective-C protocol\n");
  }
  if (contexts & CXCompletionContext_ObjCCategory) {
    fprintf(file, "Objective-C category\n");
  }
  if (contexts & CXCompletionContext_ObjCInstanceMessage) {
    fprintf(file, "Objective-C instance method\n");
  }
  if (contexts & CXCompletionContext_ObjCClassMessage) {
    fprintf(file, "Objective-C class method\n");
  }
  if (contexts & CXCompletionContext_ObjCSelectorName) {
    fprintf(file, "Objective-C selector name\n");
  }
  if (contexts & CXCompletionContext_MacroName) {
    fprintf(file, "Macro name\n");
  }
  if (contexts & CXCompletionContext_NaturalLanguage) {
    fprintf(file, "Natural language\n");
  }
}

int my_stricmp(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    int c1 = tolower((unsigned char)*s1), c2 = tolower((unsigned char)*s2);
    if (c1 < c2)
      return -1;
    else if (c1 > c2)
      return 1;
    
    ++s1;
    ++s2;
  }
  
  if (*s1)
    return 1;
  else if (*s2)
    return -1;
  return 0;
}

int perform_code_completion(int argc, const char **argv, int timing_only) {
  const char *input = argv[1];
  char *filename = 0;
  unsigned line;
  unsigned column;
  CXIndex CIdx;
  int errorCode;
  struct CXUnsavedFile *unsaved_files = 0;
  int num_unsaved_files = 0;
  CXCodeCompleteResults *results = 0;
  CXProgram Pgm = 0;
  unsigned I, Repeats = 1;
  unsigned completionOptions = lfort_defaultCodeCompleteOptions();
  
  if (getenv("CINDEXTEST_CODE_COMPLETE_PATTERNS"))
    completionOptions |= CXCodeComplete_IncludeCodePatterns;
  if (getenv("CINDEXTEST_COMPLETION_BRIEF_COMMENTS"))
    completionOptions |= CXCodeComplete_IncludeBriefComments;
  
  if (timing_only)
    input += strlen("-code-completion-timing=");
  else
    input += strlen("-code-completion-at=");

  if ((errorCode = parse_file_line_column(input, &filename, &line, &column,
                                          0, 0)))
    return errorCode;

  if (parse_remapped_files(argc, argv, 2, &unsaved_files, &num_unsaved_files))
    return -1;

  CIdx = lfort_createIndex(0, 0);
  
  if (getenv("CINDEXTEST_EDITING"))
    Repeats = 5;
  
  Pgm = lfort_parseProgram(CIdx, 0,
                                  argv + num_unsaved_files + 2,
                                  argc - num_unsaved_files - 2,
                                  0, 0, getDefaultParsingOptions());
  if (!Pgm) {
    fprintf(stderr, "Unable to load translation unit!\n");
    return 1;
  }

  if (lfort_reparseProgram(Pgm, 0, 0, lfort_defaultReparseOptions(Pgm))) {
    fprintf(stderr, "Unable to reparse translation init!\n");
    return 1;
  }
  
  for (I = 0; I != Repeats; ++I) {
    results = lfort_codeCompleteAt(Pgm, filename, line, column,
                                   unsaved_files, num_unsaved_files,
                                   completionOptions);
    if (!results) {
      fprintf(stderr, "Unable to perform code completion!\n");
      return 1;
    }
    if (I != Repeats-1)
      lfort_disposeCodeCompleteResults(results);
  }

  if (results) {
    unsigned i, n = results->NumResults, containerIsIncomplete = 0;
    unsigned long long contexts;
    enum CXCursorKind containerKind;
    CXString objCSelector;
    const char *selectorString;
    if (!timing_only) {      
      /* Sort the code-completion results based on the typed text. */
      lfort_sortCodeCompletionResults(results->Results, results->NumResults);

      for (i = 0; i != n; ++i)
        print_completion_result(results->Results + i, stdout);
    }
    n = lfort_codeCompleteGetNumDiagnostics(results);
    for (i = 0; i != n; ++i) {
      CXDiagnostic diag = lfort_codeCompleteGetDiagnostic(results, i);
      PrintDiagnostic(diag);
      lfort_disposeDiagnostic(diag);
    }
    
    contexts = lfort_codeCompleteGetContexts(results);
    print_completion_contexts(contexts, stdout);
    
    containerKind = lfort_codeCompleteGetContainerKind(results,
                                                       &containerIsIncomplete);
    
    if (containerKind != CXCursor_InvalidCode) {
      /* We have found a container */
      CXString containerUSR, containerKindSpelling;
      containerKindSpelling = lfort_getCursorKindSpelling(containerKind);
      printf("Container Kind: %s\n", lfort_getCString(containerKindSpelling));
      lfort_disposeString(containerKindSpelling);
      
      if (containerIsIncomplete) {
        printf("Container is incomplete\n");
      }
      else {
        printf("Container is complete\n");
      }
      
      containerUSR = lfort_codeCompleteGetContainerUSR(results);
      printf("Container USR: %s\n", lfort_getCString(containerUSR));
      lfort_disposeString(containerUSR);
    }
    
    objCSelector = lfort_codeCompleteGetObjCSelector(results);
    selectorString = lfort_getCString(objCSelector);
    if (selectorString && strlen(selectorString) > 0) {
      printf("Objective-C selector: %s\n", selectorString);
    }
    lfort_disposeString(objCSelector);
    
    lfort_disposeCodeCompleteResults(results);
  }
  lfort_disposeProgram(Pgm);
  lfort_disposeIndex(CIdx);
  free(filename);

  free_remapped_files(unsaved_files, num_unsaved_files);

  return 0;
}

typedef struct {
  char *filename;
  unsigned line;
  unsigned column;
} CursorSourceLocation;

static int inspect_cursor_at(int argc, const char **argv) {
  CXIndex CIdx;
  int errorCode;
  struct CXUnsavedFile *unsaved_files = 0;
  int num_unsaved_files = 0;
  CXProgram Pgm;
  CXCursor Cursor;
  CursorSourceLocation *Locations = 0;
  unsigned NumLocations = 0, Loc;
  unsigned Repeats = 1;
  unsigned I;
  
  /* Count the number of locations. */
  while (strstr(argv[NumLocations+1], "-cursor-at=") == argv[NumLocations+1])
    ++NumLocations;

  /* Parse the locations. */
  assert(NumLocations > 0 && "Unable to count locations?");
  Locations = (CursorSourceLocation *)malloc(
                                  NumLocations * sizeof(CursorSourceLocation));
  for (Loc = 0; Loc < NumLocations; ++Loc) {
    const char *input = argv[Loc + 1] + strlen("-cursor-at=");
    if ((errorCode = parse_file_line_column(input, &Locations[Loc].filename,
                                            &Locations[Loc].line,
                                            &Locations[Loc].column, 0, 0)))
      return errorCode;
  }

  if (parse_remapped_files(argc, argv, NumLocations + 1, &unsaved_files,
                           &num_unsaved_files))
    return -1;

  if (getenv("CINDEXTEST_EDITING"))
    Repeats = 5;

  /* Parse the translation unit. When we're testing lfort_getCursor() after
     reparsing, don't remap unsaved files until the second parse. */
  CIdx = lfort_createIndex(1, 1);
  Pgm = lfort_parseProgram(CIdx, argv[argc - 1],
                                  argv + num_unsaved_files + 1 + NumLocations,
                                  argc - num_unsaved_files - 2 - NumLocations,
                                  unsaved_files,
                                  Repeats > 1? 0 : num_unsaved_files,
                                  getDefaultParsingOptions());
                                                 
  if (!Pgm) {
    fprintf(stderr, "unable to parse input\n");
    return -1;
  }

  if (checkForErrors(Pgm) != 0)
    return -1;

  for (I = 0; I != Repeats; ++I) {
    if (Repeats > 1 &&
        lfort_reparseProgram(Pgm, num_unsaved_files, unsaved_files, 
                                     lfort_defaultReparseOptions(Pgm))) {
      lfort_disposeProgram(Pgm);
      return 1;
    }

    if (checkForErrors(Pgm) != 0)
      return -1;
    
    for (Loc = 0; Loc < NumLocations; ++Loc) {
      CXFile file = lfort_getFile(Pgm, Locations[Loc].filename);
      if (!file)
        continue;

      Cursor = lfort_getCursor(Pgm,
                               lfort_getLocation(Pgm, file, Locations[Loc].line,
                                                 Locations[Loc].column));

      if (checkForErrors(Pgm) != 0)
        return -1;

      if (I + 1 == Repeats) {
        CXCompletionString completionString = lfort_getCursorCompletionString(
                                                                        Cursor);
        CXSourceLocation CursorLoc = lfort_getCursorLocation(Cursor);
        CXString Spelling;
        const char *cspell;
        unsigned line, column;
        lfort_getSpellingLocation(CursorLoc, 0, &line, &column, 0);
        printf("%d:%d ", line, column);
        PrintCursor(Cursor, NULL);
        PrintCursorExtent(Cursor);
        Spelling = lfort_getCursorSpelling(Cursor);
        cspell = lfort_getCString(Spelling);
        if (cspell && strlen(cspell) != 0) {
          unsigned pieceIndex;
          printf(" Spelling=%s (", cspell);
          for (pieceIndex = 0; ; ++pieceIndex) {
            CXSourceRange range =
              lfort_Cursor_getSpellingNameRange(Cursor, pieceIndex, 0);
            if (lfort_Range_isNull(range))
              break;
            PrintRange(range, 0);
          }
          printf(")");
        }
        lfort_disposeString(Spelling);
        if (lfort_Cursor_getObjCSelectorIndex(Cursor) != -1)
          printf(" Selector index=%d",lfort_Cursor_getObjCSelectorIndex(Cursor));
        if (lfort_Cursor_isDynamicCall(Cursor))
          printf(" Dynamic-call");
        if (Cursor.kind == CXCursor_ObjCMessageExpr) {
          CXType T = lfort_Cursor_getReceiverType(Cursor);
          CXString S = lfort_getTypeKindSpelling(T.kind);
          printf(" Receiver-type=%s", lfort_getCString(S));
          lfort_disposeString(S);
        }

        {
          CXModule mod = lfort_Cursor_getModule(Cursor);
          CXString name;
          unsigned i, numHeaders;
          if (mod) {
            name = lfort_Module_getFullName(mod);
            numHeaders = lfort_Module_getNumTopLevelHeaders(mod);
            printf(" ModuleName=%s Headers(%d):",
                   lfort_getCString(name), numHeaders);
            lfort_disposeString(name);
            for (i = 0; i < numHeaders; ++i) {
              CXFile file = lfort_Module_getTopLevelHeader(mod, i);
              CXString filename = lfort_getFileName(file);
              printf("\n%s", lfort_getCString(filename));
              lfort_disposeString(filename);
            }
          }
        }

        if (completionString != NULL) {
          printf("\nCompletion string: ");
          print_completion_string(completionString, stdout);
        }
        printf("\n");
        free(Locations[Loc].filename);
      }
    }
  }
  
  PrintDiagnostics(Pgm);
  lfort_disposeProgram(Pgm);
  lfort_disposeIndex(CIdx);
  free(Locations);
  free_remapped_files(unsaved_files, num_unsaved_files);
  return 0;
}

static enum CXVisitorResult findFileRefsVisit(void *context,
                                         CXCursor cursor, CXSourceRange range) {
  if (lfort_Range_isNull(range))
    return CXVisit_Continue;

  PrintCursor(cursor, NULL);
  PrintRange(range, "");
  printf("\n");
  return CXVisit_Continue;
}

static int find_file_refs_at(int argc, const char **argv) {
  CXIndex CIdx;
  int errorCode;
  struct CXUnsavedFile *unsaved_files = 0;
  int num_unsaved_files = 0;
  CXProgram Pgm;
  CXCursor Cursor;
  CursorSourceLocation *Locations = 0;
  unsigned NumLocations = 0, Loc;
  unsigned Repeats = 1;
  unsigned I;
  
  /* Count the number of locations. */
  while (strstr(argv[NumLocations+1], "-file-refs-at=") == argv[NumLocations+1])
    ++NumLocations;

  /* Parse the locations. */
  assert(NumLocations > 0 && "Unable to count locations?");
  Locations = (CursorSourceLocation *)malloc(
                                  NumLocations * sizeof(CursorSourceLocation));
  for (Loc = 0; Loc < NumLocations; ++Loc) {
    const char *input = argv[Loc + 1] + strlen("-file-refs-at=");
    if ((errorCode = parse_file_line_column(input, &Locations[Loc].filename,
                                            &Locations[Loc].line,
                                            &Locations[Loc].column, 0, 0)))
      return errorCode;
  }

  if (parse_remapped_files(argc, argv, NumLocations + 1, &unsaved_files,
                           &num_unsaved_files))
    return -1;

  if (getenv("CINDEXTEST_EDITING"))
    Repeats = 5;

  /* Parse the translation unit. When we're testing lfort_getCursor() after
     reparsing, don't remap unsaved files until the second parse. */
  CIdx = lfort_createIndex(1, 1);
  Pgm = lfort_parseProgram(CIdx, argv[argc - 1],
                                  argv + num_unsaved_files + 1 + NumLocations,
                                  argc - num_unsaved_files - 2 - NumLocations,
                                  unsaved_files,
                                  Repeats > 1? 0 : num_unsaved_files,
                                  getDefaultParsingOptions());
                                                 
  if (!Pgm) {
    fprintf(stderr, "unable to parse input\n");
    return -1;
  }

  if (checkForErrors(Pgm) != 0)
    return -1;

  for (I = 0; I != Repeats; ++I) {
    if (Repeats > 1 &&
        lfort_reparseProgram(Pgm, num_unsaved_files, unsaved_files, 
                                     lfort_defaultReparseOptions(Pgm))) {
      lfort_disposeProgram(Pgm);
      return 1;
    }

    if (checkForErrors(Pgm) != 0)
      return -1;
    
    for (Loc = 0; Loc < NumLocations; ++Loc) {
      CXFile file = lfort_getFile(Pgm, Locations[Loc].filename);
      if (!file)
        continue;

      Cursor = lfort_getCursor(Pgm,
                               lfort_getLocation(Pgm, file, Locations[Loc].line,
                                                 Locations[Loc].column));

      if (checkForErrors(Pgm) != 0)
        return -1;

      if (I + 1 == Repeats) {
        CXCursorAndRangeVisitor visitor = { 0, findFileRefsVisit };
        PrintCursor(Cursor, NULL);
        printf("\n");
        lfort_findReferencesInFile(Cursor, file, visitor);
        free(Locations[Loc].filename);

        if (checkForErrors(Pgm) != 0)
          return -1;
      }
    }
  }
  
  PrintDiagnostics(Pgm);
  lfort_disposeProgram(Pgm);
  lfort_disposeIndex(CIdx);
  free(Locations);
  free_remapped_files(unsaved_files, num_unsaved_files);
  return 0;
}

#define MAX_IMPORTED_ASTFILES 200

typedef struct {
  char **filenames;
  unsigned num_files;
} ImportedASTFilesData;

static ImportedASTFilesData *importedASTs_create() {
  ImportedASTFilesData *p;
  p = malloc(sizeof(ImportedASTFilesData));
  p->filenames = malloc(MAX_IMPORTED_ASTFILES * sizeof(const char *));
  p->num_files = 0;
  return p;
}

static void importedASTs_dispose(ImportedASTFilesData *p) {
  unsigned i;
  if (!p)
    return;

  for (i = 0; i < p->num_files; ++i)
    free(p->filenames[i]);
  free(p->filenames);
  free(p);
}

static void importedASTS_insert(ImportedASTFilesData *p, const char *file) {
  unsigned i;
  assert(p && file);
  for (i = 0; i < p->num_files; ++i)
    if (strcmp(file, p->filenames[i]) == 0)
      return;
  assert(p->num_files + 1 < MAX_IMPORTED_ASTFILES);
  p->filenames[p->num_files++] = strdup(file);
}

typedef struct {
  const char *check_prefix;
  int first_check_printed;
  int fail_for_error;
  int abort;
  const char *main_filename;
  ImportedASTFilesData *importedASTs;
} IndexData;

static void printCheck(IndexData *data) {
  if (data->check_prefix) {
    if (data->first_check_printed) {
      printf("// %s-NEXT: ", data->check_prefix);
    } else {
      printf("// %s     : ", data->check_prefix);
      data->first_check_printed = 1;
    }
  }
}

static void printCXIndexFile(CXIdxClientFile file) {
  CXString filename = lfort_getFileName((CXFile)file);
  printf("%s", lfort_getCString(filename));
  lfort_disposeString(filename);
}

static void printCXIndexLoc(CXIdxLoc loc, CXClientData client_data) {
  IndexData *index_data;
  CXString filename;
  const char *cname;
  CXIdxClientFile file;
  unsigned line, column;
  int isMainFile;
  
  index_data = (IndexData *)client_data;
  lfort_indexLoc_getFileLocation(loc, &file, 0, &line, &column, 0);
  if (line == 0) {
    printf("<invalid>");
    return;
  }
  if (!file) {
    printf("<no idxfile>");
    return;
  }
  filename = lfort_getFileName((CXFile)file);
  cname = lfort_getCString(filename);
  if (strcmp(cname, index_data->main_filename) == 0)
    isMainFile = 1;
  else
    isMainFile = 0;
  lfort_disposeString(filename);

  if (!isMainFile) {
    printCXIndexFile(file);
    printf(":");
  }
  printf("%d:%d", line, column);
}

static unsigned digitCount(unsigned val) {
  unsigned c = 1;
  while (1) {
    if (val < 10)
      return c;
    ++c;
    val /= 10;
  }
}

static CXIdxClientContainer makeClientContainer(const CXIdxEntityInfo *info,
                                                CXIdxLoc loc) {
  const char *name;
  char *newStr;
  CXIdxClientFile file;
  unsigned line, column;
  
  name = info->name;
  if (!name)
    name = "<anon-tag>";

  lfort_indexLoc_getFileLocation(loc, &file, 0, &line, &column, 0);
  /* FIXME: free these.*/
  newStr = (char *)malloc(strlen(name) +
                          digitCount(line) + digitCount(column) + 3);
  sprintf(newStr, "%s:%d:%d", name, line, column);
  return (CXIdxClientContainer)newStr;
}

static void printCXIndexContainer(const CXIdxContainerInfo *info) {
  CXIdxClientContainer container;
  container = lfort_index_getClientContainer(info);
  if (!container)
    printf("[<<NULL>>]");
  else
    printf("[%s]", (const char *)container);
}

static const char *getEntityKindString(CXIdxEntityKind kind) {
  switch (kind) {
  case CXIdxEntity_Unexposed: return "<<UNEXPOSED>>";
  case CXIdxEntity_Typedef: return "typedef";
  case CXIdxEntity_Subprogram: return "function";
  case CXIdxEntity_Variable: return "variable";
  case CXIdxEntity_Field: return "field";
  case CXIdxEntity_EnumConstant: return "enumerator";
  case CXIdxEntity_ObjCClass: return "objc-class";
  case CXIdxEntity_ObjCProtocol: return "objc-protocol";
  case CXIdxEntity_ObjCCategory: return "objc-category";
  case CXIdxEntity_ObjCInstanceMethod: return "objc-instance-method";
  case CXIdxEntity_ObjCClassMethod: return "objc-class-method";
  case CXIdxEntity_ObjCProperty: return "objc-property";
  case CXIdxEntity_ObjCIvar: return "objc-ivar";
  case CXIdxEntity_Enum: return "enum";
  case CXIdxEntity_Struct: return "struct";
  case CXIdxEntity_Union: return "union";
  case CXIdxEntity_CXXClass: return "c++-class";
  case CXIdxEntity_CXXNamespace: return "namespace";
  case CXIdxEntity_CXXNamespaceAlias: return "namespace-alias";
  case CXIdxEntity_CXXStaticVariable: return "c++-static-var";
  case CXIdxEntity_CXXStaticMethod: return "c++-static-method";
  case CXIdxEntity_CXXInstanceMethod: return "c++-instance-method";
  case CXIdxEntity_CXXConstructor: return "constructor";
  case CXIdxEntity_CXXDestructor: return "destructor";
  case CXIdxEntity_CXXConversionSubprogram: return "conversion-func";
  case CXIdxEntity_CXXTypeAlias: return "type-alias";
  case CXIdxEntity_CXXInterface: return "c++-__interface";
  }
  assert(0 && "Garbage entity kind");
  return 0;
}

static const char *getEntityTemplateKindString(CXIdxEntityCXXTemplateKind kind) {
  switch (kind) {
  case CXIdxEntity_NonTemplate: return "";
  case CXIdxEntity_Template: return "-template";
  case CXIdxEntity_TemplatePartialSpecialization:
    return "-template-partial-spec";
  case CXIdxEntity_TemplateSpecialization: return "-template-spec";
  }
  assert(0 && "Garbage entity kind");
  return 0;
}

static const char *getEntityLanguageString(CXIdxEntityLanguage kind) {
  switch (kind) {
  case CXIdxEntityLang_None: return "<none>";
  case CXIdxEntityLang_C: return "C";
  case CXIdxEntityLang_ObjC: return "ObjC";
  case CXIdxEntityLang_CXX: return "C++";
  }
  assert(0 && "Garbage language kind");
  return 0;
}

static void printEntityInfo(const char *cb,
                            CXClientData client_data,
                            const CXIdxEntityInfo *info) {
  const char *name;
  IndexData *index_data;
  unsigned i;
  index_data = (IndexData *)client_data;
  printCheck(index_data);

  if (!info) {
    printf("%s: <<NULL>>", cb);
    return;
  }

  name = info->name;
  if (!name)
    name = "<anon-tag>";

  printf("%s: kind: %s%s", cb, getEntityKindString(info->kind),
         getEntityTemplateKindString(info->templateKind));
  printf(" | name: %s", name);
  printf(" | USR: %s", info->USR);
  printf(" | lang: %s", getEntityLanguageString(info->lang));

  for (i = 0; i != info->numAttributes; ++i) {
    const CXIdxAttrInfo *Attr = info->attributes[i];
    printf("     <attribute>: ");
    PrintCursor(Attr->cursor, NULL);
  }
}

static void printBaseClassInfo(CXClientData client_data,
                               const CXIdxBaseClassInfo *info) {
  printEntityInfo("     <base>", client_data, info->base);
  printf(" | cursor: ");
  PrintCursor(info->cursor, NULL);
  printf(" | loc: ");
  printCXIndexLoc(info->loc, client_data);
}

static void printProtocolList(const CXIdxObjCProtocolRefListInfo *ProtoInfo,
                              CXClientData client_data) {
  unsigned i;
  for (i = 0; i < ProtoInfo->numProtocols; ++i) {
    printEntityInfo("     <protocol>", client_data,
                    ProtoInfo->protocols[i]->protocol);
    printf(" | cursor: ");
    PrintCursor(ProtoInfo->protocols[i]->cursor, NULL);
    printf(" | loc: ");
    printCXIndexLoc(ProtoInfo->protocols[i]->loc, client_data);
    printf("\n");
  }
}

static void index_diagnostic(CXClientData client_data,
                             CXDiagnosticSet diagSet, void *reserved) {
  CXString str;
  const char *cstr;
  unsigned numDiags, i;
  CXDiagnostic diag;
  IndexData *index_data;
  index_data = (IndexData *)client_data;
  printCheck(index_data);

  numDiags = lfort_getNumDiagnosticsInSet(diagSet);
  for (i = 0; i != numDiags; ++i) {
    diag = lfort_getDiagnosticInSet(diagSet, i);
    str = lfort_formatDiagnostic(diag, lfort_defaultDiagnosticDisplayOptions());
    cstr = lfort_getCString(str);
    printf("[diagnostic]: %s\n", cstr);
    lfort_disposeString(str);  
  
    if (getenv("CINDEXTEST_FAILONERROR") &&
        lfort_getDiagnosticSeverity(diag) >= CXDiagnostic_Error) {
      index_data->fail_for_error = 1;
    }
  }
}

static CXIdxClientFile index_enteredMainFile(CXClientData client_data,
                                       CXFile file, void *reserved) {
  IndexData *index_data;
  CXString filename;

  index_data = (IndexData *)client_data;
  printCheck(index_data);

  filename = lfort_getFileName(file);
  index_data->main_filename = lfort_getCString(filename);
  lfort_disposeString(filename);

  printf("[enteredMainFile]: ");
  printCXIndexFile((CXIdxClientFile)file);
  printf("\n");

  return (CXIdxClientFile)file;
}

static CXIdxClientFile index_ppIncludedFile(CXClientData client_data,
                                            const CXIdxIncludedFileInfo *info) {
  IndexData *index_data;
  index_data = (IndexData *)client_data;
  printCheck(index_data);

  printf("[ppIncludedFile]: ");
  printCXIndexFile((CXIdxClientFile)info->file);
  printf(" | name: \"%s\"", info->filename);
  printf(" | hash loc: ");
  printCXIndexLoc(info->hashLoc, client_data);
  printf(" | isImport: %d | isAngled: %d | isModule: %d\n",
         info->isImport, info->isAngled, info->isModuleImport);

  return (CXIdxClientFile)info->file;
}

static CXIdxClientFile index_importedASTFile(CXClientData client_data,
                                         const CXIdxImportedASTFileInfo *info) {
  IndexData *index_data;
  index_data = (IndexData *)client_data;
  printCheck(index_data);

  if (index_data->importedASTs) {
    CXString filename = lfort_getFileName(info->file);
    importedASTS_insert(index_data->importedASTs, lfort_getCString(filename));
    lfort_disposeString(filename);
  }
  
  printf("[importedASTFile]: ");
  printCXIndexFile((CXIdxClientFile)info->file);
  if (info->module) {
    CXString name = lfort_Module_getFullName(info->module);
    printf(" | loc: ");
    printCXIndexLoc(info->loc, client_data);
    printf(" | name: \"%s\"", lfort_getCString(name));
    printf(" | isImplicit: %d\n", info->isImplicit);
    lfort_disposeString(name);
  } else {
    /* PCH file, the rest are not relevant. */
    printf("\n");
  }

  return (CXIdxClientFile)info->file;
}

static CXIdxClientContainer index_startedProgram(CXClientData client_data,
                                                   void *reserved) {
  IndexData *index_data;
  index_data = (IndexData *)client_data;
  printCheck(index_data);

  printf("[startedProgram]\n");
  return (CXIdxClientContainer)"Pgm";
}

static void index_indexDeclaration(CXClientData client_data,
                                   const CXIdxDeclInfo *info) {
  IndexData *index_data;
  const CXIdxObjCCategoryDeclInfo *CatInfo;
  const CXIdxObjCInterfaceDeclInfo *InterInfo;
  const CXIdxObjCProtocolRefListInfo *ProtoInfo;
  const CXIdxObjCPropertyDeclInfo *PropInfo;
  const CXIdxCXXClassDeclInfo *CXXClassInfo;
  unsigned i;
  index_data = (IndexData *)client_data;

  printEntityInfo("[indexDeclaration]", client_data, info->entityInfo);
  printf(" | cursor: ");
  PrintCursor(info->cursor, NULL);
  printf(" | loc: ");
  printCXIndexLoc(info->loc, client_data);
  printf(" | semantic-container: ");
  printCXIndexContainer(info->semanticContainer);
  printf(" | lexical-container: ");
  printCXIndexContainer(info->lexicalContainer);
  printf(" | isRedecl: %d", info->isRedeclaration);
  printf(" | isDef: %d", info->isDefinition);
  if (info->flags & CXIdxDeclFlag_Skipped) {
    assert(!info->isContainer);
    printf(" | isContainer: skipped");
  } else {
    printf(" | isContainer: %d", info->isContainer);
  }
  printf(" | isImplicit: %d\n", info->isImplicit);

  for (i = 0; i != info->numAttributes; ++i) {
    const CXIdxAttrInfo *Attr = info->attributes[i];
    printf("     <attribute>: ");
    PrintCursor(Attr->cursor, NULL);
    printf("\n");
  }

  if (lfort_index_isEntityObjCContainerKind(info->entityInfo->kind)) {
    const char *kindName = 0;
    CXIdxObjCContainerKind K = lfort_index_getObjCContainerDeclInfo(info)->kind;
    switch (K) {
    case CXIdxObjCContainer_ForwardRef:
      kindName = "forward-ref"; break;
    case CXIdxObjCContainer_Interface:
      kindName = "interface"; break;
    case CXIdxObjCContainer_Implementation:
      kindName = "implementation"; break;
    }
    printCheck(index_data);
    printf("     <ObjCContainerInfo>: kind: %s\n", kindName);
  }

  if ((CatInfo = lfort_index_getObjCCategoryDeclInfo(info))) {
    printEntityInfo("     <ObjCCategoryInfo>: class", client_data,
                    CatInfo->objcClass);
    printf(" | cursor: ");
    PrintCursor(CatInfo->classCursor, NULL);
    printf(" | loc: ");
    printCXIndexLoc(CatInfo->classLoc, client_data);
    printf("\n");
  }

  if ((InterInfo = lfort_index_getObjCInterfaceDeclInfo(info))) {
    if (InterInfo->superInfo) {
      printBaseClassInfo(client_data, InterInfo->superInfo);
      printf("\n");
    }
  }

  if ((ProtoInfo = lfort_index_getObjCProtocolRefListInfo(info))) {
    printProtocolList(ProtoInfo, client_data);
  }

  if ((PropInfo = lfort_index_getObjCPropertyDeclInfo(info))) {
    if (PropInfo->getter) {
      printEntityInfo("     <getter>", client_data, PropInfo->getter);
      printf("\n");
    }
    if (PropInfo->setter) {
      printEntityInfo("     <setter>", client_data, PropInfo->setter);
      printf("\n");
    }
  }

  if ((CXXClassInfo = lfort_index_getCXXClassDeclInfo(info))) {
    for (i = 0; i != CXXClassInfo->numBases; ++i) {
      printBaseClassInfo(client_data, CXXClassInfo->bases[i]);
      printf("\n");
    }
  }

  if (info->declAsContainer)
    lfort_index_setClientContainer(info->declAsContainer,
                              makeClientContainer(info->entityInfo, info->loc));
}

static void index_indexEntityReference(CXClientData client_data,
                                       const CXIdxEntityRefInfo *info) {
  printEntityInfo("[indexEntityReference]", client_data, info->referencedEntity);
  printf(" | cursor: ");
  PrintCursor(info->cursor, NULL);
  printf(" | loc: ");
  printCXIndexLoc(info->loc, client_data);
  printEntityInfo(" | <parent>:", client_data, info->parentEntity);
  printf(" | container: ");
  printCXIndexContainer(info->container);
  printf(" | refkind: ");
  switch (info->kind) {
  case CXIdxEntityRef_Direct: printf("direct"); break;
  case CXIdxEntityRef_Implicit: printf("implicit"); break;
  }
  printf("\n");
}

static int index_abortQuery(CXClientData client_data, void *reserved) {
  IndexData *index_data;
  index_data = (IndexData *)client_data;
  return index_data->abort;
}

static IndexerCallbacks IndexCB = {
  index_abortQuery,
  index_diagnostic,
  index_enteredMainFile,
  index_ppIncludedFile,
  index_importedASTFile,
  index_startedProgram,
  index_indexDeclaration,
  index_indexEntityReference
};

static unsigned getIndexOptions(void) {
  unsigned index_opts;
  index_opts = 0;
  if (getenv("CINDEXTEST_SUPPRESSREFS"))
    index_opts |= CXIndexOpt_SuppressRedundantRefs;
  if (getenv("CINDEXTEST_INDEXLOCALSYMBOLS"))
    index_opts |= CXIndexOpt_IndexSubprogramLocalSymbols;
  if (!getenv("CINDEXTEST_DISABLE_SKIPPARSEDBODIES"))
    index_opts |= CXIndexOpt_SkipParsedBodiesInSession;

  return index_opts;
}

static int index_compile_args(int num_args, const char **args,
                              CXIndexAction idxAction,
                              ImportedASTFilesData *importedASTs,
                              const char *check_prefix) {
  IndexData index_data;
  unsigned index_opts;
  int result;

  if (num_args == 0) {
    fprintf(stderr, "no compiler arguments\n");
    return -1;
  }

  index_data.check_prefix = check_prefix;
  index_data.first_check_printed = 0;
  index_data.fail_for_error = 0;
  index_data.abort = 0;
  index_data.main_filename = "";
  index_data.importedASTs = importedASTs;

  index_opts = getIndexOptions();
  result = lfort_indexSourceFile(idxAction, &index_data,
                                 &IndexCB,sizeof(IndexCB), index_opts,
                                 0, args, num_args, 0, 0, 0,
                                 getDefaultParsingOptions());
  if (index_data.fail_for_error)
    result = -1;

  return result;
}

static int index_ast_file(const char *ast_file,
                          CXIndex Idx,
                          CXIndexAction idxAction,
                          ImportedASTFilesData *importedASTs,
                          const char *check_prefix) {
  CXProgram Pgm;
  IndexData index_data;
  unsigned index_opts;
  int result;

  if (!CreateProgram(Idx, ast_file, &Pgm))
    return -1;

  index_data.check_prefix = check_prefix;
  index_data.first_check_printed = 0;
  index_data.fail_for_error = 0;
  index_data.abort = 0;
  index_data.main_filename = "";
  index_data.importedASTs = importedASTs;

  index_opts = getIndexOptions();
  result = lfort_indexProgram(idxAction, &index_data,
                                      &IndexCB,sizeof(IndexCB),
                                      index_opts, Pgm);
  if (index_data.fail_for_error)
    result = -1;

  lfort_disposeProgram(Pgm);
  return result;
}

static int index_file(int argc, const char **argv, int full) {
  const char *check_prefix;
  CXIndex Idx;
  CXIndexAction idxAction;
  ImportedASTFilesData *importedASTs;
  int result;

  check_prefix = 0;
  if (argc > 0) {
    if (strstr(argv[0], "-check-prefix=") == argv[0]) {
      check_prefix = argv[0] + strlen("-check-prefix=");
      ++argv;
      --argc;
    }
  }

  if (!(Idx = lfort_createIndex(/* excludeDeclsFromPCH */ 1,
                                /* displayDiagnosics=*/1))) {
    fprintf(stderr, "Could not create Index\n");
    return 1;
  }
  idxAction = lfort_IndexAction_create(Idx);
  importedASTs = 0;
  if (full)
    importedASTs = importedASTs_create();

  result = index_compile_args(argc, argv, idxAction, importedASTs, check_prefix);
  if (result != 0)
    goto finished;

  if (full) {
    unsigned i;
    for (i = 0; i < importedASTs->num_files && result == 0; ++i) {
      result = index_ast_file(importedASTs->filenames[i], Idx, idxAction,
                              importedASTs, check_prefix);
    }
  }

finished:
  importedASTs_dispose(importedASTs);
  lfort_IndexAction_dispose(idxAction);
  lfort_disposeIndex(Idx);
  return result;
}

static int index_tu(int argc, const char **argv) {
  const char *check_prefix;
  CXIndex Idx;
  CXIndexAction idxAction;
  int result;

  check_prefix = 0;
  if (argc > 0) {
    if (strstr(argv[0], "-check-prefix=") == argv[0]) {
      check_prefix = argv[0] + strlen("-check-prefix=");
      ++argv;
      --argc;
    }
  }

  if (!(Idx = lfort_createIndex(/* excludeDeclsFromPCH */ 1,
                                /* displayDiagnosics=*/1))) {
    fprintf(stderr, "Could not create Index\n");
    return 1;
  }
  idxAction = lfort_IndexAction_create(Idx);

  result = index_ast_file(argv[0], Idx, idxAction,
                          /*importedASTs=*/0, check_prefix);

  lfort_IndexAction_dispose(idxAction);
  lfort_disposeIndex(Idx);
  return result;
}

static int index_compile_db(int argc, const char **argv) {
  const char *check_prefix;
  CXIndex Idx;
  CXIndexAction idxAction;
  int errorCode = 0;

  check_prefix = 0;
  if (argc > 0) {
    if (strstr(argv[0], "-check-prefix=") == argv[0]) {
      check_prefix = argv[0] + strlen("-check-prefix=");
      ++argv;
      --argc;
    }
  }

  if (argc == 0) {
    fprintf(stderr, "no compilation database\n");
    return -1;
  }

  if (!(Idx = lfort_createIndex(/* excludeDeclsFromPCH */ 1,
                                /* displayDiagnosics=*/1))) {
    fprintf(stderr, "Could not create Index\n");
    return 1;
  }
  idxAction = lfort_IndexAction_create(Idx);

  {
    const char *database = argv[0];
    CXCompilationDatabase db = 0;
    CXCompileCommands CCmds = 0;
    CXCompileCommand CCmd;
    CXCompilationDatabase_Error ec;
    CXString wd;
#define MAX_COMPILE_ARGS 512
    CXString cxargs[MAX_COMPILE_ARGS];
    const char *args[MAX_COMPILE_ARGS];
    char *tmp;
    unsigned len;
    char *buildDir;
    int i, a, numCmds, numArgs;

    len = strlen(database);
    tmp = (char *) malloc(len+1);
    memcpy(tmp, database, len+1);
    buildDir = dirname(tmp);

    db = lfort_CompilationDatabase_fromDirectory(buildDir, &ec);

    if (db) {

      if (ec!=CXCompilationDatabase_NoError) {
        printf("unexpected error %d code while loading compilation database\n", ec);
        errorCode = -1;
        goto cdb_end;
      }

      if (chdir(buildDir) != 0) {
        printf("Could not chdir to %s\n", buildDir);
        errorCode = -1;
        goto cdb_end;
      }

      CCmds = lfort_CompilationDatabase_getAllCompileCommands(db);
      if (!CCmds) {
        printf("compilation db is empty\n");
        errorCode = -1;
        goto cdb_end;
      }

      numCmds = lfort_CompileCommands_getSize(CCmds);

      if (numCmds==0) {
        fprintf(stderr, "should not get an empty compileCommand set\n");
        errorCode = -1;
        goto cdb_end;
      }

      for (i=0; i<numCmds && errorCode == 0; ++i) {
        CCmd = lfort_CompileCommands_getCommand(CCmds, i);

        wd = lfort_CompileCommand_getDirectory(CCmd);
        if (chdir(lfort_getCString(wd)) != 0) {
          printf("Could not chdir to %s\n", lfort_getCString(wd));
          errorCode = -1;
          goto cdb_end;
        }
        lfort_disposeString(wd);

        numArgs = lfort_CompileCommand_getNumArgs(CCmd);
        if (numArgs > MAX_COMPILE_ARGS){
          fprintf(stderr, "got more compile arguments than maximum\n");
          errorCode = -1;
          goto cdb_end;
        }
        for (a=0; a<numArgs; ++a) {
          cxargs[a] = lfort_CompileCommand_getArg(CCmd, a);
          args[a] = lfort_getCString(cxargs[a]);
        }

        errorCode = index_compile_args(numArgs, args, idxAction,
                                       /*importedASTs=*/0, check_prefix);

        for (a=0; a<numArgs; ++a)
          lfort_disposeString(cxargs[a]);
      }
    } else {
      printf("database loading failed with error code %d.\n", ec);
      errorCode = -1;
    }

  cdb_end:
    lfort_CompileCommands_dispose(CCmds);
    lfort_CompilationDatabase_dispose(db);
    free(tmp);

  }

  lfort_IndexAction_dispose(idxAction);
  lfort_disposeIndex(Idx);
  return errorCode;
}

int perform_token_annotation(int argc, const char **argv) {
  const char *input = argv[1];
  char *filename = 0;
  unsigned line, second_line;
  unsigned column, second_column;
  CXIndex CIdx;
  CXProgram Pgm = 0;
  int errorCode;
  struct CXUnsavedFile *unsaved_files = 0;
  int num_unsaved_files = 0;
  CXToken *tokens;
  unsigned num_tokens;
  CXSourceRange range;
  CXSourceLocation startLoc, endLoc;
  CXFile file = 0;
  CXCursor *cursors = 0;
  unsigned i;

  input += strlen("-test-annotate-tokens=");
  if ((errorCode = parse_file_line_column(input, &filename, &line, &column,
                                          &second_line, &second_column)))
    return errorCode;

  if (parse_remapped_files(argc, argv, 2, &unsaved_files, &num_unsaved_files)) {
    free(filename);
    return -1;
  }

  CIdx = lfort_createIndex(0, 1);
  Pgm = lfort_parseProgram(CIdx, argv[argc - 1],
                                  argv + num_unsaved_files + 2,
                                  argc - num_unsaved_files - 3,
                                  unsaved_files,
                                  num_unsaved_files,
                                  getDefaultParsingOptions());
  if (!Pgm) {
    fprintf(stderr, "unable to parse input\n");
    lfort_disposeIndex(CIdx);
    free(filename);
    free_remapped_files(unsaved_files, num_unsaved_files);
    return -1;
  }
  errorCode = 0;

  if (checkForErrors(Pgm) != 0) {
    errorCode = -1;
    goto teardown;
  }

  if (getenv("CINDEXTEST_EDITING")) {
    for (i = 0; i < 5; ++i) {
      if (lfort_reparseProgram(Pgm, num_unsaved_files, unsaved_files,
                                       lfort_defaultReparseOptions(Pgm))) {
        fprintf(stderr, "Unable to reparse translation unit!\n");
        errorCode = -1;
        goto teardown;
      }
    }
  }

  if (checkForErrors(Pgm) != 0) {
    errorCode = -1;
    goto teardown;
  }

  file = lfort_getFile(Pgm, filename);
  if (!file) {
    fprintf(stderr, "file %s is not in this translation unit\n", filename);
    errorCode = -1;
    goto teardown;
  }

  startLoc = lfort_getLocation(Pgm, file, line, column);
  if (lfort_equalLocations(lfort_getNullLocation(), startLoc)) {
    fprintf(stderr, "invalid source location %s:%d:%d\n", filename, line,
            column);
    errorCode = -1;
    goto teardown;
  }

  endLoc = lfort_getLocation(Pgm, file, second_line, second_column);
  if (lfort_equalLocations(lfort_getNullLocation(), endLoc)) {
    fprintf(stderr, "invalid source location %s:%d:%d\n", filename,
            second_line, second_column);
    errorCode = -1;
    goto teardown;
  }

  range = lfort_getRange(startLoc, endLoc);
  lfort_tokenize(Pgm, range, &tokens, &num_tokens);

  if (checkForErrors(Pgm) != 0) {
    errorCode = -1;
    goto teardown;
  }

  cursors = (CXCursor *)malloc(num_tokens * sizeof(CXCursor));
  lfort_annotateTokens(Pgm, tokens, num_tokens, cursors);

  if (checkForErrors(Pgm) != 0) {
    errorCode = -1;
    goto teardown;
  }

  for (i = 0; i != num_tokens; ++i) {
    const char *kind = "<unknown>";
    CXString spelling = lfort_getTokenSpelling(Pgm, tokens[i]);
    CXSourceRange extent = lfort_getTokenExtent(Pgm, tokens[i]);
    unsigned start_line, start_column, end_line, end_column;

    switch (lfort_getTokenKind(tokens[i])) {
    case CXToken_Punctuation: kind = "Punctuation"; break;
    case CXToken_Keyword: kind = "Keyword"; break;
    case CXToken_Identifier: kind = "Identifier"; break;
    case CXToken_Literal: kind = "Literal"; break;
    case CXToken_Comment: kind = "Comment"; break;
    }
    lfort_getSpellingLocation(lfort_getRangeStart(extent),
                              0, &start_line, &start_column, 0);
    lfort_getSpellingLocation(lfort_getRangeEnd(extent),
                              0, &end_line, &end_column, 0);
    printf("%s: \"%s\" ", kind, lfort_getCString(spelling));
    lfort_disposeString(spelling);
    PrintExtent(stdout, start_line, start_column, end_line, end_column);
    if (!lfort_isInvalid(cursors[i].kind)) {
      printf(" ");
      PrintCursor(cursors[i], NULL);
    }
    printf("\n");
  }
  free(cursors);
  lfort_disposeTokens(Pgm, tokens, num_tokens);

 teardown:
  PrintDiagnostics(Pgm);
  lfort_disposeProgram(Pgm);
  lfort_disposeIndex(CIdx);
  free(filename);
  free_remapped_files(unsaved_files, num_unsaved_files);
  return errorCode;
}

static int
perform_test_compilation_db(const char *database, int argc, const char **argv) {
  CXCompilationDatabase db;
  CXCompileCommands CCmds;
  CXCompileCommand CCmd;
  CXCompilationDatabase_Error ec;
  CXString wd;
  CXString arg;
  int errorCode = 0;
  char *tmp;
  unsigned len;
  char *buildDir;
  int i, j, a, numCmds, numArgs;

  len = strlen(database);
  tmp = (char *) malloc(len+1);
  memcpy(tmp, database, len+1);
  buildDir = dirname(tmp);

  db = lfort_CompilationDatabase_fromDirectory(buildDir, &ec);

  if (db) {

    if (ec!=CXCompilationDatabase_NoError) {
      printf("unexpected error %d code while loading compilation database\n", ec);
      errorCode = -1;
      goto cdb_end;
    }

    for (i=0; i<argc && errorCode==0; ) {
      if (strcmp(argv[i],"lookup")==0){
        CCmds = lfort_CompilationDatabase_getCompileCommands(db, argv[i+1]);

        if (!CCmds) {
          printf("file %s not found in compilation db\n", argv[i+1]);
          errorCode = -1;
          break;
        }

        numCmds = lfort_CompileCommands_getSize(CCmds);

        if (numCmds==0) {
          fprintf(stderr, "should not get an empty compileCommand set for file"
                          " '%s'\n", argv[i+1]);
          errorCode = -1;
          break;
        }

        for (j=0; j<numCmds; ++j) {
          CCmd = lfort_CompileCommands_getCommand(CCmds, j);

          wd = lfort_CompileCommand_getDirectory(CCmd);
          printf("workdir:'%s'", lfort_getCString(wd));
          lfort_disposeString(wd);

          printf(" cmdline:'");
          numArgs = lfort_CompileCommand_getNumArgs(CCmd);
          for (a=0; a<numArgs; ++a) {
            if (a) printf(" ");
            arg = lfort_CompileCommand_getArg(CCmd, a);
            printf("%s", lfort_getCString(arg));
            lfort_disposeString(arg);
          }
          printf("'\n");
        }

        lfort_CompileCommands_dispose(CCmds);

        i += 2;
      }
    }
    lfort_CompilationDatabase_dispose(db);
  } else {
    printf("database loading failed with error code %d.\n", ec);
    errorCode = -1;
  }

cdb_end:
  free(tmp);

  return errorCode;
}

/******************************************************************************/
/* USR printing.                                                              */
/******************************************************************************/

static int insufficient_usr(const char *kind, const char *usage) {
  fprintf(stderr, "USR for '%s' requires: %s\n", kind, usage);
  return 1;
}

static unsigned isUSR(const char *s) {
  return s[0] == 'c' && s[1] == ':';
}

static int not_usr(const char *s, const char *arg) {
  fprintf(stderr, "'%s' argument ('%s') is not a USR\n", s, arg);
  return 1;
}

static void print_usr(CXString usr) {
  const char *s = lfort_getCString(usr);
  printf("%s\n", s);
  lfort_disposeString(usr);
}

static void display_usrs() {
  fprintf(stderr, "-print-usrs options:\n"
        " ObjCCategory <class name> <category name>\n"
        " ObjCClass <class name>\n"
        " ObjCIvar <ivar name> <class USR>\n"
        " ObjCMethod <selector> [0=class method|1=instance method] "
            "<class USR>\n"
          " ObjCProperty <property name> <class USR>\n"
          " ObjCProtocol <protocol name>\n");
}

int print_usrs(const char **I, const char **E) {
  while (I != E) {
    const char *kind = *I;
    unsigned len = strlen(kind);
    switch (len) {
      case 8:
        if (memcmp(kind, "ObjCIvar", 8) == 0) {
          if (I + 2 >= E)
            return insufficient_usr(kind, "<ivar name> <class USR>");
          if (!isUSR(I[2]))
            return not_usr("<class USR>", I[2]);
          else {
            CXString x;
            x.data = (void*) I[2];
            x.private_flags = 0;
            print_usr(lfort_constructUSR_ObjCIvar(I[1], x));
          }

          I += 3;
          continue;
        }
        break;
      case 9:
        if (memcmp(kind, "ObjCClass", 9) == 0) {
          if (I + 1 >= E)
            return insufficient_usr(kind, "<class name>");
          print_usr(lfort_constructUSR_ObjCClass(I[1]));
          I += 2;
          continue;
        }
        break;
      case 10:
        if (memcmp(kind, "ObjCMethod", 10) == 0) {
          if (I + 3 >= E)
            return insufficient_usr(kind, "<method selector> "
                "[0=class method|1=instance method] <class USR>");
          if (!isUSR(I[3]))
            return not_usr("<class USR>", I[3]);
          else {
            CXString x;
            x.data = (void*) I[3];
            x.private_flags = 0;
            print_usr(lfort_constructUSR_ObjCMethod(I[1], atoi(I[2]), x));
          }
          I += 4;
          continue;
        }
        break;
      case 12:
        if (memcmp(kind, "ObjCCategory", 12) == 0) {
          if (I + 2 >= E)
            return insufficient_usr(kind, "<class name> <category name>");
          print_usr(lfort_constructUSR_ObjCCategory(I[1], I[2]));
          I += 3;
          continue;
        }
        if (memcmp(kind, "ObjCProtocol", 12) == 0) {
          if (I + 1 >= E)
            return insufficient_usr(kind, "<protocol name>");
          print_usr(lfort_constructUSR_ObjCProtocol(I[1]));
          I += 2;
          continue;
        }
        if (memcmp(kind, "ObjCProperty", 12) == 0) {
          if (I + 2 >= E)
            return insufficient_usr(kind, "<property name> <class USR>");
          if (!isUSR(I[2]))
            return not_usr("<class USR>", I[2]);
          else {
            CXString x;
            x.data = (void*) I[2];
            x.private_flags = 0;
            print_usr(lfort_constructUSR_ObjCProperty(I[1], x));
          }
          I += 3;
          continue;
        }
        break;
      default:
        break;
    }
    break;
  }

  if (I != E) {
    fprintf(stderr, "Invalid USR kind: %s\n", *I);
    display_usrs();
    return 1;
  }
  return 0;
}

int print_usrs_file(const char *file_name) {
  char line[2048];
  const char *args[128];
  unsigned numChars = 0;

  FILE *fp = fopen(file_name, "r");
  if (!fp) {
    fprintf(stderr, "error: cannot open '%s'\n", file_name);
    return 1;
  }

  /* This code is not really all that safe, but it works fine for testing. */
  while (!feof(fp)) {
    char c = fgetc(fp);
    if (c == '\n') {
      unsigned i = 0;
      const char *s = 0;

      if (numChars == 0)
        continue;

      line[numChars] = '\0';
      numChars = 0;

      if (line[0] == '/' && line[1] == '/')
        continue;

      s = strtok(line, " ");
      while (s) {
        args[i] = s;
        ++i;
        s = strtok(0, " ");
      }
      if (print_usrs(&args[0], &args[i]))
        return 1;
    }
    else
      line[numChars++] = c;
  }

  fclose(fp);
  return 0;
}

/******************************************************************************/
/* Command line processing.                                                   */
/******************************************************************************/
int write_pch_file(const char *filename, int argc, const char *argv[]) {
  CXIndex Idx;
  CXProgram Pgm;
  struct CXUnsavedFile *unsaved_files = 0;
  int num_unsaved_files = 0;
  int result = 0;
  
  Idx = lfort_createIndex(/* excludeDeclsFromPCH */1, /* displayDiagnosics=*/1);
  
  if (parse_remapped_files(argc, argv, 0, &unsaved_files, &num_unsaved_files)) {
    lfort_disposeIndex(Idx);
    return -1;
  }
  
  Pgm = lfort_parseProgram(Idx, 0,
                                  argv + num_unsaved_files,
                                  argc - num_unsaved_files,
                                  unsaved_files,
                                  num_unsaved_files,
                                  CXProgram_Incomplete |
                                    CXProgram_ForSerialization);
  if (!Pgm) {
    fprintf(stderr, "Unable to load translation unit!\n");
    free_remapped_files(unsaved_files, num_unsaved_files);
    lfort_disposeIndex(Idx);
    return 1;
  }

  switch (lfort_saveProgram(Pgm, filename, 
                                    lfort_defaultSaveOptions(Pgm))) {
  case CXSaveError_None:
    break;

  case CXSaveError_TranslationErrors:
    fprintf(stderr, "Unable to write PCH file %s: translation errors\n", 
            filename);
    result = 2;    
    break;

  case CXSaveError_InvalidPgm:
    fprintf(stderr, "Unable to write PCH file %s: invalid translation unit\n", 
            filename);
    result = 3;    
    break;

  case CXSaveError_Unknown:
  default:
    fprintf(stderr, "Unable to write PCH file %s: unknown error \n", filename);
    result = 1;
    break;
  }
  
  lfort_disposeProgram(Pgm);
  free_remapped_files(unsaved_files, num_unsaved_files);
  lfort_disposeIndex(Idx);
  return result;
}

/******************************************************************************/
/* Serialized diagnostics.                                                    */
/******************************************************************************/

static const char *getDiagnosticCodeStr(enum CXLoadDiag_Error error) {
  switch (error) {
    case CXLoadDiag_CannotLoad: return "Cannot Load File";
    case CXLoadDiag_None: break;
    case CXLoadDiag_Unknown: return "Unknown";
    case CXLoadDiag_InvalidFile: return "Invalid File";
  }
  return "None";
}

static const char *getSeverityString(enum CXDiagnosticSeverity severity) {
  switch (severity) {
    case CXDiagnostic_Note: return "note";
    case CXDiagnostic_Error: return "error";
    case CXDiagnostic_Fatal: return "fatal";
    case CXDiagnostic_Ignored: return "ignored";
    case CXDiagnostic_Warning: return "warning";
  }
  return "unknown";
}

static void printIndent(unsigned indent) {
  if (indent == 0)
    return;
  fprintf(stderr, "+");
  --indent;
  while (indent > 0) {
    fprintf(stderr, "-");
    --indent;
  }
}

static void printLocation(CXSourceLocation L) {
  CXFile File;
  CXString FileName;
  unsigned line, column, offset;
  
  lfort_getExpansionLocation(L, &File, &line, &column, &offset);
  FileName = lfort_getFileName(File);
  
  fprintf(stderr, "%s:%d:%d", lfort_getCString(FileName), line, column);
  lfort_disposeString(FileName);
}

static void printRanges(CXDiagnostic D, unsigned indent) {
  unsigned i, n = lfort_getDiagnosticNumRanges(D);
  
  for (i = 0; i < n; ++i) {
    CXSourceLocation Start, End;
    CXSourceRange SR = lfort_getDiagnosticRange(D, i);
    Start = lfort_getRangeStart(SR);
    End = lfort_getRangeEnd(SR);
    
    printIndent(indent);
    fprintf(stderr, "Range: ");
    printLocation(Start);
    fprintf(stderr, " ");
    printLocation(End);
    fprintf(stderr, "\n");
  }
}

static void printFixIts(CXDiagnostic D, unsigned indent) {
  unsigned i, n = lfort_getDiagnosticNumFixIts(D);
  fprintf(stderr, "Number FIXITs = %d\n", n);
  for (i = 0 ; i < n; ++i) {
    CXSourceRange ReplacementRange;
    CXString text;
    text = lfort_getDiagnosticFixIt(D, i, &ReplacementRange);
    
    printIndent(indent);
    fprintf(stderr, "FIXIT: (");
    printLocation(lfort_getRangeStart(ReplacementRange));
    fprintf(stderr, " - ");
    printLocation(lfort_getRangeEnd(ReplacementRange));
    fprintf(stderr, "): \"%s\"\n", lfort_getCString(text));
    lfort_disposeString(text);
  }  
}

static void printDiagnosticSet(CXDiagnosticSet Diags, unsigned indent) {
  unsigned i, n;

  if (!Diags)
    return;
  
  n = lfort_getNumDiagnosticsInSet(Diags);
  for (i = 0; i < n; ++i) {
    CXSourceLocation DiagLoc;
    CXDiagnostic D;
    CXFile File;
    CXString FileName, DiagSpelling, DiagOption, DiagCat;
    unsigned line, column, offset;
    const char *DiagOptionStr = 0, *DiagCatStr = 0;
    
    D = lfort_getDiagnosticInSet(Diags, i);
    DiagLoc = lfort_getDiagnosticLocation(D);
    lfort_getExpansionLocation(DiagLoc, &File, &line, &column, &offset);
    FileName = lfort_getFileName(File);
    DiagSpelling = lfort_getDiagnosticSpelling(D);
    
    printIndent(indent);
    
    fprintf(stderr, "%s:%d:%d: %s: %s",
            lfort_getCString(FileName),
            line,
            column,
            getSeverityString(lfort_getDiagnosticSeverity(D)),
            lfort_getCString(DiagSpelling));

    DiagOption = lfort_getDiagnosticOption(D, 0);
    DiagOptionStr = lfort_getCString(DiagOption);
    if (DiagOptionStr) {
      fprintf(stderr, " [%s]", DiagOptionStr);
    }
    
    DiagCat = lfort_getDiagnosticCategoryText(D);
    DiagCatStr = lfort_getCString(DiagCat);
    if (DiagCatStr) {
      fprintf(stderr, " [%s]", DiagCatStr);
    }
    
    fprintf(stderr, "\n");
    
    printRanges(D, indent);
    printFixIts(D, indent);
    
    /* Print subdiagnostics. */
    printDiagnosticSet(lfort_getChildDiagnostics(D), indent+2);

    lfort_disposeString(FileName);
    lfort_disposeString(DiagSpelling);
    lfort_disposeString(DiagOption);
  }  
}

static int read_diagnostics(const char *filename) {
  enum CXLoadDiag_Error error;
  CXString errorString;
  CXDiagnosticSet Diags = 0;
  
  Diags = lfort_loadDiagnostics(filename, &error, &errorString);
  if (!Diags) {
    fprintf(stderr, "Trouble deserializing file (%s): %s\n",
            getDiagnosticCodeStr(error),
            lfort_getCString(errorString));
    lfort_disposeString(errorString);
    return 1;
  }
  
  printDiagnosticSet(Diags, 0);
  fprintf(stderr, "Number of diagnostics: %d\n",
          lfort_getNumDiagnosticsInSet(Diags));
  lfort_disposeDiagnosticSet(Diags);
  return 0;
}

/******************************************************************************/
/* Command line processing.                                                   */
/******************************************************************************/

static CXCursorVisitor GetVisitor(const char *s) {
  if (s[0] == '\0')
    return FilteredPrintingVisitor;
  if (strcmp(s, "-usrs") == 0)
    return USRVisitor;
  if (strncmp(s, "-memory-usage", 13) == 0)
    return GetVisitor(s + 13);
  return NULL;
}

static void print_usage(void) {
  fprintf(stderr,
    "usage: c-index-test -code-completion-at=<site> <compiler arguments>\n"
    "       c-index-test -code-completion-timing=<site> <compiler arguments>\n"
    "       c-index-test -cursor-at=<site> <compiler arguments>\n"
    "       c-index-test -file-refs-at=<site> <compiler arguments>\n");
  fprintf(stderr,
    "       c-index-test -index-file [-check-prefix=<FileCheck prefix>] <compiler arguments>\n"
    "       c-index-test -index-file-full [-check-prefix=<FileCheck prefix>] <compiler arguments>\n"
    "       c-index-test -index-tu [-check-prefix=<FileCheck prefix>] <AST file>\n"
    "       c-index-test -index-compile-db [-check-prefix=<FileCheck prefix>] <compilation database>\n"
    "       c-index-test -test-file-scan <AST file> <source file> "
          "[FileCheck prefix]\n");
  fprintf(stderr,
    "       c-index-test -test-load-tu <AST file> <symbol filter> "
          "[FileCheck prefix]\n"
    "       c-index-test -test-load-tu-usrs <AST file> <symbol filter> "
           "[FileCheck prefix]\n"
    "       c-index-test -test-load-source <symbol filter> {<args>}*\n");
  fprintf(stderr,
    "       c-index-test -test-load-source-memory-usage "
    "<symbol filter> {<args>}*\n"
    "       c-index-test -test-load-source-reparse <trials> <symbol filter> "
    "          {<args>}*\n"
    "       c-index-test -test-load-source-usrs <symbol filter> {<args>}*\n"
    "       c-index-test -test-load-source-usrs-memory-usage "
          "<symbol filter> {<args>}*\n"
    "       c-index-test -test-annotate-tokens=<range> {<args>}*\n"
    "       c-index-test -test-inclusion-stack-source {<args>}*\n"
    "       c-index-test -test-inclusion-stack-tu <AST file>\n");
  fprintf(stderr,
    "       c-index-test -test-print-linkage-source {<args>}*\n"
    "       c-index-test -test-print-typekind {<args>}*\n"
    "       c-index-test -test-print-bitwidth {<args>}*\n"
    "       c-index-test -print-usr [<CursorKind> {<args>}]*\n"
    "       c-index-test -print-usr-file <file>\n"
    "       c-index-test -write-pch <file> <compiler arguments>\n");
  fprintf(stderr,
    "       c-index-test -compilation-db [lookup <filename>] database\n");
  fprintf(stderr,
    "       c-index-test -read-diagnostics <file>\n\n");
  fprintf(stderr,
    " <symbol filter> values:\n%s",
    "   all - load all symbols, including those from PCH\n"
    "   local - load all symbols except those in PCH\n"
    "   category - only load ObjC categories (non-PCH)\n"
    "   interface - only load ObjC interfaces (non-PCH)\n"
    "   protocol - only load ObjC protocols (non-PCH)\n"
    "   function - only load functions (non-PCH)\n"
    "   typedef - only load typdefs (non-PCH)\n"
    "   scan-function - scan function bodies (non-PCH)\n\n");
}

/***/

int cindextest_main(int argc, const char **argv) {
  lfort_enableStackTraces();
  if (argc > 2 && strcmp(argv[1], "-read-diagnostics") == 0)
      return read_diagnostics(argv[2]);
  if (argc > 2 && strstr(argv[1], "-code-completion-at=") == argv[1])
    return perform_code_completion(argc, argv, 0);
  if (argc > 2 && strstr(argv[1], "-code-completion-timing=") == argv[1])
    return perform_code_completion(argc, argv, 1);
  if (argc > 2 && strstr(argv[1], "-cursor-at=") == argv[1])
    return inspect_cursor_at(argc, argv);
  if (argc > 2 && strstr(argv[1], "-file-refs-at=") == argv[1])
    return find_file_refs_at(argc, argv);
  if (argc > 2 && strcmp(argv[1], "-index-file") == 0)
    return index_file(argc - 2, argv + 2, /*full=*/0);
  if (argc > 2 && strcmp(argv[1], "-index-file-full") == 0)
    return index_file(argc - 2, argv + 2, /*full=*/1);
  if (argc > 2 && strcmp(argv[1], "-index-tu") == 0)
    return index_tu(argc - 2, argv + 2);
  if (argc > 2 && strcmp(argv[1], "-index-compile-db") == 0)
    return index_compile_db(argc - 2, argv + 2);
  else if (argc >= 4 && strncmp(argv[1], "-test-load-tu", 13) == 0) {
    CXCursorVisitor I = GetVisitor(argv[1] + 13);
    if (I)
      return perform_test_load_tu(argv[2], argv[3], argc >= 5 ? argv[4] : 0, I,
                                  NULL);
  }
  else if (argc >= 5 && strncmp(argv[1], "-test-load-source-reparse", 25) == 0){
    CXCursorVisitor I = GetVisitor(argv[1] + 25);
    if (I) {
      int trials = atoi(argv[2]);
      return perform_test_reparse_source(argc - 4, argv + 4, trials, argv[3], I, 
                                         NULL);
    }
  }
  else if (argc >= 4 && strncmp(argv[1], "-test-load-source", 17) == 0) {
    CXCursorVisitor I = GetVisitor(argv[1] + 17);
    
    PostVisitPgm postVisit = 0;
    if (strstr(argv[1], "-memory-usage"))
      postVisit = PrintMemoryUsage;
    
    if (I)
      return perform_test_load_source(argc - 3, argv + 3, argv[2], I,
                                      postVisit);
  }
  else if (argc >= 4 && strcmp(argv[1], "-test-file-scan") == 0)
    return perform_file_scan(argv[2], argv[3],
                             argc >= 5 ? argv[4] : 0);
  else if (argc > 2 && strstr(argv[1], "-test-annotate-tokens=") == argv[1])
    return perform_token_annotation(argc, argv);
  else if (argc > 2 && strcmp(argv[1], "-test-inclusion-stack-source") == 0)
    return perform_test_load_source(argc - 2, argv + 2, "all", NULL,
                                    PrintInclusionStack);
  else if (argc > 2 && strcmp(argv[1], "-test-inclusion-stack-tu") == 0)
    return perform_test_load_tu(argv[2], "all", NULL, NULL,
                                PrintInclusionStack);
  else if (argc > 2 && strcmp(argv[1], "-test-print-linkage-source") == 0)
    return perform_test_load_source(argc - 2, argv + 2, "all", PrintLinkage,
                                    NULL);
  else if (argc > 2 && strcmp(argv[1], "-test-print-typekind") == 0)
    return perform_test_load_source(argc - 2, argv + 2, "all",
                                    PrintTypeKind, 0);
  else if (argc > 2 && strcmp(argv[1], "-test-print-bitwidth") == 0)
    return perform_test_load_source(argc - 2, argv + 2, "all",
                                    PrintBitWidth, 0);
  else if (argc > 1 && strcmp(argv[1], "-print-usr") == 0) {
    if (argc > 2)
      return print_usrs(argv + 2, argv + argc);
    else {
      display_usrs();
      return 1;
    }
  }
  else if (argc > 2 && strcmp(argv[1], "-print-usr-file") == 0)
    return print_usrs_file(argv[2]);
  else if (argc > 2 && strcmp(argv[1], "-write-pch") == 0)
    return write_pch_file(argv[2], argc - 3, argv + 3);
  else if (argc > 2 && strcmp(argv[1], "-compilation-db") == 0)
    return perform_test_compilation_db(argv[argc-1], argc - 3, argv + 2);

  print_usage();
  return 1;
}

/***/

/* We intentionally run in a separate thread to ensure we at least minimal
 * testing of a multithreaded environment (for example, having a reduced stack
 * size). */

typedef struct thread_info {
  int argc;
  const char **argv;
  int result;
} thread_info;
void thread_runner(void *client_data_v) {
  thread_info *client_data = client_data_v;
  client_data->result = cindextest_main(client_data->argc, client_data->argv);
#ifdef __CYGWIN__
  fflush(stdout);  /* stdout is not flushed on Cygwin. */
#endif
}

int main(int argc, const char **argv) {
  thread_info client_data;

#ifdef LFORT_HAVE_LIBXML
  LIBXML_TEST_VERSION
#endif

  if (getenv("CINDEXTEST_NOTHREADS"))
    return cindextest_main(argc, argv);

  client_data.argc = argc;
  client_data.argv = argv;
  lfort_executeOnThread(thread_runner, &client_data, 0);
  return client_data.result;
}
