/*
 * introspection-signals.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Garrett Regier
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

#include <girepository.h>

#include "libpeas/peas.h"

#include "introspection-signals.h"

G_DEFINE_INTERFACE(IntrospectionSignals, introspection_signals, 0)

enum {
  A_SIGNAL,
  EMIT_A_SIGNAL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

void
introspection_signals_default_init (IntrospectionSignalsInterface *iface)
{
  GType the_type = G_TYPE_FROM_INTERFACE (iface);
  static gboolean initialized = FALSE;

  if (initialized)
    return;

  /* Use g_signal_new_class_handler() so there is no VFunc */

  signals[A_SIGNAL] =
    g_signal_new_class_handler ("a-signal",
                                the_type,
                                G_SIGNAL_RUN_LAST,
                                NULL, NULL, NULL,
                                gi_cclosure_marshal_generic,
                                G_TYPE_INT,
                                1,
                                G_TYPE_INT);

  signals[EMIT_A_SIGNAL] =
    g_signal_new_class_handler ("emit-a-signal",
                                the_type,
                                G_SIGNAL_RUN_LAST,
                                NULL, NULL, NULL,
                                gi_cclosure_marshal_generic,
                                G_TYPE_NONE,
                                1,
                                G_TYPE_INT);

  initialized = TRUE;
}
