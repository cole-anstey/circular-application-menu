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
#include "cmmcircularmainmenu.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <libgnome/gnome-desktop-item.h>

#include <math.h>
#include <string.h>
#include <unistd.h>

#include "../pixmaps/close-menu-prelight.h"
#include "../pixmaps/close-menu-normal.h"

/* Base functions. */
static void _ca_circular_applications_menu_class_init (CaCircularApplicationsMenuClass* klass);
static void _ca_circular_applications_menu_init (CaCircularApplicationsMenu* widget);
static GObject* _ca_circular_applications_menu_constructor (GType type, guint n_construct_params, GObjectConstructParam* construct_params);
static gboolean _ca_circular_applications_menu_expose(GtkWidget* widget, GdkEventExpose *event);
static void _ca_circular_applications_menu_size_request(GtkWidget* widget, GtkRequisition* requisition);
static gboolean _ca_circular_applications_menu_button_press(GtkWidget* widget, GdkEventButton* event);
static gboolean _ca_circular_applications_menu_button_release(GtkWidget* widget, GdkEventButton* event);
static gboolean _ca_circular_applications_menu_key_release(GtkWidget* widget, GdkEventKey* event);
static gboolean _ca_circular_applications_menu_motion_notify(GtkWidget* widget, GdkEventMotion* event);

/* Private functions. */
static void _ca_get_point_from_source_offset(gint source_x, gint source_y, gdouble angle, gdouble radius, gint* destination_x, gint* destination_y);
static gdouble _ca_circular_applications_menu_point_distance(gint x1, gint y1, gint x2, gint y2);
static gboolean _ca_circular_applications_menu_circle_contains_point(gint point_x, gint point_y, gint circle_x, gint circle_y, gint radius);
static gboolean _ca_circular_applications_menu_segment_contains_point(gint point_x, gint point_y, CaFileItem* fileitem);
static gdouble _ca_circular_applications_menu_circumference_from_radius(gdouble radius);
static gdouble _ca_circular_applications_menu_calculate_radius(CaFileLeaf* fileleaf);
static void _ca_circular_applications_menu_position_fileleaf_files(CaFileLeaf* fileleaf, gdouble calculated_radius, gdouble angle);
static void _ca_circular_applications_menu_render(CaCircularApplicationsMenu* circular_applications_menu, cairo_t* cr);
static void _ca_circular_applications_menu_render_fileleaf(CaCircularApplicationsMenu* circular_applications_menu, CaFileLeaf* fileleaf, cairo_t* cr);
static void _ca_circular_applications_menu_render_centred_text(CaCircularApplicationsMenu* circular_applications_menu, gint y, const gchar* text, cairo_t* cr);
static GlyphType _ca_circular_applications_menu_hittest(CaCircularApplicationsMenu* circular_applications_menu, gint x, gint y, CaFileLeaf** found_fileleaf, CaFileItem** found_fileitem);
static GlyphType _ca_circular_applications_menu_hittest_fileleaf(CaCircularApplicationsMenu* circular_applications_menu, CaFileLeaf* fileleaf, gint x, gint y, CaFileLeaf** found_fileleaf, CaFileItem** found_fileitem);
static CaFileLeaf* _ca_circular_applications_menu_show_fileitem(GMenuTreeDirectory* menutreedirectory, LeafType leaftype, CaFileItem* fileitem, gboolean disassociated);
static void _ca_circular_applications_menu_view_centre_fileleaf(CaCircularApplicationsMenu* circular_applications_menu, CaFileLeaf* fileleaf, gint x, gint y);
static void _ca_circular_applications_menu_close_menu(CaCircularApplicationsMenu* circular_applications_menu, CaFileLeaf* fileleaf);
static gdouble _ca_circular_applications_menu_calculate_angle_offset(gdouble angle, gdouble offset);
static gdouble _ca_circular_applications_menu_angle_between_points(gdouble x1, gdouble y1, gdouble x2, gdouble y2);
static gboolean _ca_circular_applications_menu_is_angle_between_angles(gdouble angle, gdouble angleLower, gdouble angleHigher);
static void _ca_circular_applications_get_segment_angles(CaFileItem* fileitem, gint radius, gdouble* from_angle, gdouble* to_angle);
static GdkPixbuf* _ca_circular_applications_icon_loader_get_icon_spec(const char *name, int width, int height);
static const gchar* _ca_circular_applications_imagefinder_path(const gchar *path);

typedef struct _CaCircularApplicationsMenuPrivate CaCircularApplicationsMenuPrivate;

struct _CaCircularApplicationsMenuPrivate
{
    gint view_x_offset;            /* See: OFFSET_2_SCREEN. */
    gint view_y_offset;            /* See: OFFSET_2_SCREEN. */
    gint view_width;
    gint view_height;

    /*< Options >*/
    gboolean hideindicator;
    gint glyphsize;
};

#define CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CA_TYPE_CIRCULAR_APPLICATIONS_MENU, CaCircularApplicationsMenuPrivate))

static gchar *image_paths[] = {
	"/usr/share/pixmaps",
	"/usr/local/share/pixmaps",
	"/usr/kde/3.4/share/icons/default.kde/64x64/actions",
	"/usr/kde/3.4/share/icons/default.kde/64x64/apps",
	"/usr/kde/3.4/share/icons/default.kde/64x64/devices",
	"/usr/kde/3.4/share/icons/default.kde/64x64/filesystems",
	"/usr/kde/3.4/share/icons/default.kde/64x64/mimetypes",
	"/usr/share/icons/",
	"/usr/share/icons/HighContrastLargePrint/48x48/apps",
	"/usr/share/icons/HighContrastLargePrint/48x48/devices",
	"/usr/share/icons/HighContrastLargePrint/48x48/emblems",
	"/usr/share/icons/HighContrastLargePrint/48x48/filesystems",
	"/usr/share/icons/HighContrastLargePrint/48x48/mimetypes",
	"/usr/share/icons/Rodent/48x48/apps",
	"/usr/share/icons/hicolor/48x48/apps",
	NULL
};

gint icon_width                         = 0;
gint icon_height                        = 0;
gboolean g_xwarp_mouse_pointer          = TRUE;

typedef struct _RGBA RGBA;

struct _RGBA
{
    gdouble _r;
    gdouble _g;
    gdouble _b;
    gdouble _a_pen;
    gdouble _a_fill;
    gdouble _line_width;
};

RGBA g_normal_segment_rgba = { 0.0, 0.0, 0.0, 0.80, 0.60, 0.0 };
RGBA g_prelight_segment_rgba = { 0.0, 0.0, 0.0, 1.0, 0.80, 0.0 };
RGBA g_outer_inner_rgba = { 0.0, 0.0, 0.0, 1.0, 0.60, 0.5 };
RGBA g_text_box_rgba = { 0.0, 0.0, 0.0, 1.0, 0.80, 0.0 };
RGBA g_text_rgba = { 1.0, 1.0, 1.0, 0.0, 1.0, 0.0 };

/* Constants. */
#define TEXT_BOUNDARY                   1.0		// The boundary surrounding the text.
#define RADIUS_SPACER                   4.0
#define CENTRE_ICONSIZE                 24.0
#define SEGMENT_ARROW_WIDTH             6.0		// The width of an arrow.
#define SEGMENT_ARROW_HEIGHT            4.0		// The height of an arrow.
#define SEGMENT_CIRCLE_RADIUS           3.0		// The bevel of a fileitem segment.
#define CIRCULAR_SEPERATOR              4.0
#define SPOKE_SEPERATOR                 RADIUS_SPACER
#define SPOKE_LENGTH                    SPOKE_SEPERATOR + CLOSEST_TAB_CIRCLE_RADIUS + TAB_HEIGHT + CLOSEST_TAB_CIRCLE_RADIUS + RADIUS_SPACER
#define RADIUS_SEPERATOR                CIRCULAR_SEPERATOR
#define RADIUS_ICON_SPACER              10.0
#define CIRCULAR_ICON_SPACER            10.0
#define SEGMENT_INNER_SPACER            ((NORMAL_ICONSIZE / 2) + CIRCULAR_ICON_SPACER)
#define SEGMENT_OUTER_SPACER            ((NORMAL_ICONSIZE / 2) + CIRCULAR_ICON_SPACER + SEGMENT_ARROW_HEIGHT)
#define MIN_RADIUS_ICONAREA             (RADIUS_ICON_SPACER + NORMAL_ICONSIZE + RADIUS_ICON_SPACER);
#define INITIAL_RADIUS                  (CENTRE_ICONSIZE / 2) + CIRCULAR_ICON_SPACER + CIRCULAR_SEPERATOR + SEGMENT_INNER_SPACER

#define CLOSEST_TAB_CIRCLE_RADIUS       10.0	// The innermost tab bevel.
#define FARTHEST_TAB_CIRCLE_RADIUS      10.0	// The outermost tab bevel.
#define TAB_HEIGHT						((RADIUS_ICON_SPACER + NORMAL_ICONSIZE + RADIUS_ICON_SPACER) - (RADIUS_SPACER + CLOSEST_TAB_CIRCLE_RADIUS + FARTHEST_TAB_CIRCLE_RADIUS))

#define MAX_FILEITEMS_PER_FILELEAF      15
#ifdef WINDOWS
#define M_PI                            3.14159
#endif
#define RENDER_DEBUG                    TRUE
#define CA_VIEW_X_OFFSET_START          16384
#define CA_VIEW_Y_OFFSET_START          16384
#define OFFSET_2_SCREEN(xy, offset_xy)  (xy - offset_xy)
#define SCREEN_2_OFFSET(xy, offset_xy)  (xy + offset_xy)
#define CLOSE_MENU_TEXT                 "close the menu"
#define CLOSE_SUB_MENU_TEXT             "close the sub menu"
#define RADIAN_2_DEGREE(radian)         (radian * (180.0 / M_PI))
#define DEGREE_2_RADIAN(degree)         (degree * (M_PI / 180.0))

/* Local data. */
static GtkWidgetClass* parent_class = NULL;

static CaFileLeaf* g_root_fileleaf = NULL;
static CaFileLeaf* g_last_opened_fileleaf = NULL;
static CaFileLeaf* g_tabbed_fileleaf = NULL;
static CaFileLeaf* g_current_fileleaf = NULL;
static CaFileItem* g_current_fileitem = NULL;
static GlyphType g_current_type = GLYPH_UNKNOWN;
static CaFileLeaf* g_disassociated_fileleaf = NULL;

gint NORMAL_ICONSIZE                    = 0;
gint TAB_WIDTH                          = (RADIUS_ICON_SPACER + RADIUS_ICON_SPACER);

CaFileLeaf*
ca_circular_applications_menu(GMenuTreeDirectory *root)
{
    return ca_circular_applications_menu_show_leaf(root, ROOT_LEAF, NULL, FALSE);
}

GtkWidget *
ca_circular_applications_menu_new (gint width, gint height, gboolean hideindicator, gint glyphsize)
{
    GObject* object;
    CaCircularApplicationsMenuPrivate* private;

    object = g_object_new (ca_circular_applications_menu_get_type(), NULL);

    private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(object);

    /* Initialise. */
    private->view_width = width;
    private->view_height = height;
    private->view_x_offset = CA_VIEW_X_OFFSET_START - (private->view_width / 2);
    private->view_y_offset = CA_VIEW_Y_OFFSET_START - (private->view_height / 2);
    private->hideindicator = hideindicator;
    private->glyphsize = glyphsize;

	return GTK_WIDGET(object);
}

static GObject*
_ca_circular_applications_menu_constructor (GType type, guint n_construct_params, GObjectConstructParam* construct_params)
{
    GObject* object;
    CaCircularApplicationsMenuPrivate* private;

    /* Call base functionality. */
    object = G_OBJECT_CLASS (parent_class)->constructor(type, n_construct_params, construct_params);
    private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(object);

    return object;
}

/**
 * ca_circular_applications_menu_get_type:
 *
 * Gets the type of the corresponding widget.
 *
 * Returns: a GType of the current widget.
 **/
