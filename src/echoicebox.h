/*
 * echoicebox -- virtual listbox smart object for evas
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

#ifndef ECHOICEBOX_H
#define ECHOICEBOX_H

#include <stdbool.h>
#include <Evas.h>

typedef void (*choicebox_handler_t)(Evas_Object* choicebox,
                                    int item_num,
                                    bool is_alt,
                                    void* param);

typedef void (*choicebox_draw_handler_t)(Evas_Object* choicebox,
                                         Evas_Object* item,
                                         int item_num,
                                         int page_position,
                                         void* param);

typedef void (*choicebox_page_updated_t)(Evas_Object* choicebox,
                                         int cur_page,
                                         int total_pages,
                                         void* param);

/*
 * Choicebox uses groups $item_group from theme file.
 *
 * This group should have data item called 'min_height'. As soon as edje can
 * read the min size from the group, this requirement will be lifted.
 *
 * Instances will be sent the following signals:
 * - select, for setting selected state
 * - deselect, for unsetting selected state
 * - set_even, for setting "even" variant of state, if necessary
 * - set_odd, for setting "odd" variant of state, if necessary
 * - empty, for setting "empty" variant of state, for items beyond the end of list
 *
 * Choicebox does not manipulate content of the items, providing callbacks for
 * this.
 */
Evas_Object* choicebox_new(Evas* evas,
                           const char* theme_file, /* It sucks, do you know better way? */
                           const char* item_group,
                           choicebox_handler_t handler,
                           choicebox_draw_handler_t draw_handler,
                           choicebox_page_updated_t page_handler,
                           void* param);

void choicebox_set_size(Evas_Object* e, int size);
void choicebox_invalidate_item(Evas_Object* e, int item_num);
/* Invalidates [item_from,item_to) half-open interval */
void choicebox_invalidate_interval(Evas_Object* e, int item_from, int item_to);

/* This is mostly for keyboard handling */

void choicebox_prev(Evas_Object* e);
void choicebox_next(Evas_Object* e);
void choicebox_prevpage(Evas_Object* e);
void choicebox_nextpage(Evas_Object* e);

void choicebox_activate_nth_visible(Evas_Object* e, int nth, bool is_alt);
void choicebox_activate_current(Evas_Object* e, bool is_alt);

void choicebox_set_selection(Evas_Object* e, int item_num);
/*
 * Returns -1 if no item is selected.
 */
int choicebox_get_selection(Evas_Object* e);

/*
 * Auxiliary utility functions
 */

/*
 * Sample implementation of footer (page) handler, drawing (l10ed) "N/M" text in
 * given edje text field.
 *
 * Usage:
 *
 * void page_handler(Evas_Object* choicebox, int cur_page, int total_pages, void* param)
 * {
 *     Evas* canvas = evas_object_evas_get(choicebox);
 *     Evas_Object* footer = evas_object_name_find(canvas, "footer");
 *
 *     choicebox_aux_footer_handler(footer, "pagination", cur_page, total_pages);
 * }
 */
void choicebox_aux_edje_footer_handler(Evas_Object* footer, const char* part,
                                      int cur_page, int total_pages);

/*
 * Sample implementation of Key_Down handler:
 * - 1,2..,9,0 activate 0..10th item
 * - Up/Down moves cursor up/down
 * - Left/Right/Prior/Next switches page
 * - Enter activates current item
 *
 * Note that Escape is not handled!
 *
 * Usage:
 *
 * void key_down_handler(void* param, Evas* e, Evas_Object* o, void* event_info)
 * {
 *     Evas_Event_Key_down* ev = (Evas_Event_Key_Down*)event_info;
 *     / * custom processing as needed * /
 *
 *     choicebox_aux_key_down_handler(o, ev);
 * }
 */

void choicebox_aux_key_down_handler(Evas_Object* choicebox,
                                    Evas_Event_Key_Down* ev);

#endif
