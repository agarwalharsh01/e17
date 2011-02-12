#include "e_mod_main.h"
#include <X11/extensions/shape.h>

#define WINDOW_HEIGHT 200

static int            _ngi_win_free(Ngi_Win *win);
static Ngi_Win *      _ngi_win_new(Ng *ng);

static Eina_Bool      _ngi_cb_container_resize(void *data, int type, void *event);

static Eina_Bool      _ngi_win_cb_mouse_in(void *data, int type, void *event);
static Eina_Bool      _ngi_win_cb_mouse_out(void *data, int type, void *event);
static Eina_Bool      _ngi_win_cb_mouse_down(void *data, int type, void *event);
static Eina_Bool      _ngi_win_cb_mouse_up(void *data, int type, void *event);
static Eina_Bool      _ngi_win_cb_mouse_move(void *data, int type, void *event);
static Eina_Bool      _ngi_win_cb_mouse_wheel(void *data, int type, void *event);

static Eina_Bool      _ngi_win_cb_desk_show(void *data, int type, void *event);
static Eina_Bool      _ngi_win_cb_border_event(void *data, int type, void *event);

static void           _ngi_zoom_in(Ng *ng);
static void           _ngi_zoom_out(Ng *ng);
static void           _ngi_item_appear(Ng *ng, Ngi_Item *it);
static void           _ngi_item_disappear(Ng *ng, Ngi_Item *it);
static void           _ngi_zoom_function(Ng *ng, double d, double *disp);
static Eina_Bool      _ngi_animator(void *data);
static void           _ngi_redraw(Ng *ng);
static int            _ngi_autohide(Ng *ng, int hide);

static void           _ngi_item_activate(Ng *ng);
static Ngi_Item *     _ngi_item_at_position_get(Ng *ng);

void                  ngi_hide_clip(Ng *ng);

static int initialized = 0;

E_Config_DD *ngi_conf_edd = NULL;
E_Config_DD *ngi_conf_item_edd = NULL;
E_Config_DD *ngi_conf_box_edd = NULL;
E_Config_DD *ngi_conf_gadcon_edd = NULL;

Config *ngi_config = NULL;

E_Int_Menu_Augmentation *maug = NULL;

#define TEXT_DIST       45
#define SIDE_OFFSET     30

static int
_ngi_check_fullscreen(E_Desk *desk)
{
   Eina_List *l;
   E_Border *bd;

   EINA_LIST_FOREACH(e_border_client_list(), l, bd)
   {
      if (bd->desk != desk)
         continue;

      if (bd->fullscreen)
         return 1;
   }

   return 0;
}

static Eina_Bool
_ngi_win_cb_desk_show(void *data, int type, void *event)
{
   E_Event_Desk_Show *ev = event;
   Ng *ng = data;
   int fullscreen;

   if (ng->cfg->stacking == below_fullscreen)
     {
	fullscreen = _ngi_check_fullscreen(ev->desk);

	if (fullscreen)
	  e_popup_hide(ng->win->popup);
	else
	  e_popup_show(ng->win->popup);
     }
   else if (ng->cfg->autohide == AUTOHIDE_FULLSCREEN)
     {
	fullscreen = _ngi_check_fullscreen(ev->desk);

	if (ng->hide_fullscreen != fullscreen)
	  ngi_animate(ng);

	ng->hide_fullscreen = fullscreen;
     }
   return EINA_TRUE;
}

static Eina_Bool
_ngi_win_cb_border_event(void *data, int type, void *event)
{
   E_Event_Border_Property *ev = event;
   Ng *ng = data;
   E_Desk *desk;
   int fullscreen;

   if (ng->cfg->stacking == below_fullscreen)
     {
	desk = e_desk_current_get(e_util_zone_current_get(e_manager_current_get()));

	if (ev->border->desk != desk)
	  return EINA_TRUE;

	fullscreen = _ngi_check_fullscreen(desk);

	if (fullscreen)
	  e_popup_hide(ng->win->popup);
	else
	  e_popup_show(ng->win->popup);
     }
   else if (ng->cfg->autohide == AUTOHIDE_FULLSCREEN)
     {
	desk = e_desk_current_get(e_util_zone_current_get(e_manager_current_get()));

	if (ev->border->desk != desk)
	  return EINA_TRUE;

	fullscreen = _ngi_check_fullscreen(desk);

	if (ng->hide_fullscreen != fullscreen)
	  ngi_animate(ng);

	ng->hide_fullscreen = fullscreen;
     }

   return EINA_TRUE;
}

Eina_Bool
ngi_object_theme_set(Evas_Object *obj, const char *part)
{
   int ok = 0;

   ok = e_theme_edje_object_set(obj, "base/theme/modules/itask-ng", part);

   if (!ok)
     ok = edje_object_file_set(obj, ngi_config->theme_path, part);

   return ok;
}