GType
ca_circular_applications_menu_get_type(void)
{
    /* Could use: G_DEFINE_TYPE (CaCircularApplicationsMenu, ca_circular_applications_menu_menu, GTK_TYPE_DRAWING_AREA); */
    static GType circular_applications_menu_type = 0;

    if (!circular_applications_menu_type)
    {
        static const GTypeInfo circular_applications_menu_info =
        {
            sizeof (CaCircularApplicationsMenuClass),
            NULL,
            NULL,
            (GClassInitFunc) _ca_circular_applications_menu_class_init,
            NULL,
            NULL,
            sizeof (CaCircularApplicationsMenu),
            0,
            (GInstanceInitFunc) _ca_circular_applications_menu_init,
        };

        circular_applications_menu_type = g_type_register_static (gtk_drawing_area_get_type(), "CaCircularApplicationsMenu", &circular_applications_menu_info, 0);
    }

    return circular_applications_menu_type;
}

static void
_ca_circular_applications_menu_class_init (CaCircularApplicationsMenuClass* klass)
{
    GObjectClass* gobject_class;
    GtkObjectClass* object_class;
    GtkWidgetClass* widget_class;

    parent_class = g_type_class_peek_parent (klass);

    gobject_class = G_OBJECT_CLASS (klass);
    object_class = GTK_OBJECT_CLASS (klass);
    widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->constructor = _ca_circular_applications_menu_constructor;

	widget_class->expose_event = _ca_circular_applications_menu_expose;
    widget_class->size_request = _ca_circular_applications_menu_size_request;
    widget_class->button_press_event = _ca_circular_applications_menu_button_press;
    widget_class->button_release_event = _ca_circular_applications_menu_button_release;
    widget_class->key_release_event = _ca_circular_applications_menu_key_release;
    widget_class->motion_notify_event = _ca_circular_applications_menu_motion_notify;

    /* Install the widgets class child properties. */

    /* Install the widgets class properties. */

    /* Install the widgets private struture. */
    g_type_class_add_private (klass, sizeof (CaCircularApplicationsMenuPrivate));
}

