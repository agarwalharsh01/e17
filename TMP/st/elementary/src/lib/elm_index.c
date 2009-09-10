#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Index Index
 *
 * XXX
 *
 * Signals that you can add callbacks for are:
 *
 * xxx - XXX.
 * 
 * xxx - XXX.
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Item Item;

struct _Widget_Data
{
   Evas_Object *base;
   Evas_Object *event[2];
   Evas_Object *bx[2]; // 2 - for now all that's supported
   Eina_List *items; // 1 list. yes N levels, but only 2 for now and # of items will be small
   int level;
   Evas_Coord dx, dy;
   Ecore_Timer *delay;
   Eina_Bool level_active[2];
   Eina_Bool horizontal : 1;
   Eina_Bool active : 1;
   Eina_Bool down : 1;
};

struct _Item
{
   Evas_Object *obj;
   const char *letter;
   const void *data;
   int level;
   Evas_Object *base;
   Eina_Bool selected : 1;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _index_box_auto_fill(Evas_Object *obj, Evas_Object *box, int level);
static void _index_box_clear(Evas_Object *obj, Evas_Object *box, int level);
static void _item_free(Item *it);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   Eina_List *l, *clear = NULL;
   if (!wd) return;
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
   _index_box_clear(obj, wd->bx[0], 0);
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        clear = eina_list_append(clear, it);
     }
   EINA_LIST_FREE(clear, it)
     {
        _item_free(it);
     }
   if (wd->delay) ecore_timer_del(wd->delay);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _index_box_clear(obj, wd->bx[0], 0);
   _index_box_clear(obj, wd->bx[1], 1);
   if (wd->horizontal)
     _elm_theme_set(wd->base, "index", "base/horizontal", elm_widget_style_get(obj));
   else
     _elm_theme_set(wd->base, "index", "base/vertical", elm_widget_style_get(obj));
   edje_object_part_swallow(wd->base, "elm.swallow.event.0", wd->event[0]);
   edje_object_part_swallow(wd->base, "elm.swallow.index.0", wd->bx[0]);
   if (edje_object_part_exists(wd->base, "elm.swallow.index.1"))
     {
        if (!wd->bx[1])
          {
             wd->bx[1] = _els_smart_box_add(evas_object_evas_get(wd->base));
             _els_smart_box_orientation_set(wd->bx[1], wd->horizontal);
             _els_smart_box_homogenous_set(wd->bx[1], 1);
             elm_widget_sub_object_add(obj, wd->bx[1]);
          }
        edje_object_part_swallow(wd->base, "elm.swallow.index.1", wd->bx[1]);
        evas_object_show(wd->bx[1]);
     }
   else if (wd->bx[1])
     {
        evas_object_del(wd->bx[1]);
        wd->bx[1] = NULL;
     }
   if (edje_object_part_exists(wd->base, "elm.swallow.event.1"))
     {
        if (!wd->event[1])
          {
             Evas_Coord minw, minh;
             
             wd->event[1] = evas_object_rectangle_add(evas_object_evas_get(wd->base));
             evas_object_color_set(wd->event[1], 0, 0, 0, 0);
             evas_object_size_hint_min_set(wd->event[1], minw, minh);
             minw = minh = 0;
             elm_coords_finger_size_adjust(1, &minw, 1, &minh);
             elm_widget_sub_object_add(obj, wd->event[1]);
          }
        edje_object_part_swallow(wd->base, "elm.swallow.event.1", wd->event[1]);
     }
   else if (wd->event[1])
     {
        evas_object_del(wd->event[1]);
        wd->event[1] = NULL;
     }
   edje_object_message_signal_process(wd->base);
   edje_object_scale_set(wd->base, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
   _index_box_auto_fill(obj, wd->bx[0], 0);
   if (wd->active)
     {
        if (wd->level == 1)
          _index_box_auto_fill(obj, wd->bx[1], 1);
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->base, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static Item *
_item_new(Evas_Object *obj, const char *letter, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   it = calloc(1, sizeof(Item));
   if (!it) return NULL;
   it->obj = obj;
   it->letter = eina_stringshare_add(letter);
   it->data = item;
   it->level = wd->level;
   return it;
}

static Item *
_item_find(Evas_Object *obj, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Item *it;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (it->data == item) return it;
     }
   return NULL;
}

static void
_item_free(Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   wd->items = eina_list_remove(wd->items, it);
   if (it->base) evas_object_del(it->base);
   eina_stringshare_del(it->letter);
   free(it);
}

// FIXME: always have index filled
static void
_index_box_auto_fill(Evas_Object *obj, Evas_Object *box, int level)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Item *it, **itfit = NULL;
   Evas_Coord mw, mh, w, h;
   int i;
   char buf[1024];
        
   if (wd->level_active[level]) return;
   i = 0;
   evas_object_geometry_get(box, NULL, NULL, &w, &h);
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        Evas_Object *o;
        const char *stacking;
        
        if (it->level != level) continue;
        o = edje_object_add(evas_object_evas_get(obj));
        it->base = o;
        if (i & 0x1)
          _elm_theme_set(o, "index", "item_odd/vertical", "default");
        else
          _elm_theme_set(o, "index", "item/vertical", "default");
        edje_object_part_text_set(o, "elm.text", it->letter);
        edje_object_size_min_restricted_calc(o, &mw, &mh, 0, 0);
        evas_object_size_hint_min_set(o, mw, mh);
        evas_object_size_hint_weight_set(o, 1.0, 1.0);
        evas_object_size_hint_align_set(o, -1.0, -1.0);
        elm_widget_sub_object_add(obj, o);
        _els_smart_box_pack_end(box, o);
        stacking = edje_object_data_get(o, "stacking");
        if (stacking)
          {
             if (!strcmp(stacking, "below")) evas_object_lower(o);
             else if (!strcmp(stacking, "above")) evas_object_raise(o);
          }
        evas_object_show(o);
        i++;
        evas_object_size_hint_min_get(box, &mw, &mh);
        if (mh > h)
          {
             _index_box_clear(obj, box, level);
             if (i > 0)
               {
                  // FIXME: only i objects fit! try again. overflows right now
               }
          }
     }
   snprintf(buf, sizeof(buf), "elm.swallow.index.%i", level);