Ng *
ngi_new(Config_Item *cfg)
{
   Ng *ng;
   E_Zone *zone;
   Config_Box *cfg_box;
   Eina_List *l;

   zone = (E_Zone *)e_util_container_zone_number_get(cfg->container, cfg->zone);

   if (!zone)
      return NULL;

   ng = E_NEW(Ng, 1);
   cfg->ng = ng;
   ng->cfg = cfg;

   ngi_config->instances = eina_list_append(ngi_config->instances, ng);

   ng->cfg->mouse_over_anim = 0;

   ng->zone = zone;
   ng->win = _ngi_win_new(ng);

   ng->o_bg = NULL;
   ng->o_frame = NULL;
   ng->animator = NULL;
   ng->items_scaling = NULL;
   ng->item_active = NULL;
   ng->item_drag = NULL;
   ng->zoom = 1.0;
   ng->state = unzoomed;
   ng->hide_step = 0;
   ng->hide_state = show;
   ng->mouse_in = 0;
   ng->dnd = 0;

   ng->mouse_in_timer = NULL;
   ng->menu_wait_timer = NULL;
   ng->effect_timer = NULL;

   ng->start = 0;
   ng->w = 0;
   ng->h = 0;
   ng->pos = 0;
   ng->show_bar = 0;

   ng->size = ng->cfg->size;

   ng->clip = evas_object_rectangle_add(ng->evas);
   evas_object_move(ng->clip, -100004, -100004);
   evas_object_resize(ng->clip, 200008, 200008);
   evas_object_color_set(ng->clip, 255, 255, 255, 255);

   ng->bg_clip = evas_object_rectangle_add(ng->evas);
   evas_object_move(ng->bg_clip, -100004, -100004);
   evas_object_resize(ng->bg_clip, 200008, 200008);
   evas_object_color_set(ng->bg_clip, cfg->alpha, cfg->alpha,
                         cfg->alpha, cfg->alpha);

   ng->o_bg    = edje_object_add(ng->evas);
   ng->o_frame = edje_object_add(ng->evas);
   ng->o_label = edje_object_add(ng->evas);

   switch(cfg->orient)
     {
      case E_GADCON_ORIENT_BOTTOM:
	 ngi_object_theme_set(ng->o_bg,	  "e/modules/itask-ng/bg_bottom");
	 ngi_object_theme_set(ng->o_frame,"e/modules/itask-ng/frame_bottom");
	 ngi_object_theme_set(ng->o_label,"e/modules/itask-ng/label_bottom");
	 break;

      case E_GADCON_ORIENT_TOP:
	 ngi_object_theme_set(ng->o_bg,	  "e/modules/itask-ng/bg_top");
	 ngi_object_theme_set(ng->o_frame,"e/modules/itask-ng/frame_top");
	 ngi_object_theme_set(ng->o_label,"e/modules/itask-ng/label_top");
         break;

      case E_GADCON_ORIENT_LEFT:
	 ngi_object_theme_set(ng->o_bg,	  "e/modules/itask-ng/bg_left");
	 ngi_object_theme_set(ng->o_frame,"e/modules/itask-ng/frame_left");
	 ngi_object_theme_set(ng->o_label,"e/modules/itask-ng/label_left");
         break;

      case E_GADCON_ORIENT_RIGHT:
	 ngi_object_theme_set(ng->o_bg,	  "e/modules/itask-ng/bg_right");
	 ngi_object_theme_set(ng->o_frame,"e/modules/itask-ng/frame_right");
	 ngi_object_theme_set(ng->o_label,"e/modules/itask-ng/label_right");
         break;
     }

   const char *data = edje_object_data_get(ng->o_frame, "clip_separator");
   if (data) ng->opt.clip_separator = atoi(data);

   data = edje_object_data_get(ng->o_frame, "icon_spacing");
   if (data) ng->opt.item_spacing = atoi(data);

   data = edje_object_data_get(ng->o_frame, "edge_spacing");
   if (data) ng->opt.edge_spacing = atoi(data);

   data = edje_object_data_get(ng->o_frame, "separator_width");
   if (data) ng->opt.separator_width = atoi(data);

   evas_object_show(ng->o_frame);
   evas_object_show(ng->o_bg);
   evas_object_clip_set(ng->o_bg, ng->bg_clip);

   if (cfg->show_label)
     evas_object_show(ng->o_label);

   if (cfg->show_background)
     evas_object_show(ng->bg_clip);

   EINA_LIST_FOREACH (cfg->boxes, l, cfg_box)
   {
      switch (cfg_box->type)
        {
         case launcher:
            ngi_launcher_new(ng, cfg_box);
            break;

         case taskbar:
            ngi_taskbar_new(ng, cfg_box);
            break;

         case gadcon:
            ngi_gadcon_new(ng, cfg_box);
            break;
        }
   }

   ng->handlers = eina_list_append
         (ng->handlers, ecore_event_handler_add
            (ECORE_X_EVENT_MOUSE_IN, _ngi_win_cb_mouse_in, ng));
   ng->handlers = eina_list_append
         (ng->handlers, ecore_event_handler_add
            (ECORE_X_EVENT_MOUSE_OUT, _ngi_win_cb_mouse_out, ng));
   ng->handlers = eina_list_append
         (ng->handlers, ecore_event_handler_add
            (ECORE_EVENT_MOUSE_BUTTON_DOWN, _ngi_win_cb_mouse_down, ng));
   ng->handlers = eina_list_append
         (ng->handlers, ecore_event_handler_add
            (ECORE_EVENT_MOUSE_BUTTON_UP, _ngi_win_cb_mouse_up, ng));
   ng->handlers = eina_list_append
         (ng->handlers, ecore_event_handler_add
            (ECORE_EVENT_MOUSE_WHEEL, _ngi_win_cb_mouse_wheel, ng));
   ng->handlers = eina_list_append
         (ng->handlers, ecore_event_handler_add
            (ECORE_EVENT_MOUSE_MOVE, _ngi_win_cb_mouse_move, ng));
   ng->handlers = eina_list_append
     (ng->handlers, ecore_event_handler_add
      (E_EVENT_DESK_SHOW, _ngi_win_cb_desk_show, ng));
   ng->handlers = eina_list_append
     (ng->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_PROPERTY, _ngi_win_cb_border_event, ng));


   if (ng->cfg->autohide == AUTOHIDE_FULLSCREEN)
     {
	E_Desk *desk = e_desk_current_get(e_util_zone_current_get(e_manager_current_get()));
	int fullscreen = _ngi_check_fullscreen(desk);

	if (ng->hide_fullscreen != fullscreen)
	  ngi_animate(ng);

	ng->hide_fullscreen = fullscreen;
     }
   else if (ng->cfg->stacking == below_fullscreen)
     {
	E_Desk *desk = e_desk_current_get(e_util_zone_current_get(e_manager_current_get()));
	int fullscreen = _ngi_check_fullscreen(desk);

	if (fullscreen)
	  e_popup_hide(ng->win->popup);
	else
	  e_popup_show(ng->win->popup);
     }

   if ((ng->cfg->autohide == AUTOHIDE_NORMAL) || ng->hide_fullscreen)
     {
        ng->hide_step = ng->size * 2;
        ng->hide_state = hidden;
     }

   ngi_thaw(ng);

   return ng;
}