static void
_ca_circular_applications_menu_init (CaCircularApplicationsMenu* widget)
{
    GdkScreen* screen;
    GtkSettings* settings;
//    GtkIconSize iconsize;

    GTK_WIDGET_SET_FLAGS (GTK_WIDGET(widget), GTK_CAN_FOCUS);   /* Required to receive key events. */

    /* Retrieve the icon size. */
    screen = gtk_widget_get_screen (GTK_WIDGET(widget));
    settings = gtk_settings_get_for_screen (screen);
    /*
    TODO: stick this all in the constructor.

    switch(private->glyphsize)
    {
        case 2:
            iconsize = GTK_ICON_SIZE_LARGE_TOOLBAR;
        break;
        case 1:
            iconsize = GTK_ICON_SIZE_SMALL_TOOLBAR;
        break;
        default:
            iconsize = GTK_ICON_SIZE_MENU;
        break;
    }
    */
    if (!gtk_icon_size_lookup_for_settings (settings, GTK_ICON_SIZE_MENU, &icon_width, &icon_height))
    {
        g_warning("Invalid icon size\n");
    }
icon_width = 32;
icon_height = 32;

    {
        // Work out the hypotenuse so a square image fits onto a tab.
        gint side;

        side = MAX(icon_width * icon_width, icon_height * icon_height);

        NORMAL_ICONSIZE = MAX(icon_width, icon_height);
        TAB_WIDTH += sqrt(side * 2);
    }

    /* Add additional events which are missing from the base widget. */
    gtk_widget_add_events (
        GTK_WIDGET(widget),
        GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_BUTTON1_MOTION_MASK|GDK_POINTER_MOTION_HINT_MASK|GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
}

static gboolean
_ca_circular_applications_menu_expose (GtkWidget* widget, GdkEventExpose* event)
{
    CaCircularApplicationsMenu* circular_applications_menu;
	CaCircularApplicationsMenuPrivate* private;
	cairo_t* cr;
    gint y;

	circular_applications_menu = CA_CIRCULAR_APPLICATIONS_MENU(widget);
	private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(circular_applications_menu);

	/* get a cairo_t */
	cr = gdk_cairo_create (widget->window);

	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
	//cairo_set_fill_rule (cr, CAIRO_FILL_RULE_WINDING);

    /* draw the background */
    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    /* Draw a gradient circle. */
    /* FIXME: Bit slow this and not working quite right.  Turn off for now.
    {
        cairo_set_source_rgba(cr, 1, 1, 1, 0.9);
        cairo_rectangle(cr, 0, 0, 500, 500);
        cairo_fill(cr);
        cairo_stroke(cr);

		cairo_pattern_t* pattern;

		pattern = cairo_pattern_create_radial(
			widget->allocation.width / 2,
			widget->allocation.height / 2,
			widget->allocation.height / 8,
			widget->allocation.width / 2,
			widget->allocation.height / 2,
			widget->allocation.height);

        cairo_pattern_add_color_stop_rgba (pattern, 0.0, 0.0, 0.0, 0.0, 1.0);
        cairo_pattern_add_color_stop_rgba (pattern, 1.0, 0.0, 0.0, 0.1, 0.0);

		cairo_set_source (cr, pattern);
		cairo_mask (cr, pattern);

        cairo_pattern_destroy (pattern);
    }
    */

	cairo_rectangle (
        cr,
        event->area.x,
        event->area.y,
        event->area.width,
        event->area.height);

	cairo_clip (cr);

	_ca_circular_applications_menu_render (circular_applications_menu, cr);

    /* Render text. */
//    if (NULL == g_disassociated_fileleaf)
    {
        /* Calculate the text position. */
        y = private->view_height - (private->view_height / 7);

        if (g_current_type == GLYPH_ROOT_CENTRE)
        {
            _ca_circular_applications_menu_render_centred_text(
	            circular_applications_menu,
	            y,
	            CLOSE_MENU_TEXT,
	            cr);
        }
        else if (g_current_type == GLYPH_FILE_MENU_CENTRE)
        {
            _ca_circular_applications_menu_render_centred_text(
	            circular_applications_menu,
	            y,
	            CLOSE_SUB_MENU_TEXT,
	            cr);
        }
        else if ((g_current_type != GLYPH_UNKNOWN) &&
            (g_current_fileitem != NULL) &&
            (g_current_fileitem->_menutreeitem != NULL) &&
            (g_current_fileleaf != g_disassociated_fileleaf))
        {
	        const gchar* name;
	        const gchar* comment;

	        name = NULL;
	        comment = NULL;

            if (g_current_type == GLYPH_FILE_MENU)
            {
                name = gmenu_tree_directory_get_name(GMENU_TREE_DIRECTORY(g_current_fileitem->_menutreeitem));
                comment = gmenu_tree_directory_get_comment(GMENU_TREE_DIRECTORY(g_current_fileitem->_menutreeitem));
            }
            else
            {
                name = gmenu_tree_entry_get_name(GMENU_TREE_ENTRY(g_current_fileitem->_menutreeitem));
                comment = gmenu_tree_entry_get_comment(GMENU_TREE_ENTRY(g_current_fileitem->_menutreeitem));
            }

            _ca_circular_applications_menu_render_centred_text(
	            circular_applications_menu,
	            y,
	            (name != NULL) ? name : "",
	            cr);

            y += 25;

            _ca_circular_applications_menu_render_centred_text(
	            circular_applications_menu,
	            y,
	            (comment != NULL) ? comment : "",
	            cr);
        }
    }

	cairo_destroy (cr);

	return FALSE;
}

/**
 * _ca_circular_applications_menu_size_request:
 * @widget: a GtkWidget pointer to the current widget.
 * @requisition: a pointer to the GtkRequisition structure.
 *
 * Generated 'size_request' caused when a windows default size is being requested.
 **/
static void
_ca_circular_applications_menu_size_request(GtkWidget* widget, GtkRequisition* requisition)
{
    CaCircularApplicationsMenuPrivate* private;

    private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(widget);

    requisition->width = private->view_width;
    requisition->height = private->view_height;
}

/**
 * _ca_circular_applications_menu_key_release:
 * @widget: a GtkWidget pointer to the current widget.
 * @event: a pointer to the current event structure.
 *
 * Generated 'key_release_event' caused by a key press or a key release.
 *
 * Returns: %TRUE if the event is handled.
 **/
static gboolean
_ca_circular_applications_menu_key_release(GtkWidget* widget, GdkEventKey* event)
{
    CaCircularApplicationsMenu* circular_applications_menu;
    CaFileLeaf* position_fileleaf;

    g_return_val_if_fail(GTK_WIDGET(widget) != NULL, FALSE);
    g_return_val_if_fail(CA_IS_CIRCULAR_APPLICATIONS_MENU(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    circular_applications_menu = CA_CIRCULAR_APPLICATIONS_MENU(widget);

    position_fileleaf = NULL;

    switch(event->keyval)
    {
        case GDK_Escape:
            /* Quit out of application. */
            gtk_main_quit();

            return FALSE;
        case GDK_Down:
            /* Close the current menu. */

            _ca_circular_applications_menu_close_menu(circular_applications_menu, g_tabbed_fileleaf);

            /* Invalidate the widget. */
            gtk_widget_queue_draw(widget);

            /* Handle any pending events. */
            while (gtk_events_pending())
            {
                /* Process all events currently in the queue. */
                gtk_main_iteration();
            }

            return FALSE;
        case GDK_Home:
            /* Move to the last opened fileleaf. */
            position_fileleaf = g_last_opened_fileleaf;

            break;
        case GDK_End:
            /* Move to the root fileleaf. */
            position_fileleaf = g_root_fileleaf;

            break;
        case GDK_Page_Up:
        {
            /* Move to the next fileleaf. */
            if (g_tabbed_fileleaf == g_last_opened_fileleaf)
                position_fileleaf = g_root_fileleaf;
            else
                position_fileleaf = g_tabbed_fileleaf->_child_fileleaf;

            break;
        }
        case GDK_Page_Down:
        {
            /* Move to the previous fileleaf. */
            if (g_tabbed_fileleaf == g_root_fileleaf)
                position_fileleaf = g_last_opened_fileleaf;
            else
                position_fileleaf = g_tabbed_fileleaf->_parent_fileleaf;

            break;
        }
    }

    if (position_fileleaf != NULL)
    {
        /* Position view at the tabebd fileleaf. */

        g_tabbed_fileleaf = position_fileleaf;

        _ca_circular_applications_menu_view_centre_fileleaf(circular_applications_menu, g_tabbed_fileleaf, -1, -1);

        /* Invalidate the widget. */
        gtk_widget_queue_draw(widget);

        /* Handle any pending events. */
        while (gtk_events_pending())
        {
            /* Process all events currently in the queue. */
            gtk_main_iteration();
        }
    }

    return FALSE;
}

/**
 * _ca_circular_applications_menu_motion_notify:
 * @widget: a GtkWidget pointer to the current widget.
 * @event: a pointer to the current event structure.
 *
 * Generated 'motion_notify_event' caused when the pointer moves.
 *
 * Returns: %TRUE if the event is handled.
 **/
static gboolean
_ca_circular_applications_menu_motion_notify(GtkWidget* widget, GdkEventMotion* event)
{
    CaCircularApplicationsMenu* circular_applications_menu;
    CaCircularApplicationsMenuPrivate* private;
    gint x, y;
    GdkModifierType state;
	CaFileLeaf* previous_fileleaf;
	CaFileItem* previous_fileitem;
	GlyphType previous_type;

    g_return_val_if_fail(GTK_WIDGET(widget) != NULL, FALSE);
    g_return_val_if_fail(CA_IS_CIRCULAR_APPLICATIONS_MENU(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    circular_applications_menu = CA_CIRCULAR_APPLICATIONS_MENU(widget);

    private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(widget);

    if (event->is_hint)
        /* This function will cause another motion event arrive. */
        gdk_window_get_pointer (event->window, &x, &y, &state);
    else
    {
        x = (gint)event->x;
        y = (gint)event->y;
        state = event->state;
    }

	/* Update the current item. */
	previous_fileleaf = g_current_fileleaf;
	previous_fileitem = g_current_fileitem;
	previous_type = g_current_type;

	g_current_type = _ca_circular_applications_menu_hittest(
		circular_applications_menu,
		SCREEN_2_OFFSET((gint)event->x, private->view_x_offset),
		SCREEN_2_OFFSET((gint)event->y, private->view_y_offset),
		&g_current_fileleaf,
		&g_current_fileitem);

	// Display a disassociated fileleaf.
	if (GLYPH_TAB == g_current_type)
	{
		/* Invalidate the widget. */
		gtk_widget_queue_draw(widget);

        /* Handle any pending events. */
        while (gtk_events_pending())
        {
            /* Process all events currently in the queue. */
            gtk_main_iteration();
        }
	}
	else if ((g_current_fileleaf != previous_fileleaf) ||
			 (g_current_fileitem != previous_fileitem) ||
			 (g_current_type != previous_type))
	{
		/* The selected item has changed. */

		if (FALSE == private->hideindicator)
		{
			if (g_disassociated_fileleaf != NULL)
			{
				/* Close the disassociated fileleaf. */

				_ca_circular_applications_menu_close_menu(circular_applications_menu, g_disassociated_fileleaf);
				g_disassociated_fileleaf = NULL;
			}

			if ((g_current_fileitem != NULL) &&
				(g_current_fileitem->_type == GLYPH_FILE_MENU))
			{
				/* Show the disassociated fileleaf. */

				g_assert(g_disassociated_fileleaf == NULL);
				g_disassociated_fileleaf = _ca_circular_applications_menu_show_fileitem(
					GMENU_TREE_DIRECTORY(g_current_fileitem->_menutreeitem),
					GLYPH_FILE_MENU_CENTRE,
					g_current_fileitem,
					TRUE);
			}
		}

		/* Invalidate the widget. */
		gtk_widget_queue_draw(widget);

        /* Handle any pending events. */
        while (gtk_events_pending())
        {
            /* Process all events currently in the queue. */
            gtk_main_iteration();
        }
	}

    return TRUE;
}
/**
 * _ca_circular_applications_menu_button_press:
 * @widget: a GtkWidget pointer to the current widget.
 * @event: a pointer to the current event structure.
 *
 * Generated 'button_press_event' caused when a mouse button is pressed.
 *
 * Returns: %TRUE if the event is handled.
 **/
static gboolean
_ca_circular_applications_menu_button_press(GtkWidget* widget, GdkEventButton* event)
{
    CaCircularApplicationsMenu* circular_applications_menu;

    circular_applications_menu = CA_CIRCULAR_APPLICATIONS_MENU(widget);

    if ((event->type == GDK_2BUTTON_PRESS) ||
        (event->type == GDK_3BUTTON_PRESS))
    {
        /* Double clicked. */
    }

    return FALSE;
}

/**
 * _ca_circular_applications_menu_button_release:
 * @widget: a GtkWidget pointer to the current widget.
 * @event: a pointer to the current event structure.
 *
 * Generated 'button_press_event' caused when a mouse button is released.
 *
 * Returns: %TRUE if the event is handled.
 **/
static gboolean
_ca_circular_applications_menu_button_release(GtkWidget* widget, GdkEventButton* event)
{
    CaCircularApplicationsMenu* circular_applications_menu;
    CaCircularApplicationsMenuPrivate* private;
    CaFileLeaf* fileleaf;
    CaFileItem* fileitem;

    circular_applications_menu = CA_CIRCULAR_APPLICATIONS_MENU(widget);

    private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(widget);

    /* Copy if changed during a 'motion-notify'. */
    fileleaf = g_current_fileleaf;
    fileitem = g_current_fileitem;

    /* Check for a right click. */
    if (event->button == 3) /* None=0, Left=1, Middle=2, Right=3 */
    {
        /* Position view at mouse. */

        _ca_circular_applications_menu_view_centre_fileleaf(
            circular_applications_menu,
            fileleaf,
            SCREEN_2_OFFSET((gint)event->x, private->view_x_offset),
            SCREEN_2_OFFSET((gint)event->y, private->view_y_offset));

        /* Invalidate the widget. */
        gtk_widget_queue_draw(widget);

        /* Handle any pending events. */
        while (gtk_events_pending())
        {
            /* Process all events currently in the queue. */
            gtk_main_iteration();
        }

        return FALSE;
    }

    if (fileitem != NULL)
    {
        /* A fileitem has been hit. */

        if (fileitem->_type == GLYPH_FILE_MENU)
        {
            /* Open the current menu. */
            CaFileLeaf* sub_fileleaf;

            if (g_disassociated_fileleaf != NULL)
            {
                /* Close the disassociated fileleaf. */

                _ca_circular_applications_menu_close_menu(circular_applications_menu, g_disassociated_fileleaf);
                g_disassociated_fileleaf = NULL;
            }

            sub_fileleaf = _ca_circular_applications_menu_show_fileitem(
                GMENU_TREE_DIRECTORY(fileitem->_menutreeitem),
                GLYPH_FILE_MENU_CENTRE,
                fileitem,
                FALSE);

            g_current_fileitem = sub_fileleaf->_central_glyph;    /* Disassociated current fileitem so the text changes. */

            _ca_circular_applications_menu_view_centre_fileleaf(circular_applications_menu, sub_fileleaf, -1, -1);

            /* Update the current navigational fileleafs. */
            g_last_opened_fileleaf = sub_fileleaf;
            g_tabbed_fileleaf = sub_fileleaf;

            /* Update to reflect the new view position. */

            g_current_type = _ca_circular_applications_menu_hittest(
                circular_applications_menu,
                SCREEN_2_OFFSET((gint)event->x, private->view_x_offset),
                SCREEN_2_OFFSET((gint)event->y, private->view_y_offset),
                &g_current_fileleaf,
                &g_current_fileitem);

            /* Invalidate the widget. */
            gtk_widget_queue_draw(widget);

            /* Handle any pending events. */
            while (gtk_events_pending())
            {
                /* Process all events currently in the queue. */
                gtk_main_iteration();
            }

            /* Move the mouse pointer to the centre of the screen. */
            if (TRUE == g_xwarp_mouse_pointer)
            {
                // Move that pointer!
                XWarpPointer (
                    GDK_DISPLAY (),
                    None,
                    GDK_ROOT_WINDOW (),
                    0, 0, 0, 0,
                    private->view_width / 2,
                    private->view_height / 2);
            }
        }
        else if (fileitem->_type == GLYPH_FILE_MENU_CENTRE)
        {
            /* Close the current menu. */
            _ca_circular_applications_menu_close_menu(circular_applications_menu, fileleaf);

            /* Update to reflect the new view position. */

            g_current_type = _ca_circular_applications_menu_hittest(
                circular_applications_menu,
                SCREEN_2_OFFSET((gint)event->x, private->view_x_offset),
                SCREEN_2_OFFSET((gint)event->y, private->view_y_offset),
                &g_current_fileleaf,
                &g_current_fileitem);

            /* Invalidate the widget. */
            gtk_widget_queue_draw(widget);

            /* Handle any pending events. */
            while (gtk_events_pending())
            {
                /* Process all events currently in the queue. */
                gtk_main_iteration();
            }

            /* Move the mouse pointer to the centre of the screen. */
            if (TRUE == g_xwarp_mouse_pointer)
            {
                // Move that pointer!
                XWarpPointer (
                    GDK_DISPLAY (),
                    None,
                    GDK_ROOT_WINDOW (),
                    0, 0, 0, 0,
                    private->view_width / 2,
                    private->view_height / 2);
            }
        }
        else if (fileitem->_type == GLYPH_FILE)
        {
            /* Launch file. */
            GnomeDesktopItem* desktopitem;
            const gchar* desktopfile;

            desktopfile = gmenu_tree_entry_get_desktop_file_path(GMENU_TREE_ENTRY(fileitem->_menutreeitem));
            g_assert(NULL != desktopfile);

            desktopitem = gnome_desktop_item_new_from_file(
                desktopfile,
                GNOME_DESKTOP_ITEM_LOAD_ONLY_IF_EXISTS|GNOME_DESKTOP_ITEM_LOAD_NO_TRANSLATIONS,
                NULL);

            /*
            FIXME: The following just does not want to work, so use execlp instead.
	        gnome_desktop_item_launch (desktopitem, NULL, GNOME_DESKTOP_ITEM_LAUNCH_ONLY_ONE, NULL);
	        */
	        {
            	const char* app;
                char arg[255];
                gint i;

                app = gnome_desktop_item_get_string(desktopitem, GNOME_DESKTOP_ITEM_EXEC);

                /* Strip off any arguments. */
                for (i = 0; app[i] && app[i] != ' '; ++i)
                {
                    arg[i] = app[i];
                }

                arg[i] = '\0';

                execlp(arg, arg, NULL);
	        }

            gnome_desktop_item_unref (desktopitem);

            /* Quit out of application. */
            gtk_main_quit();
        }
        else if (fileitem->_type == GLYPH_ROOT_CENTRE)
        {
            /* Quit out of application. */
            gtk_main_quit();
        }
    }
    else if (g_current_fileleaf == NULL)
    {
        /* Nothing hit. */

        /* Quit out of application. */
        gtk_main_quit();
    }

    return FALSE;
}

static void
_ca_circular_applications_menu_close_menu(CaCircularApplicationsMenu* circular_applications_menu, CaFileLeaf* fileleaf)
{
    CaFileLeaf* parent_fileleaf;

    parent_fileleaf = fileleaf->_parent_fileleaf;
    g_assert(parent_fileleaf != NULL);

    ca_circular_applications_menu_close_fileleaf(fileleaf);

    g_current_fileitem = parent_fileleaf->_central_glyph;    /* Disassociated current fileitem so the text changes. */
    g_current_fileleaf = parent_fileleaf->_central_glyph->_assigned_fileleaf;
	g_current_type = GLYPH_UNKNOWN;

    if (fileleaf != g_disassociated_fileleaf)
	{
        _ca_circular_applications_menu_view_centre_fileleaf(circular_applications_menu, parent_fileleaf, -1, -1);

		/* Update the current navigational fileleafs. */
		g_last_opened_fileleaf = parent_fileleaf;
	}

    g_tabbed_fileleaf = parent_fileleaf;
}

static GlyphType
_ca_circular_applications_menu_hittest(CaCircularApplicationsMenu* circular_applications_menu, gint x, gint y, CaFileLeaf** found_fileleaf, CaFileItem** found_fileitem)
{
    *found_fileleaf = NULL;
    *found_fileitem = NULL;

    if (NULL != g_root_fileleaf)
    {
		/* Render all fileleafs. */
        CaFileLeaf* current_fileleaf;
		GlyphType hit_type;

		/* There should always at least one fileleaf. */
		g_assert(NULL != g_last_opened_fileleaf);

		// Walk from the last opened fileleaf to the root fileleaf so overlapped
		// fileitems take precedence.
        current_fileleaf = g_last_opened_fileleaf;

        /* Iterate the fileleafs. */
        do
        {
            GList* sub_list;

			hit_type = _ca_circular_applications_menu_hittest_fileleaf(circular_applications_menu, current_fileleaf, x, y, found_fileleaf, found_fileitem);

            if (GLYPH_UNKNOWN != hit_type)
                return hit_type;    /* Found. */

            /* Iterate the sub fileleafs. */
            sub_list = g_list_first(current_fileleaf->_sub_fileleaves_list);

            while (sub_list)
            {
                CaFileLeaf* sub_fileleaf;

                sub_fileleaf = (CaFileLeaf*)sub_list->data;
                g_assert(sub_fileleaf != NULL);

                hit_type = _ca_circular_applications_menu_hittest_fileleaf(circular_applications_menu, sub_fileleaf, x, y, found_fileleaf, found_fileitem);

				if (GLYPH_UNKNOWN != hit_type)
					return hit_type;	/* Found. */

                sub_list = g_list_next(sub_list);
            }

            /* Get next child fileleaf. */
            current_fileleaf = current_fileleaf->_parent_fileleaf;
        }
        while (current_fileleaf != NULL);
    }

    return GLYPH_UNKNOWN;	/* Not found. */
}

static GlyphType
_ca_circular_applications_menu_hittest_fileleaf(CaCircularApplicationsMenu* circular_applications_menu, CaFileLeaf* fileleaf, gint x, gint y, CaFileLeaf** found_fileleaf, CaFileItem** found_fileitem)
{
    CaCircularApplicationsMenuPrivate* private;
    GList* file_list;
    CaFileItem* associated_fileitem;
	gint tab_x;
	gint tab_y;

    private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(circular_applications_menu);

	if (NULL != fileleaf->_central_glyph->_associated_fileitem)
	{
		/* Check the fileleaf. */
		if (_ca_circular_applications_menu_circle_contains_point(
			x,
			y,
			fileleaf->_central_glyph->x,
			fileleaf->_central_glyph->y,
			fileleaf->radius + (gint)(RADIUS_SPACER + CLOSEST_TAB_CIRCLE_RADIUS + TAB_HEIGHT + FARTHEST_TAB_CIRCLE_RADIUS)))
		{
			/* Check the fileleaf tab. */
			_ca_get_point_from_source_offset(
				fileleaf->_central_glyph->x,
				fileleaf->_central_glyph->y,
				fileleaf->_central_glyph->_parent_angle,
				fileleaf->radius + ((RADIUS_SPACER + CLOSEST_TAB_CIRCLE_RADIUS + TAB_HEIGHT + FARTHEST_TAB_CIRCLE_RADIUS) / 2),
				&tab_x,
				&tab_y);

			if (_ca_circular_applications_menu_circle_contains_point(x, y, tab_x, tab_y, fileleaf->_central_glyph->_associated_fileitem->size))
			{
				*found_fileitem = fileleaf->_central_glyph->_associated_fileitem;

				return GLYPH_TAB;	/* Found. */
			}
		}
	}

	/* Check the fileleaf. */
    if (_ca_circular_applications_menu_circle_contains_point(x, y, fileleaf->x, fileleaf->y, fileleaf->radius))
	{
		*found_fileleaf = fileleaf;

		/* Check the fileleaf central glyph. */
		if (_ca_circular_applications_menu_circle_contains_point(x, y, fileleaf->_central_glyph->x, fileleaf->_central_glyph->y, fileleaf->_central_glyph->size))
		{
			*found_fileitem = fileleaf->_central_glyph;

			return (*found_fileitem)->_type;	/* Found. */
		}

		/* Retieve the fileleafs associated sub fileitem. */
		if (fileleaf->_child_fileleaf != NULL)
			associated_fileitem = fileleaf->_child_fileleaf->_central_glyph->_associated_fileitem;
		else
			associated_fileitem = NULL;

		/* Iterate the fileleaf fileitems. */
		file_list = g_list_first(fileleaf->_fileitem_list);

		while (file_list)
		{
			CaFileItem* fileitem;

			fileitem = (CaFileItem*)file_list->data;
			g_assert(fileitem != NULL);

			if (associated_fileitem != fileitem)    /* Do not render as it is a sub fileleaf. */
			{
				/* Check the fileleaf fileitems. */
				//if (_ca_circular_applications_menu_circle_contains_point(x, y, fileitem->x, fileitem->y, fileitem->size / 2))
				if (_ca_circular_applications_menu_segment_contains_point(x, y, fileitem))
				{
					*found_fileitem = fileitem;

					return (*found_fileitem)->_type;	/* Found. */
				}
			}

			file_list = g_list_next(file_list);
		}
	}

    return GLYPH_UNKNOWN;	/* Not found. */
}

static void
_ca_circular_applications_menu_render(CaCircularApplicationsMenu* circular_applications_menu, cairo_t* cr)
{
    CaFileLeaf* current_fileleaf;

    if (g_root_fileleaf == NULL)
        return;

    /* Render all fileleafs. */

    current_fileleaf = g_root_fileleaf;  /* There should always at least one fileleaf at this point. */

    /* Iterate the fileleafs. */
    do
    {
        GList* sub_list;

#ifdef RENDER_DEBUG
        _ca_circular_applications_menu_render_fileleaf(circular_applications_menu, current_fileleaf, cr);
#endif  /* RENDER_DEBUG */

        /* Iterate the sub fileleafs. */
        sub_list = g_list_first(current_fileleaf->_sub_fileleaves_list);

        while (sub_list)
        {
            CaFileLeaf* sub_fileleaf;

            sub_fileleaf = (CaFileLeaf*)sub_list->data;
            g_assert(sub_fileleaf != NULL);

#ifdef RENDER_DEBUG
            _ca_circular_applications_menu_render_fileleaf(circular_applications_menu, sub_fileleaf, cr);
#endif  /* RENDER_DEBUG */

            sub_list = g_list_next(sub_list);
        }

        /* Get next child fileleaf. */
        current_fileleaf = current_fileleaf->_child_fileleaf;
    }
    while (current_fileleaf != NULL);

#ifdef RENDER_DEBUG
    if (g_disassociated_fileleaf != NULL)
        /* Render the disassociated fileleaf. */
        _ca_circular_applications_menu_render_fileleaf(circular_applications_menu, g_disassociated_fileleaf, cr);
#endif  /* RENDER_DEBUG */
}

static void
_ca_circular_applications_get_segment_angles(CaFileItem* fileitem, gint radius, gdouble* from_angle, gdouble* to_angle)
{
	gdouble half_circular_angle_share;
	gdouble circumference_percentage;
	gdouble common_angle;

	g_assert(NULL != fileitem);
	g_assert(NULL != from_angle);
	g_assert(NULL != to_angle);

	half_circular_angle_share = fileitem->_circular_angle_share / 2;

	// Smaller the circumference then larger the angle.
	circumference_percentage = (RADIUS_SEPERATOR + (SEGMENT_CIRCLE_RADIUS * 2)) / _ca_circular_applications_menu_circumference_from_radius(radius);
	common_angle = (360.0 * circumference_percentage) / 2;

	*from_angle = _ca_circular_applications_menu_calculate_angle_offset(
		fileitem->_parent_angle,
		-(half_circular_angle_share - common_angle));
	*to_angle = _ca_circular_applications_menu_calculate_angle_offset(
		fileitem->_parent_angle,
		half_circular_angle_share - common_angle);
}

static void
_ca_circular_applications_menu_render_fileleaf(CaCircularApplicationsMenu* circular_applications_menu, CaFileLeaf* fileleaf, cairo_t* cr)
{
    CaCircularApplicationsMenuPrivate* private;
    GList* file_list;
    CaFileItem* associated_fileitem;

    private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(circular_applications_menu);

    /* Render the fileleaf. */
    if (fileleaf == g_disassociated_fileleaf)
    {
        // Render the disassociated menu outer.
        cairo_arc (
			cr,
			OFFSET_2_SCREEN(fileleaf->x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->y, private->view_y_offset),
			fileleaf->radius,
			0,
			DEGREE_2_RADIAN(360));

		cairo_new_sub_path (cr);

        cairo_arc (
			cr,
			OFFSET_2_SCREEN(fileleaf->x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->y, private->view_y_offset),
			fileleaf->radius + RADIUS_SPACER,
			0,
			DEGREE_2_RADIAN(360));

        /* Render to the cairo context. */
        cairo_set_line_width (cr, g_outer_inner_rgba._line_width);
		cairo_set_source_rgba (cr, g_outer_inner_rgba._r, g_outer_inner_rgba._g, g_outer_inner_rgba._b, g_outer_inner_rgba._a_fill);
		cairo_fill_preserve (cr);
        cairo_set_source_rgba (cr, g_outer_inner_rgba._r, g_outer_inner_rgba._g, g_outer_inner_rgba._b, g_outer_inner_rgba._a_pen);
		cairo_stroke (cr);
    }
    else if (fileleaf == g_root_fileleaf)
    {
        // Render the root menu outer.
        cairo_arc (
			cr,
			OFFSET_2_SCREEN(fileleaf->x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->y, private->view_y_offset),
			fileleaf->radius + RADIUS_SPACER,
			0,
			DEGREE_2_RADIAN(360));

		cairo_new_sub_path (cr);

		cairo_arc (
			cr,
			OFFSET_2_SCREEN(fileleaf->x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->y, private->view_y_offset),
			fileleaf->radius,
			0,
			DEGREE_2_RADIAN(360));

        /* Render to the cairo context. */
        cairo_set_line_width (cr, g_outer_inner_rgba._line_width);
		cairo_set_source_rgba (cr, g_outer_inner_rgba._r, g_outer_inner_rgba._g, g_outer_inner_rgba._b, g_outer_inner_rgba._a_fill);
		cairo_fill_preserve (cr);
        cairo_set_source_rgba (cr, g_outer_inner_rgba._r, g_outer_inner_rgba._g, g_outer_inner_rgba._b, g_outer_inner_rgba._a_pen);
		cairo_stroke (cr);
    }
    else
    {
        // Render a child menu outer.
		/*
		 C   E
		 B   F
		A     A
		A     A
		A     A
		 AAAAA
		*/

		gdouble parent_angle;

		parent_angle = fileleaf->_central_glyph->_parent_angle;

		if (fileleaf->_menu_render == NULL)
		{
			gdouble circumference_percentage;
			gdouble common_angle;

			fileleaf->_menu_render = g_new(CaMenuRender, 1);

			// Smaller the circumference then larger the angle.
			circumference_percentage = TAB_WIDTH / _ca_circular_applications_menu_circumference_from_radius(fileleaf->radius + RADIUS_SPACER);
			common_angle = (360.0 * circumference_percentage);

			// Calculate the nearest lowest and highest angle.
			fileleaf->_menu_render->lowest_angle_nearest = _ca_circular_applications_menu_calculate_angle_offset(parent_angle, -(common_angle / 2));
			fileleaf->_menu_render->highest_angle_nearest = _ca_circular_applications_menu_calculate_angle_offset(parent_angle, (common_angle / 2));

			// Get B F information
			fileleaf->_menu_render->Bcircle_lowest_angle = _ca_circular_applications_menu_calculate_angle_offset(parent_angle, 90.0);
			fileleaf->_menu_render->Bcircle_highest_angle = _ca_circular_applications_menu_calculate_angle_offset(fileleaf->_menu_render->lowest_angle_nearest, 180.0);

			fileleaf->_menu_render->Fcircle_lowest_angle = _ca_circular_applications_menu_calculate_angle_offset(fileleaf->_menu_render->highest_angle_nearest, 180.0);
			fileleaf->_menu_render->Fcircle_highest_angle = _ca_circular_applications_menu_calculate_angle_offset(parent_angle, -90.0);

			_ca_get_point_from_source_offset(
				fileleaf->_central_glyph->x,
				fileleaf->_central_glyph->y,
				fileleaf->_menu_render->lowest_angle_nearest,
				fileleaf->radius + RADIUS_SPACER + CLOSEST_TAB_CIRCLE_RADIUS,
				&fileleaf->_menu_render->Bcircle_x,
				&fileleaf->_menu_render->Bcircle_y);

			_ca_get_point_from_source_offset(
				fileleaf->_central_glyph->x,
				fileleaf->_central_glyph->y,
				fileleaf->_menu_render->highest_angle_nearest,
				fileleaf->radius + RADIUS_SPACER + CLOSEST_TAB_CIRCLE_RADIUS,
				&fileleaf->_menu_render->Fcircle_x,
				&fileleaf->_menu_render->Fcircle_y);

			_ca_get_point_from_source_offset(
				fileleaf->_menu_render->Bcircle_x,
				fileleaf->_menu_render->Bcircle_y,
				fileleaf->_menu_render->Bcircle_lowest_angle,
				CLOSEST_TAB_CIRCLE_RADIUS,
				&fileleaf->_menu_render->Bcircle_arc_x,
				&fileleaf->_menu_render->Bcircle_arc_y);

			// Get gap information.

			// Smaller the circumference then larger the angle.
			circumference_percentage = (TAB_WIDTH  - (FARTHEST_TAB_CIRCLE_RADIUS * 2)) / _ca_circular_applications_menu_circumference_from_radius(fileleaf->radius + RADIUS_SPACER + CLOSEST_TAB_CIRCLE_RADIUS + TAB_HEIGHT + FARTHEST_TAB_CIRCLE_RADIUS);
			common_angle = (360.0 * circumference_percentage) / 2;

			fileleaf->_menu_render->lowest_angle_farthest = _ca_circular_applications_menu_calculate_angle_offset(parent_angle, -(common_angle / 2));
			fileleaf->_menu_render->highest_angle_farthest = _ca_circular_applications_menu_calculate_angle_offset(parent_angle, common_angle / 2);

			// Get C E information
			fileleaf->_menu_render->Ccircle_lowest_angle = _ca_circular_applications_menu_calculate_angle_offset(parent_angle, -90.0);
			fileleaf->_menu_render->Ccircle_highest_angle = parent_angle;

			fileleaf->_menu_render->Ecircle_lowest_angle = parent_angle;
			fileleaf->_menu_render->Ecircle_highest_angle = _ca_circular_applications_menu_calculate_angle_offset(parent_angle, 90.0);

			_ca_get_point_from_source_offset(
				fileleaf->_central_glyph->x,
				fileleaf->_central_glyph->y,
				fileleaf->_menu_render->lowest_angle_farthest,
				fileleaf->radius + RADIUS_SPACER + CLOSEST_TAB_CIRCLE_RADIUS + TAB_HEIGHT,
				&fileleaf->_menu_render->Ccircle_x,
				&fileleaf->_menu_render->Ccircle_y);

			_ca_get_point_from_source_offset(
				fileleaf->_central_glyph->x,
				fileleaf->_central_glyph->y,
				fileleaf->_menu_render->highest_angle_farthest,
				fileleaf->radius + RADIUS_SPACER + CLOSEST_TAB_CIRCLE_RADIUS + TAB_HEIGHT,
				&fileleaf->_menu_render->Ecircle_x,
				&fileleaf->_menu_render->Ecircle_y);

			_ca_get_point_from_source_offset(
				fileleaf->_menu_render->Ccircle_x,
				fileleaf->_menu_render->Ccircle_y,
				fileleaf->_menu_render->Ccircle_lowest_angle,
				FARTHEST_TAB_CIRCLE_RADIUS,
				&fileleaf->_menu_render->Ccircle_arc_x,
				&fileleaf->_menu_render->Ccircle_arc_y);

			/* Render the parent fileleaf association on the tab. */
			fileleaf->_menu_render->tab_glyph_radius = fileleaf->radius + ((RADIUS_SPACER + CLOSEST_TAB_CIRCLE_RADIUS + TAB_HEIGHT + FARTHEST_TAB_CIRCLE_RADIUS) / 2);

			_ca_get_point_from_source_offset(
				fileleaf->_central_glyph->x,
				fileleaf->_central_glyph->y,
				parent_angle,
				fileleaf->_menu_render->tab_glyph_radius,
				&fileleaf->_menu_render->tab_glyph_x,
				&fileleaf->_menu_render->tab_glyph_y);
		}

		// Render A
		cairo_arc(
			cr,
			OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
			fileleaf->radius + RADIUS_SPACER,
			DEGREE_2_RADIAN(fileleaf->_menu_render->highest_angle_nearest),
			DEGREE_2_RADIAN(fileleaf->_menu_render->lowest_angle_nearest));

		// Render B
		cairo_arc_negative(
			cr,
			OFFSET_2_SCREEN(fileleaf->_menu_render->Bcircle_x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->_menu_render->Bcircle_y, private->view_y_offset),
			CLOSEST_TAB_CIRCLE_RADIUS,
			DEGREE_2_RADIAN(fileleaf->_menu_render->Bcircle_highest_angle),
			DEGREE_2_RADIAN(fileleaf->_menu_render->Bcircle_lowest_angle));

		// Render C
		cairo_arc(
			cr,
			OFFSET_2_SCREEN(fileleaf->_menu_render->Ccircle_x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->_menu_render->Ccircle_y, private->view_y_offset),
			FARTHEST_TAB_CIRCLE_RADIUS,
			DEGREE_2_RADIAN(fileleaf->_menu_render->Ccircle_lowest_angle),
			DEGREE_2_RADIAN(fileleaf->_menu_render->Ccircle_highest_angle));

		// Render E
		cairo_arc(
			cr,
			OFFSET_2_SCREEN(fileleaf->_menu_render->Ecircle_x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->_menu_render->Ecircle_y, private->view_y_offset),
			FARTHEST_TAB_CIRCLE_RADIUS,
			DEGREE_2_RADIAN(fileleaf->_menu_render->Ecircle_lowest_angle),
			DEGREE_2_RADIAN(fileleaf->_menu_render->Ecircle_highest_angle));

		// Render F
		cairo_arc_negative(
			cr,
			OFFSET_2_SCREEN(fileleaf->_menu_render->Fcircle_x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->_menu_render->Fcircle_y, private->view_y_offset),
			CLOSEST_TAB_CIRCLE_RADIUS,
			DEGREE_2_RADIAN(fileleaf->_menu_render->Fcircle_highest_angle),
			DEGREE_2_RADIAN(fileleaf->_menu_render->Fcircle_lowest_angle));

		cairo_close_path(cr);
		cairo_new_sub_path (cr);

		cairo_arc (
			cr,
			OFFSET_2_SCREEN(fileleaf->x, private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->y, private->view_y_offset),
			fileleaf->radius,
			DEGREE_2_RADIAN(0.0),
			DEGREE_2_RADIAN(360.0));

        /* Render to the cairo context. */
        cairo_set_line_width (cr, g_outer_inner_rgba._line_width);
		cairo_set_source_rgba (cr, g_outer_inner_rgba._r, g_outer_inner_rgba._g, g_outer_inner_rgba._b, g_outer_inner_rgba._a_fill);
		cairo_fill_preserve (cr);
        cairo_set_source_rgba (cr, g_outer_inner_rgba._r, g_outer_inner_rgba._g, g_outer_inner_rgba._b, g_outer_inner_rgba._a_pen);
		cairo_stroke (cr);

		/* Render the parent fileleaf association on the tab. */
        g_assert(fileleaf->_central_glyph->_associated_fileitem != NULL);

        gdk_cairo_set_source_pixbuf (
            cr,
            fileleaf->_central_glyph->_associated_fileitem->_pixbuf,
			OFFSET_2_SCREEN(fileleaf->_menu_render->tab_glyph_x - (icon_width / 2), private->view_x_offset),
			OFFSET_2_SCREEN(fileleaf->_menu_render->tab_glyph_y - (icon_height / 2), private->view_y_offset));
        cairo_paint_with_alpha(cr, 1.0);
    }

    /* Render the surround of the fileleaf central glyph. */
    cairo_arc (
		cr,
		OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
		OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
		fileleaf->_central_glyph->size + CIRCULAR_ICON_SPACER,
		0,
		DEGREE_2_RADIAN(360));

    /* Render to the cairo context. */
    cairo_set_line_width (cr, g_outer_inner_rgba._line_width);
	cairo_set_source_rgba (cr, g_outer_inner_rgba._r, g_outer_inner_rgba._g, g_outer_inner_rgba._b, g_outer_inner_rgba._a_fill);
	cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, g_outer_inner_rgba._r, g_outer_inner_rgba._g, g_outer_inner_rgba._b, g_outer_inner_rgba._a_pen);
	cairo_stroke (cr);

    /* Render the fileleaf central glyph. */
    {
        GdkPixbuf* pixbuf;

	    // Check whether the item is selected.
	    if (((GLYPH_ROOT_CENTRE == g_current_type) && (fileleaf == g_root_fileleaf)) ||
	        ((GLYPH_FILE_MENU_CENTRE == g_current_type) && (fileleaf == g_current_fileleaf)))
		{
		    pixbuf = gdk_pixbuf_new_from_inline (-1, close_menu_prelight, FALSE, NULL);
		}
	    else
		{
		    pixbuf = gdk_pixbuf_new_from_inline (-1, close_menu_normal, FALSE, NULL);
		}

        g_assert(pixbuf != NULL);

        gdk_cairo_set_source_pixbuf (
            cr,
            pixbuf,
		    OFFSET_2_SCREEN(fileleaf->_central_glyph->x- (CENTRE_ICONSIZE / 2), private->view_x_offset),
		    OFFSET_2_SCREEN(fileleaf->_central_glyph->y- (CENTRE_ICONSIZE / 2), private->view_y_offset));
        cairo_paint_with_alpha(cr, 1.0);

        g_object_unref(pixbuf);
    }

    /* Retieve the fileleafs associated sub fileitem. */
    if (fileleaf->_child_fileleaf != NULL)
        associated_fileitem = fileleaf->_child_fileleaf->_central_glyph->_associated_fileitem;
    else
        associated_fileitem = NULL;

    if (fileleaf->_fileitem_list_count == 1)
    {
		// Only a single fileitem.
        CaFileItem* fileitem;
        gdouble inner_radius;
        gdouble outer_radius;

        file_list = g_list_first(fileleaf->_fileitem_list);

        fileitem = (CaFileItem*)file_list->data;
        g_assert(fileitem != NULL);

        inner_radius = fileitem->_parent_radius - SEGMENT_INNER_SPACER;
        outer_radius = fileitem->_parent_radius + SEGMENT_OUTER_SPACER;

        cairo_arc(cr,
            OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
            OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
            outer_radius,
            DEGREE_2_RADIAN(0.0),
            DEGREE_2_RADIAN(360.0));

        cairo_arc(cr,
            OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
            OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
            inner_radius,
            DEGREE_2_RADIAN(0.0),
            DEGREE_2_RADIAN(360.0));

		// Check whether the item is selected.
		if (fileitem == g_current_fileitem)
		{
			/* Prelight. */

            /* Render to the cairo context. */
            cairo_set_line_width (cr, g_prelight_segment_rgba._line_width);
            cairo_set_source_rgba (cr, g_prelight_segment_rgba._r, g_prelight_segment_rgba._g, g_prelight_segment_rgba._b, g_prelight_segment_rgba._a_fill);
            cairo_fill_preserve (cr);
            cairo_set_source_rgba (cr, g_prelight_segment_rgba._r, g_prelight_segment_rgba._g, g_prelight_segment_rgba._b, g_prelight_segment_rgba._a_pen);
            cairo_stroke (cr);
	    }
		else
		{
			/* Prelight. */

            /* Render to the cairo context. */
            cairo_set_line_width (cr, g_normal_segment_rgba._line_width);
            cairo_set_source_rgba (cr, g_normal_segment_rgba._r, g_normal_segment_rgba._g, g_normal_segment_rgba._b, g_normal_segment_rgba._a_fill);
            cairo_fill_preserve (cr);
            cairo_set_source_rgba (cr, g_normal_segment_rgba._r, g_normal_segment_rgba._g, g_normal_segment_rgba._b, g_normal_segment_rgba._a_pen);
            cairo_stroke (cr);
		}
    }
    else
    {
		// multiple fileitems.

        /* Iterate the fileleaf fileitems. */
        file_list = g_list_first(fileleaf->_fileitem_list);

        // Render all the circular segments.
        while (file_list)
        {
            CaFileItem* fileitem;

            fileitem = (CaFileItem*)file_list->data;
            g_assert(fileitem != NULL);

            if (associated_fileitem != fileitem)	/* Do not render as it is a sub fileleaf. */
            {
				/*
				CDC
				BAB
				*/
				if (fileitem->_circular_angle_share == 360.0)
				{
					if (fileitem->_segment_render == NULL)
					{
						fileitem->_segment_render = g_new(CaSegmentRender, 1);

						if (GLYPH_FILE_MENU == fileitem->_type)
						{
							// Add an arrow.
							gdouble arrow_percentage;

							fileitem->_segment_render->outer_arrow_radius = (gint)((fileitem->_parent_radius + SEGMENT_OUTER_SPACER) - (SEGMENT_ARROW_HEIGHT / 2));

							_ca_get_point_from_source_offset(
								fileleaf->_central_glyph->x,
								fileleaf->_central_glyph->y,
								fileitem->_parent_angle,
								fileitem->_segment_render->outer_arrow_radius,
								&fileitem->_segment_render->arrow_point_x,
								&fileitem->_segment_render->arrow_point_y);

							fileitem->_segment_render->inner_arrow_radius = fileitem->_segment_render->outer_arrow_radius - (gint)SEGMENT_ARROW_HEIGHT;

							// Smaller the circumference then larger the angle.
							arrow_percentage = SEGMENT_ARROW_WIDTH / _ca_circular_applications_menu_circumference_from_radius(fileitem->_segment_render->inner_arrow_radius);
							fileitem->_segment_render->arrow_angle = (360.0 * arrow_percentage);

							_ca_get_point_from_source_offset(
								fileleaf->_central_glyph->x,
								fileleaf->_central_glyph->y,
								_ca_circular_applications_menu_calculate_angle_offset(fileitem->_parent_angle, (fileitem->_segment_render->arrow_angle / 2)),
								fileitem->_segment_render->inner_arrow_radius,
								&fileitem->_segment_render->arrow_side_x,
								&fileitem->_segment_render->arrow_side_y);
						}
					}

					// Render a single segment.
					cairo_arc(cr,
						OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
						OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
						fileitem->_parent_radius + SEGMENT_OUTER_SPACER,
						DEGREE_2_RADIAN(0.0),
						DEGREE_2_RADIAN(360.0));

					cairo_arc(cr,
						OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
						OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
						fileitem->_parent_radius - SEGMENT_INNER_SPACER,
						DEGREE_2_RADIAN(0.0),
						DEGREE_2_RADIAN(360.0));

					if (GLYPH_FILE_MENU == fileitem->_type)
					{
						// Add an arrow.
						cairo_new_sub_path (cr);

						cairo_move_to(
							cr,
							OFFSET_2_SCREEN(fileitem->_segment_render->arrow_point_x, private->view_x_offset),
							OFFSET_2_SCREEN(fileitem->_segment_render->arrow_point_y, private->view_y_offset));

						cairo_line_to(
							cr,
							OFFSET_2_SCREEN(fileitem->_segment_render->arrow_side_x, private->view_x_offset),
							OFFSET_2_SCREEN(fileitem->_segment_render->arrow_side_y, private->view_y_offset));

						cairo_arc_negative(cr,
							OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
							OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
							fileitem->_segment_render->inner_arrow_radius,
							DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_parent_angle, (fileitem->_segment_render->arrow_angle / 2))),
							DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_parent_angle, -(fileitem->_segment_render->arrow_angle / 2))));

						cairo_close_path(cr);
					}

					// Check whether the item is selected.
					if (fileitem == g_current_fileitem)
					{
						/* Prelight. */

                        /* Render to the cairo context. */
                        cairo_set_line_width (cr, g_prelight_segment_rgba._line_width);
		                cairo_set_source_rgba (cr, g_prelight_segment_rgba._r, g_prelight_segment_rgba._g, g_prelight_segment_rgba._b, g_prelight_segment_rgba._a_fill);
		                cairo_fill_preserve (cr);
                        cairo_set_source_rgba (cr, g_prelight_segment_rgba._r, g_prelight_segment_rgba._g, g_prelight_segment_rgba._b, g_prelight_segment_rgba._a_pen);
                        cairo_stroke (cr);
				    }
					else
					{
    					/* Prelight. */

                        /* Render to the cairo context. */
                        cairo_set_line_width (cr, g_normal_segment_rgba._line_width);
		                cairo_set_source_rgba (cr, g_normal_segment_rgba._r, g_normal_segment_rgba._g, g_normal_segment_rgba._b, g_normal_segment_rgba._a_fill);
		                cairo_fill_preserve (cr);
                        cairo_set_source_rgba (cr, g_normal_segment_rgba._r, g_normal_segment_rgba._g, g_normal_segment_rgba._b, g_normal_segment_rgba._a_pen);
                        cairo_stroke (cr);
					}
				}
				else
				{
					// Render multiple segments.

					if (fileitem->_segment_render == NULL)
					{
						fileitem->_segment_render = g_new(CaSegmentRender, 1);

						// Smaller the circumference then larger the angle.
						fileitem->_segment_render->Aradius = (gint)(fileitem->_parent_radius - SEGMENT_INNER_SPACER);
						fileitem->_segment_render->Bradius = (gint)(fileitem->_segment_render->Aradius + SEGMENT_CIRCLE_RADIUS);
						fileitem->_segment_render->Dradius = (gint)(fileitem->_parent_radius + SEGMENT_OUTER_SPACER);
						fileitem->_segment_render->Cradius = (gint)(fileitem->_segment_render->Dradius - SEGMENT_CIRCLE_RADIUS);

						_ca_circular_applications_get_segment_angles(fileitem, fileitem->_segment_render->Aradius, &fileitem->_segment_render->Afrom_angle, &fileitem->_segment_render->Ato_angle);
						_ca_circular_applications_get_segment_angles(fileitem, fileitem->_segment_render->Bradius, &fileitem->_segment_render->Bfrom_angle, &fileitem->_segment_render->Bto_angle);
						_ca_circular_applications_get_segment_angles(fileitem, fileitem->_segment_render->Cradius, &fileitem->_segment_render->Cfrom_angle, &fileitem->_segment_render->Cto_angle);
						_ca_circular_applications_get_segment_angles(fileitem, fileitem->_segment_render->Dradius, &fileitem->_segment_render->Dfrom_angle, &fileitem->_segment_render->Dto_angle);

						// Calculate B lowest
						_ca_get_point_from_source_offset(
							fileleaf->_central_glyph->x,
							fileleaf->_central_glyph->y,
							fileitem->_segment_render->Bfrom_angle,
							fileitem->_segment_render->Bradius,
							&fileitem->_segment_render->B_lowest_circle_x,
							&fileitem->_segment_render->B_lowest_circle_y);

						// Calculate C lowest
						_ca_get_point_from_source_offset(
							fileleaf->_central_glyph->x,
							fileleaf->_central_glyph->y,
							fileitem->_segment_render->Cfrom_angle,
							fileitem->_segment_render->Cradius,
							&fileitem->_segment_render->C_lowest_circle_x,
							&fileitem->_segment_render->C_lowest_circle_y);

						// Calculate C highest
						_ca_get_point_from_source_offset(
							fileleaf->_central_glyph->x,
							fileleaf->_central_glyph->y,
							fileitem->_segment_render->Cto_angle,
							fileitem->_segment_render->Cradius,
							&fileitem->_segment_render->C_highest_circle_x,
							&fileitem->_segment_render->C_highest_circle_y);

						// Calculate B highest
						_ca_get_point_from_source_offset(
							fileleaf->_central_glyph->x,
							fileleaf->_central_glyph->y,
							fileitem->_segment_render->Bto_angle,
							fileitem->_segment_render->Bradius,
							&fileitem->_segment_render->B_highest_circle_x,
							&fileitem->_segment_render->B_highest_circle_y);

						if (GLYPH_FILE_MENU == fileitem->_type)
						{
							// Add an arrow.
							gdouble arrow_percentage;

							fileitem->_segment_render->outer_arrow_radius = (gint)((fileitem->_parent_radius + SEGMENT_OUTER_SPACER) - (SEGMENT_ARROW_HEIGHT / 2));

							_ca_get_point_from_source_offset(
								fileleaf->_central_glyph->x,
								fileleaf->_central_glyph->y,
								fileitem->_parent_angle,
								fileitem->_segment_render->outer_arrow_radius,
								&fileitem->_segment_render->arrow_point_x,
								&fileitem->_segment_render->arrow_point_y);

							fileitem->_segment_render->inner_arrow_radius = fileitem->_segment_render->outer_arrow_radius - (gint)SEGMENT_ARROW_HEIGHT;

							// Smaller the circumference then larger the angle.
							arrow_percentage = SEGMENT_ARROW_WIDTH / _ca_circular_applications_menu_circumference_from_radius(fileitem->_segment_render->inner_arrow_radius);
							fileitem->_segment_render->arrow_angle = (360.0 * arrow_percentage);

							_ca_get_point_from_source_offset(
								fileleaf->_central_glyph->x,
								fileleaf->_central_glyph->y,
								_ca_circular_applications_menu_calculate_angle_offset(fileitem->_parent_angle, (fileitem->_segment_render->arrow_angle / 2)),
								fileitem->_segment_render->inner_arrow_radius,
								&fileitem->_segment_render->arrow_side_x,
								&fileitem->_segment_render->arrow_side_y);
						}
					}

					// Render B lowest
					cairo_arc(cr,
						OFFSET_2_SCREEN(fileitem->_segment_render->B_lowest_circle_x, private->view_x_offset),
						OFFSET_2_SCREEN(fileitem->_segment_render->B_lowest_circle_y, private->view_y_offset),
						SEGMENT_CIRCLE_RADIUS,
						DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_segment_render->Bfrom_angle, -180.0)),
						DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_segment_render->Bfrom_angle, -90.0)));

					// Render C lowest
					cairo_arc(cr,
						OFFSET_2_SCREEN(fileitem->_segment_render->C_lowest_circle_x, private->view_x_offset),
						OFFSET_2_SCREEN(fileitem->_segment_render->C_lowest_circle_y, private->view_y_offset),
						SEGMENT_CIRCLE_RADIUS,
						DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_segment_render->Cfrom_angle, -90.0)),
						DEGREE_2_RADIAN(fileitem->_segment_render->Cfrom_angle));

					// Render D
					cairo_arc(cr,
						OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
						OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
						fileitem->_segment_render->Dradius,
						DEGREE_2_RADIAN(fileitem->_segment_render->Dfrom_angle),
						DEGREE_2_RADIAN(fileitem->_segment_render->Dto_angle));

					// Render C highest
					cairo_arc(cr,
						OFFSET_2_SCREEN(fileitem->_segment_render->C_highest_circle_x, private->view_x_offset),
						OFFSET_2_SCREEN(fileitem->_segment_render->C_highest_circle_y, private->view_y_offset),
						SEGMENT_CIRCLE_RADIUS,
						DEGREE_2_RADIAN(fileitem->_segment_render->Cto_angle),
						DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_segment_render->Cto_angle, 90.0)));

					// Render B highest
					cairo_arc(cr,
						OFFSET_2_SCREEN(fileitem->_segment_render->B_highest_circle_x, private->view_x_offset),
						OFFSET_2_SCREEN(fileitem->_segment_render->B_highest_circle_y, private->view_y_offset),
						SEGMENT_CIRCLE_RADIUS,
						DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_segment_render->Bto_angle, 90.0)),
						DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_segment_render->Bto_angle, 180.0)));

					// Render A
					cairo_arc_negative(cr,
						OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
						OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
						fileitem->_segment_render->Aradius,
						DEGREE_2_RADIAN(fileitem->_segment_render->Ato_angle),
						DEGREE_2_RADIAN(fileitem->_segment_render->Afrom_angle));

					if (GLYPH_FILE_MENU == fileitem->_type)
					{
						// Add an arrow.
						cairo_new_sub_path (cr);

						cairo_move_to(
							cr,
							OFFSET_2_SCREEN(fileitem->_segment_render->arrow_point_x, private->view_x_offset),
							OFFSET_2_SCREEN(fileitem->_segment_render->arrow_point_y, private->view_y_offset));

						cairo_line_to(
							cr,
							OFFSET_2_SCREEN(fileitem->_segment_render->arrow_side_x, private->view_x_offset),
							OFFSET_2_SCREEN(fileitem->_segment_render->arrow_side_y, private->view_y_offset));

						cairo_arc_negative(cr,
							OFFSET_2_SCREEN(fileleaf->_central_glyph->x, private->view_x_offset),
							OFFSET_2_SCREEN(fileleaf->_central_glyph->y, private->view_y_offset),
							fileitem->_segment_render->inner_arrow_radius,
							DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_parent_angle, (fileitem->_segment_render->arrow_angle / 2))),
							DEGREE_2_RADIAN(_ca_circular_applications_menu_calculate_angle_offset(fileitem->_parent_angle, -(fileitem->_segment_render->arrow_angle / 2))));

                        cairo_close_path(cr);
					}

					// Check whether the item is selected.
					if (fileitem == g_current_fileitem)
					{
						/* Prelight. */

                        /* Render to the cairo context. */
                        cairo_set_line_width (cr, g_prelight_segment_rgba._line_width);
		                cairo_set_source_rgba (cr, g_prelight_segment_rgba._r, g_prelight_segment_rgba._g, g_prelight_segment_rgba._b, g_prelight_segment_rgba._a_fill);
		                cairo_fill_preserve (cr);
                        cairo_set_source_rgba (cr, g_prelight_segment_rgba._r, g_prelight_segment_rgba._g, g_prelight_segment_rgba._b, g_prelight_segment_rgba._a_pen);
                        cairo_stroke (cr);
				    }
					else
					{
    					/* Prelight. */

                        /* Render to the cairo context. */
                        cairo_set_line_width (cr, g_normal_segment_rgba._line_width);
		                cairo_set_source_rgba (cr, g_normal_segment_rgba._r, g_normal_segment_rgba._g, g_normal_segment_rgba._b, g_normal_segment_rgba._a_fill);
		                cairo_fill_preserve (cr);
                        cairo_set_source_rgba (cr, g_normal_segment_rgba._r, g_normal_segment_rgba._g, g_normal_segment_rgba._b, g_normal_segment_rgba._a_pen);
                        cairo_stroke (cr);
					}
				}
            }

            file_list = g_list_next(file_list);
        }
    }

    file_list = g_list_first(fileleaf->_fileitem_list);

    // Render all the fileitems.
    while (file_list)
    {
        CaFileItem* fileitem;

        fileitem = (CaFileItem*)file_list->data;
        g_assert(fileitem != NULL);

        if (associated_fileitem != fileitem)    /* Do not render as it is a sub fileleaf. */
        {
            /* Render the fileleaf fileitems. */
            g_assert(fileitem->_pixbuf != NULL);

            gdk_cairo_set_source_pixbuf (
                cr,
                fileitem->_pixbuf,
			    OFFSET_2_SCREEN(fileitem->x - (icon_width / 2), private->view_x_offset),
			    OFFSET_2_SCREEN(fileitem->y - (icon_height / 2), private->view_y_offset));
            cairo_paint_with_alpha(cr, 1.0);
        }

        file_list = g_list_next(file_list);
    }
}

