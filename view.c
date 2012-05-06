#include "view.h"
#include "chrome.h"
#include <string.h>

void _pupulate_list_ready (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GtkTreeIter iter;
    GtkListStore *list_store = (GtkListStore *)user_data;
    GFileInfo *file_info;
    GError *error = NULL;

    GFileEnumerator *file_enum = g_file_enumerate_children_finish((GFile *)source_object, res, &error);
    if (error) {
        g_object_unref(source_object);
        return;
    }

    gtk_list_store_append(list_store, &iter);
    gtk_list_store_set(list_store, &iter, 0, NULL, 1, "..", 2, "", 3, "", 4, "", 5, "", -1);
    while ((file_info = g_file_enumerator_next_file(file_enum, NULL, &error))) {
        if (error) {
            error = NULL;
            continue;
        }

        const char *filename = g_file_info_get_name(file_info);
        if (!filename || !strcmp(filename, ".") || !strcmp(filename, "..")) {
            g_object_unref(file_info);
            continue;
        }
        if (filename[0] == '.') {
            g_object_unref(file_info);
            continue;
        }
        GIcon *icon = g_file_info_get_icon(file_info);
        goffset size = g_file_info_get_size(file_info);
        const char *content_type = g_file_info_get_content_type(file_info);
        char *size_string = g_format_size_full(size, G_FORMAT_SIZE_IEC_UNITS);
        char *suffix = strrchr(filename, '.');
        GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
        GtkIconInfo *icon_info = gtk_icon_theme_lookup_by_gicon(icon_theme, icon, 24, GTK_ICON_LOOKUP_USE_BUILTIN);
        GdkPixbuf *icon_pixbuf = gtk_icon_info_load_icon(icon_info, &error);
        if (error) {
            printf("Unable to load icon ...\n");
            icon_pixbuf = NULL;
        }

        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, icon_pixbuf, 1, filename, 2, (!suffix || suffix == filename) ? "" : suffix, 3, size_string, 4,
                content_type, 5, "-", -1);
        g_object_unref(file_info);
        error = NULL;
    }
    g_object_unref(source_object);
}

void populate_list(GtkListStore *list_store, const char *path)
{
    gtk_list_store_clear(list_store);
    GFile *parent_dir = g_file_new_for_path(path);
    const char *attrs = "standard::name,standard::content-type,standard::type,standard::size,standard::icon";

    g_file_enumerate_children_async(parent_dir, attrs, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
            G_PRIORITY_DEFAULT, NULL, _pupulate_list_ready, list_store);
}

void view_move_up(GObject *object, gpointer user_data)
{
    struct _view *view = chrome->selected_view;
    GtkTreePath *path = NULL;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(view->tree_view), &path, NULL);
    if (!path) {
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view->tree_view), 0, 0, &path, NULL, NULL, NULL) == FALSE) {
            return;
        }
    }
    gtk_tree_path_prev(path);
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(view->tree_view), path, NULL, FALSE);
    gtk_tree_path_free(path);
}

void view_move_down(GObject *object, gpointer user_data)
{
    struct _view *view = chrome->selected_view;
    GtkTreePath *path = NULL;
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(view->tree_view), &path, NULL);
    if (!path) {
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view->tree_view), 0, 0, &path, NULL, NULL, NULL) == FALSE) {
            return;
        }
    }
    gtk_tree_path_next(path);
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(view->tree_view), path, NULL, FALSE);
    gtk_tree_path_free(path);

}
void view_focus_location(GObject *object, gpointer user_data)
{
    struct _view *view = chrome->selected_view;
    gtk_widget_grab_focus(view->entry_path);
}

void install_view_keyboard_bindings(struct _view *view)
{
    GtkAccelGroup *gtk_accel = gtk_accel_group_new ();
    GClosure *closure;

    // Move up
    closure = g_cclosure_new(G_CALLBACK(view_move_up), (gpointer)view, NULL);
    gtk_accel_group_connect(gtk_accel, gdk_keyval_from_name("k"),0,
        GTK_ACCEL_VISIBLE, closure);
    g_closure_unref(closure);
    
    // Move down
    closure = g_cclosure_new(G_CALLBACK(view_move_down), (gpointer)view, NULL);
    gtk_accel_group_connect(gtk_accel, gdk_keyval_from_name("j"), 0,
        GTK_ACCEL_VISIBLE, closure);
    g_closure_unref(closure);
    
    // Change location
    closure = g_cclosure_new(G_CALLBACK(view_focus_location), (gpointer)view, NULL);
    gtk_accel_group_connect(gtk_accel, gdk_keyval_from_name("l"), GDK_CONTROL_MASK,
        GTK_ACCEL_VISIBLE, closure);
    g_closure_unref(closure);
    
    gtk_window_add_accel_group (GTK_WINDOW(chrome->window), gtk_accel);
}

void view_change_path(GtkEntry *entry, gpointer user_data)
{
    const char *new_path = gtk_entry_get_text(entry);
    struct _view *view = (struct _view *)user_data;
    populate_list(view->list_store, new_path);
    gtk_widget_grab_focus(view->tree_view);
}

struct _view *create_view()
{
    GtkWidget *scrolled,
              *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    struct _view *view = g_malloc(sizeof(struct _view));

    view->list_store = gtk_list_store_new(6, GDK_TYPE_PIXBUF, G_TYPE_STRING,
            G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    view->tree_view = gtk_tree_view_new();
    view->entry_path = gtk_entry_new();
    // Disable search so it won't effect keyboard binding.
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(view->tree_view), FALSE);
    g_signal_connect(view->entry_path, "activate", G_CALLBACK(view_change_path), view);
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    //////////////////////////////////////////////////////////////////
    // TREE VIEW CONSTRUCTION
    //////////////////////////////////////////////////////////////////

    // Column 'Icon'
    renderer = gtk_cell_renderer_pixbuf_new();
    column = gtk_tree_view_column_new_with_attributes ("", renderer, "gicon", 0, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(view->tree_view), column);
    // Column 'Filename'
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes ("Filename", renderer, "text", 1, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(view->tree_view), column);
    gtk_tree_view_column_set_sort_column_id(column, 1);
    g_object_set(G_OBJECT(renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    gtk_tree_view_column_set_min_width(column, 200);
    gtk_tree_view_column_set_resizable(column, TRUE);
    // Column 'Suffix'
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes ("Suffix", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 2);
    gtk_tree_view_append_column (GTK_TREE_VIEW(view->tree_view), column);
    // Column 'Size'
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes ("Size", renderer, "text", 3, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(view->tree_view), column);
    // Column 'Mode'
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes ("Mode", renderer, "text", 4, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 4);
    gtk_tree_view_append_column (GTK_TREE_VIEW(view->tree_view), column);
    // Column 'Owner'
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes ("Owner", renderer, "text", 5, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 5);
    gtk_tree_view_append_column (GTK_TREE_VIEW(view->tree_view), column);
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(view->tree_view), GTK_TREE_MODEL(view->list_store));
    //////////////////////////////////////////////////////////////////

    view->current_path = g_strdup(g_get_home_dir());
    populate_list(view->list_store, view->current_path);
    gtk_entry_set_text(GTK_ENTRY(view->entry_path), view->current_path);

    g_object_unref(view->list_store);
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled), view->tree_view);

    gtk_box_pack_start(GTK_BOX(box), view->entry_path, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 2);
    view->view = box;

    install_view_keyboard_bindings(view);

    return view;
}
