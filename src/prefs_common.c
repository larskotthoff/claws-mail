/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2001 Hiroyuki Yamamoto
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defs.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "intl.h"
#include "main.h"
#include "prefs.h"
#include "prefs_common.h"
#include "prefs_display_header.h"
#include "prefs_summary_column.h"
#include "mainwindow.h"
#include "summaryview.h"
#include "messageview.h"
#include "manage_window.h"
#include "inc.h"
#include "menu.h"
#include "codeconv.h"
#include "utils.h"
#include "gtkutils.h"
#include "alertpanel.h"
#include "folder.h"
#include "filesel.h"
#include "folderview.h"

#if USE_PSPELL
#include "gtkspell.h"
#endif

PrefsCommon prefs_common;

static PrefsDialog dialog;

static struct Receive {
	GtkWidget *checkbtn_incext;
	GtkWidget *entry_incext;
	GtkWidget *button_incext;

	GtkWidget *checkbtn_local;
	GtkWidget *checkbtn_filter_on_inc;
	GtkWidget *entry_spool;

	GtkWidget *checkbtn_autochk;
	GtkWidget *spinbtn_autochk;
	GtkObject *spinbtn_autochk_adj;

	GtkWidget *checkbtn_chkonstartup;
	GtkWidget *checkbtn_noerrorpanel;
	GtkWidget *checkbtn_scan_after_inc;


	GtkWidget *spinbtn_maxarticle;
	GtkObject *spinbtn_maxarticle_adj;
} receive;

static struct Send {
	GtkWidget *checkbtn_extsend;
	GtkWidget *entry_extsend;
	GtkWidget *button_extsend;

	GtkWidget *checkbtn_savemsg;
	GtkWidget *checkbtn_queuemsg;

	GtkWidget *optmenu_charset;
	
	GtkWidget *checkbtn_returnreceipt;
} send;

static struct Compose {
	GtkWidget *checkbtn_autosig;
	GtkWidget *entry_sigsep;

	GtkWidget *entry_fw_quotemark;
	GtkWidget *text_fw_quotefmt;

	GtkWidget *checkbtn_autoextedit;

	GtkWidget *checkbtn_reply_account_autosel;
	GtkWidget *checkbtn_forward_account_autosel;
	GtkWidget *checkbtn_reedit_account_autosel;

	GtkWidget *spinbtn_linewrap;
	GtkObject *spinbtn_linewrap_adj;
	GtkWidget *checkbtn_wrapquote;
	GtkWidget *checkbtn_wrapatsend;

	GtkWidget *checkbtn_quote;
	GtkWidget * checkbtn_forward_as_attachment;
	GtkWidget * checkbtn_smart_wrapping;
	GtkWidget * checkbtn_block_cursor;

	/* spelling */
#if USE_PSPELL
	GtkWidget *checkbtn_enable_pspell;
	GtkWidget *entry_pspell_path;
	GtkWidget *btn_pspell_path;
	GtkWidget *optmenu_dictionary;
#endif

} compose;

static struct Quote {
	GtkWidget *entry_quotemark;
	GtkWidget *text_quotefmt;

	GtkWidget *entry_fw_quotemark;
	GtkWidget *text_fw_quotefmt;
} quote;

static struct Display {
	GtkWidget *entry_textfont;
	GtkWidget *button_textfont;

	GtkWidget *entry_smallfont;
	GtkWidget *entry_normalfont;
	GtkWidget *entry_boldfont;

	GtkWidget *chkbtn_folder_unread;

	GtkWidget *chkbtn_transhdr;

	GtkWidget *chkbtn_swapfrom;
	GtkWidget *chkbtn_hscrollbar;
	GtkWidget *chkbtn_useaddrbook;
	GtkWidget *chkbtn_expand_thread;
	GtkWidget *chkbtn_bold_unread;

	GtkWidget *entry_datefmt;
} display;

static struct Message {
	GtkWidget *chkbtn_enablecol;
	GtkWidget *button_edit_col;
	GtkWidget *chkbtn_mbalnum;
	GtkWidget *chkbtn_disphdrpane;
	GtkWidget *chkbtn_disphdr;
	GtkWidget *spinbtn_linespc;
	GtkObject *spinbtn_linespc_adj;
	GtkWidget *chkbtn_headspc;

	GtkWidget *chkbtn_smoothscroll;
	GtkWidget *spinbtn_scrollstep;
	GtkObject *spinbtn_scrollstep_adj;
	GtkWidget *chkbtn_halfpage;
} message;

#if USE_GPGME
static struct Privacy {
	GtkWidget *checkbtn_default_encrypt;
        GtkWidget *checkbtn_ascii_armored;
	GtkWidget *checkbtn_default_sign;
	GtkWidget *checkbtn_auto_check_signatures;
	GtkWidget *checkbtn_gpg_signature_popup;
	GtkWidget *checkbtn_passphrase_grab;
	GtkWidget *checkbtn_gpg_warning;
	GtkWidget *optmenu_default_signkey;
} privacy;
#endif

static struct Interface {
	/* GtkWidget *checkbtn_emacs; */
	GtkWidget *checkbtn_openunread;
	GtkWidget *checkbtn_openinbox;
	GtkWidget *checkbtn_immedexec;
	GtkWidget *checkbtn_confonexit;
	GtkWidget *checkbtn_cleanonexit;
	GtkWidget *checkbtn_askonclean;
	GtkWidget *checkbtn_warnqueued;
	GtkWidget *checkbtn_addaddrbyclick;
	GtkWidget *optmenu_recvdialog;
 	GtkWidget *optmenu_nextunreadmsgdialog;
} interface;

static struct Other {
	GtkWidget *uri_combo;
	GtkWidget *uri_entry;
	GtkWidget *printcmd_entry;
	GtkWidget *exteditor_combo;
	GtkWidget *exteditor_entry;
} other;

static struct MessageColorButtons {
	GtkWidget *quote_level1_btn;
	GtkWidget *quote_level2_btn;
	GtkWidget *quote_level3_btn;
	GtkWidget *uri_btn;
	GtkWidget *tgt_folder_btn;
} color_buttons;

static GtkWidget *quote_desc_win;
static GtkWidget *font_sel_win;
static guint font_sel_conn_id; 
static GtkWidget *quote_color_win;
static GtkWidget *color_dialog;

static void prefs_common_charset_set_data_from_optmenu(PrefParam *pparam);
static void prefs_common_charset_set_optmenu	      (PrefParam *pparam);
#if USE_GPGME
static void prefs_common_default_signkey_set_data_from_optmenu
							(PrefParam *pparam);
static void prefs_common_default_signkey_set_optmenu	(PrefParam *pparam);
static void prefs_common_ascii_armored_warning(GtkWidget* widget, 
				               gpointer unused);
#endif
static void prefs_common_recv_dialog_set_data_from_optmenu(PrefParam *pparam);
static void prefs_common_recv_dialog_set_optmenu(PrefParam *pparam);
static void prefs_nextunreadmsgdialog_set_data_from_optmenu(PrefParam *pparam);
static void prefs_nextunreadmsgdialog_set_optmenu(PrefParam *pparam);

#if USE_PSPELL
static void prefs_dictionary_set_data_from_optmenu(PrefParam *param);
static void prefs_dictionary_set_optmenu(PrefParam *pparam);
#endif


/*
   parameter name, default value, pointer to the prefs variable, data type,
   pointer to the widget pointer,
   pointer to the function for data setting,
   pointer to the function for widget setting
 */