static void
_ca_circular_applications_menu_render_centred_text(CaCircularApplicationsMenu* circular_applications_menu, gint y, const gchar* text, cairo_t* cr)
{
	CaCircularApplicationsMenuPrivate* private;
	gint x;
	gint radius;
    cairo_text_extents_t text_extents;
    cairo_text_extents_t complete_text_extents;
	cairo_font_extents_t font_extents;

	private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(circular_applications_menu);

    if ((text == NULL) ||
        (strlen(text) == 0))
        return;

    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 15.0);
	cairo_font_extents(cr, &font_extents);

    cairo_text_extents (cr, text, &text_extents);
    cairo_text_extents (cr, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", &complete_text_extents);

	/* Calculate the text position. */
	x = (private->view_width - (gint)text_extents.width) / 2;
	radius = (gint)(TEXT_BOUNDARY + font_extents.height + TEXT_BOUNDARY) / 2;

	// Render the text surround.
	cairo_arc(
		cr,
		x - TEXT_BOUNDARY,
		y - (complete_text_extents.height / 2),
		radius,
		DEGREE_2_RADIAN(90.0),
		DEGREE_2_RADIAN(270.0));

	cairo_arc(
		cr,
		x + text_extents.width + TEXT_BOUNDARY,
		y - (complete_text_extents.height / 2),
		radius,
		DEGREE_2_RADIAN(270.0),
		DEGREE_2_RADIAN(90.0));

	cairo_set_source_rgba (cr, g_text_box_rgba._r, g_text_box_rgba._g, g_text_box_rgba._b, g_text_box_rgba._a_fill);
	cairo_fill_preserve (cr);
	cairo_set_source_rgba (cr, g_text_box_rgba._r, g_text_box_rgba._g, g_text_box_rgba._b, g_text_box_rgba._a_pen);
	cairo_set_line_width (cr, g_text_box_rgba._line_width);
	cairo_stroke (cr);

	// Render the text.
    cairo_move_to (cr, x, y - (font_extents.descent / 2));
    cairo_text_path (cr, text);
	cairo_set_source_rgba (cr, g_text_rgba._r, g_text_rgba._g, g_text_rgba._b, g_text_rgba._a_fill);
    cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, g_text_rgba._r, g_text_rgba._g, g_text_rgba._b, g_text_rgba._a_pen);
    cairo_set_line_width (cr, g_text_rgba._line_width);
    cairo_stroke (cr);
}

