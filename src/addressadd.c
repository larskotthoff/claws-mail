/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 2001 Match Grun
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Add address to address book dialog.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defs.h"

#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkhbbox.h>
#include <gtk/gtkbutton.h>

#include "intl.h"
#include "gtkutils.h"
#include "stock_pixmap.h"
#include "prefs_common.h"
#include "addressadd.h"
#include "addritem.h"
#include "addrbook.h"
#include "addrindex.h"
#include "manage_window.h"

typedef struct {
	AddressBookFile	*book;
	ItemFolder	*folder;
} FolderInfo;

static struct _AddressAdd_dlg {
	GtkWidget *window;
	GtkWidget *entry_name;
	GtkWidget *label_address;
	GtkWidget *entry_remarks;
	GtkWidget *tree_folder;
	GtkWidget *ok_btn;
	GtkWidget *cancel_btn;
	GtkWidget *statusbar;
	gint status_cid;
	FolderInfo *fiSelected;
} addressadd_dlg;

static GdkPixmap *folderXpm;
static GdkBitmap *folderXpmMask;
static GdkPixmap *bookXpm;
static GdkBitmap *bookXpmMask;

static gboolean addressadd_cancelled;

static FolderInfo *addressadd_create_folderinfo( AddressBookFile *abf, ItemFolder *folder )
{
	FolderInfo *fi = g_new0( FolderInfo, 1 );
	fi->book   = abf;
	fi->folder = folder;
	return fi;
}

static void addressadd_free_folderinfo( FolderInfo *fi ) {
	fi->book   = NULL;
	fi->folder = NULL;
	g_free( fi );
}

/*
* Edit functions.
*/
static void addressadd_status_show( gchar *msg ) {
	if( addressadd_dlg.statusbar != NULL ) {
		gtk_statusbar_pop( GTK_STATUSBAR(addressadd_dlg.statusbar), addressadd_dlg.status_cid );
		if( msg ) {
			gtk_statusbar_push( GTK_STATUSBAR(addressadd_dlg.statusbar), addressadd_dlg.status_cid, msg );
		}
	}
}

static gint addressadd_delete_event( GtkWidget *widget, GdkEventAny *event, gboolean *cancelled ) {
	addressadd_cancelled = TRUE;
	gtk_main_quit();
	return TRUE;
}

static gboolean addressadd_key_pressed( GtkWidget *widget, GdkEventKey *event, gboolean *cancelled ) {
	if (event && event->keyval == GDK_Escape) {
		addressadd_cancelled = TRUE;
		gtk_main_quit();
	}
	return FALSE;
}

static void addressadd_ok( GtkWidget *widget, gboolean *cancelled ) {
	addressadd_cancelled = FALSE;
	gtk_main_quit();
}

static void addressadd_cancel( GtkWidget *widget, gboolean *cancelled ) {
	addressadd_cancelled = TRUE;
	gtk_main_quit();
}

static void addressadd_folder_select( GtkCTree *ctree, gint row, gint column,
					GdkEvent *event, gpointer data )
{
	addressadd_dlg.fiSelected = gtk_clist_get_row_data( GTK_CLIST(ctree), row );
}

static gboolean addressadd_tree_button( GtkCTree *ctree, GdkEventButton *event, gpointer data ) {
	if( ! event ) return FALSE;
	if( event->button == 1 ) {
		/* Handle double click */
		if( event->type == GDK_2BUTTON_PRESS ) {
			addressadd_cancelled = FALSE;
			gtk_main_quit();
		}
	}

	return FALSE;
}

