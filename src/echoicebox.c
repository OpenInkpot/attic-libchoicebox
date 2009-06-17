/*
 * choicebox -- virtual listbox smart object for evas
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

#define _GNU_SOURCE 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <Edje.h>
#include "echoicebox.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))

typedef struct
{
    int size;
    int sel;
    int top_item;
    int pagesize;
} choicebox_state_t;

typedef struct
{
    /*
     * Invariant: st.sel == -1 || (st.sel >= st.top_item
     *                            && st.sel < (st.top_item + st.page_size))
     * Invariant: st.size == 0 || st.top_item < st.size
     */
    choicebox_state_t st;

    /* Data */
    choicebox_handler_t handler;
    choicebox_draw_handler_t draw_handler;
    choicebox_page_updated_t page_handler;
    void* param;

    /* Theme info */
    char* theme_file;
    char* item_group;
    int item_minh;

    /* Widgets */
    Evas_Object* clip;
    Eina_Array* items;

} choicebox_t;

typedef struct
{
    bool is_used;
    bool is_selected;
    int num;
} _choicebox_item_info_t;

static _choicebox_item_info_t _choicebox_calc_item_info(
    int nth,
    const choicebox_state_t* state)
{
    _choicebox_item_info_t item_info = {};

    if(nth >= state->pagesize)
    {
        item_info.num = -1;
        return item_info;
    }

    item_info.num = state->top_item + nth;
    if(item_info.num == state->sel)
        item_info.is_selected = true;
    if(item_info.num < state->size)
        item_info.is_used = true;

    return item_info;
}

static void _choicebox_update_item(Evas_Object* o, int nth,
                                   const _choicebox_item_info_t* old,
                                   const _choicebox_item_info_t* new)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    Evas_Object* item = eina_array_data_get(data->items, nth);

    if(old->is_selected && !new->is_selected)
        edje_object_signal_emit(item, "deselect", "");
    if(!old->is_selected && new->is_selected)
        edje_object_signal_emit(item, "select", "");

    if(!new->is_used)
    {
        if(old->is_used)
            edje_object_signal_emit(item, "empty", "");
        return;
    }

    if(!old->is_used)
    {
        if(nth%2)
            edje_object_signal_emit(item, "set_odd", "");
        else
            edje_object_signal_emit(item, "set_even", "");
    }

    if(!old->is_used || old->num != new->num)
        (*data->draw_handler)(o, item, new->num, nth, data->param);
}

static void _run_page_handler(Evas_Object* o)
{
    choicebox_t* data = evas_object_smart_data_get(o);

   int page = data->st.pagesize ? DIV_CEIL(data->st.top_item, data->st.pagesize) : 0;
   int pages = data->st.pagesize ? DIV_CEIL(data->st.size, data->st.pagesize) : 0;

   (*data->page_handler)(o, page, pages, data->param);
}

static void _choicebox_update(Evas_Object* o, const choicebox_state_t* new)
{
    choicebox_t* data = evas_object_smart_data_get(o);

    choicebox_state_t old = data->st;
    data->st = *new;

    int i;
    for(i = 0; i < new->pagesize; ++i)
    {
        _choicebox_item_info_t old_i = _choicebox_calc_item_info(i, &old);
        _choicebox_item_info_t new_i = _choicebox_calc_item_info(i, new);
        _choicebox_update_item(o, i, &old_i, &new_i);
    }

    if(old.top_item != new->top_item || old.pagesize != new->pagesize)
       _run_page_handler(o);
}

/**
 * Activates given item.
 *
 * No bounds checking.
 */
static void _choicebox_activate(Evas_Object* o, int item_num, bool is_alt)
{
    choicebox_t* data = evas_object_smart_data_get(o);

    if(data->handler)
        (*data->handler)(o, item_num, is_alt, data->param);
}

static void _choicebox_display(Evas_Object* o, int ox, int oy, int ow, int oh)
{
    choicebox_t* data = evas_object_smart_data_get(o);

    choicebox_state_t new = data->st;

    /** Update clip **/

    evas_object_move(data->clip, ox, oy);
    evas_object_resize(data->clip, ow, oh);

    /** Update items **/

    /* Calculate pagesize */
    new.pagesize = oh / data->item_minh;

    /* Fix the widgets amount */
    int curitems = eina_array_count_get(data->items);
    if(new.pagesize > curitems)
    {
        Evas* evas = evas_object_evas_get(o);

        int i;
        for(i = 0; i < new.pagesize - curitems; ++i)
        {
            Evas_Object* item = edje_object_add(evas);
            char f[256];
            snprintf(f, 256, "choicebox/%p/item/%d", o, i);
            evas_object_name_set(item, f);

            evas_object_stack_above(item, data->clip);
            edje_object_file_set(item, data->theme_file, data->item_group);
            evas_object_show(item);
            evas_object_clip_set(item, data->clip);
            eina_array_push(data->items, item);
        }
    }
    if(new.pagesize < curitems)
    {
        int i;
        for(i = 0; i < curitems - new.pagesize; ++i)
        {
            Evas_Object* item = eina_array_pop(data->items);
            evas_object_del(item);
        }
    }

    /* Ajust selection if necessary */
    if(new.sel >= new.top_item + new.pagesize)
        new.sel = new.top_item + new.pagesize - 1;

    if(new.pagesize != 0)
    {
        /* Fix the widgets position */

        int item_h = oh / new.pagesize;
        int gap = oh - item_h*new.pagesize; /* To be spread amongst the items */

        int i;
        int item_x = ox;
        int item_y = oy;
        int item_w = ow;
        for(i = 0; i < new.pagesize; ++i)
        {
            Evas_Object* item = eina_array_data_get(data->items, i);
            evas_object_move(item, item_x, item_y);
            evas_object_resize(item, item_w, item_h);

            item_y += item_h;
            if(i == new.pagesize - gap - 1) /* gaps */
                item_h++;
        }
    }

    _choicebox_update(o, &new);
}