static PrefParam param[] = {
	/* Receive */
	{"use_ext_inc", "FALSE", &prefs_common.use_extinc, P_BOOL,
	 &receive.checkbtn_incext,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"ext_inc_path", DEFAULT_INC_PATH, &prefs_common.extinc_cmd, P_STRING,
	 &receive.entry_incext,
	 prefs_set_data_from_entry, prefs_set_entry},

	{"inc_local", "TRUE", &prefs_common.inc_local, P_BOOL,
	 &receive.checkbtn_local,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"filter_on_inc_local", "FALSE", &prefs_common.filter_on_inc, P_BOOL,
	 &receive.checkbtn_filter_on_inc,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"spool_path", DEFAULT_SPOOL_PATH, &prefs_common.spool_path, P_STRING,
	 &receive.entry_spool,
	 prefs_set_data_from_entry, prefs_set_entry},

	{"autochk_newmail", "FALSE", &prefs_common.autochk_newmail, P_BOOL,
	 &receive.checkbtn_autochk,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"autochk_interval", "10", &prefs_common.autochk_itv, P_INT,
	 &receive.spinbtn_autochk,
	 prefs_set_data_from_spinbtn, prefs_set_spinbtn},
	{"check_on_startup", "FALSE", &prefs_common.chk_on_startup, P_BOOL,
	 &receive.checkbtn_chkonstartup,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"noerrorpanel", "FALSE", &prefs_common.noerrorpanel, P_BOOL,
	 &receive.checkbtn_noerrorpanel,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"scan_all_after_inc", "FALSE", &prefs_common.scan_all_after_inc,
	 P_BOOL, &receive.checkbtn_scan_after_inc,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"max_news_articles", "300", &prefs_common.max_articles, P_INT,
	 &receive.spinbtn_maxarticle,
	 prefs_set_data_from_spinbtn, prefs_set_spinbtn},

	/* Send */
	{"use_ext_sendmail", "FALSE", &prefs_common.use_extsend, P_BOOL,
	 &send.checkbtn_extsend,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"ext_sendmail_cmd", DEFAULT_SENDMAIL_CMD,
	 &prefs_common.extsend_cmd, P_STRING,
	 &send.entry_extsend, prefs_set_data_from_entry, prefs_set_entry},
	{"save_message", "TRUE", &prefs_common.savemsg, P_BOOL,
	 &send.checkbtn_savemsg,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"queue_message", "FALSE", &prefs_common.queue_msg, P_BOOL,
	 &send.checkbtn_queuemsg,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	{"outgoing_charset", CS_AUTO, &prefs_common.outgoing_charset, P_STRING,
	 &send.optmenu_charset,
	 prefs_common_charset_set_data_from_optmenu,
	 prefs_common_charset_set_optmenu},

	/* Compose */
	{"auto_signature", "TRUE", &prefs_common.auto_sig, P_BOOL,
	 &compose.checkbtn_autosig,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"signature_separator", "-- ", &prefs_common.sig_sep, P_STRING,
	 &compose.entry_sigsep, prefs_set_data_from_entry, prefs_set_entry},

	{"auto_ext_editor", "FALSE", &prefs_common.auto_exteditor, P_BOOL,
	 &compose.checkbtn_autoextedit,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	{"linewrap_length", "74", &prefs_common.linewrap_len, P_INT,
	 &compose.spinbtn_linewrap,
	 prefs_set_data_from_spinbtn, prefs_set_spinbtn},
	{"linewrap_quotation", "FALSE", &prefs_common.linewrap_quote, P_BOOL,
	 &compose.checkbtn_wrapquote,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"linewrap_before_sending", "FALSE",
	 &prefs_common.linewrap_at_send, P_BOOL,
	 &compose.checkbtn_wrapatsend,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"forward_as_attachment", "FALSE", &prefs_common.forward_as_attachment,
	 P_BOOL, &compose.checkbtn_forward_as_attachment,
	 prefs_set_data_from_toggle, prefs_set_toggle},
        {"smart_wrapping", "TRUE", &prefs_common.smart_wrapping,
	 P_BOOL, &compose.checkbtn_smart_wrapping,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"block_cursor", "FALSE", &prefs_common.block_cursor,
	 P_BOOL, &compose.checkbtn_block_cursor,
	 prefs_set_data_from_toggle, prefs_set_toggle},
#if USE_PSPELL
	{"enable_pspell", "TRUE", &prefs_common.enable_pspell,
	 P_BOOL, &compose.checkbtn_enable_pspell,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"pspell_path", "/usr/local/share/pspell/", &prefs_common.pspell_path, 
	 P_STRING, &compose.entry_pspell_path, 
	 prefs_set_data_from_entry, prefs_set_entry},
	{"dictionary",  "", &prefs_common.dictionary,
	 P_STRING, &compose.optmenu_dictionary, 
	 prefs_dictionary_set_data_from_optmenu, prefs_dictionary_set_optmenu },
#endif
	{"reply_with_quote", "TRUE", &prefs_common.reply_with_quote, P_BOOL,
	 NULL, NULL, NULL},

	/* Account autoselection */
	{"reply_account_autoselect", "TRUE",
	 &prefs_common.reply_account_autosel, P_BOOL,
	 &compose.checkbtn_reply_account_autosel,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"forward_account_autoselect", "TRUE",
	 &prefs_common.forward_account_autosel, P_BOOL,
	 &compose.checkbtn_forward_account_autosel,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"reedit_account_autoselect", "TRUE",
	 &prefs_common.reedit_account_autosel, P_BOOL,
	 &compose.checkbtn_reedit_account_autosel,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	{"show_ruler", "TRUE", &prefs_common.show_ruler, P_BOOL,
	 NULL, NULL, NULL},

	/* Quote */
	{"reply_quote_mark", "> ", &prefs_common.quotemark, P_STRING,
	 &quote.entry_quotemark, prefs_set_data_from_entry, prefs_set_entry},
	{"reply_quote_format", "On %d\\n%f wrote:\\n\\n%Q",
	 &prefs_common.quotefmt, P_STRING, &quote.text_quotefmt,
	 prefs_set_data_from_text, prefs_set_text},

	{"fw_quote_mark", "> ", &prefs_common.fw_quotemark, P_STRING,
	 &quote.entry_fw_quotemark,
	 prefs_set_data_from_entry, prefs_set_entry},
	{"fw_quote_format",
	 "\\n\\nBegin forwarded message:\\n\\n"
	 "?d(Date: %d\\n)?f(From: %f\\n)?t(To: %t\\n)?c(Cc: %c\\n)"
	 "?n(Newsgroups: %n\\n)?s(Subject: %s\\n)\\n\\n%M",
	 &prefs_common.fw_quotefmt, P_STRING, &quote.text_fw_quotefmt,
	 prefs_set_data_from_text, prefs_set_text},

	/* Display */
	{"widget_font", NULL, &prefs_common.widgetfont, P_STRING,
	 NULL, NULL, NULL},
	{"message_font", "-misc-fixed-medium-r-normal--14-*-*-*-*-*-*-*",
	 &prefs_common.textfont, P_STRING,
	 &display.entry_textfont,
	 prefs_set_data_from_entry, prefs_set_entry},
	{"small_font",   "-*-helvetica-medium-r-normal--10-*-*-*-*-*-*-*",
	 &prefs_common.smallfont,   P_STRING,
	 &display.entry_smallfont,
	 prefs_set_data_from_entry, prefs_set_entry},
	{"bold_font",    "-*-helvetica-bold-r-normal--12-*-*-*-*-*-*-*",
	 &prefs_common.boldfont,    P_STRING,
	 &display.entry_boldfont,
	 prefs_set_data_from_entry, prefs_set_entry},
	{"normal_font",  "-*-helvetica-medium-r-normal--12-*-*-*-*-*-*-*",
	 &prefs_common.normalfont,  P_STRING,
	 &display.entry_normalfont, 
	 prefs_set_data_from_entry, prefs_set_entry},


	{"display_folder_unread_num", "TRUE",
	 &prefs_common.display_folder_unread, P_BOOL,
	 &display.chkbtn_folder_unread,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	{"translate_header", "TRUE", &prefs_common.trans_hdr, P_BOOL,
	 &display.chkbtn_transhdr,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	/* Display: Summary View */
	{"enable_swap_from", "TRUE", &prefs_common.swap_from, P_BOOL,
	 &display.chkbtn_swapfrom,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"enable_hscrollbar", "TRUE", &prefs_common.enable_hscrollbar, P_BOOL,
	 &display.chkbtn_hscrollbar,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"use_address_book", "TRUE", &prefs_common.use_addr_book, P_BOOL,
	 &display.chkbtn_useaddrbook,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"date_format", "%y/%m/%d(%a) %H:%M", &prefs_common.date_format,
	 P_STRING, &display.entry_datefmt,
	 prefs_set_data_from_entry, prefs_set_entry},
	{"expand_thread", "TRUE", &prefs_common.expand_thread, P_BOOL,
	 &display.chkbtn_expand_thread,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"bold_unread", "TRUE", &prefs_common.bold_unread, P_BOOL,
	 &display.chkbtn_bold_unread,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	{"enable_thread", "TRUE", &prefs_common.enable_thread, P_BOOL,
	 NULL, NULL, NULL},
	
	{"toolbar_style", "3", &prefs_common.toolbar_style, P_ENUM,
	 NULL, NULL, NULL},
	{"show_statusbar", "TRUE", &prefs_common.show_statusbar, P_BOOL,
	 NULL, NULL, NULL},

	{"summary_col_show_mark", "TRUE",
	 &prefs_common.summary_col_visible[S_COL_MARK], P_BOOL, NULL, NULL, NULL},
	{"summary_col_show_unread", "TRUE",
	 &prefs_common.summary_col_visible[S_COL_UNREAD], P_BOOL, NULL, NULL, NULL},
	{"summary_col_show_mime", "TRUE",
	 &prefs_common.summary_col_visible[S_COL_MIME], P_BOOL, NULL, NULL, NULL},
	{"summary_col_show_subject", "TRUE",
	 &prefs_common.summary_col_visible[S_COL_SUBJECT], P_BOOL, NULL, NULL, NULL},
	{"summary_col_show_from", "TRUE",
	 &prefs_common.summary_col_visible[S_COL_FROM], P_BOOL, NULL, NULL, NULL},
	{"summary_col_show_date", "TRUE",
	 &prefs_common.summary_col_visible[S_COL_DATE], P_BOOL, NULL, NULL, NULL},
	{"summary_col_show_size", "TRUE",
	 &prefs_common.summary_col_visible[S_COL_SIZE], P_BOOL, NULL, NULL, NULL},
	{"summary_col_show_number", "FALSE",
	 &prefs_common.summary_col_visible[S_COL_NUMBER], P_BOOL, NULL, NULL, NULL},
	{"summary_col_show_score", "FALSE",
	 &prefs_common.summary_col_visible[S_COL_SCORE], P_BOOL, NULL, NULL, NULL},
	{"summary_col_show_locked", "FALSE",
	 &prefs_common.summary_col_visible[S_COL_LOCKED], P_BOOL, NULL, NULL, NULL},

	{"summary_col_pos_mark", "0",
	  &prefs_common.summary_col_pos[S_COL_MARK], P_INT, NULL, NULL, NULL},
	{"summary_col_pos_unread", "1",
	  &prefs_common.summary_col_pos[S_COL_UNREAD], P_INT, NULL, NULL, NULL},
	{"summary_col_pos_mime", "2",
	  &prefs_common.summary_col_pos[S_COL_MIME], P_INT, NULL, NULL, NULL},
	{"summary_col_pos_subject", "3",
	  &prefs_common.summary_col_pos[S_COL_SUBJECT], P_INT, NULL, NULL, NULL},
	{"summary_col_pos_from", "4",
	  &prefs_common.summary_col_pos[S_COL_FROM], P_INT, NULL, NULL, NULL},
	{"summary_col_pos_date", "5",
	  &prefs_common.summary_col_pos[S_COL_DATE], P_INT, NULL, NULL, NULL},
	{"summary_col_pos_size", "6",
	  &prefs_common.summary_col_pos[S_COL_SIZE], P_INT, NULL, NULL, NULL},
	{"summary_col_pos_number", "7",
	  &prefs_common.summary_col_pos[S_COL_NUMBER], P_INT, NULL, NULL, NULL},
	{"summary_col_pos_score", "8",
	 &prefs_common.summary_col_pos[S_COL_SCORE], P_INT, NULL, NULL, NULL},
	{"summary_col_pos_locked", "9",
	 &prefs_common.summary_col_pos[S_COL_LOCKED], P_INT, NULL, NULL, NULL},

	{"summary_col_size_mark", "10",
	 &prefs_common.summary_col_size[S_COL_MARK], P_INT, NULL, NULL, NULL},
	{"summary_col_size_unread", "13",
	 &prefs_common.summary_col_size[S_COL_UNREAD], P_INT, NULL, NULL, NULL},
	{"summary_col_size_mime", "10",
	 &prefs_common.summary_col_size[S_COL_MIME], P_INT, NULL, NULL, NULL},
	{"summary_col_size_subject", "200",
	 &prefs_common.summary_col_size[S_COL_SUBJECT], P_INT, NULL, NULL, NULL},
	{"summary_col_size_from", "120",
	 &prefs_common.summary_col_size[S_COL_FROM], P_INT, NULL, NULL, NULL},
	{"summary_col_size_date", "118",
	 &prefs_common.summary_col_size[S_COL_DATE], P_INT, NULL, NULL, NULL},
	{"summary_col_size_size", "45",
	 &prefs_common.summary_col_size[S_COL_SIZE], P_INT, NULL, NULL, NULL},
	{"summary_col_size_number", "40",
	 &prefs_common.summary_col_size[S_COL_NUMBER], P_INT, NULL, NULL, NULL},
	{"summary_col_size_score", "40",
	 &prefs_common.summary_col_size[S_COL_SCORE], P_INT, NULL, NULL, NULL},
	{"summary_col_size_locked", "13",
	 &prefs_common.summary_col_size[S_COL_LOCKED], P_INT, NULL, NULL, NULL},

	/* Widget size */
	{"folderview_width", "179", &prefs_common.folderview_width, P_INT,
	 NULL, NULL, NULL},
	{"folderview_height", "600", &prefs_common.folderview_height, P_INT,
	 NULL, NULL, NULL},
	{"folder_col_folder", "150", &prefs_common.folder_col_folder, P_INT,
	 NULL, NULL, NULL},
	{"folder_col_new", "32", &prefs_common.folder_col_new, P_INT,
	 NULL, NULL, NULL},
	{"folder_col_unread", "32", &prefs_common.folder_col_unread, P_INT,
	 NULL, NULL, NULL},
	{"folder_col_total", "32", &prefs_common.folder_col_total, P_INT,
	 NULL, NULL, NULL},

	{"summaryview_width", "600", &prefs_common.summaryview_width, P_INT,
	 NULL, NULL, NULL},
	{"summaryview_height", "173", &prefs_common.summaryview_height, P_INT,
	 NULL, NULL, NULL},

	{"mainview_x", "64", &prefs_common.mainview_x, P_INT,
	 NULL, NULL, NULL},
	{"mainview_y", "64", &prefs_common.mainview_y, P_INT,
	 NULL, NULL, NULL},
	{"mainview_width", "600", &prefs_common.mainview_width, P_INT,
	 NULL, NULL, NULL},
	{"mainview_height", "600", &prefs_common.mainview_height, P_INT,
	 NULL, NULL, NULL},
	{"mainwin_x", "64", &prefs_common.mainwin_x, P_INT,
	 NULL, NULL, NULL},
	{"mainwin_y", "64", &prefs_common.mainwin_y, P_INT,
	 NULL, NULL, NULL},
	{"mainwin_width", "800", &prefs_common.mainwin_width, P_INT,
	 NULL, NULL, NULL},
	{"mainwin_height", "600", &prefs_common.mainwin_height, P_INT,
	 NULL, NULL, NULL},
	{"messagewin_width", "600", &prefs_common.msgwin_width, P_INT,
	 NULL, NULL, NULL},
	{"messagewin_height", "540", &prefs_common.msgwin_height, P_INT,
	 NULL, NULL, NULL},
	{"compose_width", "600", &prefs_common.compose_width, P_INT,
	 NULL, NULL, NULL},
	{"compose_height", "560", &prefs_common.compose_height, P_INT,
	 NULL, NULL, NULL},

	/* Message */
	{"enable_color", "TRUE", &prefs_common.enable_color, P_BOOL,
	 &message.chkbtn_enablecol,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	{"quote_level1_color", "179", &prefs_common.quote_level1_col, P_INT,
	 NULL, NULL, NULL},
	{"quote_level2_color", "179", &prefs_common.quote_level2_col, P_INT,
	 NULL, NULL, NULL},
	{"quote_level3_color", "179", &prefs_common.quote_level3_col, P_INT,
	 NULL, NULL, NULL},
	{"uri_color", "32512", &prefs_common.uri_col, P_INT,
	 NULL, NULL, NULL},
	{"target_folder_color", "14294218", &prefs_common.tgt_folder_col, P_INT,
	 NULL, NULL, NULL},
	{"signature_color", "0", &prefs_common.sig_col, P_USHORT,
	 NULL, NULL, NULL},
	{"recycle_quote_colors", "FALSE", &prefs_common.recycle_quote_colors,
	 P_BOOL, NULL, NULL, NULL},

	{"convert_mb_alnum", "FALSE", &prefs_common.conv_mb_alnum, P_BOOL,
	 &message.chkbtn_mbalnum,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"display_header_pane", "TRUE", &prefs_common.display_header_pane,
	 P_BOOL, &message.chkbtn_disphdrpane,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"display_header", "TRUE", &prefs_common.display_header, P_BOOL,
	 &message.chkbtn_disphdr,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"line_space", "2", &prefs_common.line_space, P_INT,
	 &message.spinbtn_linespc,
	 prefs_set_data_from_spinbtn, prefs_set_spinbtn},
	{"enable_head_space", "FALSE", &prefs_common.head_space, P_BOOL,
	 &message.chkbtn_headspc,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	{"enable_smooth_scroll", "FALSE",
	 &prefs_common.enable_smooth_scroll, P_BOOL,
	 &message.chkbtn_smoothscroll,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"scroll_step", "1", &prefs_common.scroll_step, P_INT,
	 &message.spinbtn_scrollstep,
	 prefs_set_data_from_spinbtn, prefs_set_spinbtn},
	{"scroll_half_page", "FALSE", &prefs_common.scroll_halfpage, P_BOOL,
	 &message.chkbtn_halfpage,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	{"show_other_header", "FALSE", &prefs_common.show_other_header, P_BOOL,
	 NULL, NULL, NULL},

	/* MIME viewer */
	{"mime_image_viewer", "display '%s'",
	 &prefs_common.mime_image_viewer, P_STRING, NULL, NULL, NULL},
	{"mime_audio_player", "play '%s'",
	 &prefs_common.mime_audio_player, P_STRING, NULL, NULL, NULL},
	{"mime_open_command", "gedit '%s'",
	 &prefs_common.mime_open_cmd, P_STRING, NULL, NULL, NULL},

#if USE_GPGME
	/* Privacy */
	{"default_encrypt", "FALSE", &prefs_common.default_encrypt, P_BOOL,
	 &privacy.checkbtn_default_encrypt,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"ascii_armored", "FALSE", &prefs_common.ascii_armored, P_BOOL,
	 &privacy.checkbtn_ascii_armored,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"default_sign", "FALSE", &prefs_common.default_sign, P_BOOL,
	 &privacy.checkbtn_default_sign,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"auto_check_signatures", "TRUE",
	 &prefs_common.auto_check_signatures, P_BOOL,
	 &privacy.checkbtn_auto_check_signatures,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"gpg_signature_popup", "FALSE",
	 &prefs_common.gpg_signature_popup, P_BOOL,
	 &privacy.checkbtn_gpg_signature_popup,
	 prefs_set_data_from_toggle, prefs_set_toggle},
#ifndef __MINGW32__
	{"passphrase_grab", "FALSE", &prefs_common.passphrase_grab, P_BOOL,
	 &privacy.checkbtn_passphrase_grab,
	 prefs_set_data_from_toggle, prefs_set_toggle},
#endif /* __MINGW32__ */
	{"gpg_warning", "TRUE", &prefs_common.gpg_warning, P_BOOL,
	 &privacy.checkbtn_gpg_warning,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"default_signkey", CS_AUTO, &prefs_common.default_signkey, P_STRING,
	 &privacy.optmenu_default_signkey,
	 prefs_common_default_signkey_set_data_from_optmenu,
	 prefs_common_default_signkey_set_optmenu},
#endif /* USE_GPGME */

	/* Interface */
	{"separate_folder", "FALSE", &prefs_common.sep_folder, P_BOOL,
	 NULL, NULL, NULL},
	{"separate_message", "FALSE", &prefs_common.sep_msg, P_BOOL,
	 NULL, NULL, NULL},

	/* {"emulate_emacs", "FALSE", &prefs_common.emulate_emacs, P_BOOL,
	 NULL, NULL, NULL}, */

	{"open_unread_on_enter", "FALSE", &prefs_common.open_unread_on_enter,
	 P_BOOL, &interface.checkbtn_openunread,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"open_inbox_on_inc", "TRUE", &prefs_common.open_inbox_on_inc,
	 P_BOOL, &interface.checkbtn_openinbox,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"immediate_execution", "TRUE", &prefs_common.immediate_exec, P_BOOL,
	 &interface.checkbtn_immedexec,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"receive_dialog_mode", "1", &prefs_common.recv_dialog_mode, P_ENUM,
	 &interface.optmenu_recvdialog,
	 prefs_common_recv_dialog_set_data_from_optmenu,
	 prefs_common_recv_dialog_set_optmenu},
	{"nextunreadmsg_dialog", NULL, &prefs_common.next_unread_msg_dialog, P_ENUM,
	 &interface.optmenu_nextunreadmsgdialog,
	 prefs_nextunreadmsgdialog_set_data_from_optmenu,
	 prefs_nextunreadmsgdialog_set_optmenu},

	{"add_address_by_click", "FALSE", &prefs_common.add_address_by_click,
	 P_BOOL, &interface.checkbtn_addaddrbyclick,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"confirm_on_exit", "TRUE", &prefs_common.confirm_on_exit, P_BOOL,
	 &interface.checkbtn_confonexit,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"send_return_receipt", "TRUE", &prefs_common.return_receipt, P_BOOL,
	 &send.checkbtn_returnreceipt,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"clean_trash_on_exit", "FALSE", &prefs_common.clean_on_exit, P_BOOL,
	 &interface.checkbtn_cleanonexit,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"ask_on_cleaning", "TRUE", &prefs_common.ask_on_clean, P_BOOL,
	 &interface.checkbtn_askonclean,
	 prefs_set_data_from_toggle, prefs_set_toggle},
	{"warn_queued_on_exit", "TRUE", &prefs_common.warn_queued_on_exit,
	 P_BOOL, &interface.checkbtn_warnqueued,
	 prefs_set_data_from_toggle, prefs_set_toggle},

	/* Other */
	{"uri_open_command", "netscape -remote 'openURL(%s,raise)'",
	 &prefs_common.uri_cmd, P_STRING,
	 &other.uri_entry, prefs_set_data_from_entry, prefs_set_entry},
	{"print_command", "lpr %s", &prefs_common.print_cmd, P_STRING,
	 &other.printcmd_entry, prefs_set_data_from_entry, prefs_set_entry},
	{"ext_editor_command", "gedit %s",
	 &prefs_common.ext_editor_cmd, P_STRING,
	 &other.exteditor_entry, prefs_set_data_from_entry, prefs_set_entry},

	{"kill_score", "-9999", &prefs_common.kill_score, P_INT,
	 NULL, NULL, NULL},
	{"important_score", "1", &prefs_common.important_score, P_INT,
	 NULL, NULL, NULL},

	{NULL, NULL, NULL, P_OTHER, NULL, NULL, NULL}
};

/* widget creating functions */
static void prefs_common_create		(void);
static void prefs_receive_create	(void);
static void prefs_send_create		(void);
static void prefs_compose_create	(void);
static void prefs_quote_create		(void);
static void prefs_display_create	(void);
static void prefs_message_create	(void);
#if USE_GPGME
static void prefs_privacy_create	(void);
#endif
static void prefs_interface_create	(void);
static void prefs_other_create		(void);

static void date_format_ok_btn_clicked		(GtkButton	*button,
						 GtkWidget     **widget);
static void date_format_cancel_btn_clicked	(GtkButton	*button,
						 GtkWidget     **widget);
static void date_format_key_pressed		(GtkWidget	*keywidget,
						 GdkEventKey	*event,
						 GtkWidget     **widget);
static gboolean date_format_on_delete		(GtkWidget	*dialogwidget,
						 GdkEventAny	*event,
						 GtkWidget     **widget);
static void date_format_entry_on_change		(GtkEditable	*editable,
						 GtkLabel	*example);
static void date_format_select_row		(GtkWidget	*date_format_list,
						 gint		 row,
						 gint		 column,
						 GdkEventButton	*event,
						 GtkWidget	*date_format);
static GtkWidget *date_format_create		(GtkButton	*button,
						 void		*data);

static void prefs_quote_description_create	(void);
static void prefs_quote_description_key_pressed	(GtkWidget	*widget,
						 GdkEventKey	*event,
						 gpointer	 data);

static void prefs_quote_colors_dialog		(void);
static void prefs_quote_colors_dialog_create	(void);
static void prefs_quote_colors_key_pressed	(GtkWidget	*widget,
						 GdkEventKey	*event,
						 gpointer	 data);
static void quote_color_set_dialog		(GtkWidget	*widget,
						 gpointer	 data);
static void quote_colors_set_dialog_ok		(GtkWidget	*widget,
						 gpointer	 data);
static void quote_colors_set_dialog_cancel	(GtkWidget	*widget,
						 gpointer	 data);
static void quote_colors_set_dialog_key_pressed	(GtkWidget	*widget,
						 GdkEventKey	*event,
						 gpointer	 data);
static void set_button_bg_color			(GtkWidget	*widget,
						 gint		 color);
static void prefs_enable_message_color_toggled	(void);
static void prefs_recycle_colors_toggled	(GtkWidget	*widget);

static void prefs_font_select	(GtkButton *button, GtkEntry *entry);

static void prefs_font_selection_key_pressed	(GtkWidget	*widget,
						 GdkEventKey	*event,
						 gpointer	 data);
static void prefs_font_selection_ok		(GtkButton	*button, GtkEntry *entry);

static gint prefs_common_deleted	(GtkWidget	*widget,
					 GdkEventAny	*event,
					 gpointer	 data);
static void prefs_common_key_pressed	(GtkWidget	*widget,
					 GdkEventKey	*event,
					 gpointer	 data);
static void prefs_common_ok		(void);
static void prefs_common_apply		(void);
static void prefs_common_cancel		(void);

void prefs_common_init() {
	prefs_common.fltlist = NULL;
	prefs_common.disphdr_list = NULL;
}

void prefs_common_read_config(void)
{
	FILE *fp;
	gchar *path;
	gchar buf[PREFSBUFSIZE];

	prefs_read_config(param, "Common", COMMON_RC);

	path = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, COMMAND_HISTORY,
			   NULL);
	if ((fp = fopen(path, "r")) == NULL) {
		if (ENOENT != errno) FILE_OP_ERROR(path, "fopen");
		g_free(path);
		return;
	}
	g_free(path);
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		g_strstrip(buf);
		if (buf[0] == '\0') continue;
		prefs_common.mime_open_cmd_history =
			add_history(prefs_common.mime_open_cmd_history, buf);
	}
	fclose(fp);

	prefs_common.mime_open_cmd_history =
		g_list_reverse(prefs_common.mime_open_cmd_history);
}

void prefs_common_save_config(void)
{
	GList *cur;
	FILE *fp;
	gchar *path;

	prefs_save_config(param, "Common", COMMON_RC);

	path = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, COMMAND_HISTORY,
			   NULL);
	if ((fp = fopen(path, "w")) == NULL) {
		FILE_OP_ERROR(path, "fopen");
		g_free(path);
		return;
	}

	for (cur = prefs_common.mime_open_cmd_history;
	     cur != NULL; cur = cur->next) {
		fputs((gchar *)cur->data, fp);
		fputc('\n', fp);
	}

	fclose(fp);
	g_free(path);
}

void prefs_common_open(void)
{
	if (prefs_rc_is_readonly(COMMON_RC))
		return;

	inc_lock();

	if (!dialog.window) {
		prefs_common_create();
	}

	manage_window_set_transient(GTK_WINDOW(dialog.window));
	gtk_notebook_set_page(GTK_NOTEBOOK(dialog.notebook), 0);
	gtk_widget_grab_focus(dialog.ok_btn);

	prefs_set_dialog(param);

	gtk_widget_show(dialog.window);
}

static void prefs_common_create(void)
{
	gint page = 0;

	debug_print(_("Creating common preferences window...\n"));

	prefs_dialog_create(&dialog);
	gtk_window_set_title (GTK_WINDOW(dialog.window),
			      _("Common Preferences"));
	gtk_signal_connect (GTK_OBJECT(dialog.window), "delete_event",
			    GTK_SIGNAL_FUNC(prefs_common_deleted), NULL);
	gtk_signal_connect (GTK_OBJECT(dialog.window), "key_press_event",
			    GTK_SIGNAL_FUNC(prefs_common_key_pressed), NULL);
	gtk_signal_connect (GTK_OBJECT(dialog.window), "focus_in_event",
			    GTK_SIGNAL_FUNC(manage_window_focus_in), NULL);
	gtk_signal_connect (GTK_OBJECT(dialog.window), "focus_out_event",
			    GTK_SIGNAL_FUNC(manage_window_focus_out), NULL);
	gtk_signal_connect (GTK_OBJECT(dialog.ok_btn), "clicked",
			    GTK_SIGNAL_FUNC(prefs_common_ok), NULL);
	gtk_signal_connect (GTK_OBJECT(dialog.apply_btn), "clicked",
			    GTK_SIGNAL_FUNC(prefs_common_apply), NULL);
	gtk_signal_connect_object (GTK_OBJECT(dialog.cancel_btn), "clicked",
				   GTK_SIGNAL_FUNC(prefs_common_cancel),
				   GTK_OBJECT(dialog.window));

	/* create all widgets on notebook */
	prefs_receive_create();
	SET_NOTEBOOK_LABEL(dialog.notebook, _("Receive"),   page++);
	prefs_send_create();
	SET_NOTEBOOK_LABEL(dialog.notebook, _("Send"),      page++);
	prefs_compose_create();
	SET_NOTEBOOK_LABEL(dialog.notebook, _("Compose"),   page++);
	prefs_quote_create();
	SET_NOTEBOOK_LABEL(dialog.notebook, _("Quote"),   page++);
	prefs_display_create();
	SET_NOTEBOOK_LABEL(dialog.notebook, _("Display"),   page++);
	prefs_message_create();
	SET_NOTEBOOK_LABEL(dialog.notebook, _("Message"),   page++);
#if USE_GPGME
	prefs_privacy_create();
	SET_NOTEBOOK_LABEL(dialog.notebook, _("Privacy"),   page++);
#endif
	prefs_interface_create();
	SET_NOTEBOOK_LABEL(dialog.notebook, _("Interface"), page++);
	prefs_other_create();
	SET_NOTEBOOK_LABEL(dialog.notebook, _("Other"),     page++);

	gtk_widget_show_all(dialog.window);
}

static void prefs_receive_create(void)
{
	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *frame_incext;
	GtkWidget *checkbtn_incext;
	GtkWidget *hbox;
	GtkWidget *label_incext;
	GtkWidget *entry_incext;
	/* GtkWidget *button_incext; */

	GtkWidget *frame_spool;
	GtkWidget *checkbtn_local;
	GtkWidget *checkbtn_filter_on_inc;
	GtkWidget *label_spool;
	GtkWidget *entry_spool;

	GtkWidget *hbox_autochk;
	GtkWidget *checkbtn_autochk;
	GtkWidget *label_autochk1;
	GtkObject *spinbtn_autochk_adj;
	GtkWidget *spinbtn_autochk;
	GtkWidget *label_autochk2;
	GtkWidget *checkbtn_chkonstartup;
	GtkWidget *checkbtn_noerrorpanel;
	GtkWidget *checkbtn_scan_after_inc;


	GtkWidget *frame_news;
	GtkWidget *label_maxarticle;
	GtkWidget *spinbtn_maxarticle;
	GtkObject *spinbtn_maxarticle_adj;

	vbox1 = gtk_vbox_new (FALSE, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dialog.notebook), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	PACK_FRAME(vbox1, frame_incext, _("External program"));

	vbox2 = gtk_vbox_new (FALSE, VSPACING_NARROW);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (frame_incext), vbox2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 8);

	/* Use of external incorporation program */
	PACK_CHECK_BUTTON (vbox2, checkbtn_incext,
			   _("Use external program for incorporation"));

	hbox = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
	SET_TOGGLE_SENSITIVITY (checkbtn_incext, hbox);

	label_incext = gtk_label_new (_("Command"));
	gtk_widget_show (label_incext);
	gtk_box_pack_start (GTK_BOX (hbox), label_incext, FALSE, FALSE, 0);

	entry_incext = gtk_entry_new ();
	gtk_widget_show (entry_incext);
	gtk_box_pack_start (GTK_BOX (hbox), entry_incext, TRUE, TRUE, 0);

#if 0
	button_incext = gtk_button_new_with_label ("... ");
	gtk_widget_show (button_incext);
	gtk_box_pack_start (GTK_BOX (hbox), button_incext, FALSE, FALSE, 0);
#endif

	PACK_FRAME(vbox1, frame_spool, _("Local spool"));

	vbox2 = gtk_vbox_new (FALSE, VSPACING_NARROW);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (frame_spool), vbox2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 8);

	hbox = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON (hbox, checkbtn_local, _("Incorporate from spool"));
	PACK_CHECK_BUTTON (hbox, checkbtn_filter_on_inc,
			   _("Filter on incorporation"));
	SET_TOGGLE_SENSITIVITY (checkbtn_local, checkbtn_filter_on_inc);

	hbox = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
	SET_TOGGLE_SENSITIVITY (checkbtn_local, hbox);

	label_spool = gtk_label_new (_("Spool directory"));
	gtk_widget_show (label_spool);
	gtk_box_pack_start (GTK_BOX (hbox), label_spool, FALSE, FALSE, 0);

	entry_spool = gtk_entry_new ();
	gtk_widget_show (entry_spool);
	gtk_box_pack_start (GTK_BOX (hbox), entry_spool, TRUE, TRUE, 0);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox2, FALSE, FALSE, 0);

	/* Auto-checking */
	hbox_autochk = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox_autochk);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox_autochk, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON (hbox_autochk, checkbtn_autochk,
			   _("Auto-check new mail"));

	label_autochk1 = gtk_label_new (_("every"));
	gtk_widget_show (label_autochk1);
	gtk_box_pack_start (GTK_BOX (hbox_autochk), label_autochk1, FALSE, FALSE, 0);

	spinbtn_autochk_adj = gtk_adjustment_new (5, 1, 100, 1, 10, 10);
	spinbtn_autochk = gtk_spin_button_new
		(GTK_ADJUSTMENT (spinbtn_autochk_adj), 1, 0);
	gtk_widget_show (spinbtn_autochk);
	gtk_box_pack_start (GTK_BOX (hbox_autochk), spinbtn_autochk, FALSE, FALSE, 0);
	gtk_widget_set_usize (spinbtn_autochk, 64, -1);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbtn_autochk), TRUE);

	label_autochk2 = gtk_label_new (_("minute(s)"));
	gtk_widget_show (label_autochk2);
	gtk_box_pack_start (GTK_BOX (hbox_autochk), label_autochk2, FALSE, FALSE, 0);

	SET_TOGGLE_SENSITIVITY(checkbtn_autochk, label_autochk1);
	SET_TOGGLE_SENSITIVITY(checkbtn_autochk, spinbtn_autochk);
	SET_TOGGLE_SENSITIVITY(checkbtn_autochk, label_autochk2);

	PACK_CHECK_BUTTON (vbox2, checkbtn_chkonstartup,
			   _("Check new mail on startup"));

	PACK_CHECK_BUTTON (vbox2, checkbtn_noerrorpanel,
			   _("No error popup on receive error"));
	PACK_CHECK_BUTTON (vbox2, checkbtn_scan_after_inc,
			   _("Update all local folders after incorporation"));

	PACK_FRAME(vbox1, frame_news, _("News"));

	hbox = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (frame_news), hbox);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);

	label_maxarticle = gtk_label_new
		(_("Maximum number of articles to download\n"
		   "(unlimited if 0 is specified)"));
	gtk_widget_show (label_maxarticle);
	gtk_box_pack_start (GTK_BOX (hbox), label_maxarticle, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label_maxarticle), GTK_JUSTIFY_LEFT);

	spinbtn_maxarticle_adj =
		gtk_adjustment_new (300, 0, 10000, 10, 100, 100);
	spinbtn_maxarticle = gtk_spin_button_new
		(GTK_ADJUSTMENT (spinbtn_maxarticle_adj), 10, 0);
	gtk_widget_show (spinbtn_maxarticle);
	gtk_box_pack_start (GTK_BOX (hbox), spinbtn_maxarticle,
			    FALSE, FALSE, 0);
	gtk_widget_set_usize (spinbtn_maxarticle, 64, -1);
	gtk_spin_button_set_numeric
		(GTK_SPIN_BUTTON (spinbtn_maxarticle), TRUE);

	receive.checkbtn_incext = checkbtn_incext;
	receive.entry_incext    = entry_incext;
	/* receive.button_incext   = button_incext; */

	receive.checkbtn_local         = checkbtn_local;
	receive.checkbtn_filter_on_inc = checkbtn_filter_on_inc;
	receive.entry_spool            = entry_spool;

	receive.checkbtn_autochk    = checkbtn_autochk;
	receive.spinbtn_autochk     = spinbtn_autochk;
	receive.spinbtn_autochk_adj = spinbtn_autochk_adj;

	receive.checkbtn_chkonstartup = checkbtn_chkonstartup;
	receive.checkbtn_noerrorpanel = checkbtn_noerrorpanel;
	receive.checkbtn_scan_after_inc = checkbtn_scan_after_inc;


	receive.spinbtn_maxarticle     = spinbtn_maxarticle;
	receive.spinbtn_maxarticle_adj = spinbtn_maxarticle_adj;
}