void
ngi_free(Ng *ng)
{
   Ngi_Box *box;
   Ecore_Event_Handler *h;

   ngi_config->instances = eina_list_remove(ngi_config->instances, ng);

   while(ng->boxes)
     {
        box = ng->boxes->data;
        if (box->cfg->type == taskbar)
           ngi_taskbar_remove(box);
        else if (box->cfg->type == launcher)
           ngi_launcher_remove(box);
        else if (box->cfg->type == gadcon)
           ngi_gadcon_remove(box);
     }

   if (ng->animator)
      ecore_animator_del(ng->animator);

   if (ng->menu_wait_timer)
      ecore_timer_del(ng->menu_wait_timer);

   if (ng->mouse_in_timer)
      ecore_timer_del(ng->mouse_in_timer);

   if (ng->effect_timer)
      ecore_timer_del(ng->effect_timer);

   if (ng->o_bg)
      evas_object_del(ng->o_bg);

   if (ng->o_frame)
      evas_object_del(ng->o_frame);

   if (ng->clip)
      evas_object_del(ng->clip);

   if (ng->bg_clip)
      evas_object_del(ng->bg_clip);

   if (ng->win)
      _ngi_win_free(ng->win);

   EINA_LIST_FREE(ng->handlers, h)
   ecore_event_handler_del(h);

   free(ng);
}

/***************************************************************************/

EAPI E_Module_Api e_modapi = { E_MODULE_API_VERSION, "itask-ng" };

static int
_ngi_init_timer_cb(void *data)
{
   Eina_List *l;
   Config_Item *ci;

   EINA_LIST_FOREACH (ngi_config->items, l, ci)
   ngi_new(ci);

   initialized = 1;

   return 0;
}

/* menu item callback(s) */
static void
_e_mod_run_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   ngi_instances_config(e_util_zone_current_get(e_manager_current_get())->container, NULL);
}

/* menu item add hook */
static void
_e_mod_menu_add(void *data, E_Menu *m)
{
   E_Menu_Item *mi;

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, D_("Itask-NG"));
   e_util_menu_item_edje_icon_set(mi, "preferences-desktop-shelf");
   e_menu_item_callback_set(mi, _e_mod_run_cb, NULL);
}

EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[4096];
   ngi_config = NULL;

   /* Location of message catalogs for localization */
   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   ngi_conf_gadcon_edd = E_CONFIG_DD_NEW("Ngi_Config_Gadcon", Config_Gadcon);
   E_CONFIG_VAL(ngi_conf_gadcon_edd, Config_Gadcon, name, STR);

   ngi_conf_box_edd = E_CONFIG_DD_NEW("Ngi_Config_Item_Box", Config_Box);
   E_CONFIG_VAL(ngi_conf_box_edd, Config_Box, type, INT);
   E_CONFIG_VAL(ngi_conf_box_edd, Config_Box, launcher_app_dir, STR);
   E_CONFIG_VAL(ngi_conf_box_edd, Config_Box, launcher_lock_dnd, INT);
   E_CONFIG_VAL(ngi_conf_box_edd, Config_Box, taskbar_skip_dialogs, INT);
   E_CONFIG_VAL(ngi_conf_box_edd, Config_Box, taskbar_adv_bordermenu, INT);
   E_CONFIG_VAL(ngi_conf_box_edd, Config_Box, taskbar_show_iconified, INT);
   E_CONFIG_VAL(ngi_conf_box_edd, Config_Box, taskbar_show_desktop, INT);
   E_CONFIG_VAL(ngi_conf_box_edd, Config_Box, taskbar_append_right, INT);
   E_CONFIG_VAL(ngi_conf_box_edd, Config_Box, taskbar_group_apps, INT);
   E_CONFIG_LIST(ngi_conf_box_edd, Config_Box, gadcon_items, ngi_conf_gadcon_edd);

   ngi_conf_item_edd = E_CONFIG_DD_NEW("Ngi_Config_Item", Config_Item);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, show_label, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, show_background, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, container, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, zone, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, orient, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, size, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, autohide, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, autohide_show_urgent, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, hide_timeout, FLOAT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, zoom_duration, FLOAT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, zoomfactor, FLOAT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, zoom_range, FLOAT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, zoom_one, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, hide_below_windows, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, alpha, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, sia_remove, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, stacking, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, mouse_over_anim, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, lock_deskswitch, INT);
   E_CONFIG_VAL(ngi_conf_item_edd, Config_Item, ecomorph_features, INT);
   E_CONFIG_LIST(ngi_conf_item_edd, Config_Item, boxes, ngi_conf_box_edd);

   ngi_conf_edd = E_CONFIG_DD_NEW("Ngi_Config", Config);
   E_CONFIG_LIST(ngi_conf_edd, Config, items, ngi_conf_item_edd);

   ngi_config = (Config *)e_config_domain_load("module.itask-ng", ngi_conf_edd);

   if (!ngi_config)
      ngi_config = E_NEW(Config, 1);

   ngi_config->cfd = NULL;
   ngi_config->module = m;

   snprintf(buf, sizeof(buf), "%s/itask-ng.edj", e_module_dir_get(m));
   ngi_config->theme_path = strdup(buf);

   snprintf(buf, sizeof(buf), "%s/.e/e/applications/bar/", e_user_homedir_get());
   if (!ecore_file_mkdir(buf) && !ecore_file_is_dir(buf))
     {
        e_error_message_show("Error creating directory:\n %s\n",buf);
        return m;
     }

   e_configure_registry_item_add("extensions/itask_ng", 40,
                                 D_("Itask NG"), NULL,
                                 "preferences-desktop-shelf",
                                 ngi_instances_config);

   ngi_config->handlers = eina_list_append
         (ngi_config->handlers, ecore_event_handler_add
            (E_EVENT_CONTAINER_RESIZE, _ngi_cb_container_resize, NULL));

   ngi_taskbar_init();
   ngi_gadcon_init();

   if (ecore_x_screen_is_composited(0) || e_config->use_composite)
      ngi_config->use_composite = 1;
   else
      ngi_config->use_composite = 0;

   e_module_delayed_set(m, 1);
   _ngi_init_timer_cb(NULL);

   maug = e_int_menus_menu_augmentation_add
         ("config/1", _e_mod_menu_add, NULL, NULL, NULL);

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   Ecore_Event_Handler *h;
   Ng *ng;
   Eina_List *l, *ll;
   Config_Item *ci;
   Config_Box *cfg_box;
   Config_Gadcon *cg;

   if (maug)
     {
        e_int_menus_menu_augmentation_del("config/1", maug);
        maug = NULL;
     }

   ngi_gadcon_shutdown();

   EINA_LIST_FOREACH_SAFE(ngi_config->instances, l, ll, ng)
   {
      if (ng->cfg->config_dialog)
         e_object_del(E_OBJECT(ng->cfg->config_dialog));

      ngi_free(ng);
   }

   EINA_LIST_FREE(ngi_config->handlers, h)
   ecore_event_handler_del(h);

   EINA_LIST_FREE(ngi_config->items, ci)
     {
        EINA_LIST_FREE(ci->boxes, cfg_box)
        {
           if (cfg_box->type == launcher && cfg_box->launcher_app_dir)
              eina_stringshare_del(cfg_box->launcher_app_dir);

           if (cfg_box->type == gadcon)
             {
                EINA_LIST_FREE(cfg_box->gadcon_items, cg)
		  eina_stringshare_del(cg->name);
             }
        }
        free(ci);
     }

   free(ngi_config->theme_path);
   free(ngi_config);
   ngi_config = NULL;

   E_CONFIG_DD_FREE(ngi_conf_gadcon_edd);
   E_CONFIG_DD_FREE(ngi_conf_box_edd);
   E_CONFIG_DD_FREE(ngi_conf_item_edd);
   E_CONFIG_DD_FREE(ngi_conf_edd);

   e_configure_registry_item_del("extensions/itask_ng");

   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   e_config_domain_save("module.itask-ng", ngi_conf_edd, ngi_config);
   return 1;
}

