/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2006 Hiroyuki Yamamoto & the Sylpheed-Claws team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "privacy.h"
#include "procmime.h"

static GSList *systems = NULL;

PrivacySystem *privacy_data_get_system(PrivacyData *data)
{
	/* Make sure the cached system is still registered */
	if (data->system && g_slist_find(systems, data->system))
		return data->system;
	else
		return NULL;
}
/**
 * Register a new Privacy System
 *
 * \param system The Privacy System that should be registered
 */
void privacy_register_system(PrivacySystem *system)
{
	systems = g_slist_append(systems, system);
}

/**
 * Unregister a new Privacy System. The system must not be in
 * use anymore when it is unregistered.
 *
 * \param system The Privacy System that should be unregistered
 */
void privacy_unregister_system(PrivacySystem *system)
{
	systems = g_slist_remove(systems, system);
}

/**
 * Free a PrivacyData of a PrivacySystem
 *
 * \param privacydata The data to free
 */
void privacy_free_privacydata(PrivacyData *privacydata)
{
	PrivacySystem *system = NULL;
	
	g_return_if_fail(privacydata != NULL);

	system = privacy_data_get_system(privacydata);
	if (!system)
		return;
	system->free_privacydata(privacydata);
}

/**
 * Check if a MimeInfo is signed with one of the available
 * privacy system. If a privacydata is set in the MimeInfo
 * it will directory return the return value by the system
 * set in the privacy data or check all available privacy
 * systems otherwise.
 *
 * \return True if the MimeInfo has a signature
 */
gboolean privacy_mimeinfo_is_signed(MimeInfo *mimeinfo)
{
	GSList *cur;
	g_return_val_if_fail(mimeinfo != NULL, FALSE);

	if (mimeinfo->privacy != NULL) {
		PrivacySystem *system = 
			privacy_data_get_system(mimeinfo->privacy);

		if (system == NULL) {
			mimeinfo->privacy = NULL;
			goto try_others;
		}

		if (system->is_signed != NULL)
			return system->is_signed(mimeinfo);
		else
			return FALSE;
	}
try_others:
	for(cur = systems; cur != NULL; cur = g_slist_next(cur)) {
		PrivacySystem *system = (PrivacySystem *) cur->data;

		if(system->is_signed != NULL && system->is_signed(mimeinfo))
			return TRUE;
	}

	return FALSE;
}

/**
 * Check the signature of a MimeInfo. privacy_mimeinfo_is_signed
 * should be called before otherwise it is done by this function.
 * If the MimeInfo is not signed an error code will be returned.
 *
 * \return Error code indicating the result of the check,
 *         < 0 if an error occured
 */
gint privacy_mimeinfo_check_signature(MimeInfo *mimeinfo)
{
	PrivacySystem *system;

	g_return_val_if_fail(mimeinfo != NULL, -1);

	if (mimeinfo->privacy == NULL)
		privacy_mimeinfo_is_signed(mimeinfo);
	
	if (mimeinfo->privacy == NULL)
		return -1;
	
	system = privacy_data_get_system(mimeinfo->privacy);
	if (system == NULL)
		return -1;

	if (system->check_signature == NULL)
		return -1;
	
	return system->check_signature(mimeinfo);
}

SignatureStatus privacy_mimeinfo_get_sig_status(MimeInfo *mimeinfo)
{
	PrivacySystem *system;

	g_return_val_if_fail(mimeinfo != NULL, -1);

	if (mimeinfo->privacy == NULL)
		privacy_mimeinfo_is_signed(mimeinfo);
	
	if (mimeinfo->privacy == NULL)
		return SIGNATURE_UNCHECKED;
	
	system = privacy_data_get_system(mimeinfo->privacy);
	if (system == NULL)
		return SIGNATURE_UNCHECKED;
	if (system->get_sig_status == NULL)
		return SIGNATURE_UNCHECKED;
	
	return system->get_sig_status(mimeinfo);
}

gchar *privacy_mimeinfo_sig_info_short(MimeInfo *mimeinfo)
{
	PrivacySystem *system;

	g_return_val_if_fail(mimeinfo != NULL, NULL);

	if (mimeinfo->privacy == NULL)
		privacy_mimeinfo_is_signed(mimeinfo);
	
	if (mimeinfo->privacy == NULL)
		return g_strdup(_("No signature found"));
	
	system = privacy_data_get_system(mimeinfo->privacy);
	if (system == NULL)
		return g_strdup(_("No signature found"));
	if (system->get_sig_info_short == NULL)
		return g_strdup(_("No information available"));
	
	return system->get_sig_info_short(mimeinfo);
}

gchar *privacy_mimeinfo_sig_info_full(MimeInfo *mimeinfo)
{
	PrivacySystem *system;

	g_return_val_if_fail(mimeinfo != NULL, NULL);

	if (mimeinfo->privacy == NULL)
		privacy_mimeinfo_is_signed(mimeinfo);
	
	if (mimeinfo->privacy == NULL)
		return g_strdup(_("No signature found"));
	
	system = privacy_data_get_system(mimeinfo->privacy);
	if (system == NULL)
		return g_strdup(_("No signature found"));
	if (system->get_sig_info_full == NULL)
		return g_strdup(_("No information available"));
	
	return system->get_sig_info_full(mimeinfo);
}

