/*
 * introspection-signals.h
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

#ifndef __INTROSPECTION_SIGNALS_H__
#define __INTROSPECTION_SIGNALS_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define INTROSPECTION_TYPE_SIGNALS             (introspection_signals_get_type ())
#define INTROSPECTION_SIGNALS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), INTROSPECTION_TYPE_SIGNALS, IntrospectionSignals))
#define INTROSPECTION_SIGNALS_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), INTROSPECTION_TYPE_SIGNALS, IntrospectionSignalsInterface))
#define INTROSPECTION_IS_SIGNALS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INTROSPECTION_TYPE_SIGNALS))
#define INTROSPECTION_SIGNALS_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), INTROSPECTION_TYPE_SIGNALS, IntrospectionSignalsInterface))

typedef struct _IntrospectionSignals           IntrospectionSignals; /* dummy typedef */
typedef struct _IntrospectionSignalsInterface  IntrospectionSignalsInterface;

struct _IntrospectionSignalsInterface {
  GTypeInterface g_iface;
};

/*
 * Public methods
 */
GType introspection_signals_get_type     (void) G_GNUC_CONST;

void  introspection_signals_emit_no_args (IntrospectionSignals *signals);

G_END_DECLS

#endif /* __INTROSPECTION_SIGNALS_H__ */
