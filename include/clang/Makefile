LFORT_LEVEL := ../..
DIRS := AST Basic Driver Lex Parse Sema Serialization

include $(LFORT_LEVEL)/Makefile

install-local::
	$(Echo) Installing LFort include files
	$(Verb) $(MKDIR) $(DESTDIR)$(PROJ_includedir)
	$(Verb) if test -d "$(PROJ_SRC_DIR)" ; then \
	  cd $(PROJ_SRC_DIR)/.. && \
	  for  hdr in `find lfort -type f \
	      '(' -name LICENSE.TXT \
	       -o -name '*.def' \
	       -o -name '*.h' \
	       -o -name '*.inc' \
	      ')' -print \
              | grep -v CVS | grep -v .svn | grep -v .dir` ; do \
	    instdir=$(DESTDIR)`dirname "$(PROJ_includedir)/$$hdr"` ; \
	    if test \! -d "$$instdir" ; then \
	      $(EchoCmd) Making install directory $$instdir ; \
	      $(MKDIR) $$instdir ;\
	    fi ; \
	    $(DataInstall) $$hdr $(DESTDIR)$(PROJ_includedir)/$$hdr ; \
	  done ; \
	fi
ifneq ($(PROJ_SRC_ROOT),$(PROJ_OBJ_ROOT))
	$(Verb) if test -d "$(PROJ_OBJ_ROOT)/tools/lfort/include/lfort" ; then \
	  cd $(PROJ_OBJ_ROOT)/tools/lfort/include && \
	  for hdr in `find lfort -type f \
	      '(' -name LICENSE.TXT \
	       -o -name '*.def' \
	       -o -name '*.h' \
	       -o -name '*.inc' \
	      ')' -print \
            | grep -v CVS | grep -v .tmp | grep -v .dir` ; do \
	    instdir=$(DESTDIR)`dirname "$(PROJ_includedir)/$$hdr"` ; \
	    if test \! -d "$$instdir" ; then \
	      $(EchoCmd) Making install directory $$instdir ; \
	      $(MKDIR) $$instdir ;\
	    fi ; \
	    $(DataInstall) $$hdr $(DESTDIR)$(PROJ_includedir)/$$hdr ; \
	  done ; \
	fi
endif
