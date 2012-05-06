#ifndef __CHROME_H__
#define __CHROME_H__

#include <gtk/gtk.h>

struct _chrome {
    GtkWidget *grid,
              *window;
    GList *views;
    struct _view *selected_view;
};

struct _chrome *chrome;
void init_main_window();

#endif /* __CHROME_H__ */
