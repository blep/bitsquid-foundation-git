#pragma once

/* This header is a proxy to include windows.h 
 * in a lightweight way (reducing the number
 * of indirect includes) to speed up compilation.
 */

#if defined(_WIN32)

// Limits headers included by Windows.h
# define WIN32_LEAN_AND_MEAN
# define NOSERVICE
# define NOMCX
# define NOIME
# define NOSOUND
# define NOCOMM
# define NORPC
# define NOGDI
# define NOUSER
# define NODRIVERS
# define NOLOGERROR
# define NOPROFILER
# define NOMEMMGR
# define NOLFILEIO
# define NOOPENFILE
# define NORESOURCE
# define NOATOM
# define NOLANGUAGE
# define NOLSTRING
# define NODBCS
# define NOKEYBOARDINFO
# define NOGDICAPMASKS
# define NOCOLOR
# define NOGDIOBJ
# define NODRAWTEXT
# define NOTEXTMETRIC
# define NOSCALABLEFONT
# define NOBITMAP
# define NORASTEROPS
# define NOMETAFILE
# define NOSYSMETRICS
# define NOSYSTEMPARAMSINFO
# define NOMSG
# define NOWINSTYLES
# define NOWINOFFSETS
# define NOSHOWWINDOW
# define NODEFERWINDOWPOS
# define NOVIRTUALKEYCODES
# define NOKEYSTATES
# define NOWH
# define NOMENUS
# define NOSCROLL
# define NOCLIPBOARD
# define NOICONS
# define NOMB
# define NOSYSCOMMANDS
# define NOMDI
# define NOCTLMGR
# define NOWINMESSAGES
# include <windows.h>
#endif
