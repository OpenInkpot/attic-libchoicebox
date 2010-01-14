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

#define _GNU_SOURCE

#include <libintl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_X.h>
#include <Edje.h>

#include <libchoicebox.h>

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static int
exit_handler(void *param, int ev_type, void *event)
{
    ecore_main_loop_quit();
    return 1;
}

static void
main_win_close_handler(Ecore_Evas * main_win)
{
    ecore_main_loop_quit();
}

static void
exit_app(void *param)
{
    ecore_main_loop_quit();
}

static bool is_hinted;

static void
close_handler(Evas_Object * choicebox, void *param)
{
    is_hinted = !is_hinted;
    choicebox_set_hinted(choicebox, is_hinted);
}

static void
draw_handler(Evas_Object * choicebox, Evas_Object * item,
             int item_num, int page_position, void *param)
{
    char f[512];
    sprintf(f, "Item %d", item_num);
    edje_object_part_text_set(item, "text", f);
}

static void
page_handler(Evas_Object * choicebox, int cur_page, int total_pages,
             void *param)
{
}

static void
item_handler(Evas_Object * choicebox, int item_num, bool is_alt,
             void *param)
{
    printf("Item %d %d\n", item_num, is_alt ? 1 : 0);
}

static void
main_win_resize_handler(Ecore_Evas * main_win)
{
    Evas *canvas = ecore_evas_get(main_win);
    int w, h;
    evas_output_size_get(canvas, &w, &h);

    Evas_Object *choicebox = evas_object_name_find(canvas, "choicebox");
    evas_object_resize(choicebox, w, h);
}

static void
run()
{
    ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);

    Ecore_Evas *main_win =
        ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
    ecore_evas_title_set(main_win, "Language Selector");
    ecore_evas_name_class_set(main_win, "language-selector",
                              "language-selector");

    Evas *main_canvas = ecore_evas_get(main_win);

    ecore_evas_callback_delete_request_set(main_win,
                                           main_win_close_handler);

    choicebox_info_t info = {
        NULL,
        "../themes/choicebox.edj",
        "full",
        "../themes/choicebox.edj",
        "item-default",
        item_handler,
        draw_handler,
        page_handler,
        close_handler,
    };

    Evas_Object *choicebox = choicebox_new(main_canvas, &info, NULL);
    if (!choicebox)
        die("Unable to create choicebox");

    choicebox_set_size(choicebox, 60000);
    evas_object_name_set(choicebox, "choicebox");
    evas_object_show(choicebox);

    evas_object_focus_set(choicebox, true);
    choicebox_aux_subscribe_key_up(choicebox);

    ecore_evas_callback_resize_set(main_win, main_win_resize_handler);
    ecore_evas_show(main_win);

    ecore_x_io_error_handler_set(exit_app, NULL);

    ecore_main_loop_begin();
}

int
main(int argc, char **argv)
{
    if (!evas_init())
        die("Unable to initialize Evas\n");
    if (!ecore_init())
        die("Unable to initialize Ecore\n");
    if (!ecore_evas_init())
        die("Unable to initialize Ecore_Evas\n");
    if (!edje_init())
        die("Unable to initialize Edje\n");

    run();

    edje_shutdown();
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
    return 0;
}