static void prefs_send_create(void)
{
	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *frame_extsend;
	GtkWidget *vbox_extsend;
	GtkWidget *checkbtn_extsend;
	GtkWidget *hbox1;
	GtkWidget *label_extsend;
	GtkWidget *entry_extsend;
	/* GtkWidget *button_extsend; */
	GtkWidget *checkbtn_savemsg;
	GtkWidget *checkbtn_queuemsg;
	GtkWidget *label_outcharset;
	GtkWidget *optmenu;
	GtkWidget *optmenu_menu;
	GtkWidget *menuitem;
	GtkWidget *checkbtn_returnreceipt;
	GtkWidget *label_charset_desc;

	vbox1 = gtk_vbox_new (FALSE, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dialog.notebook), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	PACK_FRAME(vbox1, frame_extsend, _("External program"));

	vbox_extsend = gtk_vbox_new (FALSE, VSPACING_NARROW);
	gtk_widget_show (vbox_extsend);
	gtk_container_add (GTK_CONTAINER (frame_extsend), vbox_extsend);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_extsend), 8);

	PACK_CHECK_BUTTON (vbox_extsend, checkbtn_extsend,
			   _("Use external program for sending"));

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox_extsend), hbox1, FALSE, FALSE, 0);
	SET_TOGGLE_SENSITIVITY(checkbtn_extsend, hbox1);

	label_extsend = gtk_label_new (_("Command"));
	gtk_widget_show (label_extsend);
	gtk_box_pack_start (GTK_BOX (hbox1), label_extsend, FALSE, FALSE, 0);

	entry_extsend = gtk_entry_new ();
	gtk_widget_show (entry_extsend);
	gtk_box_pack_start (GTK_BOX (hbox1), entry_extsend, TRUE, TRUE, 0);

#if 0
	button_extsend = gtk_button_new_with_label ("... ");
	gtk_widget_show (button_extsend);
	gtk_box_pack_start (GTK_BOX (hbox1), button_extsend, FALSE, FALSE, 0);
#endif

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox2, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON (vbox2, checkbtn_savemsg,
			   _("Save sent messages to outbox"));
	PACK_CHECK_BUTTON (vbox2, checkbtn_queuemsg,
			   _("Queue messages that fail to send"));
	PACK_CHECK_BUTTON (vbox2, checkbtn_returnreceipt,
			   _("Send return receipt on request"));

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

	label_outcharset = gtk_label_new (_("Outgoing codeset"));
	gtk_widget_show (label_outcharset);
	gtk_box_pack_start (GTK_BOX (hbox1), label_outcharset, FALSE, FALSE, 0);

	optmenu = gtk_option_menu_new ();
	gtk_widget_show (optmenu);
	gtk_box_pack_start (GTK_BOX (hbox1), optmenu, FALSE, FALSE, 0);

	optmenu_menu = gtk_menu_new ();

#define SET_MENUITEM(str, charset) \
{ \
	MENUITEM_ADD(optmenu_menu, menuitem, str, charset); \
}

	SET_MENUITEM(_("Automatic (Recommended)"),	 CS_AUTO);
	SET_MENUITEM(_("7bit ascii (US-ASCII)"),	 CS_US_ASCII);
#if HAVE_LIBJCONV
	SET_MENUITEM(_("Unicode (UTF-8)"),		 CS_UTF_8);
#endif
	SET_MENUITEM(_("Western European (ISO-8859-1)"), CS_ISO_8859_1);
	SET_MENUITEM(_("Central European (ISO-8859-2)"), CS_ISO_8859_2);
	SET_MENUITEM(_("Baltic (ISO-8859-13)"),		 CS_ISO_8859_13);
	SET_MENUITEM(_("Baltic (ISO-8859-4)"),		 CS_ISO_8859_4);
	SET_MENUITEM(_("Greek (ISO-8859-7)"),		 CS_ISO_8859_7);
	SET_MENUITEM(_("Turkish (ISO-8859-9)"),		 CS_ISO_8859_9);
#if HAVE_LIBJCONV
	SET_MENUITEM(_("Cyrillic (ISO-8859-5)"),	 CS_ISO_8859_5);
#endif
	SET_MENUITEM(_("Cyrillic (KOI8-R)"),		 CS_KOI8_R);
#if HAVE_LIBJCONV
	SET_MENUITEM(_("Cyrillic (Windows-1251)"),	 CS_CP1251);
	SET_MENUITEM(_("Cyrillic (KOI8-U)"),		 CS_KOI8_U);
#endif
	SET_MENUITEM(_("Japanese (ISO-2022-JP)"),	 CS_ISO_2022_JP);
#if 0
	SET_MENUITEM(_("Japanese (EUC-JP)"),		 CS_EUC_JP);
	SET_MENUITEM(_("Japanese (Shift_JIS)"),		 CS_SHIFT_JIS);
#endif /* 0 */
	SET_MENUITEM(_("Simplified Chinese (GB2312)"),	 CS_GB2312);
	SET_MENUITEM(_("Traditional Chinese (Big5)"),	 CS_BIG5);
#if 0
	SET_MENUITEM(_("Traditional Chinese (EUC-TW)"),  CS_EUC_TW);
	SET_MENUITEM(_("Chinese (ISO-2022-CN)"),	 CS_ISO_2022_CN);
#endif /* 0 */
	SET_MENUITEM(_("Korean (EUC-KR)"),		 CS_EUC_KR);
	SET_MENUITEM(_("Thai (TIS-620)"),		 CS_TIS_620);
	SET_MENUITEM(_("Thai (Windows-874)"),		 CS_WINDOWS_874);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (optmenu), optmenu_menu);

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

	label_charset_desc = gtk_label_new
		(_("If `Automatic' is selected, the optimal encoding\n"
		   "for the current locale will be used."));
	gtk_widget_show (label_charset_desc);
	gtk_box_pack_start (GTK_BOX (hbox1), label_charset_desc,
			    FALSE, FALSE, 0);
	gtk_label_set_justify(GTK_LABEL (label_charset_desc), GTK_JUSTIFY_LEFT);

	send.checkbtn_extsend = checkbtn_extsend;
	send.entry_extsend    = entry_extsend;
	/* send.button_extsend   = button_extsend; */

	send.checkbtn_savemsg  = checkbtn_savemsg;
	send.checkbtn_queuemsg = checkbtn_queuemsg;
	send.checkbtn_returnreceipt = checkbtn_returnreceipt;

	send.optmenu_charset = optmenu;
}

