#include "shelm.h"

void 
shelm_textinfo_dialog(const char *window_title, const char *window_text, int window_width, int window_height, const char *window_background, const char *textinfo_filename, Eina_Bool textinfo_editable, Eina_Bool textinfo_nowrap)
{
  Evas_Object *window, *background, *frame, *box, *scroller, *textinfo, *buttonbar, *button_cancel, *button_ok;
  char buf[PATH_MAX];

  if (window_title)
    window = create_window("shellementary-textinfodialog", window_title, cancel_callback);
  else
    window = create_window("shellementary-textinfodialog", _("Text Information"), cancel_callback);

  background = create_background(window, window_background, EINA_TRUE);
  elm_win_resize_object_add(window, background);
  evas_object_show(background);

  frame = create_frame(window, EINA_TRUE);
  elm_win_resize_object_add(window, frame);
  evas_object_show(frame);

  box = create_box(window, EINA_FALSE);
  elm_object_content_set(frame, box);
  evas_object_show(box);

  if (window_text)
    {
      Evas_Object *label;
      label = create_label(window, window_text);
      elm_box_pack_end(box, label);
      evas_object_show(label);
    }

  scroller = create_scroller(window, EINA_FALSE);
  elm_box_pack_end(box, scroller);
  evas_object_show(scroller);

  textinfo = create_entry(window, EINA_FALSE, NULL, EINA_FALSE, textinfo_nowrap, textinfo_editable);
  if (!textinfo_filename)
    ecore_main_fd_handler_add (0, ECORE_FD_READ | ECORE_FD_ERROR, _read_stdin_entry, textinfo, NULL, NULL);
  else
    {
      if (ecore_file_exists(textinfo_filename))
        {
		  FILE *fp;
          char *s;
          char *buffer;
          long numbytes;
		  // open file
          fp = fopen(textinfo_filename, "r");
          // get number of bytes
          fseek(fp, 0L, SEEK_END);
          numbytes = ftell(fp);
          fseek(fp, 0L, SEEK_SET);
          // allocate buffer
          buffer = (char *) calloc(numbytes, sizeof(char));
          if (buffer)
            {
              // read file
              fread(buffer, sizeof(char), numbytes, fp);
              // close file
              fclose(fp);
              // insert into entry
              s = elm_entry_utf8_to_markup(buffer);
              elm_entry_entry_set(textinfo, s);
              // free everything
              E_FREE(s);
              E_FREE(buffer);
            }
        }
    }
  elm_object_content_set(scroller, textinfo);
  evas_object_show(textinfo);

  buttonbar = create_box(window, EINA_TRUE);
  elm_box_pack_end(box, buttonbar);
  evas_object_show(buttonbar);

  snprintf(buf, sizeof(buf), "%s/icon-cancel.png", PACKAGE_DATA_DIR);
  button_cancel = create_button(window, buf, _("Cancel"));
  evas_object_smart_callback_add(button_cancel, "clicked", cancel_callback, NULL);
  elm_box_pack_start(buttonbar, button_cancel);
  evas_object_show(button_cancel);

  snprintf(buf, sizeof(buf), "%s/icon-ok.png", PACKAGE_DATA_DIR);
  button_ok = create_button(window, buf, _("OK"));
  evas_object_smart_callback_add(button_ok, "clicked", entry_callback, textinfo);
  elm_box_pack_end(buttonbar, button_ok);
  evas_object_show(button_ok);

  if (!window_width)
    evas_object_geometry_get(window, NULL, NULL, &window_width, NULL);
  if (!window_height)
    window_height = 400;

  evas_object_resize(window, window_width, window_height);

  evas_object_show(window);
}
