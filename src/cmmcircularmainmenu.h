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
#ifndef __CA_CIRCULAR_APPLICATIONS_MENU_H__
#define __CA_CIRCULAR_APPLICATIONS_MENU_H__

#include <gtk/gtk.h>

#define GMENU_I_KNOW_THIS_IS_UNSTABLE
#include <gnome-menus/gmenu-tree.h>

G_BEGIN_DECLS

#define CA_TYPE_CIRCULAR_APPLICATIONS_MENU          (ca_circular_applications_menu_get_type ())
#define CA_CIRCULAR_APPLICATIONS_MENU(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), CA_TYPE_CIRCULAR_APPLICATIONS_MENU, CaCircularApplicationsMenu))
#define CA_CIRCULAR_APPLICATIONS_MENU_CLASS(obj)    (G_TYPE_CHECK_CLASS_CAST ((obj), CA_CIRCULAR_APPLICATIONS_MENU, CaCircularApplicationsMenuClass))
#define CA_IS_CIRCULAR_APPLICATIONS_MENU(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CA_TYPE_CIRCULAR_APPLICATIONS_MENU))
#define CA_IS_CIRCULAR_APPLICATIONS_MENU_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), EFF_TYPE_CIRCULAR_APPLICATIONS_MENU))
#define CA_CIRCULAR_APPLICATIONS_MENU_GET_CLASS	    (G_TYPE_INSTANCE_GET_CLASS ((obj), CA_TYPE_CIRCULAR_APPLICATIONS_MENU, CaCircularApplicationsMenuClass))

typedef enum
{
    ROOT_LEAF,
    FILE_LEAF,
    SUB_FILE_LEAF
} LeafType;

typedef enum
{
	GLYPH_UNKNOWN,
    GLYPH_ROOT_CENTRE,
    GLYPH_FILE_MENU_CENTRE,
    GLYPH_FILE,
    GLYPH_FILE_MENU,
	GLYPH_TAB
} GlyphType;

typedef enum
{
    NO_STATE,
    INITIAL_OPEN
} AnimationState;

typedef struct _CaCircularApplicationsMenu        CaCircularApplicationsMenu;
typedef struct _CaCircularApplicationsMenuClass   CaCircularApplicationsMenuClass;

struct _CaCircularApplicationsMenu
{
    GtkDrawingArea parent;

    /*< private >*/

    /*< public >*/
};

struct _CaCircularApplicationsMenuClass
{
	GtkDrawingAreaClass parent_class;
};

typedef struct _CaMenuRender CaMenuRender;
typedef struct _CaSegmentRender CaSegmentRender;
typedef struct _CaFileItem CaFileItem;
typedef struct _CaFileLeaf CaFileLeaf;


struct _CaMenuRender
{
	// Render a child menu.
	/*
	 C   E
	 B   F
	A     A
	A     A
	A     A
	 AAAAA
	*/
	gint Bcircle_x;
	gint Bcircle_y;
	gint Ccircle_x;
	gint Ccircle_y;
	gint Ecircle_x;
	gint Ecircle_y;
	gint Fcircle_x;
	gint Fcircle_y;
	gint Bcircle_arc_x;
	gint Bcircle_arc_y;
	gint Ccircle_arc_x;
	gint Ccircle_arc_y;
	gdouble lowest_angle_nearest;
	gdouble highest_angle_nearest;
	gdouble Bcircle_lowest_angle;
	gdouble Bcircle_highest_angle;
	gdouble Fcircle_lowest_angle;
	gdouble Fcircle_highest_angle;
	gdouble lowest_angle_farthest;
	gdouble highest_angle_farthest;
	gdouble Ccircle_lowest_angle;
	gdouble Ccircle_highest_angle;
	gdouble Ecircle_lowest_angle;
	gdouble Ecircle_highest_angle;

	gdouble tab_glyph_radius;
	gint tab_glyph_x;
	gint tab_glyph_y;
};

struct _CaSegmentRender
{
	/*
	Segment

	CDC
	BAB
	*/
	gint Aradius;
	gint Bradius;
	gint Cradius;
	gint Dradius;
	gdouble Afrom_angle;
	gdouble Bfrom_angle;
	gdouble Cfrom_angle;
	gdouble Dfrom_angle;
	gdouble Ato_angle;
	gdouble Bto_angle;
	gdouble Cto_angle;
	gdouble Dto_angle;
	gint B_lowest_circle_x;
	gint B_lowest_circle_y;
	gint C_lowest_circle_x;
	gint C_lowest_circle_y;
	gint B_highest_circle_x;
	gint B_highest_circle_y;
	gint C_highest_circle_x;
	gint C_highest_circle_y;

	// Arrow.
	gdouble arrow_percentage;
	gdouble common_angle;
	gint arrow_point_x;
	gint arrow_point_y;
	gint arrow_side_x;
	gint arrow_side_y;
	gint outer_arrow_radius;
	gint inner_arrow_radius;
	gdouble arrow_angle;
};

struct _CaFileItem
{
    GlyphType _type;
    GMenuTreeItem* _menutreeitem;
    const gchar* _icon;
    GdkPixbuf* _pixbuf;
    CaFileLeaf* _assigned_fileleaf;
    CaFileItem* _associated_fileitem;
    gdouble _parent_angle;
    gdouble _circular_angle_share;
    gdouble _parent_radius;
	CaSegmentRender* _segment_render;

    /*< Drawing >*/
    gint x;
    gint y;
    gint size;
};

struct _CaFileLeaf
{
    LeafType _type;
    GList* _fileitem_list;
    gint _fileitem_list_count;
    AnimationState _current_animation_state;
    gint _current_animation_tick;
    CaFileLeaf* _parent_fileleaf;
    CaFileLeaf* _child_fileleaf;
    GList* _sub_fileleaves_list;
    gint _sub_fileleaves_list_count;
    CaFileItem* _central_glyph;
	CaMenuRender* _menu_render;

    /*< Drawing >*/
    gint x;
    gint y;
    gint radius;
};

/* Construction */
GtkWidget* ca_circular_applications_menu_new (gint width, gint height, gboolean hideindicator, gint glyphsize);
GType ca_circular_applications_menu_get_type(void);

/* Implementation */
CaFileLeaf* ca_circular_applications_menu(GMenuTreeDirectory *root);
CaFileLeaf* ca_circular_applications_menu_show_leaf(GMenuTreeDirectory* menutreedirectory, LeafType leaftype, CaFileItem* fileitem, gboolean disassociated);
void ca_circular_applications_menu_close_fileleaf(CaFileLeaf* fileleaf);

/* Private functions. */

G_END_DECLS

#endif
