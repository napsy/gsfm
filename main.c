#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include "chrome.h"


int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    init_main_window();

    gtk_main();
    return EXIT_SUCCESS;
}
