#include "chrome.h"
#include "view.h"

struct _chrome {
    GtkWidget *grid,
              *window;
    GList *views;
    struct _view *selected_view;
};

struct _chrome *chrome;

void new_view_horizontal(GObject *object, gpointer user_data)
{
    struct _view *view = create_view();
    chrome->views = g_list_append(chrome->views, view);
    gtk_grid_attach_next_to(GTK_GRID(chrome->grid), view->view, chrome->selected_view->view, 
            GTK_POS_RIGHT, 1, 1);
    chrome->selected_view = view;
    gtk_widget_show_all(chrome->grid);
}
void new_view_vertical(GObject *object, gpointer user_data)
{
    struct _view *view = create_view();
    chrome->views = g_list_append(chrome->views, view);
    gtk_grid_attach_next_to(GTK_GRID(chrome->grid), view->view, chrome->selected_view->view, 
            GTK_POS_BOTTOM, 1, 1);
    chrome->selected_view = view;
    gtk_widget_show_all(chrome->grid);
}

void next_view(GObject *object, gpointer user_data)
{
    GList *l = chrome->views;
    struct _view *old_view = chrome->selected_view;

    for (; l; l = l->next) {
        struct _view *view = l->data;
        if (view == chrome->selected_view) {
            if (!l->next)
                chrome->selected_view = chrome->views->data;
            else
                chrome->selected_view = l->next->data;
            break;
        }
    }
    gtk_widget_set_state_flags(old_view->tree_view, GTK_STATE_FLAG_INSENSITIVE, TRUE);
    gtk_widget_set_state_flags(chrome->selected_view->tree_view, GTK_STATE_FLAG_NORMAL, TRUE);
    gtk_widget_grab_focus(chrome->selected_view->tree_view);
}

void install_keyboard_bindings()
{
    GtkAccelGroup *gtk_accel = gtk_accel_group_new ();
    GClosure *closure;

    // Horizontal split
    closure = g_cclosure_new(G_CALLBACK(new_view_horizontal), (gpointer)NULL, NULL);
    gtk_accel_group_connect(gtk_accel, gdk_keyval_from_name("d"), GDK_MOD1_MASK,
        GTK_ACCEL_VISIBLE, closure);
    g_closure_unref(closure);
    
    // Vertical split
    closure = g_cclosure_new(G_CALLBACK(new_view_vertical), (gpointer)NULL, NULL);
    gtk_accel_group_connect(gtk_accel, gdk_keyval_from_name("s"), GDK_MOD1_MASK,
        GTK_ACCEL_VISIBLE, closure);
    g_closure_unref(closure);

    closure = g_cclosure_new(G_CALLBACK(next_view), (gpointer)NULL, NULL);
    gtk_accel_group_connect(gtk_accel, gdk_keyval_from_name("w"), GDK_MOD1_MASK,
        GTK_ACCEL_VISIBLE, closure);
    g_closure_unref(closure);

    gtk_window_add_accel_group (GTK_WINDOW(chrome->window), gtk_accel);
}

void window_select_view(GtkWidget *widget1, GdkEventButton *event, gpointer user_data)
{
    GList *l_views = chrome->views;
    int found = 0;

    for (; l_views; l_views = l_views->next) {
        struct _view *view = l_views->data;
        GtkWidget *widget = view->view;
        gint wx, wy, ww, wh;
        gdk_window_get_geometry(gtk_widget_get_window(widget), &wx, &wy, &ww, &wh);        
        if ((event->x >= wx && event->y <= (ww + wx)) && (event->y >= wy && event->y <= (wh + wy))) { 
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("%s: BUG - view not found\n", __func__);
        return;
    }
    else
        printf("Found !!!");
}
void init_main_window()
{
    struct _view *left_view = create_view(),
                 *right_view = create_view();

    chrome = g_malloc(sizeof(*chrome));
    chrome->views = NULL;
    chrome->views = g_list_append(chrome->views, left_view);
    chrome->views = g_list_append(chrome->views, right_view);

    chrome->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    chrome->grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(chrome->grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(chrome->grid), TRUE);
    gtk_grid_attach(GTK_GRID(chrome->grid), left_view->view, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(chrome->grid), right_view->view, 1, 0, 1, 1);


    chrome->selected_view = left_view;
    gtk_window_set_title(GTK_WINDOW(chrome->window), "Gtk+ Simple File Manager [gsfm]");
    gtk_window_set_default_size(GTK_WINDOW(chrome->window), 1000, 700);
    gtk_container_add(GTK_CONTAINER(chrome->window), chrome->grid);
    gtk_container_set_border_width(GTK_CONTAINER(chrome->window), 4);

    g_signal_connect(chrome->grid, "button-press-event", G_CALLBACK(window_select_view), NULL);

    install_keyboard_bindings();


    gtk_widget_show_all(chrome->window);
}