//   evas_object_size_hint_min_get(box, &mw, &mh);
//   evas_object_size_hint_min_set(box, mw, 0);
   edje_object_part_swallow(wd->base, buf, box);
   wd->level_active[level] = 1;
}

static void
_index_box_clear(Evas_Object *obj, Evas_Object *box, int level)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Item *it;
   char buf[1024];

   if (!wd->level_active[level]) return;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (!it->base) continue;
        if (it->level != level) continue;
        evas_object_del(it->base);
        it->base = 0;
     }
   snprintf(buf, sizeof(buf), "elm.swallow.index.%i", level);
   edje_object_part_swallow(wd->base, buf, box);
   wd->level_active[level] = 0;
}

static int
_delay_change(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->delay = NULL;
   void *d;
   d = (void *)elm_index_item_selected_get(data, wd->level);
   if (d) evas_object_smart_callback_call(data, "delay,changed", d);
   return 0;
}

static void
_sel_eval(Evas_Object *obj, Evas_Coord evx, Evas_Coord evy)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it, *it_closest, *it_last;
   Eina_List *l;
   Evas_Coord x, y, w, h, bx, by, bw, bh, xx, yy;
   double cdv = 0.5;
   Evas_Coord dist;
   Eina_Bool change = 0;
   char *label = NULL, *last = NULL;
   int i;

   for (i = 0; i <= wd->level; i++)
     {
        it_last = NULL;
        it_closest  = NULL;
        dist = 0x7fffffff;
        evas_object_geometry_get(wd->bx[i], &bx, &by, &bw, &bh);
        EINA_LIST_FOREACH(wd->items, l, it)
          {
             if (!((it->level == i) && (it->base)))
               continue;
             if ((it->base) && (it->level != wd->level))
               {
                  if (it->selected)
                    {
                       it_closest = it;
                       break;
                    }
                  continue;
               }
             if (it->selected)
               {
                  it_last = it;
                  it->selected = 0;
               }
             evas_object_geometry_get(it->base, &x, &y, &w, &h);
             xx = x + (w / 2);
             yy = y + (h / 2);
             x = evx - xx;
             y = evy - yy;
             x = (x * x) + (y * y);
             if ((x < dist) || (!it_closest))
               {
                  if (wd->horizontal)
                    cdv = (double)(xx - bx) / (double)bw; 
                  else
                    cdv = (double)(yy - by) / (double)bh;
                  it_closest = it;
                  dist = x;
               }
          }
        if ((i == 0) && (wd->level == 0))
          {
             edje_object_part_drag_value_set(wd->base, "elm.dragable.index.1", cdv, cdv);
          }
        if (it_closest)
          {
             it_closest->selected = 1;
          }
        if (it_closest != it_last)
          {
             change = 1;
             if (it_last)
               {
                  const char *stacking, *selectraise;

                  it = it_last;
                  edje_object_signal_emit(it->base, "elm,state,inactive", "elm");
                  stacking = edje_object_data_get(it->base, "stacking");
                  selectraise = edje_object_data_get(it->base, "selectraise");
                  if ((selectraise) && (!strcmp(selectraise, "on")))
                    {
                       if ((stacking) && (!strcmp(stacking, "below")))
                         evas_object_lower(it->base);
                    }
               }
             if (it_closest)
               {
                  const char *selectraise;
                  
                  it = it_closest;
                  edje_object_signal_emit(it->base, "elm,state,active", "elm");
                  selectraise = edje_object_data_get(it->base, "selectraise");
                  if ((selectraise) && (!strcmp(selectraise, "on")))
                    evas_object_raise(it->base);
                  evas_object_smart_callback_call((void *)obj, "changed", it->data);
                  if (wd->delay) ecore_timer_del(wd->delay);
                  wd->delay = ecore_timer_add(0.2, _delay_change, obj);
               }
          }
        if (it_closest)
          {
             it = it_closest;
             if (!last)
               last = strdup(it->letter);
             else
               {
                  if (!label) label = strdup(last);
                  else
                    {
                       label = realloc(label, strlen(label) + strlen(last) + 1);
                       strcat(label, last);
                    }
                  free(last);
                  last = strdup(it->letter);
               }
          }
     }
   if (!label) label = strdup("");
   if (!last) last = strdup("");
   edje_object_part_text_set(wd->base, "elm.text.body", label);
   edje_object_part_text_set(wd->base, "elm.text", last);
   free(label);
   free(last);
}