static Ngi_Win *
_ngi_win_new(Ng *ng)
{
   Ngi_Win *win;

   win = E_NEW(Ngi_Win, 1);
   if (!win) return NULL;

   win->ng = ng;
   win->popup = e_popup_new(ng->zone, 0, 0, 0, 0);

   if (ngi_config->use_composite)
     {
	ecore_evas_alpha_set(win->popup->ecore_evas, 1);
	win->popup->evas_win = ecore_evas_software_x11_window_get(win->popup->ecore_evas);
	win->input = win->popup->evas_win;
	win->drop_win = E_OBJECT(win->popup);
     }
   else
     {
	ecore_evas_shaped_set(win->popup->ecore_evas, 1);

	win->input = ecore_x_window_input_new(ng->zone->container->win, 0, 0, 1, 1);
	ecore_x_window_show(win->input);

	win->fake_iwin = E_OBJECT_ALLOC(E_Win, E_WIN_TYPE, NULL);
	win->fake_iwin->evas_win = win->input;
	win->drop_win = E_OBJECT(win->fake_iwin);
     }

   ecore_x_netwm_window_type_set(win->popup->evas_win, ECORE_X_WINDOW_TYPE_DOCK);
   ng->evas = win->popup->evas;

   e_drop_xdnd_register_set(win->input, 1);
   e_container_window_raise(ng->zone->container, win->input, 999);

   ngi_win_position_calc(win);
   e_popup_show(win->popup);

   return win;
}

static int
_ngi_win_free(Ngi_Win *win)
{
   e_drop_xdnd_register_set(win->input, 0);
   e_object_del(E_OBJECT(win->popup));

   if (win->fake_iwin)
     {
   	ecore_x_window_free(win->input);
   	free(win->fake_iwin);
     }

   free(win);

   return 1;
}

static Eina_Bool
_ngi_cb_container_resize(void *data, int ev_type, void *event_info)
{
   Config_Item *ci;
   E_Zone *zone;
   Eina_List *l;
   Ng *ng;

   if (!initialized)
      return EINA_TRUE;

   EINA_LIST_FOREACH (ngi_config->items, l, ci)
   {
      if (!ci->ng)
        {
           ngi_new(ci);
           continue;
        }

      ng = ci->ng;
      zone = e_util_container_zone_number_get(ci->container, ci->zone);
      if (!zone)
        {
           ngi_free(ng);
           continue;
        }

      ngi_win_position_calc(ng->win);
      ngi_reposition(ng);
      ngi_input_extents_calc(ng);
      ngi_thaw(ng);
   }
   return EINA_TRUE;
}

void
ngi_win_position_calc(Ngi_Win *win)
{
   Ng *ng = win->ng;

   E_Gadcon_Orient orient = (E_Gadcon_Orient)ng->cfg->orient;
   int size = WINDOW_HEIGHT;

   switch (orient)
     {
      case E_GADCON_ORIENT_LEFT:
	 e_popup_move_resize(win->popup, 0, 0, size, ng->zone->h);
         ng->horizontal = 0;
         break;

      case E_GADCON_ORIENT_RIGHT:
	 e_popup_move_resize(win->popup, ng->zone->w - size, 0, size, ng->zone->h);
         ng->horizontal = 0;
         break;

      case E_GADCON_ORIENT_TOP:
	 e_popup_move_resize(win->popup, 0, 0, ng->zone->w, size);
         ng->horizontal = 1;
         break;

      case E_GADCON_ORIENT_BOTTOM:
	 e_popup_move_resize(win->popup, 0, ng->zone->h - size, ng->zone->w, size);
         ng->horizontal = 1;
         break;

      default:
         break;
     }

   if (win->fake_iwin)
     {
	win->fake_iwin->x = win->popup->x;
	win->fake_iwin->y = win->popup->y;
	win->fake_iwin->w = win->popup->w;
	win->fake_iwin->h = win->popup->h;
     }
}

static Eina_Bool
_ngi_mouse_in_timer(void *data)
{
   Ng *ng = data;

   ng->mouse_in_timer = NULL;
   ng->mouse_in = 1;

   ngi_input_extents_calc(ng);

   ngi_animate(ng);

   if (ng->item_active && ng->item_active->label && ng->item_active->label[0])
     {
	evas_object_show(ng->o_label);
	edje_object_signal_emit(ng->o_label, "e,state,label_show", "e");
	edje_object_part_text_set(ng->o_label, "e.text.label", ng->item_active->label);
     }

   return EINA_FALSE;
}

void
ngi_mouse_in(Ng *ng)
{
   if (ng->mouse_in_timer)
     ecore_timer_del(ng->mouse_in_timer);

   if (ng->hide_state == hidden)
     ng->mouse_in_timer = ecore_timer_add(0.15, _ngi_mouse_in_timer, ng);
   else
     _ngi_mouse_in_timer(ng);
}

void
ngi_mouse_out(Ng *ng)
{
   ng->mouse_in = 0;

   if (ng->mouse_in_timer)
      ecore_timer_del(ng->mouse_in_timer);
   ng->mouse_in_timer = NULL;

   ITEM_MOUSE_OUT(ng->item_active);
   edje_object_signal_emit(ng->o_label, "e,state,label_hide", "e");

   ngi_animate(ng);
}