gboolean privacy_mimeinfo_is_encrypted(MimeInfo *mimeinfo)
{
	GSList *cur;
	g_return_val_if_fail(mimeinfo != NULL, FALSE);

	for(cur = systems; cur != NULL; cur = g_slist_next(cur)) {
		PrivacySystem *system = (PrivacySystem *) cur->data;

		if(system->is_encrypted != NULL && system->is_encrypted(mimeinfo))
			return TRUE;
	}

	return FALSE;
}

static gint decrypt(MimeInfo *mimeinfo, PrivacySystem *system)
{
	MimeInfo *decryptedinfo, *parentinfo;
	gint childnumber;
	
	g_return_val_if_fail(system->decrypt != NULL, -1);
	
	decryptedinfo = system->decrypt(mimeinfo);
	if (decryptedinfo == NULL)
		return -1;

	parentinfo = procmime_mimeinfo_parent(mimeinfo);
	childnumber = g_node_child_index(parentinfo->node, mimeinfo);
	
	procmime_mimeinfo_free_all(mimeinfo);

	g_node_insert(parentinfo->node, childnumber, decryptedinfo->node);

	return 0;
}

gint privacy_mimeinfo_decrypt(MimeInfo *mimeinfo)
{
	GSList *cur;
	g_return_val_if_fail(mimeinfo != NULL, FALSE);

	for(cur = systems; cur != NULL; cur = g_slist_next(cur)) {
		PrivacySystem *system = (PrivacySystem *) cur->data;

		if(system->is_encrypted != NULL && system->is_encrypted(mimeinfo))
			return decrypt(mimeinfo, system);
	}

	return -1;
}

GSList *privacy_get_system_ids()
{
	GSList *cur;
	GSList *ret = NULL;

	for(cur = systems; cur != NULL; cur = g_slist_next(cur)) {
		PrivacySystem *system = (PrivacySystem *) cur->data;

		ret = g_slist_append(ret, g_strdup(system->id));
	}

	return ret;
}

static PrivacySystem *privacy_get_system(const gchar *id)
{
	GSList *cur;

	g_return_val_if_fail(id != NULL, NULL);

	for(cur = systems; cur != NULL; cur = g_slist_next(cur)) {
		PrivacySystem *system = (PrivacySystem *) cur->data;

		if(strcmp(id, system->id) == 0)
			return system;
	}

	return NULL;
}

const gchar *privacy_system_get_name(const gchar *id)
{
	PrivacySystem *system;

	g_return_val_if_fail(id != NULL, NULL);

	system = privacy_get_system(id);
	if (system == NULL)
		return NULL;

	return system->name;
}

gboolean privacy_system_can_sign(const gchar *id)
{
	PrivacySystem *system;

	g_return_val_if_fail(id != NULL, FALSE);

	system = privacy_get_system(id);
	if (system == NULL)
		return FALSE;

	return system->can_sign;
}

gboolean privacy_system_can_encrypt(const gchar *id)
{
	PrivacySystem *system;

	g_return_val_if_fail(id != NULL, FALSE);

	system = privacy_get_system(id);
	if (system == NULL)
		return FALSE;

	return system->can_encrypt;
}

gboolean privacy_sign(const gchar *id, MimeInfo *target, PrefsAccount *account)
{
	PrivacySystem *system;

	g_return_val_if_fail(id != NULL, FALSE);
	g_return_val_if_fail(target != NULL, FALSE);

	system = privacy_get_system(id);
	if (system == NULL)
		return FALSE;
	if (!system->can_sign)
		return FALSE;
	if (system->sign == NULL)
		return FALSE;

	return system->sign(target, account);
}

gchar *privacy_get_encrypt_data(const gchar *id, GSList *recp_names)
{
	PrivacySystem *system;

	g_return_val_if_fail(id != NULL, NULL);
	g_return_val_if_fail(recp_names != NULL, NULL);

	system = privacy_get_system(id);
	if (system == NULL)
		return NULL;
	if (!system->can_encrypt)
		return NULL;
	if (system->get_encrypt_data == NULL)
		return NULL;

	return system->get_encrypt_data(recp_names);
}

gboolean privacy_encrypt(const gchar *id, MimeInfo *mimeinfo, const gchar *encdata)
{
	PrivacySystem *system;

	g_return_val_if_fail(id != NULL, FALSE);
	g_return_val_if_fail(mimeinfo != NULL, FALSE);
	g_return_val_if_fail(encdata != NULL, FALSE);

	system = privacy_get_system(id);
	if (system == NULL)
		return FALSE;
	if (!system->can_encrypt)
		return FALSE;
	if (system->encrypt == NULL)
		return FALSE;

	return system->encrypt(mimeinfo, encdata);
}
