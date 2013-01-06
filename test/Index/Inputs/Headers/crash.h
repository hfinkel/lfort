// Produce a crash if CRASH is defined.
#ifdef CRASH
#  pragma lfort __debug crash
#endif

const char *getCrashString();
