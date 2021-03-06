/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#include <ctype.h>
#include <string.h>

#include "c_console.h"
#include "doomtype.h"
#include "doomdef.h"
#include "m_cheat.h"
#include "m_misc.h"

//
// CHEAT SEQUENCE PACKAGE
//

//
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
char    cheatkey = '\0';

int cht_CheckCheat(cheatseq_t *cht, char key)
{
    if (consolecheat[0] && !strcasecmp(consolecheat, cht->sequence))
    {
        consolecheat[0] = 0;
        if (consolecheatparm)
        {
            cht->parameter_buf[0] = consolecheatparm[0];
            cht->parameter_buf[1] = consolecheatparm[1];
            cht->param_chars_read = 2;
            consolecheatparm[0] = 0;
        }
        return true;
    }

    // [BH] you have two seconds to enter each character of a cheat sequence
    if (!idbehold)
    {
        if (cht->timeout)
            if (leveltime - cht->timeout > TIMELIMIT)
                cht->chars_read = cht->param_chars_read = 0;
        cht->timeout = leveltime;
    }

    if (cht->chars_read < strlen(cht->sequence))
    {
        // still reading characters from the cheat code
        // and verifying. reset back to the beginning
        // if a key is wrong
        if (toupper(key) == toupper(cht->sequence[cht->chars_read]))
            ++cht->chars_read;
        else
            // [BH] recognise key as first in sequence if it matches, rather than resetting
            cht->chars_read = (toupper(key) == toupper(cht->sequence[0]));

        cht->param_chars_read = 0;

        if (cht->chars_read)
            if (!cht->actionkey || (cht->actionkey && cht->chars_read > 1))
                cheatkey = key;
    }
    else if (cht->param_chars_read < cht->parameter_chars)
    {
        // we have passed the end of the cheat sequence and are
        // entering parameters now
        if (!isdigit(key))
        {
            cht->chars_read = cht->param_chars_read = cht->timeout = 0;
            return false;
        }
        else
        {
            cht->parameter_buf[cht->param_chars_read] = key;

            ++cht->param_chars_read;
        }
    }

    if (cht->chars_read >= strlen(cht->sequence)
        && cht->param_chars_read >= cht->parameter_chars)
    {
        cht->chars_read = cht->param_chars_read = cht->timeout = 0;
        return true;
    }

    // cheat not matched yet
    return false;
}

void cht_GetParam(cheatseq_t *cht, char *buffer)
{
    memcpy(buffer, cht->parameter_buf, cht->parameter_chars);
}