static void _choicebox_add(Evas_Object* o)
{
    choicebox_t* data = calloc(1, sizeof(choicebox_t));
    if(!data)
        return;

    if(!(data->items = eina_array_new(10)))
    {
        free(data);
        return;
    }

    evas_object_smart_data_set(o, data);
}

static void _choicebox_del(Evas_Object* o)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    if(data)
    {
        if(data->items)
        {
            int size = eina_array_count_get(data->items);
            while(size--)
                evas_object_del(eina_array_pop(data->items));
            eina_array_free(data->items);
        }

        evas_object_del(data->clip);

        free(data->theme_file);
        free(data->item_group);

        free(data);
    }
}

static void _choicebox_move(Evas_Object* o, Evas_Coord x, Evas_Coord y)
{
    int ow, oh;
    evas_object_geometry_get(o, NULL, NULL, &ow, &oh);

    _choicebox_display(o, x, y, ow, oh);
}

static void _choicebox_resize(Evas_Object* o, Evas_Coord w, Evas_Coord h)
{
    int ox, oy;
    evas_object_geometry_get(o, &ox, &oy, NULL, NULL);

    _choicebox_display(o, ox, oy, w, h);
}

static void _choicebox_show(Evas_Object* o)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    _run_page_handler(o);
    evas_object_show(data->clip);
}

static void _choicebox_hide(Evas_Object* o)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    evas_object_hide(data->clip);
}

static void _choicebox_clip_set(Evas_Object* o, Evas_Object* clip)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    evas_object_clip_set(data->clip, clip);
}

static void _choicebox_clip_unset(Evas_Object* o)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    evas_object_clip_unset(data->clip);
}

static Evas_Smart* _choicebox_smart_get()
{
    static Evas_Smart* smart = NULL;

    static Evas_Smart_Class class_ = {
        .name = "choicebox",
        .version = EVAS_SMART_CLASS_VERSION,
        .add = _choicebox_add,
        .del = _choicebox_del,
        .move = _choicebox_move,
        .resize = _choicebox_resize,
        .show = _choicebox_show,
        .hide = _choicebox_hide,
        .color_set = NULL,
        .clip_set = _choicebox_clip_set,
        .clip_unset = _choicebox_clip_unset,
        .calculate = NULL,
        .member_add = NULL,
        .member_del = NULL
    };

    if(!smart)
        smart = evas_smart_class_new(&class_);
    return smart;
}

static void hack_update_min_height(Evas_Object* o)
{
    /*
     * HACK: Right now Edje does not set a size_hints on a group object, so
     * let's query it from the "data" section and set manually.
     */
    const char* min_height = edje_object_data_get(o, "min_height");
    if(min_height)
    {
        long minh = atoi(min_height);
        if(minh)
            edje_extern_object_min_size_set(o, 0, minh);
    }
}

Evas_Object* choicebox_new(Evas* evas,
                           const char* theme_file,
                           const char* item_group,
                           choicebox_handler_t handler,
                           choicebox_draw_handler_t draw_handler,
                           choicebox_page_updated_t page_handler,
                           void* param)
{
    Evas_Object* o = evas_object_smart_add(evas, _choicebox_smart_get());
    choicebox_t* data = evas_object_smart_data_get(o);
    if(!data)
        goto err;

    data->st.size = 0;

    /* Data */
    data->handler = handler;
    data->draw_handler = draw_handler;
    data->page_handler = page_handler;
    data->param = param;

    /* GUI */
    int ow;
    evas_object_geometry_get(o, NULL, NULL, &ow, NULL);

    data->st.sel = -1;
    data->st.top_item = 0;
    data->st.pagesize = 0;

    /* Theme info */
    data->theme_file = strdup(theme_file);
    if(!data->theme_file)
        goto err;
    data->item_group = strdup(item_group);
    if(!data->item_group)
        goto err;

    /* Widgets */
    data->clip = evas_object_rectangle_add(evas);
    if(!data->clip)
        goto err;

    evas_object_color_set(data->clip, 255, 255, 255, 255);

    char f[256];
    snprintf(f, 256, "choicebox/%p/background", o);
    evas_object_name_set(data->clip, f);

    Evas_Object* tmpitem = edje_object_add(evas);
    if(!edje_object_file_set(tmpitem, data->theme_file, data->item_group))
    {
        evas_object_del(tmpitem);
        goto err;
    }

    hack_update_min_height(tmpitem);
    evas_object_size_hint_min_get(tmpitem, NULL, &data->item_minh);
    evas_object_del(tmpitem);

    return o;

err:
    evas_object_del(o);
    return NULL;
}