#if USE_PSPELL
static void prefs_dictionary_set_data_from_optmenu(PrefParam *param)
{
	gchar *str;
	gchar *dict_name;
	
	g_return_if_fail(param);
	g_return_if_fail(param->data);
	g_return_if_fail(param->widget);
	g_return_if_fail(*(param->widget));

	dict_name = gtkpspell_get_dictionary_menu_active_item
		(gtk_option_menu_get_menu(GTK_OPTION_MENU(*(param->widget))));
	str = *((gchar **) param->data);
	if (str)
		g_free(str);
	*((gchar **) param->data) = dict_name;
}

static void prefs_dictionary_set_optmenu(PrefParam *pparam)
{
	GList *cur;
	GtkOptionMenu *optmenu = GTK_OPTION_MENU(*pparam->widget);
	GtkWidget *menu;
	GtkWidget *menuitem;
	gchar *dict_name;
	gint n = 0;

	g_return_if_fail(optmenu != NULL);
	g_return_if_fail(pparam->data != NULL);

	if (*(gchar **) pparam->data) {
		menu = gtk_option_menu_get_menu(optmenu);
		for (cur = GTK_MENU_SHELL(menu)->children;
		     cur != NULL; cur = cur->next) {
			menuitem = GTK_WIDGET(cur->data);
			dict_name = gtk_object_get_data(GTK_OBJECT(menuitem), "dict_name");
			if (!strcmp2(dict_name, *((gchar **)pparam->data))) {
				gtk_option_menu_set_history(optmenu, n);
				return;
			}
			n++;
		}
	}		

	gtk_option_menu_set_history(optmenu, 0);
	prefs_dictionary_set_data_from_optmenu(pparam);
}

static void prefs_compose_checkbtn_enable_pspell_toggle_cb
	(GtkWidget *widget,
	 gpointer data)
{
	gboolean toggled;

	toggled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_widget_set_sensitive(compose.entry_pspell_path, toggled);
	gtk_widget_set_sensitive(compose.optmenu_dictionary, toggled);
	gtk_widget_set_sensitive(compose.btn_pspell_path, toggled);
}

static void prefs_compose_btn_pspell_path_clicked_cb(GtkWidget *widget,
						     gpointer data)
{
	gchar *file_path, *tmp;
	GtkWidget *new_menu;

	file_path = filesel_select_file(_("Select dictionaries location"),
					prefs_common.pspell_path);
	if (file_path == NULL) {
		/* don't change */	
	}
	else {
	  tmp=g_dirname(file_path);
	  
		if (prefs_common.pspell_path)
			g_free(prefs_common.pspell_path);
		prefs_common.pspell_path = g_strdup_printf("%s%s",tmp,G_DIR_SEPARATOR_S);

		new_menu = gtkpspell_dictionary_option_menu_new(prefs_common.pspell_path);
		gtk_option_menu_set_menu(GTK_OPTION_MENU(compose.optmenu_dictionary),
					 new_menu);

		gtk_entry_set_text(GTK_ENTRY(compose.entry_pspell_path), prefs_common.pspell_path);					 
		/* select first one */
		gtk_option_menu_set_history(GTK_OPTION_MENU(compose.optmenu_dictionary), 0);
		
		prefs_common.dictionary = gtkpspell_get_dictionary_menu_active_item(
				gtk_option_menu_get_menu(GTK_OPTION_MENU(compose.optmenu_dictionary)));
		g_free(tmp);
	

	}
}
#endif

static void prefs_compose_create(void)
{
	GtkWidget *vbox1;
        GtkWidget *vbox2;
        GtkWidget *vbox3;
	GtkWidget *hbox1;
	GtkWidget *hbox2;
	GtkWidget *btn_quotefmt;

	GtkWidget *frame_sig;
	GtkWidget *vbox_sig;
	GtkWidget *checkbtn_autosig;
	GtkWidget *label_sigsep;
	GtkWidget *entry_sigsep;

	GtkWidget *frame_editor;
	GtkWidget *hbox_editor;
	GtkWidget *checkbtn_autoextedit;

	GtkWidget *frame_autosel;
        GtkWidget *hbox_autosel;
        GtkWidget *vbox_autosel;
	GtkWidget *checkbtn_reply_account_autosel;
	GtkWidget *checkbtn_forward_account_autosel;
	GtkWidget *checkbtn_reedit_account_autosel;

        GtkWidget *vbox_linewrap;

 	GtkWidget *hbox3;
	GtkWidget *hbox4;
	GtkWidget *hbox5;
	GtkWidget *label_linewrap;
	GtkObject *spinbtn_linewrap_adj;
	GtkWidget *spinbtn_linewrap;
	GtkWidget *checkbtn_wrapquote;
	GtkWidget *checkbtn_wrapatsend;

	GtkWidget *frame_reply;
	GtkWidget *checkbtn_quote;
	GtkWidget *checkbtn_forward_as_attachment;
	GtkWidget *checkbtn_smart_wrapping;
	GtkWidget *checkbtn_block_cursor;

#if USE_PSPELL
	GtkWidget *frame_spell;
	GtkWidget *hbox_spell;
	GtkWidget *vbox_spell;
	GtkWidget *hbox_pspell_path;
	GtkWidget *checkbtn_enable_pspell;
	GtkWidget *label_pspell_path;
	GtkWidget *entry_pspell_path;
	GtkWidget *btn_pspell_path;
	GtkWidget *hbox_dictionary;
	GtkWidget *label_dictionary;
	GtkWidget *optmenu_dictionary;
#endif
	vbox1 = gtk_vbox_new (FALSE, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dialog.notebook), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);



	hbox1 = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox1);

	hbox2 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox2, FALSE, FALSE, 0);


	hbox1 = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

	PACK_FRAME(vbox1, frame_sig, _("Signature"));

	vbox_sig = gtk_vbox_new (FALSE, VSPACING_NARROW);
	gtk_widget_show (vbox_sig);
	gtk_container_add (GTK_CONTAINER (frame_sig), vbox_sig);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_sig), 8);

	PACK_CHECK_BUTTON (vbox_sig, checkbtn_autosig,
			   _("Insert signature automatically"));

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox_sig), hbox1, TRUE, TRUE, 0);
	label_sigsep = gtk_label_new (_("Signature separator"));
	gtk_widget_show (label_sigsep);
	gtk_box_pack_start (GTK_BOX (hbox1), label_sigsep, FALSE, FALSE, 0);

	entry_sigsep = gtk_entry_new ();
	gtk_widget_show (entry_sigsep);
	gtk_box_pack_start (GTK_BOX (hbox1), entry_sigsep, FALSE, FALSE, 0);

	gtk_widget_set_usize (entry_sigsep, 64, -1);

        /* Account autoselection */
	PACK_FRAME(vbox1, frame_autosel, _("Automatic Account Selection"));

	hbox_autosel = gtk_hbox_new (FALSE, VSPACING_NARROW);
	gtk_widget_show (hbox_autosel);
	gtk_container_add (GTK_CONTAINER (frame_autosel), hbox_autosel);
	gtk_container_set_border_width (GTK_CONTAINER (hbox_autosel), 8);

        PACK_CHECK_BUTTON (hbox_autosel, checkbtn_reply_account_autosel,
			   _("when replying"));
	PACK_CHECK_BUTTON (hbox_autosel, checkbtn_forward_account_autosel,
			   _("when forwarding"));
	PACK_CHECK_BUTTON (hbox_autosel, checkbtn_reedit_account_autosel,
			   _("when re-editing"));

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox2, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON (vbox2, checkbtn_autoextedit,
			   _("Automatically launch the external editor"));

	PACK_VSPACER (vbox2, vbox3, VSPACING_NARROW_2);

        /* line-wrapping */
	vbox_linewrap = gtk_vbox_new (FALSE, VSPACING_NARROW);
	gtk_widget_show (vbox_linewrap);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox_linewrap, FALSE, FALSE, 0);

	hbox3 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox3);
	gtk_box_pack_start (GTK_BOX (vbox_linewrap), hbox3, FALSE, FALSE, 0);


	label_linewrap = gtk_label_new (_("Wrap messages at"));
	gtk_widget_show (label_linewrap);
	gtk_box_pack_start (GTK_BOX (hbox3), label_linewrap, FALSE, FALSE, 0);

	spinbtn_linewrap_adj = gtk_adjustment_new (72, 20, 1024, 1, 10, 10);
	spinbtn_linewrap = gtk_spin_button_new
		(GTK_ADJUSTMENT (spinbtn_linewrap_adj), 1, 0);
	gtk_widget_show (spinbtn_linewrap);
	gtk_box_pack_start (GTK_BOX (hbox3), spinbtn_linewrap, FALSE, FALSE, 0);
	gtk_widget_set_usize (spinbtn_linewrap, 64, -1);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbtn_linewrap), TRUE);

	label_linewrap = gtk_label_new (_("characters"));
	gtk_widget_show (label_linewrap);
	gtk_box_pack_start (GTK_BOX (hbox3), label_linewrap, FALSE, FALSE, 0);

	PACK_VSPACER (vbox2, vbox3, VSPACING_NARROW_2);

	hbox4 = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox4);
	gtk_box_pack_start (GTK_BOX (vbox_linewrap), hbox4, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON (hbox4, checkbtn_wrapquote, _("Wrap quotation"));
	PACK_CHECK_BUTTON
		(hbox4, checkbtn_wrapatsend, _("Wrap before sending"));

	PACK_CHECK_BUTTON (vbox1, checkbtn_forward_as_attachment,
			   _("Forward as attachment"));

	PACK_CHECK_BUTTON (vbox1, checkbtn_smart_wrapping,
			   _("Smart wrapping (EXPERIMENTAL)"));
	
	PACK_CHECK_BUTTON (vbox1, checkbtn_block_cursor,
			  _("Block cursor"));

#if USE_PSPELL
	/* spell checker defaults */			   
	PACK_FRAME(vbox1, frame_spell, _("Global spelling checker settings"));
	vbox_spell = gtk_vbox_new(FALSE, VSPACING_NARROW);
	gtk_widget_show (vbox_spell);
	gtk_container_add(GTK_CONTAINER(frame_spell), vbox_spell);
	gtk_container_set_border_width(GTK_CONTAINER(vbox_spell), 8);

	PACK_CHECK_BUTTON(vbox_spell, checkbtn_enable_pspell, 
			  _("Enable spell checker (EXPERIMENTAL)"));

	gtk_signal_connect(GTK_OBJECT(checkbtn_enable_pspell), "toggled",
			   GTK_SIGNAL_FUNC(prefs_compose_checkbtn_enable_pspell_toggle_cb),
			   NULL);

	hbox_pspell_path = gtk_hbox_new (FALSE, 8);
	gtk_widget_show(hbox_pspell_path);
	gtk_box_pack_start(GTK_BOX(vbox_spell), hbox_pspell_path, TRUE, TRUE, 0);

	label_pspell_path = gtk_label_new (_("Dictionaries path"));
	gtk_widget_show(label_pspell_path);
	gtk_box_pack_start(GTK_BOX(hbox_pspell_path), label_pspell_path, FALSE, FALSE, 0);
	
	entry_pspell_path = gtk_entry_new();
	gtk_widget_show(entry_pspell_path);
	gtk_box_pack_start(GTK_BOX(hbox_pspell_path), entry_pspell_path, FALSE, FALSE, 0);
	gtk_widget_set_usize(entry_pspell_path, 150, -1);
	gtk_widget_set_sensitive(entry_pspell_path, prefs_common.enable_pspell);

	btn_pspell_path = gtk_button_new_with_label(_("..."));
	gtk_widget_show(btn_pspell_path);
	gtk_box_pack_start(GTK_BOX(hbox_pspell_path), btn_pspell_path, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(btn_pspell_path, prefs_common.enable_pspell);

	gtk_signal_connect(GTK_OBJECT(btn_pspell_path), "clicked", 
			   GTK_SIGNAL_FUNC(prefs_compose_btn_pspell_path_clicked_cb),
			   NULL);

	hbox_dictionary = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hbox_dictionary);
	gtk_box_pack_start(GTK_BOX(vbox_spell), hbox_dictionary, TRUE, TRUE, 0);

	label_dictionary = gtk_label_new(_("Dictionary"));
	gtk_widget_show(label_dictionary);
	gtk_box_pack_start(GTK_BOX(hbox_dictionary), label_dictionary, FALSE, FALSE, 0);

	optmenu_dictionary = gtk_option_menu_new();
	gtk_widget_show(optmenu_dictionary);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu_dictionary), 
				 gtkpspell_dictionary_option_menu_new(prefs_common.pspell_path));
	gtk_box_pack_start(GTK_BOX(hbox_dictionary), optmenu_dictionary, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(optmenu_dictionary, prefs_common.enable_pspell);
#endif

       /*
	compose.checkbtn_quote   = checkbtn_quote;
	compose.entry_quotemark  = entry_quotemark;
	compose.text_quotefmt    = text_quotefmt;
	*/
	compose.checkbtn_autosig = checkbtn_autosig;
	compose.entry_sigsep     = entry_sigsep;

	compose.checkbtn_autoextedit = checkbtn_autoextedit;

        compose.checkbtn_reply_account_autosel   = checkbtn_reply_account_autosel;
	compose.checkbtn_forward_account_autosel = checkbtn_forward_account_autosel;
	compose.checkbtn_reedit_account_autosel  = checkbtn_reedit_account_autosel;

	compose.spinbtn_linewrap     = spinbtn_linewrap;
	compose.spinbtn_linewrap_adj = spinbtn_linewrap_adj;
	compose.checkbtn_wrapquote   = checkbtn_wrapquote;
	compose.checkbtn_wrapatsend  = checkbtn_wrapatsend;

	compose.checkbtn_forward_as_attachment =
		checkbtn_forward_as_attachment;
	compose.checkbtn_smart_wrapping = 
		checkbtn_smart_wrapping;
	compose.checkbtn_block_cursor   =
		checkbtn_block_cursor;

#if USE_PSPELL
	compose.checkbtn_enable_pspell = checkbtn_enable_pspell;
	compose.entry_pspell_path      = entry_pspell_path;
	compose.btn_pspell_path	       = btn_pspell_path;
	compose.optmenu_dictionary     = optmenu_dictionary;
#endif
}