static Eina_Bool
_ngi_win_cb_mouse_in(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_In *ev = event;
   Ng *ng = data;

   if (ev->win != ng->win->input)
      return EINA_TRUE;

   ng->pos = ng->horizontal ?
      (ev->root.x - ng->zone->x) :
      (ev->root.y - ng->zone->y);

   ngi_mouse_in(ng);

   return EINA_TRUE;
}

static Eina_Bool
_ngi_win_cb_mouse_out(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Out *ev = event;
   Ng *ng = data;

   if (ev->win != ng->win->input)
      return EINA_TRUE;

   ngi_mouse_out(ng);

   return EINA_TRUE;
}

static Eina_Bool
_ngi_menu_wait_after_timer_cb(void *data)
{
   Ng *ng = data;

   ng->menu_wait_timer = NULL;
   ngi_animate(ng);

   return EINA_FALSE;
}

static Eina_Bool
_ngi_menu_wait_timer_cb(void *data)
{
   Ng *ng = data;

   if (e_menu_grab_window_get())
      return EINA_TRUE;

   ng->menu_wait_timer =
      ecore_timer_add(1.0, _ngi_menu_wait_after_timer_cb, ng);

   return EINA_FALSE;
}

static Eina_Bool
_ngi_win_cb_mouse_down(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Button *ev = event;
   Ng *ng = data;

   if (ev->event_window != ng->win->input)
      return EINA_TRUE;

   Ngi_Item *it = ng->item_active;

   if (ev->buttons == 2)
     {
        ngi_configure_module(ng->cfg);
        return EINA_TRUE;
     }
   else if (ng->item_active && ev->buttons == 1)
     {
        it->drag.x = ev->root.x;
        it->drag.y = ev->root.y;
        it->drag.start = 1;
        it->drag.dnd = 0;
        ng->item_drag = it;
     }
   else if (ev->buttons == 3)
     {
        edje_object_signal_emit(ng->o_label, "e,state,label_hide", "e");
     }

   ITEM_MOUSE_DOWN(it, ev);

   if (ng->menu_wait_timer)
      ecore_timer_del(ng->menu_wait_timer);

   ng->menu_wait_timer =
      ecore_timer_add(0.1, _ngi_menu_wait_timer_cb, ng);

   return EINA_TRUE;
}

