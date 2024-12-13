#pragma once

#ifndef REKSI_CUSTOM_SP
#define REKSI_CUSTOM_SP 0
#else
#define REKSI_CUSTOM_SP 1
#endif

#ifndef REKSI_THREADING
#define REKSI_THREADING 1
#endif

/*
 * Debug Definition
 */
#if !defined(REKSI_DEBUG) && !defined(REKSI_NDEBUG)
 // Set as debug
#define REKSI_DEBUG
#endif