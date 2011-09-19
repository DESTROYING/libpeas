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
  NO_ARGS,
  SINGLE_ARG,
  WITH_RETURN,
  MULTI_ARGS,
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

  signals[NO_ARGS] =
    g_signal_new_class_handler ("no-args",
                                the_type,
                                G_SIGNAL_RUN_LAST,
                                NULL, NULL, NULL,
                                gi_cclosure_marshal_generic,
                                G_TYPE_NONE,
                                0);

  signals[WITH_RETURN] =
    g_signal_new_class_handler ("with-return",
                                the_type,
                                G_SIGNAL_RUN_LAST,
                                NULL, NULL, NULL,
                                gi_cclosure_marshal_generic,
                                G_TYPE_BOOLEAN,
                                0);

  signals[SINGLE_ARG] =
    g_signal_new_class_handler ("single-arg",
                               the_type,
                               G_SIGNAL_RUN_LAST,
                               NULL, NULL, NULL,
                               gi_cclosure_marshal_generic,
                               G_TYPE_INT,
                               1,
                               G_TYPE_INT);

  signals[MULTI_ARGS] =
    g_signal_new_class_handler ("multi-args",
                                the_type,
                                G_SIGNAL_RUN_LAST,
                                NULL, NULL, NULL,
                                gi_cclosure_marshal_generic,
                                G_TYPE_INT,
                                3,
                                G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);

  initialized = TRUE;
}

void
introspection_signals_emit_no_args (IntrospectionSignals *signals)
{
  IntrospectionSignalsInterface *iface;

  g_return_if_fail (INTROSPECTION_IS_SIGNALS (signals));

  iface = INTROSPECTION_SIGNALS_GET_IFACE (signals);
  if (iface->emit_no_args != NULL)
    iface->emit_no_args (signals);
}