static void prefs_quote_create(void)
{
	GtkWidget *vbox1;
	GtkWidget *frame_quote;
	GtkWidget *vbox_quote;
	GtkWidget *hbox1;
	GtkWidget *hbox2;
	GtkWidget *label_quotemark;
	GtkWidget *entry_quotemark;
	GtkWidget *scrolledwin_quotefmt;
	GtkWidget *text_quotefmt;

	GtkWidget *entry_fw_quotemark;
	GtkWidget *text_fw_quotefmt;

	GtkWidget *btn_quotedesc;

	vbox1 = gtk_vbox_new (FALSE, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dialog.notebook), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	/* reply */

	PACK_FRAME (vbox1, frame_quote, _("Reply format"));

	vbox_quote = gtk_vbox_new (FALSE, VSPACING_NARROW);
	gtk_widget_show (vbox_quote);
	gtk_container_add (GTK_CONTAINER (frame_quote), vbox_quote);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_quote), 8);

	hbox1 = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox_quote), hbox1, FALSE, FALSE, 0);

	hbox2 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox2, FALSE, FALSE, 0);

	label_quotemark = gtk_label_new (_("Quotation mark"));
	gtk_widget_show (label_quotemark);
	gtk_box_pack_start (GTK_BOX (hbox2), label_quotemark, FALSE, FALSE, 0);

	entry_quotemark = gtk_entry_new ();
	gtk_widget_show (entry_quotemark);
	gtk_box_pack_start (GTK_BOX (hbox2), entry_quotemark, FALSE, FALSE, 0);
	gtk_widget_set_usize (entry_quotemark, 64, -1);

	scrolledwin_quotefmt = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwin_quotefmt);
	gtk_box_pack_start (GTK_BOX (vbox_quote), scrolledwin_quotefmt, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy
		(GTK_SCROLLED_WINDOW (scrolledwin_quotefmt),
		 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	text_quotefmt = gtk_text_new (NULL, NULL);
	gtk_widget_show (text_quotefmt);
	gtk_container_add(GTK_CONTAINER(scrolledwin_quotefmt), text_quotefmt);
	gtk_text_set_editable (GTK_TEXT (text_quotefmt), TRUE);
	gtk_widget_set_usize(text_quotefmt, -1, 60);

	/* forward */

	PACK_FRAME (vbox1, frame_quote, _("Forward format"));

	vbox_quote = gtk_vbox_new (FALSE, VSPACING_NARROW);
	gtk_widget_show (vbox_quote);
	gtk_container_add (GTK_CONTAINER (frame_quote), vbox_quote);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_quote), 8);

	hbox1 = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox_quote), hbox1, FALSE, FALSE, 0);

	hbox2 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox2, FALSE, FALSE, 0);

	label_quotemark = gtk_label_new (_("Quotation mark"));
	gtk_widget_show (label_quotemark);
	gtk_box_pack_start (GTK_BOX (hbox2), label_quotemark, FALSE, FALSE, 0);

	entry_fw_quotemark = gtk_entry_new ();
	gtk_widget_show (entry_fw_quotemark);
	gtk_box_pack_start (GTK_BOX (hbox2), entry_fw_quotemark,
			    FALSE, FALSE, 0);
	gtk_widget_set_usize (entry_fw_quotemark, 64, -1);

	scrolledwin_quotefmt = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwin_quotefmt);
	gtk_box_pack_start (GTK_BOX (vbox_quote), scrolledwin_quotefmt, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy
		(GTK_SCROLLED_WINDOW (scrolledwin_quotefmt),
		 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	text_fw_quotefmt = gtk_text_new (NULL, NULL);
	gtk_widget_show (text_fw_quotefmt);
	gtk_container_add(GTK_CONTAINER(scrolledwin_quotefmt),
			  text_fw_quotefmt);
	gtk_text_set_editable (GTK_TEXT (text_fw_quotefmt), TRUE);
	gtk_widget_set_usize(text_fw_quotefmt, -1, 60);

	hbox1 = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

	btn_quotedesc =
		gtk_button_new_with_label (_(" Description of symbols "));
	gtk_widget_show (btn_quotedesc);
	gtk_box_pack_start (GTK_BOX (hbox1), btn_quotedesc, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(btn_quotedesc), "clicked",
			   GTK_SIGNAL_FUNC(prefs_quote_description), NULL);

	quote.entry_quotemark    = entry_quotemark;
	quote.text_quotefmt      = text_quotefmt;
	quote.entry_fw_quotemark = entry_fw_quotemark;
	quote.text_fw_quotefmt   = text_fw_quotefmt;
}

static void prefs_display_create(void)
{
	GtkWidget *vbox1;
	GtkWidget *frame_font;
	GtkWidget *table1;
	GtkWidget *label_textfont;
	GtkWidget *entry_textfont;
	GtkWidget *button_textfont;
	GtkWidget *chkbtn_folder_unread;
	GtkWidget *chkbtn_transhdr;
	GtkWidget *frame_summary;
	GtkWidget *vbox2;
	GtkWidget *chkbtn_swapfrom;
	GtkWidget *chkbtn_hscrollbar;
	GtkWidget *chkbtn_useaddrbook;
	GtkWidget *chkbtn_expand_thread;
	GtkWidget *chkbtn_bold_unread;
	GtkWidget *vbox3;
	GtkWidget *hbox1;
	GtkWidget *label_datefmt;
	GtkWidget *button_datefmt;
	GtkWidget *entry_datefmt;
	GtkWidget *button_dispitem;
	GtkWidget *tmplabel, *tmpbutton, *tmpentry;

	vbox1 = gtk_vbox_new (FALSE, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dialog.notebook), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	PACK_FRAME(vbox1, frame_font, _("Font"));

	table1 = gtk_table_new (4, 3, FALSE);

	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (frame_font), table1);
	gtk_container_set_border_width (GTK_CONTAINER (table1), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table1), 8);
	gtk_table_set_col_spacings (GTK_TABLE (table1), 8);

	label_textfont = gtk_label_new (_("Text"));
	gtk_misc_set_alignment(GTK_MISC(label_textfont), 0, 0.5);
	gtk_widget_show (label_textfont);
	gtk_table_attach (GTK_TABLE (table1), label_textfont, 0, 1, 0, 1,
			  GTK_FILL, (GTK_EXPAND | GTK_FILL), 0, 0);

	entry_textfont = gtk_entry_new ();
	gtk_widget_show (entry_textfont);
	gtk_table_attach (GTK_TABLE (table1), entry_textfont, 1, 2, 0, 1,
			  (GTK_EXPAND | GTK_FILL), 0, 0, 0);

	button_textfont = gtk_button_new_with_label ("... ");

	gtk_widget_show (button_textfont);
	gtk_table_attach (GTK_TABLE (table1), button_textfont, 2, 3, 0, 1,
			  0, 0, 0, 0);
	gtk_signal_connect (GTK_OBJECT (button_textfont), "clicked",
			    GTK_SIGNAL_FUNC (prefs_font_select), entry_textfont);

	tmplabel = gtk_label_new (_("Small"));
	gtk_misc_set_alignment(GTK_MISC(tmplabel), 0, 0.5);
	gtk_widget_show (tmplabel);
	gtk_table_attach (GTK_TABLE (table1), tmplabel, 0, 1, 1, 2,
			  GTK_FILL, (GTK_EXPAND | GTK_FILL), 0, 0);

	tmpentry = gtk_entry_new ();
	gtk_widget_show (tmpentry);
	gtk_table_attach (GTK_TABLE (table1), tmpentry, 1, 2, 1, 2,
			  (GTK_EXPAND | GTK_FILL), 0, 0, 0);

	tmpbutton = gtk_button_new_with_label (" ... ");
	gtk_widget_show (tmpbutton);
	gtk_table_attach (GTK_TABLE (table1), tmpbutton, 2, 3, 1, 2,
			  0, 0, 0, 0);
	gtk_signal_connect (GTK_OBJECT(tmpbutton), "clicked",
			    GTK_SIGNAL_FUNC(prefs_font_select), tmpentry);
	display.entry_smallfont = tmpentry;			  

	tmplabel = gtk_label_new (_("Normal"));
	gtk_misc_set_alignment(GTK_MISC(tmplabel), 0, 0.5);
	gtk_widget_show (tmplabel);
	gtk_table_attach (GTK_TABLE (table1), tmplabel, 0, 1, 2, 3,
			  GTK_FILL, (GTK_EXPAND | GTK_FILL), 0, 0);

	tmpentry = gtk_entry_new ();
	gtk_widget_show (tmpentry);
	gtk_table_attach (GTK_TABLE (table1), tmpentry, 1, 2, 2, 3,
			  (GTK_EXPAND | GTK_FILL), 0, 0, 0);

	tmpbutton = gtk_button_new_with_label (" ... ");
	gtk_widget_show (tmpbutton);
	gtk_table_attach (GTK_TABLE (table1), tmpbutton, 2, 3, 2, 3,
			  0, 0, 0, 0);
	gtk_signal_connect (GTK_OBJECT(tmpbutton), "clicked",
				GTK_SIGNAL_FUNC(prefs_font_select), tmpentry);
	display.entry_normalfont = tmpentry;			  

	tmplabel = gtk_label_new (_("Bold"));
	gtk_misc_set_alignment(GTK_MISC(tmplabel), 0, 0.5);
	gtk_widget_show (tmplabel);
	gtk_table_attach (GTK_TABLE (table1), tmplabel, 0, 1, 3, 4,
			  GTK_FILL, (GTK_EXPAND | GTK_FILL), 0, 0);

	tmpentry = gtk_entry_new ();
	gtk_widget_show (tmpentry);
	gtk_table_attach (GTK_TABLE (table1), tmpentry, 1, 2, 3, 4,
			  (GTK_EXPAND | GTK_FILL), 0, 0, 0);

	tmpbutton = gtk_button_new_with_label (" ... ");
	gtk_widget_show (tmpbutton);
	gtk_table_attach (GTK_TABLE (table1), tmpbutton, 2, 3, 3, 4,
			  0, 0, 0, 0);
	gtk_signal_connect (GTK_OBJECT(tmpbutton), "clicked",
				GTK_SIGNAL_FUNC(prefs_font_select), tmpentry);
	display.entry_boldfont = tmpentry;

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox2, FALSE, TRUE, 0);

	PACK_CHECK_BUTTON
		(vbox2, chkbtn_transhdr,
		 _("Translate header name (such as `From:', `Subject:')"));

	PACK_CHECK_BUTTON (vbox2, chkbtn_folder_unread,
			   _("Display unread number next to folder name"));

	/* ---- Summary ---- */

	PACK_FRAME(vbox1, frame_summary, _("Summary View"));

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (frame_summary), vbox2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 8);

	PACK_CHECK_BUTTON
		(vbox2, chkbtn_swapfrom,
		 _("Display recipient on `From' column if sender is yourself"));
	PACK_CHECK_BUTTON
		(vbox2, chkbtn_useaddrbook,
		 _("Display sender using address book"));
	PACK_CHECK_BUTTON
		(vbox2, chkbtn_hscrollbar, _("Enable horizontal scroll bar"));
	PACK_CHECK_BUTTON
		(vbox2, chkbtn_expand_thread, _("Expand threads"));
	PACK_CHECK_BUTTON
		(vbox2, chkbtn_bold_unread,
		 _("Display unread messages with bold font"));

	PACK_VSPACER(vbox2, vbox3, VSPACING_NARROW_2);

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, TRUE, 0);

	label_datefmt = gtk_label_new (_("Date format"));
	gtk_widget_show (label_datefmt);
	gtk_box_pack_start (GTK_BOX (hbox1), label_datefmt, FALSE, FALSE, 0);

	entry_datefmt = gtk_entry_new ();
	gtk_widget_show (entry_datefmt);
	gtk_box_pack_start (GTK_BOX (hbox1), entry_datefmt, TRUE, TRUE, 0);

	button_datefmt = gtk_button_new_with_label ("... ");

	gtk_widget_show (button_datefmt);
	gtk_box_pack_start (GTK_BOX (hbox1), button_datefmt, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (button_datefmt), "clicked",
			    GTK_SIGNAL_FUNC (date_format_create), NULL);

	PACK_VSPACER(vbox2, vbox3, VSPACING_NARROW);

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, TRUE, 0);

	button_dispitem = gtk_button_new_with_label
		(_(" Set display item of summary... "));
	gtk_widget_show (button_dispitem);
	gtk_box_pack_start (GTK_BOX (hbox1), button_dispitem, FALSE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (button_dispitem), "clicked",
			    GTK_SIGNAL_FUNC (prefs_summary_column_open),
			    NULL);

	display.entry_textfont	= entry_textfont;
	display.button_textfont	= button_textfont;

	display.chkbtn_folder_unread = chkbtn_folder_unread;
	display.chkbtn_transhdr   = chkbtn_transhdr;

	display.chkbtn_swapfrom      = chkbtn_swapfrom;
	display.chkbtn_hscrollbar    = chkbtn_hscrollbar;
	display.chkbtn_expand_thread = chkbtn_expand_thread;
	display.chkbtn_bold_unread   = chkbtn_bold_unread;
	display.chkbtn_useaddrbook   = chkbtn_useaddrbook;
	display.entry_datefmt        = entry_datefmt;
}

static void prefs_message_create(void)
{
	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *hbox1;
	GtkWidget *chkbtn_enablecol;
	GtkWidget *button_edit_col;
	GtkWidget *chkbtn_mbalnum;
	GtkWidget *chkbtn_disphdrpane;
	GtkWidget *chkbtn_disphdr;
	GtkWidget *button_edit_disphdr;
	GtkWidget *hbox_linespc;
	GtkWidget *label_linespc;
	GtkObject *spinbtn_linespc_adj;
	GtkWidget *spinbtn_linespc;
	GtkWidget *chkbtn_headspc;

	GtkWidget *frame_scr;
	GtkWidget *vbox_scr;
	GtkWidget *chkbtn_smoothscroll;
	GtkWidget *hbox_scr;
	GtkWidget *label_scr;
	GtkObject *spinbtn_scrollstep_adj;
	GtkWidget *spinbtn_scrollstep;
	GtkWidget *chkbtn_halfpage;

	vbox1 = gtk_vbox_new (FALSE, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dialog.notebook), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, TRUE, 0);

	PACK_CHECK_BUTTON (hbox1, chkbtn_enablecol,
			   _("Enable coloration of message"));
	gtk_signal_connect (GTK_OBJECT (chkbtn_enablecol), "toggled",
						GTK_SIGNAL_FUNC (prefs_enable_message_color_toggled),
						NULL);

	button_edit_col = gtk_button_new_with_label (_(" Edit... "));
	gtk_widget_show (button_edit_col);
	gtk_box_pack_end (GTK_BOX (hbox1), button_edit_col, FALSE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (button_edit_col), "clicked",
			    GTK_SIGNAL_FUNC (prefs_quote_colors_dialog), NULL);

	SET_TOGGLE_SENSITIVITY(chkbtn_enablecol, button_edit_col);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox2, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON
		(vbox2, chkbtn_mbalnum,
		 _("Display 2-byte alphabet and numeric with 1-byte character"));
	PACK_CHECK_BUTTON(vbox2, chkbtn_disphdrpane,
			  _("Display header pane above message view"));

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, TRUE, 0);

	PACK_CHECK_BUTTON(hbox1, chkbtn_disphdr,
			  _("Display short headers on message view"));

	button_edit_disphdr = gtk_button_new_with_label (_(" Edit... "));
	gtk_widget_show (button_edit_disphdr);
	gtk_box_pack_end (GTK_BOX (hbox1), button_edit_disphdr,
			  FALSE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (button_edit_disphdr), "clicked",
			    GTK_SIGNAL_FUNC (prefs_display_header_open),
			    NULL);

	SET_TOGGLE_SENSITIVITY(chkbtn_disphdr, button_edit_disphdr);

	PACK_VSPACER(vbox2, vbox3, VSPACING_NARROW_2);

	hbox1 = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, TRUE, 0);

	hbox_linespc = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox_linespc, FALSE, TRUE, 0);

	label_linespc = gtk_label_new (_("Line space"));
	gtk_widget_show (label_linespc);
	gtk_box_pack_start (GTK_BOX (hbox_linespc), label_linespc,
			    FALSE, FALSE, 0);

	spinbtn_linespc_adj = gtk_adjustment_new (2, 0, 16, 1, 1, 16);
	spinbtn_linespc = gtk_spin_button_new
		(GTK_ADJUSTMENT (spinbtn_linespc_adj), 1, 0);
	gtk_widget_show (spinbtn_linespc);
	gtk_box_pack_start (GTK_BOX (hbox_linespc), spinbtn_linespc,
			    FALSE, FALSE, 0);
	gtk_widget_set_usize (spinbtn_linespc, 64, -1);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbtn_linespc), TRUE);

	label_linespc = gtk_label_new (_("pixel(s)"));
	gtk_widget_show (label_linespc);
	gtk_box_pack_start (GTK_BOX (hbox_linespc), label_linespc,
			    FALSE, FALSE, 0);

	PACK_CHECK_BUTTON(hbox1, chkbtn_headspc, _("Leave space on head"));

	PACK_FRAME(vbox1, frame_scr, _("Scroll"));

	vbox_scr = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox_scr);
	gtk_container_add (GTK_CONTAINER (frame_scr), vbox_scr);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_scr), 8);

	PACK_CHECK_BUTTON(vbox_scr, chkbtn_halfpage, _("Half page"));

	hbox1 = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox_scr), hbox1, FALSE, TRUE, 0);

	PACK_CHECK_BUTTON(hbox1, chkbtn_smoothscroll, _("Smooth scroll"));

	hbox_scr = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox_scr);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox_scr, FALSE, FALSE, 0);

	label_scr = gtk_label_new (_("Step"));
	gtk_widget_show (label_scr);
	gtk_box_pack_start (GTK_BOX (hbox_scr), label_scr, FALSE, FALSE, 0);

	spinbtn_scrollstep_adj = gtk_adjustment_new (1, 1, 100, 1, 10, 10);
	spinbtn_scrollstep = gtk_spin_button_new
		(GTK_ADJUSTMENT (spinbtn_scrollstep_adj), 1, 0);
	gtk_widget_show (spinbtn_scrollstep);
	gtk_box_pack_start (GTK_BOX (hbox_scr), spinbtn_scrollstep,
			    FALSE, FALSE, 0);
	gtk_widget_set_usize (spinbtn_scrollstep, 64, -1);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbtn_scrollstep),
				     TRUE);

	label_scr = gtk_label_new (_("pixel(s)"));
	gtk_widget_show (label_scr);
	gtk_box_pack_start (GTK_BOX (hbox_scr), label_scr, FALSE, FALSE, 0);

	SET_TOGGLE_SENSITIVITY (chkbtn_smoothscroll, hbox_scr)

	message.chkbtn_enablecol   = chkbtn_enablecol;
	message.button_edit_col    = button_edit_col;
	message.chkbtn_mbalnum     = chkbtn_mbalnum;
	message.chkbtn_disphdrpane = chkbtn_disphdrpane;
	message.chkbtn_disphdr     = chkbtn_disphdr;
	message.spinbtn_linespc    = spinbtn_linespc;
	message.chkbtn_headspc     = chkbtn_headspc;

	message.chkbtn_smoothscroll    = chkbtn_smoothscroll;
	message.spinbtn_scrollstep     = spinbtn_scrollstep;
	message.spinbtn_scrollstep_adj = spinbtn_scrollstep_adj;
	message.chkbtn_halfpage        = chkbtn_halfpage;
}