/*
 Retrieves the current point from a given point
*/
static void
_ca_get_point_from_source_offset(gint source_x, gint source_y, gdouble angle, gdouble radius, gint* destination_x, gint* destination_y)
{
    gdouble degree_in_radius;

    *destination_x = source_x;
    *destination_y = source_y;

    if (radius == 0.0)
        return; /* No distance away from source. */

    g_assert(angle >= 0.0);
    g_assert(angle <= 359.0);

    degree_in_radius = angle * (M_PI / 180);

    *destination_x = (gint)(source_x + (radius * cos(degree_in_radius)));
    *destination_y = (gint)(source_y + (radius * sin(degree_in_radius)));
}

static gdouble
_ca_circular_applications_menu_point_distance(gint x1, gint y1, gint x2, gint y2)
{
    gdouble distance;   /* Distance in radians. */
    gint xd;
    gint yd;

	xd = x2 - x1;
	yd = y2 - y1;
	distance = sqrt(xd*xd + yd*yd);

    return distance;
}

static gboolean
_ca_circular_applications_menu_circle_contains_point(gint point_x, gint point_y, gint circle_x, gint circle_y, gint radius)
{
    gdouble distance;

    distance = _ca_circular_applications_menu_point_distance(point_x, point_y, circle_x, circle_y);

    return (distance <= radius);
}

