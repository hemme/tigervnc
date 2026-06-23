
/*

Copyright 1990, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#include "keysymdef.h"
#include "KeysymStr.h"

/* Change the name of this to avoid conflict with libX11 */
#define _XkeyTable _vncXkeyTable

#define NEEDKTABLE
#define NEEDVTABLE
#include "ks_tables.h"

static const char *_XKeysymToString(unsigned ks)
{
    if (!ks || (ks & ((unsigned long) ~0x1fffffff)) != 0)
	return ((char *)NULL);
    if (ks == XK_VoidSymbol)
	ks = 0;
    if (ks <= 0x1fffffff)
    {
	unsigned char val1 = ks >> 24;
	unsigned char val2 = (ks >> 16) & 0xff;
	unsigned char val3 = (ks >> 8) & 0xff;
	unsigned char val4 = ks & 0xff;
	int i = ks % VTABLESIZE;
	int h = i + 1;
	int n = VMAXHASH;
	int idx;
	while ((idx = hashKeysym[i]))
	{
	    const unsigned char *entry = &_XkeyTable[idx];
	    if ((entry[0] == val1) && (entry[1] == val2) &&
                (entry[2] == val3) && (entry[3] == val4))
		return ((char *)entry + 4);
	    if (!--n)
		break;
	    i += h;
	    if (i >= VTABLESIZE)
		i -= VTABLESIZE;
	}
    }

    if (ks >= 0x01000100 && ks <= 0x0110ffff) {
        unsigned val = ks & 0xffffff;
        char *s;
        int i;
        if (val & 0xff0000)
            i = 10;
        else
            i = 6;
        s = malloc(i);
        if (s == NULL)
            return s;
        i--;
        s[i--] = '\0';
        for (; i; i--){
            unsigned char val1 = val & 0xf;
            val >>= 4;
            if (val1 < 10)
                s[i] = '0'+ val1;
            else
                s[i] = 'A'+ val1 - 10;
        }
        s[i] = 'U';
        return s;
    }
    return ((char *) NULL);
}

const char* KeySymName(unsigned keysym)
{
    const char* name;

    name = _XKeysymToString(keysym);
    if (name == NULL)
      return "[unknown keysym]";

    return name;
}

unsigned int StringToKeysym(const char* name)
{
    if (name == NULL || name[0] == '\0')
        return 0;

    // Direct key names aliases for user convenience
    if (strcasecmp(name, "Ctrl") == 0 || strcasecmp(name, "Control") == 0)
        name = "Control_L";
    else if (strcasecmp(name, "Alt") == 0)
        name = "Alt_L";
    else if (strcasecmp(name, "Shift") == 0)
        name = "Shift_L";
    else if (strcasecmp(name, "Win") == 0 || strcasecmp(name, "Super") == 0 || strcasecmp(name, "Cmd") == 0 || strcasecmp(name, "Command") == 0)
        name = "Super_L";
    else if (strcasecmp(name, "Ctrl_R") == 0 || strcasecmp(name, "Control_R") == 0)
        name = "Control_R";
    else if (strcasecmp(name, "Alt_R") == 0 || strcasecmp(name, "AltGr") == 0)
        name = "Mode_switch";
    else if (strcasecmp(name, "Shift_R") == 0)
        name = "Shift_R";
    else if (strcasecmp(name, "Win_R") == 0 || strcasecmp(name, "Super_R") == 0)
        name = "Super_R";

    // Check if it is a hex code or Unicode code (e.g., U1234 or 0x1234)
    if (name[0] == 'U' || name[0] == 'u') {
        char *endptr;
        unsigned int val = strtoul(name + 1, &endptr, 16);
        if (*endptr == '\0') {
            return val | 0x01000000;
        }
    }
    if (name[0] == '0' && (name[1] == 'x' || name[1] == 'X')) {
        char *endptr;
        unsigned int val = strtoul(name + 2, &endptr, 16);
        if (*endptr == '\0') {
            return val;
        }
    }

    size_t p = 1;
    while (p < sizeof(_XkeyTable)) {
        unsigned int ks = ((unsigned int)_XkeyTable[p + 2] << 24) |
                          ((unsigned int)_XkeyTable[p + 3] << 16) |
                          ((unsigned int)_XkeyTable[p + 4] << 8) |
                          (unsigned int)_XkeyTable[p + 5];
        const char* entry_name = (const char*)&_XkeyTable[p + 6];
        if (strcasecmp(entry_name, name) == 0) {
            if (ks == 0)
                return 0xffffff; // XK_VoidSymbol
            return ks;
        }
        size_t len = strlen(entry_name);
        p += len + 7;
    }
    return 0;
}