#if USE_GPGME
static void prefs_privacy_create(void)
{
	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *hbox1;
	GtkWidget *checkbtn_default_encrypt;
        GtkWidget *checkbtn_ascii_armored;
	GtkWidget *checkbtn_default_sign;
	GtkWidget *checkbtn_auto_check_signatures;
	GtkWidget *checkbtn_gpg_signature_popup;
	GtkWidget *checkbtn_passphrase_grab;
	GtkWidget *checkbtn_gpg_warning;
	GtkWidget *label;
	GtkWidget *menuitem;
	GtkWidget *optmenu;
	GtkWidget *optmenu_menu;

	vbox1 = gtk_vbox_new (FALSE, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dialog.notebook), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox2, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON (vbox2, checkbtn_default_encrypt,
			   _("Encrypt message by default"));

	PACK_CHECK_BUTTON (vbox2, checkbtn_ascii_armored,
			   _("Plain ASCII armored"));
	gtk_signal_connect(GTK_OBJECT(checkbtn_ascii_armored), "toggled",
				prefs_common_ascii_armored_warning, (gpointer)0);

	PACK_CHECK_BUTTON (vbox2, checkbtn_default_sign,
			   _("Sign message by default"));

	PACK_CHECK_BUTTON (vbox2, checkbtn_auto_check_signatures,
			   _("Automatically check signatures"));

	PACK_CHECK_BUTTON (vbox2, checkbtn_gpg_signature_popup,
			   _("Show signature check result in a popup window"));

#ifndef __MINGW32__
	PACK_CHECK_BUTTON (vbox2, checkbtn_passphrase_grab,
			   _("Grab input while entering a passphrase"));
#endif

	PACK_CHECK_BUTTON
		(vbox2, checkbtn_gpg_warning,
		 _("Display warning on startup if GnuPG doesn't work"));

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

	/* create default signkey box */
	label = gtk_label_new (_("Default Sign Key"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 0);
	optmenu = gtk_option_menu_new ();
	gtk_widget_show (optmenu);
	gtk_box_pack_start(GTK_BOX (hbox1), optmenu, FALSE, FALSE, 0);
	optmenu_menu = gtk_menu_new ();

	MENUITEM_ADD(optmenu_menu, menuitem, "Default Key", "def_key");
	MENUITEM_ADD(optmenu_menu, menuitem, "Second Key", "2nd_key");
	gtk_option_menu_set_menu (GTK_OPTION_MENU (optmenu), optmenu_menu);
	/* FIXME: disabled because not implemented */
	gtk_widget_set_sensitive(optmenu, FALSE);

	privacy.checkbtn_default_encrypt = checkbtn_default_encrypt;
        privacy.checkbtn_ascii_armored   = checkbtn_ascii_armored;
	privacy.checkbtn_default_sign    = checkbtn_default_sign;
	privacy.checkbtn_auto_check_signatures
					 = checkbtn_auto_check_signatures;
	privacy.checkbtn_gpg_signature_popup
					 = checkbtn_gpg_signature_popup;
	privacy.checkbtn_passphrase_grab = checkbtn_passphrase_grab;
	privacy.checkbtn_gpg_warning     = checkbtn_gpg_warning;
	privacy.optmenu_default_signkey  = optmenu;
}

static void
prefs_common_default_signkey_set_data_from_optmenu(PrefParam *pparam)
{
#if 0
	GtkWidget *menu;
	GtkWidget *menuitem;
	gchar *charset;

	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(*pparam->widget));
	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	charset = gtk_object_get_user_data(GTK_OBJECT(menuitem));
	g_free(*((gchar **)pparam->data));
	*((gchar **)pparam->data) = g_strdup(charset);
#endif
}

static void prefs_common_default_signkey_set_optmenu(PrefParam *pparam)
{
#if 0
	GList *cur;
	GtkOptionMenu *optmenu = GTK_OPTION_MENU(*pparam->widget);
	GtkWidget *menu;
	GtkWidget *menuitem;
	gchar *charset;
	gint n = 0;

	g_return_if_fail(optmenu != NULL);
	g_return_if_fail(*((gchar **)pparam->data) != NULL);

	menu = gtk_option_menu_get_menu(optmenu);
	for (cur = GTK_MENU_SHELL(menu)->children;
	     cur != NULL; cur = cur->next) {
		menuitem = GTK_WIDGET(cur->data);
		charset = gtk_object_get_user_data(GTK_OBJECT(menuitem));
		if (!strcmp(charset, *((gchar **)pparam->data))) {
			gtk_option_menu_set_history(optmenu, n);
			return;
		}
		n++;
	}

	gtk_option_menu_set_history(optmenu, 0);
	prefs_common_charset_set_data_from_optmenu(pparam);
#endif
}

static void prefs_common_ascii_armored_warning(GtkWidget* widget,
					       gpointer unused)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))
		&& gtk_notebook_get_current_page(GTK_NOTEBOOK(dialog.notebook))) {
		alertpanel_message(_("Warning - Privacy/Plain ASCII armored"),
			_("Its not recommend to use the old style plain ASCII\n"
			"armored mode for encypted messages. It doesn't comply\n"
			"with the RFC 3156 - MIME security with OpenPGP."));
	}
}
#endif /* USE_GPGME */

static void prefs_interface_create(void)
{
	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	/* GtkWidget *checkbtn_emacs; */
	GtkWidget *checkbtn_openunread;
	GtkWidget *checkbtn_openinbox;
	GtkWidget *checkbtn_immedexec;
	GtkWidget *hbox1;
	GtkWidget *label;
	GtkWidget *optmenu_recvdialog;
	GtkWidget *menu;
	GtkWidget *menuitem;

	GtkWidget *frame_addr;
        GtkWidget *vbox_addr;
	GtkWidget *checkbtn_addaddrbyclick;

	GtkWidget *frame_exit;
	GtkWidget *vbox_exit;
	GtkWidget *checkbtn_confonexit;
	GtkWidget *checkbtn_cleanonexit;
	GtkWidget *checkbtn_askonclean;
	GtkWidget *checkbtn_warnqueued;

 	GtkWidget *hbox2;
 	GtkWidget *optmenu_nextunreadmsgdialog;
 	GtkWidget *optmenu_nextunreadmsgdialog_menu;
 	GtkWidget *nextunreadmsgdialog_menuitem;

	vbox1 = gtk_vbox_new (FALSE, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dialog.notebook), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox2, FALSE, FALSE, 0);

	/* PACK_CHECK_BUTTON (vbox2, checkbtn_emacs,
			   _("Emulate the behavior of mouse operation of\n"
			     "Emacs-based mailer"));
	gtk_label_set_justify (GTK_LABEL (GTK_BIN (checkbtn_emacs)->child),
			       GTK_JUSTIFY_LEFT);   */

	PACK_CHECK_BUTTON
		(vbox2, checkbtn_openunread,
		 _("Open first unread message when entering a folder"));

	PACK_CHECK_BUTTON
		(vbox2, checkbtn_openinbox,
		 _("Go to inbox after receiving new mail"));

	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox3);
	gtk_box_pack_start (GTK_BOX (vbox2), vbox3, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON
		(vbox3, checkbtn_immedexec,
		 _("Execute immediately when moving or deleting messages"));

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox3), hbox1, FALSE, FALSE, 0);

	label = gtk_label_new
		(_("(Messages will be marked until execution\n"
		   " if this is turned off)"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 8);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

	PACK_VSPACER(vbox2, vbox3, VSPACING_NARROW);

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, FALSE, 0);

	label = gtk_label_new (_("Show receive dialog"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox1), label, FALSE, FALSE, 0);

	optmenu_recvdialog = gtk_option_menu_new ();
	gtk_widget_show (optmenu_recvdialog);
	gtk_box_pack_start (GTK_BOX (hbox1), optmenu_recvdialog,
			    FALSE, FALSE, 0);

	menu = gtk_menu_new ();
	MENUITEM_ADD (menu, menuitem, _("Always"), RECV_DIALOG_ALWAYS);
	MENUITEM_ADD (menu, menuitem, _("Only if a window is active"),
		      RECV_DIALOG_ACTIVE);
	MENUITEM_ADD (menu, menuitem, _("Never"), RECV_DIALOG_NEVER);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (optmenu_recvdialog), menu);

	PACK_FRAME (vbox1, frame_addr, _("Address book"));

	vbox_addr = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox_addr);
	gtk_container_add (GTK_CONTAINER (frame_addr), vbox_addr);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_addr), 8);

	PACK_CHECK_BUTTON
		(vbox_addr, checkbtn_addaddrbyclick,
		 _("Add address to destination when double-clicked"));

 	/* Next Unread Message Dialog */
 	hbox2 = gtk_hbox_new (FALSE, 8);
 	gtk_widget_show (hbox2);
 	gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 0);

 	label = gtk_label_new (_("Show no-unread-message dialog"));
 	gtk_widget_show (label);
 	gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);

 	optmenu_nextunreadmsgdialog = gtk_option_menu_new ();
 	gtk_widget_show (optmenu_nextunreadmsgdialog);
 	gtk_box_pack_start (GTK_BOX (hbox2), optmenu_nextunreadmsgdialog,
			    FALSE, FALSE, 0);

 	optmenu_nextunreadmsgdialog_menu = gtk_menu_new ();
 	MENUITEM_ADD (optmenu_nextunreadmsgdialog_menu, nextunreadmsgdialog_menuitem,
		      _("Always"),  NEXTUNREADMSGDIALOG_ALWAYS);
 	MENUITEM_ADD (optmenu_nextunreadmsgdialog_menu, nextunreadmsgdialog_menuitem,
		      _("Assume 'Yes'"),  NEXTUNREADMSGDIALOG_ASSUME_YES);
 	MENUITEM_ADD (optmenu_nextunreadmsgdialog_menu, nextunreadmsgdialog_menuitem,
		      _("Assume 'No'"), NEXTUNREADMSGDIALOG_ASSUME_NO);

 	gtk_option_menu_set_menu (GTK_OPTION_MENU (optmenu_nextunreadmsgdialog),
		      		  optmenu_nextunreadmsgdialog_menu);


	/* Receive Dialog */
/*	hbox = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);

	label = gtk_label_new (_("Show receive Dialog"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	recvdialog_optmenu = gtk_option_menu_new ();
	gtk_widget_show (recvdialog_optmenu);
	gtk_box_pack_start (GTK_BOX (hbox), recvdialog_optmenu, FALSE, FALSE, 0);

	recvdialog_optmenu_menu = gtk_menu_new ();

	MENUITEM_ADD (recvdialog_optmenu_menu, recvdialog_menuitem, _("Always"),  RECVDIALOG_ALWAYS);
	MENUITEM_ADD (recvdialog_optmenu_menu, recvdialog_menuitem, _("Only if a sylpheed window is active"),  RECVDIALOG_WINDOW_ACTIVE);
	MENUITEM_ADD (recvdialog_optmenu_menu, recvdialog_menuitem, _("Never"), RECVDIALOG_NEVER);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (recvdialog_optmenu), recvdialog_optmenu_menu);     */

	/* On Exit */
	PACK_FRAME (vbox1, frame_exit, _("On exit"));

	vbox_exit = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox_exit);
	gtk_container_add (GTK_CONTAINER (frame_exit), vbox_exit);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_exit), 8);

	PACK_CHECK_BUTTON (vbox_exit, checkbtn_confonexit,
			   _("Confirm on exit"));

	hbox1 = gtk_hbox_new (FALSE, 32);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox_exit), hbox1, FALSE, FALSE, 0);

	PACK_CHECK_BUTTON (hbox1, checkbtn_cleanonexit,
			   _("Empty trash on exit"));
	PACK_CHECK_BUTTON (hbox1, checkbtn_askonclean,
			   _("Ask before emptying"));
	SET_TOGGLE_SENSITIVITY (checkbtn_cleanonexit, checkbtn_askonclean);

	PACK_CHECK_BUTTON (vbox_exit, checkbtn_warnqueued,
			   _("Warn if there are queued messages"));

	/* interface.checkbtn_emacs          = checkbtn_emacs; */
	interface.checkbtn_openunread         = checkbtn_openunread;
	interface.checkbtn_openinbox          = checkbtn_openinbox;
	interface.checkbtn_immedexec          = checkbtn_immedexec;
	interface.optmenu_recvdialog	      = optmenu_recvdialog;
	interface.checkbtn_addaddrbyclick     = checkbtn_addaddrbyclick;
	interface.optmenu_nextunreadmsgdialog = optmenu_nextunreadmsgdialog;
	interface.checkbtn_confonexit         = checkbtn_confonexit;
	interface.checkbtn_cleanonexit        = checkbtn_cleanonexit;
	interface.checkbtn_askonclean         = checkbtn_askonclean;
	interface.checkbtn_warnqueued         = checkbtn_warnqueued;
}

static void prefs_other_create(void)
{
	GtkWidget *vbox1;
	GtkWidget *hbox1;

	GtkWidget *uri_frame;
	GtkWidget *uri_label;
	GtkWidget *uri_combo;
	GtkWidget *uri_entry;

	GtkWidget *print_frame;
	GtkWidget *printcmd_label;
	GtkWidget *printcmd_entry;

	GtkWidget *exteditor_frame;
	GtkWidget *exteditor_label;
	GtkWidget *exteditor_combo;
	GtkWidget *exteditor_entry;

	vbox1 = gtk_vbox_new (FALSE, VSPACING);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dialog.notebook), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), VBOX_BORDER);

	PACK_FRAME(vbox1, uri_frame,
		   _("External Web browser (%s will be replaced with URI)"));

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_container_add (GTK_CONTAINER (uri_frame), hbox1);
	gtk_container_set_border_width (GTK_CONTAINER (hbox1), 8);

	uri_label = gtk_label_new (_("Command"));
	gtk_widget_show(uri_label);
	gtk_box_pack_start (GTK_BOX (hbox1), uri_label, FALSE, TRUE, 0);

	uri_combo = gtk_combo_new ();
	gtk_widget_show (uri_combo);
	gtk_box_pack_start (GTK_BOX (hbox1), uri_combo, TRUE, TRUE, 0);
	gtkut_combo_set_items (GTK_COMBO (uri_combo),
			       "netscape -remote 'openURL(%s,raise)'",
			       "netscape '%s'",
			       "gnome-moz-remote --raise --newwin '%s'",
			       "kterm -e w3m '%s'",
			       "kterm -e lynx '%s'",
			       NULL);

	uri_entry = GTK_COMBO (uri_combo)->entry;

	PACK_FRAME(vbox1, print_frame,
		   _("Printing (%s will be replaced with file name)"));

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_container_add (GTK_CONTAINER (print_frame), hbox1);
	gtk_container_set_border_width (GTK_CONTAINER (hbox1), 8);

	printcmd_label = gtk_label_new (_("Command"));
	gtk_widget_show (printcmd_label);
	gtk_box_pack_start (GTK_BOX (hbox1), printcmd_label, FALSE, FALSE, 0);

	printcmd_entry = gtk_entry_new ();
	gtk_widget_show (printcmd_entry);
	gtk_box_pack_start (GTK_BOX (hbox1), printcmd_entry, TRUE, TRUE, 0);

	PACK_FRAME(vbox1, exteditor_frame,
		   _("External editor (%s will be replaced with file name)"));

	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_widget_show (hbox1);
	gtk_container_add (GTK_CONTAINER (exteditor_frame), hbox1);
	gtk_container_set_border_width (GTK_CONTAINER (hbox1), 8);

	exteditor_label = gtk_label_new (_("Command"));
	gtk_widget_show (exteditor_label);
	gtk_box_pack_start (GTK_BOX (hbox1), exteditor_label, FALSE, FALSE, 0);

	exteditor_combo = gtk_combo_new ();
	gtk_widget_show (exteditor_combo);
	gtk_box_pack_start (GTK_BOX (hbox1), exteditor_combo, TRUE, TRUE, 0);
	gtkut_combo_set_items (GTK_COMBO (exteditor_combo),
			       "gedit %s",
			       "kedit %s",
			       "mgedit --no-fork %s",
			       "emacs %s",
			       "xemacs %s",
			       "kterm -e jed %s",
			       "kterm -e vi %s",
			       NULL);
	exteditor_entry = GTK_COMBO (exteditor_combo)->entry;

	other.uri_combo = uri_combo;
	other.uri_entry = uri_entry;
	other.printcmd_entry = printcmd_entry;

	other.exteditor_combo = exteditor_combo;
	other.exteditor_entry = exteditor_entry;
}

static void date_format_ok_btn_clicked(GtkButton *button, GtkWidget **widget)
{
	GtkWidget *datefmt_sample = NULL;
	gchar *text;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(*widget != NULL);
	g_return_if_fail(display.entry_datefmt != NULL);

	datefmt_sample = GTK_WIDGET(gtk_object_get_data
				    (GTK_OBJECT(*widget), "datefmt_sample"));
	g_return_if_fail(datefmt_sample != NULL);

	text = gtk_editable_get_chars(GTK_EDITABLE(datefmt_sample), 0, -1);
	g_free(prefs_common.date_format);
	prefs_common.date_format = text;
	gtk_entry_set_text(GTK_ENTRY(display.entry_datefmt), text);

	gtk_widget_destroy(*widget);
	*widget = NULL;
}

static void date_format_cancel_btn_clicked(GtkButton *button,
					   GtkWidget **widget)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(*widget != NULL);

	gtk_widget_destroy(*widget);
	*widget = NULL;
}

static void date_format_key_pressed(GtkWidget *keywidget, GdkEventKey *event,
				    GtkWidget **widget)
{
	if (event && event->keyval == GDK_Escape)
		date_format_cancel_btn_clicked(NULL, widget);
}

static gboolean date_format_on_delete(GtkWidget *dialogwidget,
				      GdkEventAny *event, GtkWidget **widget)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(*widget != NULL, FALSE);

	*widget = NULL;
	return FALSE;
}

static void date_format_entry_on_change(GtkEditable *editable,
					GtkLabel *example)
{
	time_t cur_time;
	struct tm *cal_time;
	gchar buffer[100];
	gchar *text;

	cur_time = time(NULL);
	cal_time = localtime(&cur_time);
	buffer[0] = 0;
	text = gtk_editable_get_chars(editable, 0, -1);
	if (text)
		strftime(buffer, sizeof buffer, text, cal_time); 
	gtk_label_set_text(example, buffer);
}

