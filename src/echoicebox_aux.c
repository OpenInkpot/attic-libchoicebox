#include "echoicebox.h"

#include <Edje.h>
#include <ctype.h>
#include <stdio.h>
#include <libintl.h>
#include <string.h>

void choicebox_aux_edje_footer_handler(Evas_Object* footer, const char* part,
                                       int cur_page, int total_pages)
{
    if(total_pages < 2)
        edje_object_part_text_set(footer, part, "");
    else
    {
        char buf[256];
        snprintf(buf, 256, dgettext("echoicebox", "%d / %d"),
                 cur_page + 1, total_pages);
        edje_object_part_text_set(footer, part, buf);
    }
}

static void _activate(Evas_Object* o, char numchar, bool is_alt)
{
    if(numchar == '0')
        choicebox_activate_nth_visible(o, 10, is_alt);
    else
        choicebox_activate_nth_visible(0, numchar - '1', is_alt);
}

void choicebox_aux_key_down_handler(Evas_Object* o,
                                    Evas_Event_Key_Down* ev)
{
    const char* k = ev->keyname;

    if(!strcmp(k, "Up") || !strcmp(k, "Prior"))
        choicebox_prev(o);
    if(!strcmp(k, "Down") || !strcmp(k, "Next"))
        choicebox_next(o);
    if(!strcmp(k, "Left"))
        choicebox_prevpage(o);
    if(!strcmp(k, "Right"))
        choicebox_nextpage(o);
    if(!strncmp(k, "KP_", 3) && (isdigit(k[3])) && !k[4])
        _activate(o, k[3], false);
    if(isdigit(k[0]) && !k[1])
        _activate(o, k[0], false);
    if(!strcmp(k, "Return") || !strcmp(k, "KP_Return"))
        choicebox_activate_current(o, false);
}