static Eina_Bool
_ngi_win_cb_mouse_up(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Button *ev = event;
   Ng *ng = data;

   if (ev->event_window != ng->win->input)
      return EINA_TRUE;

   if (ng->item_active)
     {
        ITEM_MOUSE_UP(ng->item_active, ev);

        if (ng->item_drag)
          {
             ng->item_drag->drag.start = 0;
             ng->item_drag = NULL;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_ngi_win_cb_mouse_wheel(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Wheel *ev = event;
   Ng *ng = data;

   if (ev->event_window != ng->win->input)
      return EINA_TRUE;

   return EINA_TRUE;
}

static Eina_Bool
_ngi_win_cb_mouse_move(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Move *ev = event;
   Ng *ng = data;

   if (ev->event_window != ng->win->input)
      return EINA_TRUE;

   ng->pos = ng->horizontal ? ev->root.x : ev->root.y;

   ng->pos -= ng->horizontal ? ng->zone->x : ng->zone->y;

   if (!ng->mouse_in)
     return EINA_TRUE;

   _ngi_item_activate(ng);

   if (ng->item_drag)
     {
        int dx = ev->root.x - ng->item_drag->drag.x;
        int dy = ev->root.y - ng->item_drag->drag.y;

        if (((dx * dx) + (dy * dy)) > (e_config->drag_resist * e_config->drag_resist))
          {
             ng->item_drag->drag.dnd = 1;
             ng->item_drag->drag.start = 0;

             ITEM_DRAG_START(ng->item_drag);
             ITEM_MOUSE_OUT(ng->item_active);

             ng->item_active = NULL;
             ng->item_drag = NULL;
          }
     }

   _ngi_redraw(ng);
   /* ngi_animate(ng);  */

   return EINA_TRUE;
}

/**************************************************************************/

static void
_ngi_netwm_icon_geometry_set(E_Border *bd, int x, int y, int w, int h)
{
   unsigned int sizes[4];
   sizes[0] = x;
   sizes[1] = y;
   sizes[2] = w;
   sizes[3] = h;
   ecore_x_window_prop_card32_set
      (bd->win, ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, sizes, 4);
}

void
ngi_input_extents_calc(Ng *ng)
{
   int item_zoomed;
   Ngi_Win *win = ng->win;
   int extra = ng->size;
   Ngi_Box *box;
   Eina_List *l;
   int offset = 20;

   if (ng->mouse_in)
     item_zoomed = ng->size * ng->cfg->zoomfactor + offset;
   else if (ng->hide_state == hidden)
     item_zoomed = 2;
   else
     item_zoomed = ng->size + offset;

   switch (ng->cfg->orient)
     {
      case E_GADCON_ORIENT_BOTTOM:
         win->rect.x = ng->start - extra;
         win->rect.y = ng->win->popup->h - item_zoomed;
         win->rect.width = ng->w + 2 * extra;
         win->rect.height = item_zoomed;
         break;

      case E_GADCON_ORIENT_TOP:
         win->rect.x = ng->start;
         win->rect.y = 0;
         win->rect.width = ng->w;
	 win->rect.height = item_zoomed;
         break;

      case E_GADCON_ORIENT_LEFT:
         win->rect.x = 0;
         win->rect.y = ng->start;
         win->rect.width = item_zoomed;
         win->rect.height = ng->w;
         break;

      case E_GADCON_ORIENT_RIGHT:
         win->rect.x = ng->win->popup->w - item_zoomed;
         win->rect.y = ng->start;
         win->rect.width = item_zoomed;
         win->rect.height = ng->w;
         break;
     }

   e_container_window_raise(ng->zone->container, win->input, 999);

   if (ngi_config->use_composite)
     {
	XShapeCombineRectangles((Display *)ecore_x_display_get(),
				win->input, ShapeInput, 0, 0,
				&win->rect, 1, ShapeSet, Unsorted);
     }
   else
     {
	ecore_x_window_move_resize(win->input,
				   ng->zone->x + win->popup->x + win->rect.x,
				   ng->zone->y + win->popup->y + win->rect.y,
				   win->rect.width, win->rect.height);
     }

   EINA_LIST_FOREACH (ng->boxes, l, box)
     {
	if (!box->drop_handler)
	  continue;

	int w = box->w;
	if (w < 10) w = 10;

	switch (ng->cfg->orient)
	  {
	   case E_GADCON_ORIENT_BOTTOM:
	      e_drop_handler_geometry_set
		(box->drop_handler,
		 box->pos, win->popup->h - item_zoomed, w, item_zoomed);
	      break;

	   case E_GADCON_ORIENT_TOP:
	      e_drop_handler_geometry_set
		(box->drop_handler, box->pos, 0, w, item_zoomed);
	      break;

	   case E_GADCON_ORIENT_LEFT:
	      e_drop_handler_geometry_set
		(box->drop_handler, 0, box->pos, item_zoomed, w);
	      break;

	   case E_GADCON_ORIENT_RIGHT:
	      e_drop_handler_geometry_set
		(box->drop_handler, win->popup->w - item_zoomed, box->pos,
		 item_zoomed, w);
	      break;
	  }
     }
}

/**************************************************************************/

static Ngi_Item *
_ngi_item_at_position_get(Ng *ng)
{
   Eina_List *l, *ll;
   Ngi_Item *it;
   Ngi_Box *box;
   int size = ng->size / 2;
   int pos = ng->pos;

   EINA_LIST_FOREACH (ng->boxes, ll, box)
   {
      if (pos < box->pos)
         continue;

      EINA_LIST_FOREACH (box->items, l, it)
      {
         if (it->scale == 0.0)
            continue;

         if (pos <= it->pos + size + ng->opt.item_spacing)
           {
              if (pos + ng->opt.item_spacing < it->pos - size)
                 return NULL;
              else
                 return it;
           }
      }
   }
   return NULL;
}

static void
_ngi_item_activate(Ng *ng)
{
   Ngi_Item *it = _ngi_item_at_position_get(ng);

   if (it && ng->cfg->zoom_one)
      ng->pos = it->pos;

   if (ng->mouse_in && it && it != ng->item_active)
     {
        ITEM_MOUSE_OUT(ng->item_active);
        ITEM_MOUSE_IN(it);
        ng->item_active = it;
     }
   else if (!it || !ng->mouse_in)
     {
        ITEM_MOUSE_OUT(ng->item_active);
        ng->item_active = NULL;
     }

   if (ng->item_active && ng->cfg->show_label)
     {
        int h;
        double zoom = 1.0, disp;

	h = ng->win->popup->h;

	switch (ng->cfg->orient)
          {
           case E_GADCON_ORIENT_BOTTOM:
              evas_object_move(ng->o_label, ng->item_active->pos, (h + ng->hide_step) -
			       ((double)ng->size * ng->zoom + TEXT_DIST));
              break;

           case E_GADCON_ORIENT_TOP:
              evas_object_move(ng->o_label, ng->item_active->pos,
			       ((double)ng->size * ng->zoom + TEXT_DIST) - ng->hide_step);
              break;
          }

        if (ng->item_active->label && ng->item_active->label[0])
          {
	     evas_object_show(ng->o_label);
	     edje_object_signal_emit(ng->o_label, "e,state,label_show", "e");
	     edje_object_part_text_set(ng->o_label, "e.text.label", ng->item_active->label);
          }
        else
           evas_object_hide(ng->o_label);
     }
}

/**************************************************************************/

static void
_ngi_item_appear(Ng *ng, Ngi_Item *it)
{
   double appear_duration = 0.1;

   it->scale = (ecore_time_get() - it->start_time) / appear_duration;

   if (it->scale < 1.0)
     {
        ngi_reposition(ng);
     }
   else
     {
        it->scale = 1.0;
        it->state = normal;

        ng->items_scaling = eina_list_remove(ng->items_scaling, it);

        ngi_reposition(ng);
        ngi_input_extents_calc(ng);
     }

   _ngi_item_activate(ng);
}

static void
_ngi_item_disappear(Ng *ng, Ngi_Item *it)
{
   double appear_duration = 0.2;

   double delta = it->scale - (1.0 - (ecore_time_get() - it->start_time) / appear_duration);

   it->scale -= delta;

   if (it->scale > 0.0)
     {
        ngi_reposition(ng);
     }
   else
     {
        ng->items_scaling = eina_list_remove(ng->items_scaling, it);

        evas_object_clip_unset(it->obj);
        evas_object_clip_unset(it->over);

        it->scale = 0.0;

        if (ng->item_active == it)
           ng->item_active = NULL;

        it->box->items = eina_list_remove(it->box->items, it);
        ngi_item_free(it);

        ng->size = ng->cfg->size;

        ngi_reposition(ng);
        ngi_input_extents_calc(ng);

        ngi_animate(ng);
        ngi_hide_clip(ng);
     }

   _ngi_item_activate(ng);
}

static void
_ngi_zoom_in(Ng *ng)
{
   Config_Item *cfg = ng->cfg;
   double size = ng->size;
   double step = (ng->cfg->zoomfactor * size) / (double)e_config->framerate / cfg->zoom_duration;

   ng->zoom = (ng->zoom * size + step) / size;
   ng->state = zooming;

   if (ng->zoom >= cfg->zoomfactor)
     {
        ng->zoom = cfg->zoomfactor;
        ng->state = zoomed;
     }
}

static void
_ngi_zoom_out(Ng *ng)
{
   Config_Item *cfg = ng->cfg;
   double size = ng->size;
   double step = (ng->cfg->zoomfactor * size) / (double)e_config->framerate / cfg->zoom_duration;

   ng->zoom = (ng->zoom * size - step) / size;
   ng->state = unzooming;

   if (ng->zoom <= 1.0)
     {
        ng->zoom = 1.0;
        ng->state = unzoomed;
        ngi_input_extents_calc(ng);

        if (ng->item_active)
          {
             ITEM_MOUSE_OUT(ng->item_active);
             ng->item_active = NULL;
          }

        edje_object_signal_emit(ng->o_label, "e,state,label_hide", "e");
     }
}

static int
_ngi_autohide(Ng *ng, int hide)
{
   Config_Item *cfg = ng->cfg;
   double step;
   int size;

   if (hide)
     {
        if (ng->hide_state == hiding)
           edje_object_signal_emit(ng->o_label, "e,state,label_hide", "e");

        ng->hide_state = hiding;
     }
   else if (ng->hide_state != showing)
     {
        /* ngi_bar_show(ng); */
        ng->hide_state = showing;
     }

   size = ng->size * 2;

   step = (ng->cfg->zoomfactor * size) / (double)e_config->framerate / cfg->zoom_duration;

   if (step < 1)
      step = 1;

   if (ng->hide_state == hiding)
     {
        ng->hide_step += step;

        if (ng->hide_step > size)
          {
             ng->hide_step = size;
             goto end;
          }
     }
   else
     {
        ng->hide_step -= step;
        if (ng->hide_step < 0)
          {
             ng->hide_step = 0;
             goto end;
          }
     }
   return 1;

end:
   if (hide)
     {
        /* if (ngi_config->use_composite)
         *    ngi_bar_hide(ng); */

        ng->hide_state = hidden;
        ng->zoom = 1.0;
        ng->state = unzoomed;
        ng->item_active = NULL;
     }
   else
      ng->hide_state = show;

   ngi_input_extents_calc(ng);

   return 0;
}

void
ngi_animate(Ng *ng)
{
   ng->changed = 1;
   if (ng->animator)
      return;

   ng->animator = ecore_animator_add(_ngi_animator, ng);
}

static Eina_Bool
_ngi_animator(void *data)
{
   Ng *ng = (Ng *)data;
   Eina_Bool cont = 0;
   Eina_List *l;
   Ngi_Item *it;

   for(l = ng->items_scaling; l; )
     {
        it = (Ngi_Item *)l->data;
        l = l->next;

        if (it->state == appearing)
           _ngi_item_appear(ng, it);
        else if (it->state == disappearing)
           _ngi_item_disappear(ng, it);

        cont++;
     }

   if (ng->cfg->zoomfactor > 1.0)
     {
        if (ng->mouse_in && (ng->state != zoomed))
          {
	     _ngi_zoom_in(ng);
	     cont++;
          }
        else if (!ng->mouse_in && (ng->state != unzoomed))
          {
	     _ngi_zoom_out(ng);
	     cont++;
          }
     }

   if (ng->cfg->autohide)
     {
        if ((ng->mouse_in || ng->show_bar || ng->menu_wait_timer) ||
            (ng->cfg->autohide == AUTOHIDE_FULLSCREEN && !ng->hide_fullscreen))
          {
             if (ng->hide_state != show)
               {
		  _ngi_autohide(ng, 0);
                  cont++;
               }
          }
        else if ((ng->cfg->autohide == AUTOHIDE_NORMAL) || (ng->hide_fullscreen))
          {
             if ((ng->hide_state != hidden) && (ng->state == unzoomed))
               {
		  _ngi_autohide(ng, 1);
                  cont++;
               }
          }
     }

   cont += ng->changed;

   if (cont)
     _ngi_redraw(ng);
   else
      ng->animator = NULL;

   return cont;
}

void
ngi_reposition(Ng *ng)
{
   Ngi_Box *box;
   double pos;
   Eina_List *l, *ll;
   Ngi_Item *it;
   int size = ng->size;
   int cnt = 0;
   int width = ng->horizontal ? ng->win->popup->w : ng->win->popup->h;

   ng->w = 0;

   EINA_LIST_FOREACH (ng->boxes, l, box)
   {
      box->w = 0;

      EINA_LIST_FOREACH (box->items, ll, it)
      {
         if (it->scale == 0.0) continue;
         box->w += (size * it->scale) + ng->opt.item_spacing;
      }

      ng->w += box->w;

      if (cnt++ > 0) ng->w += ng->opt.separator_width;
   }

   ng->start = (width - ng->w) / 2;

   double distance = ng->start - SIDE_OFFSET - width/2;

   double range = ng->cfg->zoom_range * ng->size;

   double disp = erf(distance / range) * range * (ng->cfg->zoomfactor - 1.0) + 0.6;

   int end = ng->start - SIDE_OFFSET + disp;

   /* shrink bar when it becomes larger than screen height/width  */
   if (end < 0 && size > 1)
     {
        ng->size -= 1;
        ngi_reposition(ng);
        return;
     }

   cnt = 0;
   pos = ng->start;

   EINA_LIST_FOREACH (ng->boxes, l, box)
   {
      box->pos = pos;

      if (cnt++ > 0)
         pos += (ng->opt.separator_width);

      EINA_LIST_FOREACH (box->items, ll, it)
      {
         if (it->scale == 0.0)
            continue;

         pos += 0.5 * (it->scale * size + ng->opt.item_spacing);
         it->pos = (int)pos;
         pos += 0.5 * (it->scale * size + ng->opt.item_spacing);
      }
   }

   if (ng->items_scaling)
      return;

   if (ng->cfg->ecomorph_features)
     {
        EINA_LIST_FOREACH (ng->boxes, l, box)
        {
           if (!(box->cfg->type == taskbar))
              continue;

           switch(ng->cfg->orient)
             {
              case E_GADCON_ORIENT_BOTTOM:
                 EINA_LIST_FOREACH(box->items, ll, it)
		   _ngi_netwm_icon_geometry_set
		   (it->border, it->pos - size / 2,
		    (ng->win->popup->y + ng->win->popup->h) - size,
		    size, size);
                 break;

              case E_GADCON_ORIENT_TOP:
		 EINA_LIST_FOREACH(box->items, ll, it)
		   _ngi_netwm_icon_geometry_set
		   (it->border, it->pos - size / 2,
		    ng->hide_step, size, size);
		 break;

              case E_GADCON_ORIENT_LEFT:
		 EINA_LIST_FOREACH(box->items, ll, it)
		   _ngi_netwm_icon_geometry_set
		   (it->border, ng->hide_step,
		    it->pos - size / 2, size, size);
                 break;

              case E_GADCON_ORIENT_RIGHT:
		 EINA_LIST_FOREACH(box->items, ll, it)
		   _ngi_netwm_icon_geometry_set
		   (it->border,
		    (ng->win->popup->x + ng->win->popup->w) - size,
		    it->pos - size / 2, size, size);
                 break;
             }
        }
     }
}

static void
_ngi_zoom_function(Ng *ng, double d, double *disp)
{
   float range = ng->cfg->zoom_range * ng->size;

   *disp = erf(d / range) * range * (ng->zoom - 1.0) + 0.6;
}

static void
_ngi_redraw(Ng *ng)
{
   Ngi_Item *it;
   Ngi_Box *box;
   Eina_List *l, *ll;

   double distance, pos, pos2;
   int end1, end2, size_spacing, hide_step;
   int bw, bh, bx, by;

   Config_Item *cfg = ng->cfg;

   int edge_spacing = ng->opt.edge_spacing;
   int separator_width = ng->opt.separator_width;
   int size = ng->size;

   int cnt = 0;
   double disp = 0.0;
   double zoom = 1.0;

   int w = ng->win->popup->w;
   int h = ng->win->popup->h;

   if (cfg->autohide)
     hide_step = ng->hide_step;
   else
     hide_step = ng->hide_step = 0;

   size_spacing = ng->size + edge_spacing;

   if (cfg->zoomfactor < 1.2)
      ng->pos = w / 2;


   if (cfg->show_background)
     {
        _ngi_zoom_function(ng, ng->start - ng->pos, &disp);
        end1 = ng->start + disp;

        _ngi_zoom_function(ng, ng->start + ng->w - ng->pos, &disp);
        end2 = ng->start + ng->w + disp;

        switch (cfg->orient)
          {
           case E_GADCON_ORIENT_BOTTOM:
	      bx = end1;
	      by = h + hide_step - size_spacing;
	      bw = end2 - end1;
	      bh = size_spacing;
              break;

           case E_GADCON_ORIENT_TOP:
	      bx = end1;
	      by = - hide_step;
	      bw = end2 - end1;
	      bh = size_spacing;
              break;

           case  E_GADCON_ORIENT_LEFT:
	      bx = - hide_step;
	      by = end1;
	      bw = size_spacing;
	      bh = end2 - end1;
              break;

           case E_GADCON_ORIENT_RIGHT:
	      bx = w + hide_step - size_spacing;
	      by = end1;
	      bw = size_spacing;
	      bh = end2 - end1;
          }

	evas_object_move(ng->o_bg, bx, by);
	evas_object_resize(ng->o_bg, bw, bh);
	evas_object_move(ng->o_frame, bx, by);
	evas_object_resize(ng->o_frame, bw, bh);

     }

   if (ng->item_active && ng->state != unzoomed)
     {
        switch (cfg->orient)
          {
           case E_GADCON_ORIENT_BOTTOM:
              evas_object_move(ng->o_label, ng->item_active->pos, (h + hide_step) -
			       ((double)ng->size * ng->zoom + TEXT_DIST));
              break;

           case E_GADCON_ORIENT_TOP:
              evas_object_move(ng->o_label, ng->item_active->pos,
			       ((double)ng->size * ng->zoom + TEXT_DIST) - hide_step);
              break;
          }
     }

   EINA_LIST_FOREACH (ng->boxes, ll, box)
   {
      if (cnt++ > 0)
        {
           _ngi_zoom_function(ng, box->pos - ng->pos, &disp);
           pos = (box->pos + disp);

           switch (cfg->orient)
             {
              case E_GADCON_ORIENT_BOTTOM:
                 evas_object_move(box->separator, pos, h + hide_step - size_spacing);
                 evas_object_resize(box->separator, separator_width, size_spacing);
                 break;

              case E_GADCON_ORIENT_TOP:
                 evas_object_move(box->separator, pos, -hide_step);
                 evas_object_resize(box->separator, separator_width, size_spacing);
                 break;

              case E_GADCON_ORIENT_LEFT:
                 evas_object_move(box->separator, -hide_step, pos);
                 evas_object_resize(box->separator, size_spacing, separator_width);
                 break;

              case E_GADCON_ORIENT_RIGHT:
                 evas_object_move(box->separator, w + hide_step - size_spacing, pos);
                 evas_object_resize(box->separator, size_spacing, separator_width);
             }
           evas_object_show(box->separator);
        }
      else
         evas_object_hide(box->separator);

      EINA_LIST_FOREACH (box->items, l, it)
      {
         double size;

         if (!l->prev)
           {
              _ngi_zoom_function(ng, it->pos - ng->size/2 - ng->pos,  &pos);
              pos = it->pos - ng->size/2 + pos;
           }
         else
           {
              pos = pos2 + ng->opt.item_spacing;
           }

         _ngi_zoom_function(ng, it->pos + ng->size/2 - ng->pos, &pos2);

         pos2 = it->pos + ng->size/2 + pos2;

         size = (pos2 - pos) - 0.5;

         switch (cfg->orient)
           {
            case E_GADCON_ORIENT_BOTTOM:
	       bx = pos;
	       by = h + hide_step - (edge_spacing + size);
               break;

            case E_GADCON_ORIENT_TOP:
	       bx = pos;
	       by = edge_spacing - hide_step;
               break;

            case E_GADCON_ORIENT_LEFT:
	       bx = edge_spacing - hide_step;
	       by = pos;
               break;

            case E_GADCON_ORIENT_RIGHT:
	       bx = w + hide_step - (edge_spacing + size);
	       by = pos;
	       break;
           }

	 evas_object_move(it->obj,  bx, by);
	 if (!it->overlay_signal_timer)
	   evas_object_move(it->over, bx, by);

         if (size != it->size)
           {
              it->size = size;
              evas_object_resize(it->obj, size, size);
              if (!it->overlay_signal_timer)
                 evas_object_resize(it->over, size, size);
           }
      }
   }
   ng->changed = 0;
}

void
ngi_freeze(Ng *ng)
{
   if (ng->animator)
     {
        ecore_animator_del(ng->animator);
        ng->animator = NULL;
     }
}

void
ngi_hide_clip(Ng *ng)
{
   Eina_List *l;
   int found = 0;
   Ngi_Box *box;

   /* hide items clip if there are no items */
   EINA_LIST_FOREACH (ng->boxes, l, box)
   {
      if (box->items)
        {
           found = 1;
           break;
        }
   }
   if (found)
      ngi_bar_show(ng);
   else
      ngi_bar_hide(ng);
}

void
ngi_thaw(Ng *ng)
{
   ngi_hide_clip(ng);
   ngi_reposition(ng);
   ngi_input_extents_calc(ng);
   ngi_animate(ng);
}

void ngi_bar_show(Ng *ng)
{
   if (ng->boxes && !evas_object_visible_get(ng->clip))
      evas_object_show(ng->clip);
}

void ngi_bar_hide(Ng *ng)
{
   if (evas_object_visible_get(ng->clip))
      evas_object_hide(ng->clip);
}