static void date_format_select_row(GtkWidget *date_format_list, gint row,
				   gint column, GdkEventButton *event,
				   GtkWidget *date_format)
{
	gint cur_pos;
	gchar *format;
	gchar *old_format;
	gchar *new_format;
	GtkWidget *datefmt_sample;

	/* only on double click */
	if (!event || event->type != GDK_2BUTTON_PRESS) return;


	datefmt_sample = GTK_WIDGET(gtk_object_get_data
				    (GTK_OBJECT(date_format), "datefmt_sample"));

	g_return_if_fail(date_format_list != NULL);
	g_return_if_fail(date_format != NULL);
	g_return_if_fail(datefmt_sample != NULL);

	/* get format from clist */
	gtk_clist_get_text(GTK_CLIST(date_format_list), row, 0, &format);

	cur_pos = gtk_editable_get_position(GTK_EDITABLE(datefmt_sample));
	old_format = gtk_entry_get_text(GTK_ENTRY(datefmt_sample));

	/* insert the format into the text entry */
	new_format = g_malloc(strlen(old_format) + 3);

	strncpy(new_format, old_format, cur_pos);
	new_format[cur_pos] = '\0';
	strcat(new_format, format);
	strcat(new_format, &old_format[cur_pos]);

	gtk_entry_set_text(GTK_ENTRY(datefmt_sample), new_format);
	gtk_editable_set_position(GTK_EDITABLE(datefmt_sample), cur_pos + 2);

	g_free(new_format);
}

static GtkWidget *date_format_create(GtkButton *button, void *data)
{
	static GtkWidget *datefmt_win = NULL;
	GtkWidget *vbox1;
	GtkWidget *scrolledwindow1;
	GtkWidget *datefmt_clist;
	GtkWidget *table;
	GtkWidget *label1;
	GtkWidget *label2;
	GtkWidget *label3;
	GtkWidget *confirm_area;
	GtkWidget *ok_btn;
	GtkWidget *cancel_btn;
	GtkWidget *datefmt_entry;

	const struct {
		gchar *fmt;
		gchar *txt;
	} time_format[] = {
		{ "%a", _("the full abbreviated weekday name") },
		{ "%A", _("the full weekday name") },
		{ "%b", _("the abbreviated month name") },
		{ "%B", _("the full month name") },
		{ "%c", _("the preferred date and time for the current locale") },
		{ "%C", _("the century number (year/100)") },
		{ "%d", _("the day of the month as a decimal number") },
		{ "%H", _("the hour as a decimal number using a 24-hour clock") },
		{ "%I", _("the hour as a decimal number using a 12-hour clock") },
		{ "%j", _("the day of the year as a decimal number") },
		{ "%m", _("the month as a decimal number") },
		{ "%M", _("the minute as a decimal number") },
		{ "%p", _("either AM or PM") },
		{ "%S", _("the second as a decimal number") },
		{ "%w", _("the day of the week as a decimal number") },
		{ "%x", _("the preferred date for the current locale") },
		{ "%y", _("the last two digits of a year") },
		{ "%Y", _("the year as a decimal number") },
		{ "%Z", _("the time zone or name or abbreviation") }
	};

	gchar *titles[2];
	gint i;
	const gint TIME_FORMAT_ELEMS =
		sizeof time_format / sizeof time_format[0];

	if (datefmt_win) return datefmt_win;

	datefmt_win = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_container_set_border_width(GTK_CONTAINER(datefmt_win), 8);
	gtk_window_set_title(GTK_WINDOW(datefmt_win), _("Date format"));
	gtk_window_set_position(GTK_WINDOW(datefmt_win), GTK_WIN_POS_CENTER);
	gtk_widget_set_usize(datefmt_win, 440, 280);

	vbox1 = gtk_vbox_new(FALSE, 10);
	gtk_widget_show(vbox1);
	gtk_container_add(GTK_CONTAINER(datefmt_win), vbox1);

	scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy
		(GTK_SCROLLED_WINDOW(scrolledwindow1),
		 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scrolledwindow1);
	gtk_box_pack_start(GTK_BOX(vbox1), scrolledwindow1, TRUE, TRUE, 0);

	titles[0] = _("Specifier");
	titles[1] = _("Description");
	datefmt_clist = gtk_clist_new_with_titles(2, titles);
	gtk_widget_show(datefmt_clist);
	gtk_container_add(GTK_CONTAINER(scrolledwindow1), datefmt_clist);
/*	gtk_clist_set_column_width(GTK_CLIST(datefmt_clist), 0, 80);   */

	gtk_clist_set_selection_mode(GTK_CLIST(datefmt_clist),
				     GTK_SELECTION_BROWSE);

	for (i = 0; i < TIME_FORMAT_ELEMS; i++) {
		gchar *text[2];
		/* phoney casting necessary because of gtk... */
		text[0] = (gchar *)time_format[i].fmt;
		text[1] = (gchar *)time_format[i].txt;
		gtk_clist_append(GTK_CLIST(datefmt_clist), text);
	}

	table = gtk_table_new(2, 2, FALSE);
	gtk_widget_show(table);
	gtk_box_pack_start(GTK_BOX(vbox1), table, FALSE, FALSE, 0);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 8);

	label1 = gtk_label_new(_("Date format"));
	gtk_widget_show(label1);
	gtk_table_attach(GTK_TABLE(table), label1, 0, 1, 0, 1,
			 GTK_FILL, 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label1), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);

	datefmt_entry = gtk_entry_new_with_max_length(256);
	gtk_widget_show(datefmt_entry);
	gtk_table_attach(GTK_TABLE(table), datefmt_entry, 1, 2, 0, 1,
			 (GTK_EXPAND | GTK_FILL), 0, 0, 0);

	/* we need the "sample" entry box; add it as data so callbacks can
	 * get the entry box */
	gtk_object_set_data(GTK_OBJECT(datefmt_win), "datefmt_sample",
			    datefmt_entry);

	label2 = gtk_label_new(_("Example"));
	gtk_widget_show(label2);
	gtk_table_attach(GTK_TABLE(table), label2, 0, 1, 1, 2,
			 GTK_FILL, 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label2), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label2), 0, 0.5);

	label3 = gtk_label_new("");
	gtk_widget_show(label3);
	gtk_table_attach(GTK_TABLE(table), label3, 1, 2, 1, 2,
			 (GTK_EXPAND | GTK_FILL), 0, 0, 0);
	gtk_label_set_justify(GTK_LABEL(label3), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label3), 0, 0.5);

	gtkut_button_set_create(&confirm_area, &ok_btn, _("OK"),
				&cancel_btn, _("Cancel"), NULL, NULL);
	gtk_widget_grab_default(ok_btn);
	gtk_widget_show(confirm_area);

	gtk_box_pack_start(GTK_BOX(vbox1), confirm_area, FALSE, FALSE, 0);

	/* set the current format */
	gtk_entry_set_text(GTK_ENTRY(datefmt_entry), prefs_common.date_format);
	date_format_entry_on_change(GTK_EDITABLE(datefmt_entry),
				    GTK_LABEL(label3));

	gtk_signal_connect(GTK_OBJECT(ok_btn), "clicked",
			   GTK_SIGNAL_FUNC(date_format_ok_btn_clicked),
			   &datefmt_win);
	gtk_signal_connect(GTK_OBJECT(cancel_btn), "clicked",
			   GTK_SIGNAL_FUNC(date_format_cancel_btn_clicked),
			   &datefmt_win);
	gtk_signal_connect(GTK_OBJECT(datefmt_win), "key_press_event",
			   GTK_SIGNAL_FUNC(date_format_key_pressed),
			   &datefmt_win);
	gtk_signal_connect(GTK_OBJECT(datefmt_win), "delete_event",
			   GTK_SIGNAL_FUNC(date_format_on_delete),
			   &datefmt_win);
	gtk_signal_connect(GTK_OBJECT(datefmt_entry), "changed",
			   GTK_SIGNAL_FUNC(date_format_entry_on_change),
			   label3);

	gtk_signal_connect(GTK_OBJECT(datefmt_clist), "select_row",
			   GTK_SIGNAL_FUNC(date_format_select_row),
			   datefmt_win);

	gtk_window_set_position(GTK_WINDOW(datefmt_win), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(datefmt_win), TRUE);

	gtk_widget_show(datefmt_win);
	manage_window_set_transient(GTK_WINDOW(datefmt_win));

	gtk_widget_grab_focus(ok_btn);

	return datefmt_win;
}

void prefs_quote_colors_dialog(void)
{
	if (!quote_color_win)
		prefs_quote_colors_dialog_create();
	gtk_widget_show(quote_color_win);
	manage_window_set_transient(GTK_WINDOW(quote_color_win));

	gtk_main();
	gtk_widget_hide(quote_color_win);

	textview_update_message_colors();
	main_window_reflect_prefs_all();
}

