/*
 * peas-activatable.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Steve Frécinaux
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __PEAS_ACTIVATABLE_H__
#define __PEAS_ACTIVATABLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_TYPE_ACTIVATABLE             (peas_activatable_get_type ())
#define PEAS_ACTIVATABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_ACTIVATABLE, PeasActivatable))
#define PEAS_ACTIVATABLE_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), PEAS_TYPE_ACTIVATABLE, PeasActivatableInterface))
#define PEAS_IS_ACTIVATABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_ACTIVATABLE))
#define PEAS_ACTIVATABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PEAS_TYPE_ACTIVATABLE, PeasActivatableInterface))

typedef struct _PeasActivatable           PeasActivatable; /* dummy typedef */
typedef struct _PeasActivatableInterface  PeasActivatableInterface;

struct _PeasActivatableInterface {
  GTypeInterface g_iface;

  /* Virtual public methods */
  void        (*activate)                 (PeasActivatable *activatable,
                                           GObject         *object);
  void        (*deactivate)               (PeasActivatable *activatable,
                                           GObject         *object);
  void        (*update_state)             (PeasActivatable *activatable,
                                           GObject         *object);
};

/*
 * Public methods
 */
GType             peas_activatable_get_type       (void)  G_GNUC_CONST;

void              peas_activatable_activate       (PeasActivatable *plugin,
                                                   GObject         *object);
void              peas_activatable_deactivate     (PeasActivatable *plugin,
                                                   GObject         *object);
void              peas_activatable_update_state   (PeasActivatable *plugin,
                                                   GObject         *object);

G_END_DECLS

#endif /* __PEAS_ACTIVATABLE_H__ */