static gboolean
_ca_circular_applications_menu_segment_contains_point(gint point_x, gint point_y, CaFileItem* fileitem)
{
	CaFileLeaf* fileleaf;
	gdouble current_distance;
	gboolean betweenAngle;

	betweenAngle = FALSE;

	fileleaf = fileitem->_assigned_fileleaf;

	current_distance = _ca_circular_applications_menu_point_distance(
		fileleaf->_central_glyph->x,
		fileleaf->_central_glyph->y,
		point_x,
		point_y);

	// Check for the correct ring.
	if ((current_distance > (fileitem->_parent_radius - SEGMENT_INNER_SPACER)) &&
		(current_distance < (fileitem->_parent_radius + SEGMENT_OUTER_SPACER)))
	{
		if (fileitem->_circular_angle_share == 360.0)
		{
			// Only one fileitem.
			betweenAngle = TRUE;
		}
		else
		{
			// Check for the correct segment.
			gdouble half_circular_angle_share;
			gdouble from_angle;
			gdouble to_angle;
			gdouble current_angle;

			half_circular_angle_share = fileitem->_circular_angle_share / 2;
			from_angle = _ca_circular_applications_menu_calculate_angle_offset(fileitem->_parent_angle, -half_circular_angle_share);
			to_angle = _ca_circular_applications_menu_calculate_angle_offset(fileitem->_parent_angle, half_circular_angle_share);

			current_angle = _ca_circular_applications_menu_angle_between_points(fileitem->_assigned_fileleaf->x, fileitem->_assigned_fileleaf->y, point_x, point_y);
			betweenAngle = _ca_circular_applications_menu_is_angle_between_angles(current_angle, from_angle, to_angle);
		}
	}

	return betweenAngle;
}

