/*
 * peas-single-instance.c
 * This file is part of libpeas
 *
 * Copyright (C) 2011 Garrett Regier
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

#include "peas-single-instance.h"

/**
 * SECTION:peas-single-instance
 * @short_description: Interface for single instance plugins.
 *
 * #PeasSingleInstance is an interface which should be implemented by plugins
 * that should be single instances.
 *
 * By implementing #PeasSingleInstance the extension will only be created
 * once and will be destroyed upon unloading.
 *
 * This is useful if your plugin implements #PeasActivatable and
 * #PeasGtkConfigurable.
 **/

G_DEFINE_INTERFACE(PeasSingleInstance, peas_single_instance, G_TYPE_OBJECT)

void
peas_single_instance_default_init (PeasSingleInstanceInterface *iface)
{
}
