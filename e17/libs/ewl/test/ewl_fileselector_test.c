#include "ewl_test.h"

static Ewl_Widget *fs_button = NULL;

void            __file_clicked(Ewl_Widget *, void *ev_data, void *user_data);

void
__destroy_fileselector_test_window(Ewl_Widget * w, void *ev_data,
				   void *user_data)
{
	ewl_widget_destroy(w);

	ewl_callback_append(fs_button, EWL_CALLBACK_CLICKED,
			    __create_fileselector_test_window, NULL);

	return;
	ev_data = NULL;
	user_data = NULL;
}

void
__create_fileselector_test_window(Ewl_Widget * w, void *ev_data,
				  void *user_data)
{
	Ewl_Widget     *fs;
	Ewl_Widget     *fs_win;

	ewl_callback_del(w, EWL_CALLBACK_CLICKED,
			 __create_fileselector_test_window);

	fs_button = w;

	fs_win = ewl_window_new();
	ewl_window_title_set(EWL_WINDOW(fs_win), "File Selector Test");
	ewl_window_name_set(EWL_WINDOW(fs_win), "EWL Test Application");
	ewl_window_class_set(EWL_WINDOW(fs_win), "EFL Test Application");
	ewl_callback_append(fs_win, EWL_CALLBACK_DELETE_WINDOW,
			    __destroy_fileselector_test_window, NULL);
	ewl_widget_show(fs_win);

	fs = ewl_fileselector_new(__file_clicked);
	ewl_object_fill_policy_set(EWL_OBJECT(fs), EWL_FLAG_FILL_FILL);
	ewl_container_child_append(EWL_CONTAINER(fs_win), fs);
	ewl_widget_show(fs);


	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}

void
__file_clicked(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Fileselector *fs;
	
	printf("file clicked: %s\n", 
			ewl_fileselector_file_get (EWL_FILESELECTOR (fs)));

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}
