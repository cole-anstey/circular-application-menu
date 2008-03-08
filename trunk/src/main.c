/* circular-main-menu
 * Copyright (C) 2008 - Cole Anstey
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * See http://code.google.com/p/circular-application-menu/
 */
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>

#include <glib/gi18n.h>
#include <string.h>

#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gnome-menus/gmenu-tree.h>

#include "cmmcircularmainmenu.h"

static CaFileLeaf* root_fileleaf = NULL;

int
main (int argc, char **argv)
{
	GtkWidget* window;
	GtkWidget* circular_applications_menu;
    GOptionContext* optioncontext;
    GError* error = NULL;
    GdkScreen* screen;
    GdkColormap* colormap;
    GMenuTree* tree;
    GMenuTreeDirectory* root;
    gboolean hideindicator;
    gboolean glyphsize;

    GOptionEntry options[] =
    {
        { "hideindicator", 'h', 0, G_OPTION_ARG_NONE, &hideindicator, "Hides the indicator displayed when the mouse is over a folder.", NULL  },
        { "glyphsize", 'g', 0, G_OPTION_ARG_INT, &glyphsize, "The size of the glyphs [S: 1=small 2=medium 3=large (default)]. ", "S"  },
        { NULL }
    };

    gnome_vfs_init();

    tree = gmenu_tree_lookup ("applications.menu", GMENU_TREE_FLAGS_NONE);
    g_assert (tree != NULL);

    root = gmenu_tree_get_root_directory (tree);

    if (root == NULL)
    {
        g_warning (_("Menu tree is empty"));
    }

    /* Default values. */
    hideindicator = FALSE;
    glyphsize = 3;

    /* Parse the arguments. */
    optioncontext = g_option_context_new("- circular-application-menu.");
    g_option_context_add_main_entries(optioncontext, options, "circular_applications");
    g_option_context_parse(optioncontext, &argc, &argv, NULL);

    if (!g_option_context_parse (optioncontext, &argc, &argv, &error) ||
        (glyphsize < 1) ||
        (glyphsize > 3))
    {
        g_print ("Option parsing failed: %s\n", error->message);

        return -1;
    }

    g_option_context_free(optioncontext);

    /* Initialise gtk. */
	gtk_init (&argc, &argv);

    if (FALSE == gdk_display_supports_composite(gdk_display_get_default()))
    {
        g_message("The circular-main-menu only works with composited desktops.");

        return 0;
    }

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* Assign a alpha colormap to the window. */
    screen = gtk_widget_get_screen (window);
    colormap = gdk_screen_get_rgba_colormap (screen);

    if (!colormap)
    {
        colormap = gdk_screen_get_rgb_colormap (screen);
    }

    gtk_widget_set_colormap (window, colormap);
    gtk_widget_realize (window);
    gdk_window_set_decorations(window->window, 0);

	circular_applications_menu = ca_circular_applications_menu_new (gdk_screen_width(), gdk_screen_height(), hideindicator, glyphsize);
	gtk_container_add (GTK_CONTAINER (window), circular_applications_menu);

	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    /* Make the application full screen, without this it will be below any top edged panel. */
    gtk_window_fullscreen(GTK_WINDOW(window));

	gtk_widget_show_all (window);

    root_fileleaf = ca_circular_applications_menu(root);

    /* Invalidate the widget. */
    gtk_widget_queue_draw(window);

	gtk_main ();

    gmenu_tree_item_unref (root);

	return 0;
}
