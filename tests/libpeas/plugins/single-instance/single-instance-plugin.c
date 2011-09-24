/*
 * single-instance-plugin.c
 * This file is part of libpeas
 *
 * Copyright (C) 2011 - Garrett Regier
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libpeas/peas.h>

#include "single-instance-plugin.h"

struct _TestingSingleInstancePluginPrivate {
  GObject *object;
};

static void peas_single_instance_iface_init (PeasSingleInstanceInterface *iface);
static void peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingSingleInstancePlugin,
                                testing_single_instance_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_SINGLE_INSTANCE,
                                                               peas_single_instance_iface_init)
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

enum {
  PROP_0,
  PROP_OBJECT
};

static void
testing_single_instance_plugin_set_property (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
  TestingSingleInstancePlugin *plugin = TESTING_SINGLE_INSTANCE_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      plugin->priv->object = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_single_instance_plugin_get_property (GObject    *object,
                                             guint       prop_id,
                                             GValue     *value,
                                             GParamSpec *pspec)
{
  TestingSingleInstancePlugin *plugin = TESTING_SINGLE_INSTANCE_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      g_value_set_object (value, plugin->priv->object);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_single_instance_plugin_dispose (GObject *object)
{
  TestingSingleInstancePlugin *plugin = TESTING_SINGLE_INSTANCE_PLUGIN (object);

  if (plugin->priv->object != NULL)
    {
      g_object_unref (plugin->priv->object);
      plugin->priv->object = NULL;
    }

  G_OBJECT_CLASS (testing_single_instance_plugin_parent_class)->dispose (object);
}

static void
testing_single_instance_plugin_init (TestingSingleInstancePlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              TESTING_TYPE_SINGLE_INSTANCE_PLUGIN,
                                              TestingSingleInstancePluginPrivate);
}

static void
testing_single_instance_plugin_activate (PeasActivatable *activatable)
{
}

static void
testing_single_instance_plugin_deactivate (PeasActivatable *activatable)
{
}

static void
testing_single_instance_plugin_class_init (TestingSingleInstancePluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = testing_single_instance_plugin_set_property;
  object_class->get_property = testing_single_instance_plugin_get_property;
  object_class->dispose = testing_single_instance_plugin_dispose;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");

  g_type_class_add_private (klass, sizeof (TestingSingleInstancePluginPrivate));
}

static void
peas_single_instance_iface_init (PeasSingleInstanceInterface *iface)
{
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = testing_single_instance_plugin_activate;
  iface->deactivate = testing_single_instance_plugin_deactivate;
}

static void
testing_single_instance_plugin_class_finalize (TestingSingleInstancePluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  testing_single_instance_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_SINGLE_INSTANCE,
                                              TESTING_TYPE_SINGLE_INSTANCE_PLUGIN);
  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              TESTING_TYPE_SINGLE_INSTANCE_PLUGIN);
}