static void addressadd_create( void ) {
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *entry_name;
	GtkWidget *label_addr;
	GtkWidget *entry_rems;
	GtkWidget *tree_folder;
	GtkWidget *vlbox;
	GtkWidget *tree_win;
	GtkWidget *hbbox;
	GtkWidget *ok_btn;
	GtkWidget *cancel_btn;
	GtkWidget *hsbox;
	GtkWidget *statusbar;
	gint top;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request( window, 300, 400 );
	gtk_container_set_border_width( GTK_CONTAINER(window), 0 );
	gtk_window_set_title( GTK_WINDOW(window), _("Add to address book") );
	gtk_window_set_position( GTK_WINDOW(window), GTK_WIN_POS_MOUSE );
	gtk_window_set_modal( GTK_WINDOW(window), TRUE );
	g_signal_connect( G_OBJECT(window), "delete_event",
			  G_CALLBACK(addressadd_delete_event), NULL );
	g_signal_connect( G_OBJECT(window), "key_press_event",
			  G_CALLBACK(addressadd_key_pressed), NULL );

	vbox = gtk_vbox_new(FALSE, 8);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_container_set_border_width( GTK_CONTAINER(vbox), 8 );

	table = gtk_table_new(3, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
	gtk_container_set_border_width( GTK_CONTAINER(table), 8 );
	gtk_table_set_row_spacings(GTK_TABLE(table), 8);
	gtk_table_set_col_spacings(GTK_TABLE(table), 8);

	/* First row */
	top = 0;
	label = gtk_label_new(_("Name"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, top, (top + 1), GTK_FILL, 0, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	entry_name = gtk_entry_new();
	/* gtk_entry_set_max_length(); */
	gtk_entry_set_text (GTK_ENTRY(entry_name),"");
	
	gtk_table_attach(GTK_TABLE(table), entry_name, 1, 2, top, (top + 1), GTK_FILL | GTK_EXPAND , 0, 0, 0);

	/* Second row */
	top = 1;
	label = gtk_label_new(_("Address"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, top, (top + 1), GTK_FILL, 0, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);

	label_addr = gtk_label_new("");
	gtk_table_attach(GTK_TABLE(table), label_addr, 1, 2, top, (top + 1), GTK_FILL, 0, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label_addr), 0, 0.5);

	/* Third row */
	top = 2;
	label = gtk_label_new(_("Remarks"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, top, (top + 1), GTK_FILL, 0, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	entry_rems = gtk_entry_new();
	/* gtk_entry_set_max_length(); */
	gtk_table_attach(GTK_TABLE(table), entry_rems, 1, 2, top, (top + 1), GTK_FILL | GTK_EXPAND, 0, 0, 0);

	/* Address book/folder tree */
	vlbox = gtk_vbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(vbox), vlbox, TRUE, TRUE, 0);
	gtk_container_set_border_width( GTK_CONTAINER(vlbox), 8 );

	tree_win = gtk_scrolled_window_new( NULL, NULL );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(tree_win),
				        GTK_POLICY_AUTOMATIC,
				        GTK_POLICY_AUTOMATIC );
	gtk_box_pack_start( GTK_BOX(vlbox), tree_win, TRUE, TRUE, 0 );

	tree_folder = gtk_ctree_new( 1, 0 );
	gtk_container_add( GTK_CONTAINER(tree_win), tree_folder );
	gtk_clist_column_titles_show( GTK_CLIST(tree_folder) );
	gtk_clist_set_column_title( GTK_CLIST(tree_folder), 0, _( "Select Address Book Folder" ) );
	gtk_ctree_set_line_style( GTK_CTREE(tree_folder), GTK_CTREE_LINES_DOTTED );
	gtk_clist_set_selection_mode( GTK_CLIST(tree_folder), GTK_SELECTION_BROWSE );
	gtk_ctree_set_expander_style( GTK_CTREE(tree_folder), GTK_CTREE_EXPANDER_SQUARE );
	gtk_ctree_set_indent( GTK_CTREE(tree_folder), CTREE_INDENT );
	gtk_clist_set_auto_sort( GTK_CLIST(tree_folder), TRUE );

	/* Status line */
	hsbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), hsbox, FALSE, FALSE, BORDER_WIDTH);
	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(hsbox), statusbar, TRUE, TRUE, BORDER_WIDTH);

	/* Button panel */
	gtkut_button_set_create(&hbbox, &ok_btn, _("OK"),
				&cancel_btn, _("Cancel"), NULL, NULL);
	gtk_box_pack_end(GTK_BOX(vbox), hbbox, FALSE, FALSE, 0);
	gtk_container_set_border_width( GTK_CONTAINER(hbbox), 0 );
	gtk_widget_grab_default(ok_btn);

	g_signal_connect(G_OBJECT(ok_btn), "clicked",
			 G_CALLBACK(addressadd_ok), NULL);
	g_signal_connect(G_OBJECT(cancel_btn), "clicked",
			 G_CALLBACK(addressadd_cancel), NULL);
	g_signal_connect(G_OBJECT(tree_folder), "select_row",
			 G_CALLBACK(addressadd_folder_select), NULL);
	g_signal_connect(G_OBJECT(tree_folder), "button_press_event",
			 G_CALLBACK(addressadd_tree_button), NULL);

	gtk_widget_show_all(vbox);

	addressadd_dlg.window        = window;
	addressadd_dlg.entry_name    = entry_name;
	addressadd_dlg.label_address = label_addr;
	addressadd_dlg.entry_remarks = entry_rems;
	addressadd_dlg.tree_folder   = tree_folder;
	addressadd_dlg.ok_btn        = ok_btn;
	addressadd_dlg.cancel_btn    = cancel_btn;
	addressadd_dlg.statusbar     = statusbar;
	addressadd_dlg.status_cid    = gtk_statusbar_get_context_id( GTK_STATUSBAR(statusbar), "Address Add" );

	gtk_widget_show_all( window );

	stock_pixmap_gdk( window, STOCK_PIXMAP_BOOK, &bookXpm, &bookXpmMask );
	stock_pixmap_gdk( window, STOCK_PIXMAP_DIR_OPEN,
			  &folderXpm, &folderXpmMask );
}