static gdouble
_ca_circular_applications_menu_circumference_from_radius(gdouble radius)
{
    // TODO: Not used?
    gdouble circumference;

    circumference = (radius + radius) * M_PI;

    return circumference;
}

static gdouble
_ca_circular_applications_menu_calculate_radius(CaFileLeaf* fileleaf)
{
    gint fileitems_total;
    gdouble current_radius;

    fileitems_total = fileleaf->_fileitem_list_count;

    current_radius = INITIAL_RADIUS;

    while (fileitems_total > 0)
    {
        gint current_circumference;
        gint fileitems_ring_count;

        // Start a new ring.
        current_circumference = (gint)_ca_circular_applications_menu_circumference_from_radius(current_radius);
        fileitems_ring_count = current_circumference / (gint)MIN_RADIUS_ICONAREA;
        fileitems_ring_count = MIN(fileitems_total, fileitems_ring_count);

        fileitems_total -= fileitems_ring_count;

        if (fileitems_total > 0)
        {
            // Increment to the next circular ring.
            // (do not (NORMAL_ICONSIZE / 2) as two halves are incremented.
            current_radius += (SEGMENT_OUTER_SPACER + CIRCULAR_SEPERATOR + SEGMENT_INNER_SPACER);
        }
        else
        {
            // No more circular rings to process.
            current_radius += (SEGMENT_OUTER_SPACER + CIRCULAR_SEPERATOR);
        }
    }

    return current_radius;
}

static void
_ca_circular_applications_menu_position_fileleaf_files(CaFileLeaf* fileleaf, gdouble calculated_radius, gdouble angle)
{
    // TODO: Order the fileitems so the folders appear on the outside?
    GList* list;
    gint fileitems_total;
    gint current_radius;

    /* Assign the fileleaf size. */
    if (fileleaf->_type == ROOT_LEAF)
    {
        // Assign the fileleaf size.
        fileleaf->x = CA_VIEW_X_OFFSET_START;
        fileleaf->y = CA_VIEW_Y_OFFSET_START;
    }
    else
    {
        // Assign the sub fileleaf.
        gint parent_distance;

        g_assert(fileleaf->_parent_fileleaf != NULL);

        parent_distance = (gint)(fileleaf->_parent_fileleaf->radius + RADIUS_SPACER + SPOKE_LENGTH + calculated_radius);

        _ca_get_point_from_source_offset(
            fileleaf->_parent_fileleaf->x,
            fileleaf->_parent_fileleaf->y,
            angle,
            parent_distance,
            &fileleaf->x,
            &fileleaf->y);
    }

    fileitems_total = fileleaf->_fileitem_list_count;

    /* Assign the fileitems size. */
    list = g_list_first(fileleaf->_fileitem_list);

    current_radius = (gint)(INITIAL_RADIUS);

    while (fileitems_total > 0)
    {
        gint current_circumference;
        gint fileitems_ring_count;
        gdouble fileitem_angle;
        gdouble current_fileitem_angle;

        // Start a new ring.
        current_circumference = (gint)((current_radius * 2) * M_PI);
        fileitems_ring_count = current_circumference / (gint)MIN_RADIUS_ICONAREA;
        fileitems_ring_count = MIN(fileitems_total, fileitems_ring_count);

        fileitem_angle = 360.0 / fileitems_ring_count;
        current_fileitem_angle = 0.0;

        fileitems_total -= fileitems_ring_count;

        // Process all fileitems within the ring.
        while (fileitems_ring_count > 0)
        {
            CaFileItem* fileitem;

            fileitem = (CaFileItem*)list->data;
            g_assert(fileitem != NULL);

            _ca_get_point_from_source_offset(fileleaf->x, fileleaf->y, current_fileitem_angle, current_radius, &fileitem->x, &fileitem->y);

            fileitem->size = (gint)NORMAL_ICONSIZE;
            fileitem->_parent_angle = (gint)current_fileitem_angle;
            fileitem->_parent_radius = (gint)current_radius;
            fileitem->_circular_angle_share = fileitem_angle;

            current_fileitem_angle += fileitem_angle;

            fileitems_ring_count--;

            // Move onto the next fileitem.
            list = g_list_next(list);
        }

        if (fileitems_total > 0)
        {
            // Increment to the next circular ring.
            // (do not (NORMAL_ICONSIZE / 2) as two halves are incremented.
            current_radius += (gint)(SEGMENT_OUTER_SPACER + CIRCULAR_SEPERATOR + SEGMENT_INNER_SPACER);
        }
        else
        {
            // No more circular rings to process.
            current_radius += (gint)(SEGMENT_OUTER_SPACER + CIRCULAR_SEPERATOR);
        }
    }

    /* Assign the fileleafs central glyph size. */
    fileleaf->_central_glyph->x = fileleaf->x;
    fileleaf->_central_glyph->y = fileleaf->y;
    fileleaf->_central_glyph->size = (gint)CENTRE_ICONSIZE / 2;
    fileleaf->radius = current_radius; /* outer */
}

