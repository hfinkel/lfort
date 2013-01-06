LFORT_LEVEL := ../..
DIRS :=

include $(LFORT_LEVEL)/Makefile

IntIncludeDir = $(DESTDIR)$(PROJ_internal_prefix)/include

install-local::
	$(Echo) Installing LFort C API include files
	$(Verb) $(MKDIR) $(IntIncludeDir)
	$(Verb) if test -d "$(PROJ_SRC_DIR)" ; then \
	  cd $(PROJ_SRC_DIR)/.. && \
	  for  hdr in `find lfort-c -type f '!' '(' -name '*~' \
	      -o -name '.#*' -o -name '*.in' -o -name '*.txt' \
	      -o -name 'Makefile' -o -name '*.td' ')' -print \
              | grep -v CVS | grep -v .svn | grep -v .dir` ; do \
	    instdir=`dirname "$(IntIncludeDir)/$$hdr"` ; \
	    if test \! -d "$$instdir" ; then \
	      $(EchoCmd) Making install directory $$instdir ; \
	      $(MKDIR) $$instdir ;\
	    fi ; \
	    $(DataInstall) $$hdr $(IntIncludeDir)/$$hdr ; \
	  done ; \
	fi
ifneq ($(PROJ_SRC_ROOT),$(PROJ_OBJ_ROOT))
	$(Verb) if test -d "$(PROJ_OBJ_ROOT)/tools/lfort/include/lfort-c" ; then \
	  cd $(PROJ_OBJ_ROOT)/tools/lfort/include && \
	  for hdr in `find lfort-c -type f '!' '(' -name 'Makefile' ')' -print \
            | grep -v CVS | grep -v .tmp | grep -v .dir` ; do \
	    instdir=`dirname "$(IntIncludeDir)/$$hdr"` ; \
	    if test \! -d "$$instdir" ; then \
	      $(EchoCmd) Making install directory $$instdir ; \
	      $(MKDIR) $$instdir ;\
	    fi ; \
	    $(DataInstall) $$hdr $(IntIncludeDir)/$$hdr ; \
	  done ; \
	fi
endif
