/*
 * choicebox - Evas virtual listbox widget
 *
 * © 2009 Mikhail Gusarov <dottedmag@dottedmag.net>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <libchoicebox.h>

#include <stdbool.h>
#include <Edje.h>
#include <ctype.h>
#include <stdio.h>
#include <libintl.h>
#include <string.h>

#include <libkeys.h>

#define LEFT_ARROW "«"
#define RIGHT_ARROW "»"
#define NO_ARROW " "

static keys_t *_keys = NULL;

void
choicebox_aux_edje_footer_handler(Evas_Object * footer, const char *part,
                                  int cur_page, int total_pages)
{
    if (total_pages < 2)
        edje_object_part_text_set(footer, part, "");
    else {
        char buf[256];
        snprintf(buf, 256, dgettext("choicebox", "%d / %d"),
                 cur_page + 1, total_pages);
        char buf2[266];
        snprintf(buf2, 266, "%s %s %s",
                 cur_page ? LEFT_ARROW : NO_ARROW,
                 buf, cur_page < total_pages - 1 ? RIGHT_ARROW : NO_ARROW);
        edje_object_part_text_set(footer, part, buf2);
    }
}

bool
choicebox_aux_key_up_handler(Evas_Object * o, Evas_Event_Key_Up * ev)
{
    if (!_keys)
        _keys = keys_alloc("choicebox");
    if (!_keys)
        return false;

    const char *action = keys_lookup_by_event(_keys, "default", ev);

    if (!action)
        return false;

    if (!strcmp(action, "Previous"))
        choicebox_prev(o);
    else if (!strcmp(action, "Next"))
        choicebox_next(o);
    else if (!strcmp(action, "PageUp"))
        choicebox_prevpage(o);
    else if (!strncmp(action, "PagesUp,", 8)) {
        int pages = atoi(action + 8);
        if (pages <= 0)
            return false;
        choicebox_prev_pages(o, pages);
    } else if (!strcmp(action, "PageDown"))
        choicebox_nextpage(o);
    else if (!strncmp(action, "PagesDown,", 10)) {
        int pages = atoi(action + 10);
        if (pages <= 0)
            return false;
        choicebox_next_pages(o, pages);
    } else if (!strncmp(action, "Activate,", 9)) {
        int item = atoi(action + 9);
        if (item < 0)
            return false;
        choicebox_activate_nth_visible(o, item, false);
    } else if (!strncmp(action, "AltActivate,", 12)) {
        int item = atoi(action + 12);
        if (item < 0)
            return false;
        choicebox_activate_nth_visible(o, item, true);
    } else if (!strcmp(action, "ActivateCurrent"))
        choicebox_activate_current(o, false);
    else if (!strcmp(action, "AltActivateCurrent"))
        choicebox_activate_current(o, true);
    else if (!strcmp(action, "RequestClose"))
        choicebox_request_close(o);
    else
        return false;

    return true;
}

static void
_choicebox_aux_key_up_handler(void *param, Evas * e,
                              Evas_Object * o, void *event_info)
{
    choicebox_aux_key_up_handler(o, (Evas_Event_Key_Up *) event_info);
}

void
choicebox_aux_subscribe_key_up(Evas_Object * choicebox)
{
    evas_object_event_callback_add(choicebox, EVAS_CALLBACK_KEY_UP,
                                   &_choicebox_aux_key_up_handler, NULL);
}
