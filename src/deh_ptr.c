/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomtype.h"
#include "info.h"

#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"

static actionf_t codeptrs[NUMSTATES];

static int CodePointerIndex(actionf_t *ptr)
{
    int i;

    for (i = 0; i < NUMSTATES; ++i)
        if (!memcmp(&codeptrs[i], ptr, sizeof(actionf_t)))
            return i;

    return -1;
}

static void DEH_PointerInit(void)
{
    int i;
    
    // Initialize list of dehacked pointers
    for (i = 0; i < NUMSTATES; ++i)
        codeptrs[i] = states[i].action;
}

static void *DEH_PointerStart(deh_context_t *context, char *line)
{
    int frame_number = 0;

    if (sscanf(line, "Pointer %*i (%*s %i)", &frame_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return NULL;
    }

    if (frame_number < 0 || frame_number >= NUMSTATES)
    {
        DEH_Warning(context, "Invalid frame number: %i", frame_number);
        return NULL;
    }

    return &states[frame_number];
}

static void DEH_PointerParseLine(deh_context_t *context, char *line, void *tag)
{
    state_t     *state;
    char        *variable_name, *value;
    int         ivalue;
    
    if (tag == NULL)
       return;

    state = (state_t *) tag;

    // Parse the assignment
    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse
        DEH_Warning(context, "Failed to parse assignment");
        return;
    }

    // all values are integers
    ivalue = atoi(value);
    
    // set the appropriate field
    if (!strcasecmp(variable_name, "Codep frame"))
    {
        if (ivalue < 0 || ivalue >= NUMSTATES)
            DEH_Warning(context, "Invalid state '%i'", ivalue);
        else
            state->action = codeptrs[ivalue];
    }
    else
        DEH_Warning(context, "Unknown variable name '%s'", variable_name);
}

static void DEH_PointerSHA1Sum(sha1_context_t *context)
{
    int i;

    for (i=0; i<NUMSTATES; ++i)
        SHA1_UpdateInt32(context, CodePointerIndex(&states[i].action));
}

deh_section_t deh_section_pointer =
{
    "Pointer",
    DEH_PointerInit,
    DEH_PointerStart,
    DEH_PointerParseLine,
    NULL,
    DEH_PointerSHA1Sum,
};