void choicebox_set_size(Evas_Object* o, int new_size)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    choicebox_state_t new = data->st;
    new.size = new_size;

    if(data->st.pagesize)
    {
        if(new_size == 0)
        {
            new.sel = -1;
            new.top_item = 0;
        }
        else
        {
            new.top_item = MIN(new.top_item, new_size - 1);

            if(new.sel != -1)
                new.sel = MIN(new.sel, new_size - 1);
        }
    }

    _choicebox_update(o, &new);
}

void choicebox_invalidate_interval(Evas_Object* o, int item_from, int item_to)
{
    choicebox_t* data = evas_object_smart_data_get(o);

    /* [from, to) = [item_from, item_to) x [first_visible, last_visible) */
    int from = MAX(data->st.top_item, item_from);
    int to = MIN(data->st.top_item + data->st.pagesize,
                 MIN(data->st.size, item_to));
    int i;

    for(i = from; i < to; ++i)
    {
        int nth = i - data->st.top_item;
        Evas_Object* item = eina_array_data_get(data->items, nth);
        (*data->draw_handler)(o, item, i, nth, data->param);
    }
}

void choicebox_invalidate_item(Evas_Object* o, int item_num)
{
    choicebox_invalidate_interval(o, item_num, item_num+1);
}

/* Navigating items */

void choicebox_prev(Evas_Object* o)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    if(!data->st.pagesize) return;
    if(!data->st.size) return;

    choicebox_state_t new = data->st;
    if(new.sel == -1)
    {
        new.sel = MIN(new.size - 1,
                      new.top_item + new.pagesize - 1);
    }
    else
    {
        new.sel = MAX(0, new.sel - 1);
        if(new.sel < new.top_item)
            new.top_item = MAX(0, new.top_item - new.pagesize);
    }

    _choicebox_update(o, &new);
}

void choicebox_next(Evas_Object* o)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    if(!data->st.pagesize) return;
    if(!data->st.size) return;

    choicebox_state_t new = data->st;

    if(new.sel == -1)
    {
        new.sel = new.top_item;
    }
    else
    {
        new.sel = MIN(new.size - 1, new.sel + 1);
        if(new.sel >= new.top_item + new.pagesize)
            new.top_item = MIN(new.size - 1, new.top_item + new.pagesize);
    }

    _choicebox_update(o, &new);
}

void choicebox_prev_pages(Evas_Object* o, int n)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    if(!data->st.pagesize) return;

    choicebox_state_t new = data->st;
    new.top_item = MAX(0, new.top_item - new.pagesize * n);

    if(new.sel != -1)
        new.sel = MAX(0, new.sel - new.pagesize * n);

    _choicebox_update(o, &new);
}

void choicebox_next_pages(Evas_Object* o, int n)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    if(!data->st.pagesize) return;
    if(!data->st.size) return;

    /* Don't turn page if pointer would be beyond last page */
    if(data->st.top_item + data->st.pagesize * n >= data->st.size)
        return;

    choicebox_state_t new = data->st;
    new.top_item = MIN(new.size - 1, new.top_item + new.pagesize * n);

    if(new.sel != -1)
        new.sel = MIN(new.size - 1, new.sel + new.pagesize * n);

    _choicebox_update(o, &new);
}

/* Activating items */

void choicebox_activate_nth_visible(Evas_Object* o, int nth, bool is_alt)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    if(nth < 0 || nth >= data->st.pagesize)
    {
        /* Item beyond the current GUI list size. Harmless, just ignore it */
        return;
    }

    int item_num = data->st.top_item + nth;
    if(item_num >= data->st.size)
    {
        /* User tried to press the button for item beyond end of list.
           It's ok, just ignore it */
        return;
    }

    _choicebox_activate(o, item_num, is_alt);
}

void choicebox_activate_current(Evas_Object* o, bool is_alt)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    if(data->st.sel == -1) /* No item is current */
        return;

    _choicebox_activate(o, data->st.sel, is_alt);
}

void choicebox_set_selection(Evas_Object* o, int sel)
{
   choicebox_t* data = evas_object_smart_data_get(o);

   if(sel < -1 || sel > data->st.size)
      return;

   choicebox_state_t new = data->st;
   new.sel = sel;

   if(new.sel != -1 && new.pagesize != 0)
      new.top_item = new.sel / new.pagesize * new.pagesize;

   _choicebox_update(o, &new);
}

int choicebox_get_selection(Evas_Object* o)
{
    choicebox_t* data = evas_object_smart_data_get(o);
    return data->st.sel;
}
