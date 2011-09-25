/*
 * peas-extension.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
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

#include <string.h>

#include "peas-extension-wrapper.h"
#include "peas-introspection.h"

G_DEFINE_ABSTRACT_TYPE (PeasExtensionWrapper, peas_extension_wrapper, G_TYPE_OBJECT);

static void
peas_extension_wrapper_init (PeasExtensionWrapper *exten)
{
}

static void
peas_extension_wrapper_constructed (GObject *object)
{
  PeasExtensionWrapper *exten = PEAS_EXTENSION_WRAPPER (object);

  exten->constructed = TRUE;

  if (G_OBJECT_CLASS (peas_extension_wrapper_parent_class)->constructed != NULL)
    G_OBJECT_CLASS (peas_extension_wrapper_parent_class)->constructed (object);
}

static void
peas_extension_wrapper_class_init (PeasExtensionWrapperClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = peas_extension_wrapper_constructed;

  /* Don't add properties as they could shadow the instance's
   * and C plugins would not have the property.
   */
}

GType
peas_extension_wrapper_get_extension_type (PeasExtensionWrapper *exten)
{
  g_return_val_if_fail (PEAS_IS_EXTENSION_WRAPPER (exten), G_TYPE_INVALID);

  return exten->exten_type;
}

gboolean
peas_extension_wrapper_callv (PeasExtensionWrapper *exten,
                              const gchar          *method_name,
                              GIArgument           *args,
                              GIArgument           *return_value)
{
  PeasExtensionWrapperClass *klass;

  g_return_val_if_fail (PEAS_IS_EXTENSION_WRAPPER (exten), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  klass = PEAS_EXTENSION_WRAPPER_GET_CLASS (exten);
  return klass->call (exten, method_name, args, return_value);
}

gboolean
peas_extension_wrapper_emit_signal (PeasExtensionWrapper  *exten,
                                    GSignalInvocationHint *invocation_hint,
                                    guint                  n_values,
                                    const GValue          *instance_and_params,
                                    GValue                *return_value)
{
  PeasExtensionWrapperClass *klass;
  GObject *object;
  GSignalQuery signal_query;
  const GSignalInvocationHint *saved_invocation_hint;
  const GValue *saved_instance_and_params;

  g_return_val_if_fail (PEAS_IS_EXTENSION_WRAPPER (exten), FALSE);
  g_return_val_if_fail (n_values > 0, FALSE);
  g_return_val_if_fail (instance_and_params != NULL, FALSE);

  klass = PEAS_EXTENSION_WRAPPER_GET_CLASS (exten);
  if (klass->emit_signal == NULL)
    {
      g_warning ("Emitting signals is not supported by this loader.");
      return FALSE;
    }

  object = g_value_get_object (&instance_and_params[0]);

  g_signal_query (invocation_hint->signal_id, &signal_query);

  /* Because signals to the extension follow the pattern
   * below we have to guard against recursive signal emissions.
   *
   * App->Subclass->Extension->emit(object)->Extension->Subclass
   */
  if (exten->current_instance_and_params != NULL &&
      exten->current_invocation_hint->signal_id == invocation_hint->signal_id &&
      exten->current_invocation_hint->detail == invocation_hint->detail &&
      memcmp (exten->current_instance_and_params, instance_and_params,
              sizeof (GValue) * n_values) == 0)
    {
      g_signal_stop_emission (object, invocation_hint->signal_id,
                              invocation_hint->detail);
      g_debug ("Blocked recursive emit of '%s::%s'",
               G_OBJECT_TYPE_NAME (object), signal_query.signal_name);
      return FALSE;
    }

  g_debug ("Emitting '%s::%s' on real extension",
           G_OBJECT_TYPE_NAME (object), signal_query.signal_name);

  saved_invocation_hint = exten->current_invocation_hint;
  exten->current_invocation_hint = invocation_hint;
  saved_instance_and_params = exten->current_instance_and_params;
  exten->current_instance_and_params = instance_and_params;

  klass->emit_signal (exten, invocation_hint, n_values,
                      instance_and_params, return_value);

  exten->current_instance_and_params = saved_instance_and_params;
  exten->current_invocation_hint = saved_invocation_hint;

  g_signal_chain_from_overridden (instance_and_params, return_value);
  return TRUE;
}

gboolean
peas_extension_wrapper_receive_signal (PeasExtensionWrapper  *exten,
                                       GSignalInvocationHint *invocation_hint,
                                       guint                  n_values,
                                       const GValue          *instance_and_params,
                                       GValue                *return_value)
{
  GObject *object;
  GSignalQuery signal_query;
  const GSignalInvocationHint *saved_invocation_hint;
  const GValue *saved_instance_and_params;

  g_return_val_if_fail (PEAS_IS_EXTENSION_WRAPPER (exten), FALSE);
  g_return_val_if_fail (n_values > 0, FALSE);
  g_return_val_if_fail (instance_and_params != NULL, FALSE);

  object = g_value_get_object (&instance_and_params[0]);

  g_signal_query (invocation_hint->signal_id, &signal_query);

  /* Because signals to the extension follow the pattern
   * below we have to guard against recursive signal emissions.
   *
   * App->Subclass->Extension->emit(object)->Extension->Subclass
   */
  if (exten->current_instance_and_params != NULL &&
      exten->current_invocation_hint->signal_id == invocation_hint->signal_id &&
      exten->current_invocation_hint->detail == invocation_hint->detail &&
      memcmp (exten->current_instance_and_params, instance_and_params,
              sizeof (GValue) * n_values) == 0)
    {
      g_debug ("Blocked recursive receive of '%s::%s'",
               G_OBJECT_TYPE_NAME (object), signal_query.signal_name);
      return FALSE;
    }

  g_debug ("Emitting '%s::%s' on proxy extension",
           G_OBJECT_TYPE_NAME (object), signal_query.signal_name);

  saved_invocation_hint = exten->current_invocation_hint;
  exten->current_invocation_hint = invocation_hint;
  saved_instance_and_params = exten->current_instance_and_params;
  exten->current_instance_and_params = instance_and_params;

  g_signal_emitv (instance_and_params, invocation_hint->signal_id,
                  invocation_hint->detail, return_value);

  exten->current_instance_and_params = saved_instance_and_params;
  exten->current_invocation_hint = saved_invocation_hint;

  return TRUE;
}