static void prefs_quote_colors_dialog_create(void)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget *quotelevel1_label;
	GtkWidget *quotelevel2_label;
	GtkWidget *quotelevel3_label;
	GtkWidget *uri_label;
	GtkWidget *tgt_folder_label;
	GtkWidget *hbbox;
	GtkWidget *ok_btn;
	//GtkWidget *cancel_btn;
	GtkWidget *recycle_colors_btn;
	GtkWidget *frame_colors;

	window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_container_set_border_width(GTK_CONTAINER(window), 2);
	gtk_window_set_title(GTK_WINDOW(window), _("Set message colors"));
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_window_set_policy(GTK_WINDOW(window), FALSE, FALSE, FALSE);

	vbox = gtk_vbox_new (FALSE, VSPACING);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
	PACK_FRAME(vbox, frame_colors, _("Colors"));

	table = gtk_table_new (4, 2, FALSE);
	gtk_container_add (GTK_CONTAINER (frame_colors), table);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 2);
	gtk_table_set_col_spacings (GTK_TABLE (table), 5);


	color_buttons.quote_level1_btn = gtk_button_new();
	gtk_table_attach (GTK_TABLE (table), color_buttons.quote_level1_btn,
			  0, 1, 0, 1, 0, 0, 0, 0);
	gtk_widget_set_usize (color_buttons.quote_level1_btn, 40, 30);
	gtk_container_set_border_width
		(GTK_CONTAINER (color_buttons.quote_level1_btn), 5);

	color_buttons.quote_level2_btn = gtk_button_new();
	gtk_table_attach (GTK_TABLE (table), color_buttons.quote_level2_btn,
			  0, 1, 1, 2, 0, 0, 0, 0);
	gtk_widget_set_usize (color_buttons.quote_level2_btn, 40, 30);
	gtk_container_set_border_width (GTK_CONTAINER (color_buttons.quote_level2_btn), 5);

	color_buttons.quote_level3_btn = gtk_button_new_with_label ("");
	gtk_table_attach (GTK_TABLE (table), color_buttons.quote_level3_btn,
			  0, 1, 2, 3, 0, 0, 0, 0);
	gtk_widget_set_usize (color_buttons.quote_level3_btn, 40, 30);
	gtk_container_set_border_width
		(GTK_CONTAINER (color_buttons.quote_level3_btn), 5);

	color_buttons.uri_btn = gtk_button_new_with_label ("");
	gtk_table_attach (GTK_TABLE (table), color_buttons.uri_btn,
			  0, 1, 3, 4, 0, 0, 0, 0);
	gtk_widget_set_usize (color_buttons.uri_btn, 40, 30);
	gtk_container_set_border_width (GTK_CONTAINER (color_buttons.uri_btn), 5);

	color_buttons.tgt_folder_btn = gtk_button_new_with_label ("");
	gtk_table_attach (GTK_TABLE (table), color_buttons.tgt_folder_btn,
			  0, 1, 4, 5, 0, 0, 0, 0);
	gtk_widget_set_usize (color_buttons.tgt_folder_btn, 40, 30);
	gtk_container_set_border_width (GTK_CONTAINER (color_buttons.tgt_folder_btn), 5);

	quotelevel1_label = gtk_label_new (_("Quoted Text - First Level"));
	gtk_table_attach (GTK_TABLE (table), quotelevel1_label, 1, 2, 0, 1,
			  (GTK_EXPAND | GTK_FILL), 0, 0, 0);
	gtk_label_set_justify (GTK_LABEL (quotelevel1_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (quotelevel1_label), 0, 0.5);

	quotelevel2_label = gtk_label_new (_("Quoted Text - Second Level"));
	gtk_table_attach (GTK_TABLE (table), quotelevel2_label, 1, 2, 1, 2,
			  (GTK_EXPAND | GTK_FILL), 0, 0, 0);
	gtk_label_set_justify (GTK_LABEL (quotelevel2_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (quotelevel2_label), 0, 0.5);

	quotelevel3_label = gtk_label_new (_("Quoted Text - Third Level"));
	gtk_table_attach (GTK_TABLE (table), quotelevel3_label, 1, 2, 2, 3,
			  (GTK_EXPAND | GTK_FILL), 0, 0, 0);
	gtk_label_set_justify (GTK_LABEL (quotelevel3_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (quotelevel3_label), 0, 0.5);

	uri_label = gtk_label_new (_("URI link"));
	gtk_table_attach (GTK_TABLE (table), uri_label, 1, 2, 3, 4,
			  (GTK_EXPAND | GTK_FILL), 0, 0, 0);
	gtk_label_set_justify (GTK_LABEL (uri_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (uri_label), 0, 0.5);

	tgt_folder_label = gtk_label_new (_("Target folder"));
	gtk_table_attach (GTK_TABLE (table), tgt_folder_label, 1, 2, 4, 5,
			  (GTK_EXPAND | GTK_FILL), 0, 0, 0);
	gtk_label_set_justify (GTK_LABEL (tgt_folder_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (tgt_folder_label), 0, 0.5);

	PACK_CHECK_BUTTON (vbox, recycle_colors_btn,
			   _("Recycle quote colors"));

	gtkut_button_set_create(&hbbox, &ok_btn, _("OK"),
				NULL, NULL, NULL, NULL);
	gtk_box_pack_end(GTK_BOX(vbox), hbbox, FALSE, FALSE, 0);

	gtk_widget_grab_default(ok_btn);
	gtk_signal_connect(GTK_OBJECT(window), "focus_in_event",
			   GTK_SIGNAL_FUNC(manage_window_focus_in), NULL);
	gtk_signal_connect(GTK_OBJECT(window), "focus_out_event",
			   GTK_SIGNAL_FUNC(manage_window_focus_out), NULL);
	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
			   GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
	gtk_signal_connect(GTK_OBJECT(window), "key_press_event",
			   GTK_SIGNAL_FUNC(prefs_quote_colors_key_pressed),
			   NULL);

	gtk_signal_connect(GTK_OBJECT(color_buttons.quote_level1_btn), "clicked",
			   GTK_SIGNAL_FUNC(quote_color_set_dialog), "LEVEL1");
	gtk_signal_connect(GTK_OBJECT(color_buttons.quote_level2_btn), "clicked",
			   GTK_SIGNAL_FUNC(quote_color_set_dialog), "LEVEL2");
	gtk_signal_connect(GTK_OBJECT(color_buttons.quote_level3_btn), "clicked",
			   GTK_SIGNAL_FUNC(quote_color_set_dialog), "LEVEL3");
	gtk_signal_connect(GTK_OBJECT(color_buttons.uri_btn), "clicked",
			   GTK_SIGNAL_FUNC(quote_color_set_dialog), "URI");
	gtk_signal_connect(GTK_OBJECT(color_buttons.tgt_folder_btn), "clicked",
			   GTK_SIGNAL_FUNC(quote_color_set_dialog), "TGTFLD");
	gtk_signal_connect(GTK_OBJECT(recycle_colors_btn), "toggled",
			   GTK_SIGNAL_FUNC(prefs_recycle_colors_toggled), NULL);
	gtk_signal_connect(GTK_OBJECT(ok_btn), "clicked",
			   GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

	/* show message button colors and recycle options */
	set_button_bg_color(color_buttons.quote_level1_btn,
			    prefs_common.quote_level1_col);
	set_button_bg_color(color_buttons.quote_level2_btn,
			    prefs_common.quote_level2_col);
	set_button_bg_color(color_buttons.quote_level3_btn,
			    prefs_common.quote_level3_col);
	set_button_bg_color(color_buttons.uri_btn,
			    prefs_common.uri_col);
	set_button_bg_color(color_buttons.tgt_folder_btn,
			    prefs_common.tgt_folder_col);
	gtk_toggle_button_set_active((GtkToggleButton *)recycle_colors_btn,
				     prefs_common.recycle_quote_colors);

	gtk_widget_show_all(vbox);
	quote_color_win = window;
}

static void prefs_quote_colors_key_pressed(GtkWidget *widget,
					   GdkEventKey *event, gpointer data)
{
	if (event && event->keyval == GDK_Escape)
		gtk_main_quit();
}

static void quote_color_set_dialog(GtkWidget *widget, gpointer data)
{
	gchar *type = (gchar *)data;
	gchar *title = NULL;
	gdouble color[4] = {0.0, 0.0, 0.0, 0.0};
	gint rgbvalue = 0;
	GtkColorSelectionDialog *dialog;

	if(g_strcasecmp(type, "LEVEL1") == 0) {
		title = _("Pick color for quotation level 1");
		rgbvalue = prefs_common.quote_level1_col;
	} else if(g_strcasecmp(type, "LEVEL2") == 0) {
		title = _("Pick color for quotation level 2");
		rgbvalue = prefs_common.quote_level2_col;
	} else if(g_strcasecmp(type, "LEVEL3") == 0) {
		title = _("Pick color for quotation level 3");
		rgbvalue = prefs_common.quote_level3_col;
	} else if(g_strcasecmp(type, "URI") == 0) {
		title = _("Pick color for URI");
		rgbvalue = prefs_common.uri_col;
	} else if(g_strcasecmp(type, "TGTFLD") == 0) {
		title = _("Pick color for target folder");
		rgbvalue = prefs_common.tgt_folder_col;
	} else {   /* Should never be called */
		g_warning("Unrecognized datatype '%s' in quote_color_set_dialog\n", type);
		return;
	}

	color_dialog = gtk_color_selection_dialog_new(title);
	gtk_window_set_position(GTK_WINDOW(color_dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(color_dialog), TRUE);
	gtk_window_set_policy(GTK_WINDOW(color_dialog), FALSE, FALSE, FALSE);
	manage_window_set_transient(GTK_WINDOW(color_dialog));

	gtk_signal_connect(GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(color_dialog)->ok_button),
			   "clicked", GTK_SIGNAL_FUNC(quote_colors_set_dialog_ok), data);
	gtk_signal_connect(GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(color_dialog)->cancel_button),
			   "clicked", GTK_SIGNAL_FUNC(quote_colors_set_dialog_cancel), data);
	gtk_signal_connect(GTK_OBJECT(color_dialog), "key_press_event",
			   GTK_SIGNAL_FUNC(quote_colors_set_dialog_key_pressed),
			   data);

	/* preselect the previous color in the color selection dialog */
	color[0] = (gdouble) ((rgbvalue & 0xff0000) >> 16) / 255.0;
	color[1] = (gdouble) ((rgbvalue & 0x00ff00) >>  8) / 255.0;
	color[2] = (gdouble)  (rgbvalue & 0x0000ff)        / 255.0;
	dialog = GTK_COLOR_SELECTION_DIALOG(color_dialog);
	gtk_color_selection_set_color
		(GTK_COLOR_SELECTION(dialog->colorsel), color);

	gtk_widget_show(color_dialog);
}

static void quote_colors_set_dialog_ok(GtkWidget *widget, gpointer data)
{
	GtkColorSelection *colorsel = (GtkColorSelection *)
						((GtkColorSelectionDialog *)color_dialog)->colorsel;
	gdouble color[4];
	gint red, green, blue, rgbvalue;
	gchar *type = (gchar *)data;

	gtk_color_selection_get_color(colorsel, color);

	red      = (gint) (color[0] * 255.0);
	green    = (gint) (color[1] * 255.0);
	blue     = (gint) (color[2] * 255.0);
	rgbvalue = (gint) ((red * 0x10000) | (green * 0x100) | blue);

#if 0
	fprintf(stderr, "redc = %f, greenc = %f, bluec = %f\n", color[0], color[1], color[2]);
	fprintf(stderr, "red = %d, green = %d, blue = %d\n", red, green, blue);
	fprintf(stderr, "Color is %x\n", rgbvalue);
#endif

	if (g_strcasecmp(type, "LEVEL1") == 0) {
		prefs_common.quote_level1_col = rgbvalue;
		set_button_bg_color(color_buttons.quote_level1_btn, rgbvalue);
	} else if (g_strcasecmp(type, "LEVEL2") == 0) {
		prefs_common.quote_level2_col = rgbvalue;
		set_button_bg_color(color_buttons.quote_level2_btn, rgbvalue);
	} else if (g_strcasecmp(type, "LEVEL3") == 0) {
		prefs_common.quote_level3_col = rgbvalue;
		set_button_bg_color(color_buttons.quote_level3_btn, rgbvalue);
	} else if (g_strcasecmp(type, "URI") == 0) {
		prefs_common.uri_col = rgbvalue;
		set_button_bg_color(color_buttons.uri_btn, rgbvalue);
	} else if (g_strcasecmp(type, "TGTFLD") == 0) {
		prefs_common.tgt_folder_col = rgbvalue;
		set_button_bg_color(color_buttons.tgt_folder_btn, rgbvalue);
		folderview_set_target_folder_color(prefs_common.tgt_folder_col);
	} else
		fprintf( stderr, "Unrecognized datatype '%s' in quote_color_set_dialog_ok\n", type );

	gtk_widget_destroy(color_dialog);
}

static void quote_colors_set_dialog_cancel(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(color_dialog);
}

static void quote_colors_set_dialog_key_pressed(GtkWidget *widget,
						GdkEventKey *event,
						gpointer data)
{
	gtk_widget_destroy(color_dialog);
}

static void set_button_bg_color(GtkWidget *widget, gint rgbvalue)
{
	GtkStyle *newstyle;
	GdkColor color;

	gtkut_convert_int_to_gdk_color(rgbvalue, &color);
	newstyle = gtk_style_copy(gtk_widget_get_default_style());
	newstyle->bg[GTK_STATE_NORMAL]   = color;
	newstyle->bg[GTK_STATE_PRELIGHT] = color;
	newstyle->bg[GTK_STATE_ACTIVE]   = color;

	gtk_widget_set_style(GTK_WIDGET(widget), newstyle);
}

static void prefs_enable_message_color_toggled(void)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON(message.chkbtn_enablecol));
	gtk_widget_set_sensitive(message.button_edit_col, is_active);
	prefs_common.enable_color = is_active;
}

static void prefs_recycle_colors_toggled(GtkWidget *widget)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	prefs_common.recycle_quote_colors = is_active;
}

void prefs_quote_description(void)
{
	if (!quote_desc_win)
		prefs_quote_description_create();

	manage_window_set_transient(GTK_WINDOW(quote_desc_win));
	gtk_widget_show(quote_desc_win);
	gtk_main();
	gtk_widget_hide(quote_desc_win);
}

static void prefs_quote_description_create(void)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *hbbox;
	GtkWidget *label;
	GtkWidget *ok_btn;

	quote_desc_win = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(quote_desc_win),
			     _("Description of symbols"));
	gtk_container_set_border_width(GTK_CONTAINER(quote_desc_win), 8);
	gtk_window_set_position(GTK_WINDOW(quote_desc_win), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(quote_desc_win), TRUE);
	gtk_window_set_policy(GTK_WINDOW(quote_desc_win), FALSE, TRUE, FALSE);

	vbox = gtk_vbox_new(FALSE, 8);
	gtk_container_add(GTK_CONTAINER(quote_desc_win), vbox);

	hbox = gtk_hbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	label = gtk_label_new
		("SYMBOL\n\n"
                 "%d\n"		/* date */
		 "%f\n"		/* from */
		 "%N\n"		/* full name of sender */
		 "%F\n"		/* first name of sender */
		 "%I\n"		/* initial of sender */
		 "%s\n"		/* subject */
		 "%t\n"		/* to */
		 "%c\n"		/* cc */
		 "%n\n"		/* newsgroups */
		 "%i\n"		/* message id */
		 "%r\n"		/* references */
		 "\n"
		 "%x\n"
		 "?x(expr)\n"	/* condition */
		 "\n"
		 "%M\n"		/* message body */
		 "%Q\n"		/* quoted message body */
		 "%m\n"		/* message body without signature */
		 "%q\n"		/* quoted message body without signature */
		 "%%");		/* literal percent */

	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	label = gtk_label_new
		(_("DESCRIPTION\n\n"
                   "Date\n"
		   "From\n"
		   "Full Name of Sender\n"
		   "First Name of Sender\n"
		   "Initial of Sender\n"
		   "Subject\n"
		   "To\n"
		   "Cc\n"
		   "Newsgroups\n"
		   "Message-ID\n"
		   "References\n"
		   "\n"
		   "Display the information\n"
		   "If the information x is set, displays expr\n"
		   "\n"
		   "Message body\n"
		   "Quoted message body\n"
		   "Message body without signature\n"
		   "Quoted message body without signature\n"
		   "Literal %"));

	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	gtkut_button_set_create(&hbbox, &ok_btn, _("OK"),
				NULL, NULL, NULL, NULL);
	gtk_box_pack_end(GTK_BOX(vbox), hbbox, FALSE, FALSE, 0);

	gtk_widget_grab_default(ok_btn);
	gtk_signal_connect(GTK_OBJECT(ok_btn), "clicked",
			   GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
	gtk_signal_connect
		(GTK_OBJECT(quote_desc_win), "key_press_event",
		 GTK_SIGNAL_FUNC(prefs_quote_description_key_pressed),
		 NULL);
	gtk_signal_connect(GTK_OBJECT(quote_desc_win), "delete_event",
			   GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

	gtk_widget_show_all(vbox);
}

static void prefs_quote_description_key_pressed(GtkWidget *widget,
						GdkEventKey *event,
						gpointer data)
{
	if (event && event->keyval == GDK_Escape)
		gtk_main_quit();
}

static void prefs_font_select(GtkButton *button, GtkEntry *entry)
{
	gchar *font_name;
	
	g_return_if_fail(entry != NULL);
	
	if (!font_sel_win) {
		font_sel_win = gtk_font_selection_dialog_new
			(_("Font selection"));
		gtk_window_position(GTK_WINDOW(font_sel_win),
				    GTK_WIN_POS_CENTER);
		gtk_signal_connect(GTK_OBJECT(font_sel_win), "delete_event",
				   GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete),
				   NULL);
		gtk_signal_connect
			(GTK_OBJECT(font_sel_win), "key_press_event",
			 GTK_SIGNAL_FUNC(prefs_font_selection_key_pressed),
			 NULL);
		gtk_signal_connect_object
			(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(font_sel_win)->cancel_button),
			 "clicked",
			 GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete),
			 GTK_OBJECT(font_sel_win));
	}

	if(font_sel_conn_id) {
		gtk_signal_disconnect(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(font_sel_win)->ok_button), font_sel_conn_id);
	}
	font_sel_conn_id = gtk_signal_connect
		(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(font_sel_win)->ok_button),
	         "clicked",
		 GTK_SIGNAL_FUNC(prefs_font_selection_ok),
		 entry);
	printf("%i\n", font_sel_conn_id);

	font_name = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(font_sel_win), font_name);
	g_free(font_name);
	manage_window_set_transient(GTK_WINDOW(font_sel_win));
	gtk_window_set_modal(GTK_WINDOW(font_sel_win), TRUE);
	gtk_widget_grab_focus
		(GTK_FONT_SELECTION_DIALOG(font_sel_win)->ok_button);
	gtk_widget_show(font_sel_win);
}

static void prefs_font_selection_key_pressed(GtkWidget *widget,
					     GdkEventKey *event,
					     gpointer data)
{
	if (event && event->keyval == GDK_Escape)
		gtk_widget_hide(font_sel_win);
}

static void prefs_font_selection_ok(GtkButton *button, GtkEntry *entry)
{
	gchar *fontname;

	fontname = gtk_font_selection_dialog_get_font_name
		(GTK_FONT_SELECTION_DIALOG(font_sel_win));

	if (fontname) {
		gtk_entry_set_text(entry, fontname);

		g_free(fontname);
	}

	gtk_widget_hide(font_sel_win);
}

static void prefs_common_charset_set_data_from_optmenu(PrefParam *pparam)
{
	GtkWidget *menu;
	GtkWidget *menuitem;
	gchar *charset;

	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(*pparam->widget));
	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	charset = gtk_object_get_user_data(GTK_OBJECT(menuitem));
	g_free(*((gchar **)pparam->data));
	*((gchar **)pparam->data) = g_strdup(charset);
}

static void prefs_common_charset_set_optmenu(PrefParam *pparam)
{
	GList *cur;
	GtkOptionMenu *optmenu = GTK_OPTION_MENU(*pparam->widget);
	GtkWidget *menu;
	GtkWidget *menuitem;
	gchar *charset;
	gint n = 0;

	g_return_if_fail(optmenu != NULL);
	g_return_if_fail(*((gchar **)pparam->data) != NULL);

	menu = gtk_option_menu_get_menu(optmenu);
	for (cur = GTK_MENU_SHELL(menu)->children;
	     cur != NULL; cur = cur->next) {
		menuitem = GTK_WIDGET(cur->data);
		charset = gtk_object_get_user_data(GTK_OBJECT(menuitem));
		if (!strcmp(charset, *((gchar **)pparam->data))) {
			gtk_option_menu_set_history(optmenu, n);
			return;
		}
		n++;
	}

	gtk_option_menu_set_history(optmenu, 0);
	prefs_common_charset_set_data_from_optmenu(pparam);
}

static void prefs_common_recv_dialog_set_data_from_optmenu(PrefParam *pparam)
{
	GtkWidget *menu;
	GtkWidget *menuitem;

	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(*pparam->widget));
	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	*((RecvDialogMode *)pparam->data) = GPOINTER_TO_INT
		(gtk_object_get_user_data(GTK_OBJECT(menuitem)));
}

static void prefs_common_recv_dialog_set_optmenu(PrefParam *pparam)
{
	RecvDialogMode mode = *((RecvDialogMode *)pparam->data);
	GtkOptionMenu *optmenu = GTK_OPTION_MENU(*pparam->widget);
	GtkWidget *menu;
	GtkWidget *menuitem;

	switch (mode) {
	case RECV_DIALOG_ALWAYS:
		gtk_option_menu_set_history(optmenu, 0);
		break;
	case RECV_DIALOG_ACTIVE:
		gtk_option_menu_set_history(optmenu, 1);
		break;
	case RECV_DIALOG_NEVER:
		gtk_option_menu_set_history(optmenu, 2);
		break;
	default:
	}

	menu = gtk_option_menu_get_menu(optmenu);
	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	gtk_menu_item_activate(GTK_MENU_ITEM(menuitem));
}

static gint prefs_common_deleted(GtkWidget *widget, GdkEventAny *event,
				 gpointer data)
{
	prefs_common_cancel();
	return TRUE;
}

static void prefs_common_key_pressed(GtkWidget *widget, GdkEventKey *event,
				     gpointer data)
{
	if (event && event->keyval == GDK_Escape)
		prefs_common_cancel();
}

static void prefs_common_ok(void)
{
	prefs_common_apply();
	gtk_widget_hide(dialog.window);
	if (quote_desc_win && GTK_WIDGET_VISIBLE(quote_desc_win))
		gtk_widget_hide(quote_desc_win);

	inc_unlock();
}

static void prefs_common_apply(void)
{
	prefs_set_data_from_dialog(param);
	main_window_reflect_prefs_all();
	prefs_common_save_config();

	inc_autocheck_timer_remove();
	inc_autocheck_timer_set();
}

static void prefs_nextunreadmsgdialog_set_data_from_optmenu(PrefParam *pparam)
{
	GtkWidget *menu;
	GtkWidget *menuitem;

	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(*pparam->widget));
	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	*((NextUnreadMsgDialogShow *)pparam->data) = GPOINTER_TO_INT
		(gtk_object_get_user_data(GTK_OBJECT(menuitem)));
}

static void prefs_nextunreadmsgdialog_set_optmenu(PrefParam *pparam)
{
	NextUnreadMsgDialogShow dialog_show;
	GtkOptionMenu *optmenu = GTK_OPTION_MENU(*pparam->widget);
	GtkWidget *menu;
	GtkWidget *menuitem;

	dialog_show = *((NextUnreadMsgDialogShow *)pparam->data);

	switch (dialog_show) {
	case NEXTUNREADMSGDIALOG_ALWAYS:
		gtk_option_menu_set_history(optmenu, 0);
		break;
	case NEXTUNREADMSGDIALOG_ASSUME_YES:
		gtk_option_menu_set_history(optmenu, 1);
		break;
	case NEXTUNREADMSGDIALOG_ASSUME_NO:
		gtk_option_menu_set_history(optmenu, 2);
		break;
	default:
	}

	menu = gtk_option_menu_get_menu(optmenu);
	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	gtk_menu_item_activate(GTK_MENU_ITEM(menuitem));
}

static void prefs_common_cancel(void)
{
	gtk_widget_hide(dialog.window);
	inc_unlock();
}


static struct _ComposePrefs
{
	GtkWidget *window;

	GtkWidget *close_btn;
} composeprefs;

static void compose_prefs_key_pressed(GtkWidget *widget, GdkEventKey *event);
static gint compose_prefs_delete_event(GtkWidget *widget, GdkEventAny *event);
static void compose_prefs_close(GtkWidget *widget);
static void compose_prefs_fmt_create(void);

static void compose_prefs_close(GtkWidget *widget)
{
	gtk_widget_hide(composeprefs.window);
}

static gint compose_prefs_delete_event(GtkWidget *widget, GdkEventAny *event)
{
	gtk_widget_hide(composeprefs.window);

	return TRUE;
}

static void compose_prefs_key_pressed(GtkWidget *widget, GdkEventKey *event)
{
	if (event && event->keyval == GDK_Escape) {
		gtk_widget_hide(composeprefs.window);
	}
}

/* static void prefs_recvdialog_set_data_from_optmenu(PrefParam *pparam)
{
	GtkWidget *menu;
	GtkWidget *menuitem;

	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(*pparam->widget));
	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	*((RecvDialogShow *)pparam->data) = GPOINTER_TO_INT
		(gtk_object_get_user_data(GTK_OBJECT(menuitem)));
}  */

/* static void prefs_recvdialog_set_optmenu(PrefParam *pparam)
{
	RecvDialogShow dialog_show;
	GtkOptionMenu *optmenu = GTK_OPTION_MENU(*pparam->widget);
	GtkWidget *menu;
	GtkWidget *menuitem;

	dialog_show = *((RecvDialogShow *)pparam->data);

	switch (dialog_show) {
	case RECVDIALOG_ALWAYS:
		gtk_option_menu_set_history(optmenu, 0);
		break;
	case RECVDIALOG_WINDOW_ACTIVE:
		gtk_option_menu_set_history(optmenu, 1);
		break;
	case RECVDIALOG_NEVER:
		gtk_option_menu_set_history(optmenu, 2);
		break;
	default:
	}

	menu = gtk_option_menu_get_menu(optmenu);
	menuitem = gtk_menu_get_active(GTK_MENU(menu));
	gtk_menu_item_activate(GTK_MENU_ITEM(menuitem));
}     */
