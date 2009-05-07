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

typedef void (*choicebox_footer_draw_handler_t)(Evas_Object* choicebox,
                                                Evas_Object* footer,
                                                int cur_page,
                                                int total_pages,
                                                void* param);

/*
 * Choicebox uses two groups in theme file, called $group_prefix/item and
 * $group_prefix/footer.
 *
 * Both groups should have data item called 'min_height'. As soon as edje can
 * read the min size from the group, this requirement will be lifted.
 *
 * Items will be sent the following signals:
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
                           const char* group_prefix,
                           choicebox_handler_t handler,
                           choicebox_draw_handler_t draw_handler,
                           choicebox_footer_draw_handler_t footer_draw_handler,
                           void* param);

void choicebox_set_size(Evas_Object* e, int size);
void choicebox_invalidate_item(Evas_Object* e, int item_num);

/* This is mostly for keyboard handling */

void choicebox_prev(Evas_Object* e);
void choicebox_next(Evas_Object* e);
void choicebox_prevpage(Evas_Object* e);
void choicebox_nextpage(Evas_Object* e);

void choicebox_activate_nth_visible(Evas_Object* e, int nth, bool is_alt);
void choicebox_activate_current(Evas_Object* e, bool is_alt);

#endif
