# We need to execute this script at installation time because the
# DESTDIR environment variable may be unset at configuration time.
# See PR8397.

if(UNIX)
  set(LFORTXX_LINK_OR_COPY create_symlink)
  set(LFORTXX_DESTDIR $ENV{DESTDIR})
else()
  set(LFORTXX_LINK_OR_COPY copy)
endif()

# CMAKE_EXECUTABLE_SUFFIX is undefined on cmake scripts. See PR9286.
if( WIN32 )
  set(EXECUTABLE_SUFFIX ".exe")
else()
  set(EXECUTABLE_SUFFIX "")
endif()

set(bindir "${LFORTXX_DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/")
set(lfort "lfort${EXECUTABLE_SUFFIX}")
set(lfortxx "lfort++${EXECUTABLE_SUFFIX}")

message("Creating lfort++ executable based on ${lfort}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E ${LFORTXX_LINK_OR_COPY} "${lfort}" "${lfortxx}"
  WORKING_DIRECTORY "${bindir}")