static void 
_wheel(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Wheel *ev = event_info;
   Evas_Object *obj = o;
}

static void 
_mouse_down(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y;
   if (ev->button != 1) return;
   wd->down = 1;

   evas_object_geometry_get(wd->base, &x, &y, NULL, NULL);
   wd->dx = ev->canvas.x - x;
   wd->dy = ev->canvas.y - y;
   elm_index_active_set(data, 1);
   _sel_eval(data, ev->canvas.x, ev->canvas.y);
   edje_object_part_drag_value_set(wd->base, "elm.dragable.pointer", wd->dx, wd->dy);
}

static void 
_mouse_up(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Up *ev = event_info;
   void *d;
   if (ev->button != 1) return;
   wd->down = 0;
   d = (void *)elm_index_item_selected_get(data, wd->level);
   if (d) evas_object_smart_callback_call(data, "selected", d);
   elm_index_active_set(data, 0);
   edje_object_signal_emit(wd->base, "elm,state,level,0", "elm");
}

static void 
_mouse_move(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;
   Eina_List *l;
   Item *it;
   char buf[1024];

   if (!wd->down) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_geometry_get(wd->base, &x, &y, NULL, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - wd->dx;
   adx = dx;
   if (adx < 0) adx = -dx;
   dy = y - wd->dy;
   ady = dy;
   if (ady < 0) ady = -dy;
   edje_object_part_drag_value_set(wd->base, "elm.dragable.pointer", x, y);
   if (wd->horizontal)
     {
     }
   else
     {
        if (adx > minw)
          {
             if (wd->level == 0)
               { 
                  wd->level = 1;
                  snprintf(buf, sizeof(buf), "elm,state,level,%i", wd->level);
                  edje_object_signal_emit(wd->base, buf, "elm");
                  evas_object_smart_callback_call(data, "level,up", NULL);
               }
          }
        else
          {
             if (wd->level == 1)
               {
                  wd->level = 0;
                  snprintf(buf, sizeof(buf), "elm,state,level,%i", wd->level);
                  edje_object_signal_emit(wd->base, buf, "elm");
                  evas_object_smart_callback_call(data, "level,down", NULL);
               }
          }
     }
   _sel_eval(data, ev->cur.canvas.x, ev->cur.canvas.y);
}

/**
 * Add a new index to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Index
 */
EAPI Evas_Object *
elm_index_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas_Object *o;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "index");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->horizontal = EINA_FALSE;

   wd->base = edje_object_add(e);
   _elm_theme_set(wd->base, "index", "base/vertical", "default");
   elm_widget_resize_object_set(obj, wd->base);
   
   o = evas_object_rectangle_add(e);
   wd->event[0] = o;
   evas_object_color_set(o, 0, 0, 0, 0);
   minw = minh = 0;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(o, minw, minh);
   edje_object_part_swallow(wd->base, "elm.swallow.event.0", o);
   elm_widget_sub_object_add(obj, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_WHEEL, _wheel, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _mouse_up, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, obj);
   evas_object_show(o);
   if (edje_object_part_exists(wd->base, "elm.swallow.event.1"))
     {
        o = evas_object_rectangle_add(e);
        wd->event[1] = o;
        evas_object_color_set(o, 0, 0, 0, 0);
        evas_object_size_hint_min_set(o, minw, minh);
        edje_object_part_swallow(wd->base, "elm.swallow.event.1", o);
        elm_widget_sub_object_add(obj, o);
     }
     
   wd->bx[0] = _els_smart_box_add(e);
   _els_smart_box_orientation_set(wd->bx[0], 0);
   _els_smart_box_homogenous_set(wd->bx[0], 1);
   elm_widget_sub_object_add(obj, wd->bx[0]);
   edje_object_part_swallow(wd->base, "elm.swallow.index.0", wd->bx[0]);
   evas_object_show(wd->bx[0]);

   if (edje_object_part_exists(wd->base, "elm.swallow.index.1"))
     {
        wd->bx[1] = _els_smart_box_add(e);
        _els_smart_box_orientation_set(wd->bx[1], 0);
        _els_smart_box_homogenous_set(wd->bx[1], 1);
        elm_widget_sub_object_add(obj, wd->bx[1]);
        edje_object_part_swallow(wd->base, "elm.swallow.index.1", wd->bx[1]);
        evas_object_show(wd->bx[1]);
     }

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the active state of the index programatically
 *
 * @param obj The index object
 * @param active The active starte
 *
 * @ingroup Index
 */
