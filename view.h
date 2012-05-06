#ifndef __VIEW_H__
#define __VIEW_H__

#include <gtk/gtk.h>

struct _view {
    GtkWidget *tree_view,
              *entry_path,
              *view; // box
    GtkListStore *list_store;
    char *current_path;
    gboolean show_hidden,
             show_backup;
};

void view_move_up(GObject *object, gpointer user_data);
struct _view *create_view();

#endif /* __VIEW_H__ */