static void addressadd_load_folder( GtkCTreeNode *parentNode, ItemFolder *parentFolder,
					FolderInfo *fiParent )
{
	GtkCTree *tree = GTK_CTREE( addressadd_dlg.tree_folder );
	GList *list;
	ItemFolder *folder;
	gchar *fName;
	gchar **name;
	GtkCTreeNode *node;
	FolderInfo *fi;

	list = parentFolder->listFolder;
	while( list ) {
		folder = list->data;
		fName = g_strdup( ADDRITEM_NAME(folder) );
		name = &fName;
		node = gtk_ctree_insert_node( tree, parentNode, NULL, name, FOLDER_SPACING,
				folderXpm, folderXpmMask, folderXpm, folderXpmMask,
				FALSE, TRUE );
		g_free( fName );
		fi = addressadd_create_folderinfo( fiParent->book, folder );
		gtk_ctree_node_set_row_data_full( tree, node, fi,
				( GtkDestroyNotify ) addressadd_free_folderinfo );
		addressadd_load_folder( node, folder, fi );
		list = g_list_next( list );
	}
}

static void addressadd_load_data( AddressIndex *addrIndex ) {
	AddressDataSource *ds;
	GList *list, *nodeDS;
	gchar **name;
	gchar *dsName;
	ItemFolder *rootFolder;
	AddressBookFile *abf;
	FolderInfo *fi;
	GtkCTree *tree = GTK_CTREE( addressadd_dlg.tree_folder );
	GtkCTreeNode *node;

	gtk_clist_clear( GTK_CLIST( tree ) );
	list = addrindex_get_interface_list( addrIndex );
	while( list ) {
		AddressInterface *interface = list->data;
		if( interface->type == ADDR_IF_BOOK ) {
			nodeDS = interface->listSource;
			while( nodeDS ) {
				ds = nodeDS->data;
				dsName = g_strdup( addrindex_ds_get_name( ds ) );

				/* Read address book */
				if( ! addrindex_ds_get_read_flag( ds ) ) {
					addrindex_ds_read_data( ds );
				}

				/* Add node for address book */
				abf = ds->rawDataSource;
				name = &dsName;
				node = gtk_ctree_insert_node( tree, NULL, NULL,
						name, FOLDER_SPACING, bookXpm,
						bookXpmMask, bookXpm, bookXpmMask,
						FALSE, TRUE );
				g_free( dsName );

				fi = addressadd_create_folderinfo( abf, NULL );
				gtk_ctree_node_set_row_data_full( tree, node, fi,
						( GtkDestroyNotify ) addressadd_free_folderinfo );

				rootFolder = addrindex_ds_get_root_folder( ds );
				addressadd_load_folder( node, rootFolder, fi );

				nodeDS = g_list_next( nodeDS );
			}
		}
		list = g_list_next( list );
	}
}

gboolean addressadd_selection( AddressIndex *addrIndex, const gchar *name, const gchar *address, const gchar *remarks ) {
	gboolean retVal = FALSE;
	ItemPerson *person = NULL;
	FolderInfo *fi = NULL;
	addressadd_cancelled = FALSE;
	if( ! addressadd_dlg.window ) addressadd_create();
	gtk_widget_grab_focus(addressadd_dlg.ok_btn);
	gtk_widget_show(addressadd_dlg.window);
	manage_window_set_transient(GTK_WINDOW(addressadd_dlg.window));

	addressadd_dlg.fiSelected = NULL;
	addressadd_status_show( "" );
	addressadd_load_data( addrIndex );
	gtk_clist_select_row( GTK_CLIST( addressadd_dlg.tree_folder ), 0, 0 );
	gtk_widget_show(addressadd_dlg.window);

	gtk_entry_set_text( GTK_ENTRY(addressadd_dlg.entry_name ), "" );
	gtk_label_set_text( GTK_LABEL(addressadd_dlg.label_address ), "" );
	gtk_entry_set_text( GTK_ENTRY(addressadd_dlg.entry_remarks ), "" );
	if( name )
		gtk_entry_set_text( GTK_ENTRY(addressadd_dlg.entry_name ), name );
	if( address )
		gtk_label_set_text( GTK_LABEL(addressadd_dlg.label_address ), address );
	if( remarks )
		gtk_entry_set_text( GTK_ENTRY(addressadd_dlg.entry_remarks ), remarks );

	gtk_main();
	gtk_widget_hide( addressadd_dlg.window );

	if( ! addressadd_cancelled ) {
		if( addressadd_dlg.fiSelected ) {
			gchar *returned_name;
			gchar *returned_remarks;
			returned_name = gtk_editable_get_chars( GTK_EDITABLE(addressadd_dlg.entry_name), 0, -1 );
			returned_remarks = gtk_editable_get_chars( GTK_EDITABLE(addressadd_dlg.entry_remarks), 0, -1 );

			fi = addressadd_dlg.fiSelected;
			
			person = addrbook_add_contact( fi->book, fi->folder, 
							returned_name, 
							address, 
							returned_remarks);
			g_free(returned_name);
			g_free(returned_remarks);
			if( person ) retVal = TRUE;
		}
	}

	gtk_clist_clear( GTK_CLIST( addressadd_dlg.tree_folder ) );

	return retVal;
}

/*
* End of Source.
*/