EAPI void
elm_index_active_set(Evas_Object *obj, Eina_Bool active)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->active == active) return;
   wd->active = active;
   wd->level = 0;
   if (wd->active)
     {
        _index_box_clear(obj, wd->bx[1], 1);
        _index_box_auto_fill(obj, wd->bx[0], 0);
        edje_object_signal_emit(wd->base, "elm,state,active", "elm");
     }
   else
     {
        edje_object_signal_emit(wd->base, "elm,state,inactive", "elm");
     }
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_level_set(Evas_Object *obj, int level)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->level == level) return;
   wd->level = level;
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI int
elm_index_item_level_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->level;
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI const void *
elm_index_item_selected_get(Evas_Object *obj, int level)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Item *it;
   if (!wd) return NULL;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if ((it->selected) && (it->level == level)) return it->data;
     }
   return NULL;
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_append(Evas_Object *obj, const char *letter, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   if (!wd) return;
   it = _item_new(obj, letter, item);
   if (!it) return;
   wd->items = eina_list_append(wd->items, it);
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_prepend(Evas_Object *obj, const char *letter, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   if (!wd) return;
   it = _item_new(obj, letter, item);
   if (!it) return;
   wd->items = eina_list_prepend(wd->items, it);
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_append_relative(Evas_Object *obj, const char *letter, const void *item, const void *relative)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it, *it_rel;
   if (!wd) return;
   if (!relative)
     {
        elm_index_item_append(obj, letter, item);
        return;
     }
   it = _item_new(obj, letter, item);
   it_rel = _item_find(obj, relative);
   if (!it_rel)
     {
        elm_index_item_append(obj, letter, item);
        return;
     }
   if (!it) return;
   wd->items = eina_list_append_relative(wd->items, it, it_rel);
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_prepend_relative(Evas_Object *obj, const char *letter, const void *item, const void *relative)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it, *it_rel;
   if (!wd) return;
   if (!relative)
     {
        elm_index_item_prepend(obj, letter, item);
        return;
     }
   it = _item_new(obj, letter, item);
   it_rel = _item_find(obj, relative);
   if (!it_rel)
     {
        elm_index_item_append(obj, letter, item);
        return;
     }
   if (!it) return;
   wd->items = eina_list_prepend_relative(wd->items, it, it_rel);
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_del(Evas_Object *obj, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   if (!wd) return;
   it = _item_find(obj, item);
   if (!it) return;
   _item_free(it);
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_clear(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   Eina_List *l, *clear = NULL;
   if (!wd) return;
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (it->level != wd->level) continue;
        clear = eina_list_append(clear, it);
     }
   EINA_LIST_FREE(clear, it)
     {
        _item_free(it);
     }
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_go(Evas_Object *obj, int level)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   if (!wd) return;
   // XXX
   _index_box_auto_fill(obj, wd->bx[0], 0);
   if (wd->level == 1)
     _index_box_auto_fill(obj, wd->bx[1], 1);
}
