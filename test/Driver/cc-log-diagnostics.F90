! RUN: rm -f %t.log
! RUN: env RC_DEBUG_OPTIONS=1 \
! RUN:     CC_LOG_DIAGNOSTICS=1 CC_LOG_DIAGNOSTICS_FILE=%t.log \
! RUN: %lfort -Wfoobar -no-canonical-prefixes -target x86_64-apple-darwin10 -fsyntax-only %s
! RUN: FileCheck %s < %t.log

program test

end program bar

! CHECK: <dict>
! CHECK:   <key>main-file</key>
! CHECK:   <string>{{.*}}cc-log-diagnostics.F90</string>
! CHECK:   <key>dwarf-debug-flags</key>
! CHECK:   <string>{{.*}}lfort{{.*}}-fsyntax-only{{.*}}</string>
! CHECK:   <key>diagnostics</key>
! CHECK:   <array>
! CHECK:     <dict>
! CHECK:       <key>level</key>
! CHECK:       <string>warning</string>
! CHECK:       <key>message</key>
! CHECK:       <string>unknown warning option &apos;-Wfoobar&apos;; did you mean &apos;-W{{.*}}&apos;?</string>
! CHECK:     </dict>
! CHECK:     <dict>
! CHECK:       <key>level</key>
! CHECK:       <string>warning</string>
! CHECK:       <key>filename</key>
! CHECK:       <string>{{.*}}cc-log-diagnostics.F90</string>
! CHECK:       <key>line</key>
! CHECK:       <integer>9</integer>
! CHECK:       <key>column</key>
! CHECK:       <integer>13</integer>
! CHECK:       <key>message</key>
! CHECK:       <string>end program name does not match program name &apos;test&apos;</string>
! CHECK:     </dict>
! CHECK:   </array>
! CHECK: </dict>