static CaFileLeaf*
_ca_circular_applications_menu_show_fileitem(GMenuTreeDirectory* menutreedirectory, LeafType leaftype, CaFileItem* fileitem, gboolean disassociated)
{
    CaFileLeaf* fileleaf;

    fileleaf = ca_circular_applications_menu_show_leaf(
        menutreedirectory,
        leaftype,
        fileitem,
        disassociated);

    g_assert(fileleaf != NULL);
    g_assert(fileleaf->_central_glyph != NULL);

    return fileleaf;
}

CaFileLeaf*
ca_circular_applications_menu_show_leaf(GMenuTreeDirectory* menutreedirectory, LeafType leaftype, CaFileItem* fileitem, gboolean disassociated)
{
    /* TODO: make this private. */
    GSList* items;
    GSList* tmp;
    CaFileLeaf* fileleaf;
    GList* current_list;
    CaFileLeaf* parent_fileleaf;
    gdouble angle;

    if (menutreedirectory == NULL)
        return NULL;

    if (fileitem != NULL)
    {
        parent_fileleaf = fileitem->_assigned_fileleaf;
        angle = fileitem->_parent_angle;
    }
    else
    {
        /* Default values for the root fileleaf. */

        g_assert(leaftype == ROOT_LEAF);
        parent_fileleaf = NULL,
        angle = 0.0;
    }

    /* Create a new fileleaf. */
    fileleaf = g_new(CaFileLeaf, 1);
    fileleaf->_type = leaftype;
    fileleaf->_parent_fileleaf = parent_fileleaf;
    fileleaf->_child_fileleaf = NULL;
    fileleaf->_current_animation_state = INITIAL_OPEN;
    fileleaf->_current_animation_tick = 0;
    fileleaf->_fileitem_list = NULL;
    fileleaf->_fileitem_list_count = 0;
    fileleaf->_sub_fileleaves_list = NULL;
    fileleaf->_sub_fileleaves_list_count = 0;
	fileleaf->_menu_render = NULL;

    /* Create the fileleafs central glyph. */
    fileleaf->_central_glyph = g_new(CaFileItem, 1);
    fileleaf->_central_glyph->_icon = NULL;
    fileleaf->_central_glyph->_pixbuf = NULL;
    fileleaf->_central_glyph->_associated_fileitem = disassociated ? NULL : fileitem;   /* Keep a reference to the associated fileitems. */
    fileleaf->_central_glyph->_assigned_fileleaf = fileleaf;
    fileleaf->_central_glyph->_parent_angle = 0;
    fileleaf->_central_glyph->_circular_angle_share = 0.0;
    fileleaf->_central_glyph->_parent_radius = 0;
	fileleaf->_central_glyph->_segment_render = NULL;

    if (disassociated)
    {
        /* Disassociated fileleaf. */

        fileleaf->_central_glyph->_type = GLYPH_FILE_MENU_CENTRE;
    }
    else if (parent_fileleaf != NULL)
    {
        /* Sub fileleaf. */

        parent_fileleaf->_child_fileleaf = fileleaf;

        fileleaf->_central_glyph->_type = GLYPH_FILE_MENU_CENTRE;
        fileleaf->_central_glyph->_parent_angle = _ca_circular_applications_menu_calculate_angle_offset(angle, 180.0);
    }
    else
    {
        /* Root fileleaf. */

        fileleaf->_central_glyph->_type = GLYPH_ROOT_CENTRE;
    }

    current_list = NULL;

    /* Iterate all files. */
    items = gmenu_tree_directory_get_contents(menutreedirectory);
    tmp = items;
    /* Iterate all files in the source directory. */
    while (tmp != NULL)
    {
        GMenuTreeItem* current_item;
        GMenuTreeItem* aliased_item; // wool change to resolved_item
        GMenuTreeItemType itemtype;

        current_item = GMENU_TREE_ITEM(tmp->data);
        g_assert(current_item != NULL);

        itemtype = gmenu_tree_item_get_type(current_item);

        aliased_item = NULL;

        if (itemtype == GMENU_TREE_ITEM_ALIAS)
        {
            aliased_item = gmenu_tree_alias_get_item (GMENU_TREE_ALIAS (current_item));
            itemtype = gmenu_tree_item_get_type (aliased_item);

            if (itemtype != GMENU_TREE_ITEM_ENTRY)
            {
                aliased_item = NULL;
            }
        }
        else
        {
            aliased_item = current_item;
        }

        if (aliased_item == NULL)
        {
            gmenu_tree_item_unref (current_item);
        }
        else if ((itemtype == GMENU_TREE_ITEM_ENTRY) ||
            (itemtype == GMENU_TREE_ITEM_DIRECTORY))
        {
            CaFileItem* current_fileitem;

            /* Create a new fileitem. */
            current_fileitem = g_new(CaFileItem, 1);
            current_fileitem->_type = (itemtype == GMENU_TREE_ITEM_DIRECTORY) ? GLYPH_FILE_MENU : GLYPH_FILE;
            current_fileitem->_menutreeitem = aliased_item;
            current_fileitem->_icon = gmenu_tree_entry_get_icon(GMENU_TREE_ENTRY(current_fileitem->_menutreeitem));
            current_fileitem->_pixbuf = _ca_circular_applications_icon_loader_get_icon_spec(current_fileitem->_icon, icon_width, icon_height);
            current_fileitem->_assigned_fileleaf = fileleaf;
            current_fileitem->_parent_angle = 0;
            current_fileitem->_circular_angle_share = 0.0;
            current_fileitem->_parent_radius = 0;
            current_fileitem->_associated_fileitem = NULL;
		    current_fileitem->_segment_render = NULL;

            current_list = g_list_prepend(current_list, (gpointer)current_fileitem);
            fileleaf->_fileitem_list_count++;

            if (fileleaf->_fileitem_list == NULL)
                fileleaf->_fileitem_list = current_list;
        }

        tmp = tmp->next;
    }

    g_slist_free (items);

    /* Keep the refefence to the root fileleaf. */
    if (g_root_fileleaf == NULL)
    {
        g_root_fileleaf = fileleaf;
        g_last_opened_fileleaf = fileleaf;
        g_tabbed_fileleaf = fileleaf;
    }

    {
        gdouble radius;

        radius = _ca_circular_applications_menu_calculate_radius(fileleaf);

        _ca_circular_applications_menu_position_fileleaf_files(fileleaf, radius, angle);
    }

    return fileleaf;
}

void
ca_circular_applications_menu_close_fileleaf(CaFileLeaf* fileleaf)
{
    /* Clear the refefence to the root fileleaf. */
    if (g_root_fileleaf == fileleaf)
    {
        g_root_fileleaf = NULL;
        g_last_opened_fileleaf = NULL;
        g_tabbed_fileleaf = NULL;
    }

    /* Check whether their are child fileleafs open. */
    if (fileleaf->_child_fileleaf != NULL)
	{
        ca_circular_applications_menu_close_fileleaf(fileleaf->_child_fileleaf);
	}

    g_free((gpointer)fileleaf->_central_glyph);

    if (fileleaf->_fileitem_list != NULL)
    {
        GList* list;

        /* Iterate the fileitems. */
        list = g_list_last(fileleaf->_fileitem_list);

        while (list)
        {
            CaFileItem* fileitem;

            fileitem = (CaFileItem*)list->data;
            g_assert(fileitem != NULL);
            gmenu_tree_item_unref (fileitem->_menutreeitem);
            fileitem->_menutreeitem = NULL;
			g_free((gpointer)fileitem->_segment_render);
			g_object_unref(fileitem->_pixbuf);
            g_free((gpointer)fileitem);

            list = g_list_previous(list);
        }

        g_list_free(fileleaf->_fileitem_list);
        fileleaf->_fileitem_list = NULL;
    }

    /* Check whether this is a disassociated fileleaf. */
    if (fileleaf != g_disassociated_fileleaf)
    {
        if (fileleaf->_parent_fileleaf != NULL)
        {
            g_assert(fileleaf->_parent_fileleaf->_child_fileleaf == fileleaf);
            fileleaf->_parent_fileleaf->_child_fileleaf = NULL;
            fileleaf->_parent_fileleaf = NULL;
        }
    }

	g_free((gpointer)fileleaf->_menu_render);
    g_free((gpointer)fileleaf);
}

static void
_ca_circular_applications_menu_view_centre_fileleaf(CaCircularApplicationsMenu* circular_applications_menu, CaFileLeaf* fileleaf, gint x, gint y)
{
    CaCircularApplicationsMenuPrivate* private;

    private = CA_CIRCULAR_APPLICATIONS_MENU_GET_PRIVATE(circular_applications_menu);

    /* Update the view offset. */
    private->view_x_offset = ((x == -1) ? fileleaf->x : x) - (private->view_width / 2);
    private->view_y_offset = ((y == -1) ? fileleaf->y : y) - (private->view_height / 2);
}

static gdouble _ca_circular_applications_menu_angle_between_points(gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
    gdouble radians;
    gdouble degrees;

    radians = atan2((y2 - y1), (x2 - x1));

    degrees = RADIAN_2_DEGREE(radians);

	// For an angle of 270 atan2 give -90.
	if (degrees < 0.0)
	{
		degrees = _ca_circular_applications_menu_calculate_angle_offset(0.0, degrees);
	}

    return degrees;
}

static gboolean _ca_circular_applications_menu_is_angle_between_angles(gdouble angle, gdouble angleLower, gdouble angleHigher)
{
    gboolean betweenAngles = FALSE;

    if (angleHigher < angleLower)
    {
        // spans 360 boundary.
        if ((angle >= angleLower) || (angle <= angleHigher))
        {
            betweenAngles = TRUE;
        }
    }
    else
    {
        if ((angle >= angleLower) && (angle <= angleHigher))
        {
            betweenAngles = TRUE;
        }
    }

    return betweenAngles;
}

static gdouble
_ca_circular_applications_menu_calculate_angle_offset(gdouble angle, gdouble offset)
{
    angle += offset;

    if (angle > 259)
        angle -= 360;

    if (angle < 0)
        angle = 360 + angle;

    return angle;
}

static const gchar*
_ca_circular_applications_imagefinder_path(const gchar *path)
{
    gchar *temp;
    FILE *fp;

    fp = fopen(path, "r");

    if (fp)
    {
        fclose(fp);

        return path;
    }
    else
    {
		gint i = 0;

        while (TRUE)
        {
            if (image_paths[i] != NULL)
            {
                temp = g_strdup_printf("%s/%s", image_paths[i], path);

                fp = fopen(temp, "r");

                if (fp)
                {
                    fclose(fp);

                    return temp;
                }
            }
            else
            {
                return NULL;
            }

            i++;
        }
    }
}

static GdkPixbuf*
_ca_circular_applications_icon_loader_get_icon_spec(const char *name, int width, int height)
{
	GdkScreen* screen;
	GtkIconTheme* theme;
	GdkPixbuf* icon;
	GtkIconInfo* icon_info;

	screen = gdk_screen_get_default();
	theme = gtk_icon_theme_get_for_screen (screen);
	icon = NULL;

	if (!name)
	{
		return NULL;
	}

	icon_info = gtk_icon_theme_lookup_icon (gtk_icon_theme_get_default (), name, width, GTK_ICON_LOOKUP_NO_SVG);

	if (icon_info != NULL)
	{
		icon = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename (icon_info), width, -1, NULL);

		gtk_icon_info_free(icon_info);
	}

	/* first we try gtkicontheme */
	if (icon == NULL)
	{
		icon = gtk_icon_theme_load_icon(theme, name, width, GTK_ICON_LOOKUP_FORCE_SVG, NULL);
	}

	if (icon == NULL)
	{
	    /* Check all known paths. */
	    const gchar* found_path;

    	found_path = _ca_circular_applications_imagefinder_path(name);

        if (NULL != found_path)
        {
    		icon = gdk_pixbuf_new_from_file_at_scale(found_path, width, height, TRUE, NULL);

		    g_free((gpointer)found_path);
        }
	}

	if (icon == NULL)
	{
        icon = gtk_icon_theme_load_icon (
            gtk_icon_theme_get_default (),
            GTK_STOCK_FILE,
            width,
            GTK_ICON_LOOKUP_NO_SVG,
            NULL);
    }

    return icon;
}

