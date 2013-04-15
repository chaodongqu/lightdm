/*
 * Copyright (C) 2010-2011 Robert Ancell.
 * Author: Robert Ancell <robert.ancell@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 or version 3 of the License.
 * See http://www.gnu.org/copyleft/lgpl.html the full text of the license.
 */

#include <config.h>

#include <string.h>
#include <gio/gio.h>

#include "lightdm/power.h"

static GDBusProxy *upower_proxy = NULL;
static GDBusProxy *ck_proxy = NULL;
static GDBusProxy *login1_proxy = NULL;

static GVariant *
upower_call_function (const gchar *function, GError **error)
{
    GVariant *result;

    if (!upower_proxy)
    {
        upower_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                      G_DBUS_PROXY_FLAGS_NONE,
                                                      NULL,
                                                      "org.freedesktop.UPower",
                                                      "/org/freedesktop/UPower",
                                                      "org.freedesktop.UPower",
                                                      NULL,
                                                      error);
        if (!upower_proxy)
            return NULL;
    }

    return g_dbus_proxy_call_sync (upower_proxy,
                                   function,
                                   NULL,
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   error);
}

/**
 * lightdm_get_can_suspend:
 *
 * Checks if authorized to do a system suspend.
 *
 * Return value: #TRUE if can suspend the system
 **/
gboolean
lightdm_get_can_suspend (void)
{
    GVariant *result;
    gboolean can_suspend = FALSE;

    result = upower_call_function ("SuspendAllowed", NULL);
    if (!result)
        return FALSE;

    if (g_variant_is_of_type (result, G_VARIANT_TYPE ("(b)")))
        g_variant_get (result, "(b)", &can_suspend);
    g_variant_unref (result);

    return can_suspend;
}

/**
 * lightdm_suspend:
 * @error: return location for a #GError, or %NULL
 *
 * Triggers a system suspend.
 * 
 * Return value: #TRUE if suspend initiated.
 **/
gboolean
lightdm_suspend (GError **error)
{
    GVariant *result;
  
    result = upower_call_function ("Suspend", error);
    if (!result)
        return FALSE;

    g_variant_unref (result);

    return TRUE;
}

/**
 * lightdm_get_can_hibernate:
 *
 * Checks if is authorized to do a system hibernate.
 *
 * Return value: #TRUE if can hibernate the system
 **/
gboolean
lightdm_get_can_hibernate (void)
{
    GVariant *result;
    gboolean can_hibernate = FALSE;
  
    result = upower_call_function ("HibernateAllowed", NULL);
    if (!result)
        return FALSE;

    if (g_variant_is_of_type (result, G_VARIANT_TYPE ("(b)")))
        g_variant_get (result, "(b)", &can_hibernate);
    g_variant_unref (result);

    return can_hibernate;
}

/**
 * lightdm_hibernate:
 * @error: return location for a #GError, or %NULL
 *
 * Triggers a system hibernate.
 * 
 * Return value: #TRUE if hibernate initiated.
 **/
gboolean
lightdm_hibernate (GError **error)
{
    GVariant *result;
  
    result = upower_call_function ("Hibernate", error);
    if (!result)
        return FALSE;

    g_variant_unref (result);

    return TRUE;
}

static GVariant *
ck_call_function (const gchar *function, GError **error)
{
    GVariant *r;

    if (!ck_proxy)
    {
        ck_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                  G_DBUS_PROXY_FLAGS_NONE,
                                                  NULL,
                                                  "org.freedesktop.ConsoleKit",
                                                  "/org/freedesktop/ConsoleKit/Manager",
                                                  "org.freedesktop.ConsoleKit.Manager",
                                                  NULL,
                                                  error);
        if (!ck_proxy)
            return FALSE;
    }

    r = g_dbus_proxy_call_sync (ck_proxy,
                                function,
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                error);

    return r;
}

static GVariant *
login1_call_function (const gchar *function, GVariant *parameters, GError **error)
{
    GVariant *r;
    gchar *str_result;

    if (!login1_proxy)
    {
        login1_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                      G_DBUS_PROXY_FLAGS_NONE,
                                                      NULL,
                                                      "org.freedesktop.login1",
                                                      "/org/freedesktop/login1",
                                                      "org.freedesktop.login1.Manager",
                                                      NULL,
                                                      error);
        if (!login1_proxy)
            return NULL;
    }

    r = g_dbus_proxy_call_sync (login1_proxy,
                                function,
                                parameters,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                error);

    return r;
}

/**
 * lightdm_get_can_restart:
 *
 * Checks if is authorized to do a system restart.
 *
 * Return value: #TRUE if can restart the system
 **/
gboolean
lightdm_get_can_restart (void)
{
    gboolean can_restart = FALSE;
    GVariant *r;

    r = login1_call_function ("CanReboot", NULL, NULL);
    if (r)
    {
        gchar *result;
        if (g_variant_is_of_type (r, G_VARIANT_TYPE ("(s)")))
        {
            g_variant_get (r, "(&s)", &result);
            can_restart = g_strcmp0 (result, "yes") == 0;
        }
    }
    else
    {
        r = ck_call_function ("CanRestart", NULL);
        if (r && g_variant_is_of_type (r, G_VARIANT_TYPE ("(b)")))
            g_variant_get (r, "(b)", &can_restart);
    }
    if (r)
        g_variant_unref (r);

    return can_restart;
}

/**
 * lightdm_restart:
 * @error: return location for a #GError, or %NULL
 *
 * Triggers a system restart.
 *
 * Return value: #TRUE if restart initiated.
 **/
gboolean
lightdm_restart (GError **error)
{
    GVariant *r;
    gboolean restarted;

    r = login1_call_function ("Reboot", g_variant_new("(b)", FALSE), error);
    if (!r)
    {
        g_clear_error (error);
        r = ck_call_function ("Restart", error);
    }
    restarted = r != NULL;
    if (r)
        g_variant_unref (r);

    return restarted;
}

/**
 * lightdm_get_can_shutdown:
 *
 * Checks if is authorized to do a system shutdown.
 *
 * Return value: #TRUE if can shutdown the system
 **/
gboolean
lightdm_get_can_shutdown (void)
{
    gboolean can_shutdown = FALSE;
    GVariant *r;
  
    r = login1_call_function ("CanPowerOff", NULL, NULL);
    if (r)
    {
        gchar *result;
        if (g_variant_is_of_type (r, G_VARIANT_TYPE ("(s)")))
        {
            g_variant_get (r, "(&s)", &result);
            can_shutdown = g_strcmp0 (result, "yes") == 0;
        }
    }
    else
    {
        r = ck_call_function ("CanStop", NULL);
        if (r && g_variant_is_of_type (r, G_VARIANT_TYPE ("(b)")))
            g_variant_get (r, "(b)", &can_shutdown);
    }
    if (r)
        g_variant_unref (r);

    return can_shutdown;
}

/**
 * lightdm_shutdown:
 * @error: return location for a #GError, or %NULL
 *
 * Triggers a system shutdown.
 * 
 * Return value: #TRUE if shutdown initiated.
 **/
gboolean
lightdm_shutdown (GError **error)
{
    GVariant *r;
    gboolean shutdown;

    r = login1_call_function ("PowerOff", g_variant_new("(b)", FALSE), error);
    if (!r)
    {
        g_clear_error (error);
        r = ck_call_function ("Stop", error);
    }
    shutdown = r != NULL;
    if (r)
        g_variant_unref (r);

    return shutdown;
}
