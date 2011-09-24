/*
 * peas-single-instance.h
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

#ifndef __PEAS_SINGLE_INSTANCE_H__
#define __PEAS_SINGLE_INSTANCE_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_TYPE_SINGLE_INSTANCE             (peas_single_instance_get_type ())
#define PEAS_SINGLE_INSTANCE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_SINGLE_INSTANCE, PeasSingleInstance))
#define PEAS_SINGLE_INSTANCE_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), PEAS_TYPE_SINGLE_INSTANCE, PeasSingleInstanceInterface))
#define PEAS_IS_SINGLE_INSTANCE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_SINGLE_INSTANCE))
#define PEAS_SINGLE_INSTANCE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PEAS_TYPE_SINGLE_INSTANCE, PeasSingleInstanceInterface))

/**
 * PeasSingleInstance:
 *
 * Interface for single instance plugins.
 */
typedef struct _PeasSingleInstance           PeasSingleInstance; /* dummy typedef */
typedef struct _PeasSingleInstanceInterface  PeasSingleInstanceInterface;

/**
 * PeasSingleInstanceInterface:
 * @g_iface: The parent interface.
 *
 * Provides an interface for single instance plugins.
 */
struct _PeasSingleInstanceInterface {
  GTypeInterface g_iface;
};

/*
 * Public methods
 */
GType             peas_single_instance_get_type       (void)  G_GNUC_CONST;

G_END_DECLS

#endif /* __PEAS_SINGLE_INSTANCE_H__ */
