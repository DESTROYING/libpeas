/*
 * peas-extension-python.c
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <girepository.h>
/* _POSIX_C_SOURCE is defined in Python.h and in limits.h included by
 * girepository.h, so we unset it here to avoid a warning. Yep, that's bad. */
#undef _POSIX_C_SOURCE
#include <Python.h>
#include <pygobject.h>
#include <libpeas/peas-introspection.h>
#include <libpeas/peas-extension-subclasses.h>
#include "peas-extension-python.h"

G_DEFINE_TYPE (PeasExtensionPython, peas_extension_python, PEAS_TYPE_EXTENSION_WRAPPER);

static void
call_g_signal_emitv (GObject               *instance,
                     GSignalInvocationHint *invocation_hint,
                     guint                  n_values,
                     const GValue          *instance_and_params,
                     GValue                *return_value)
{
  GValue *real_instance_and_params;

  real_instance_and_params = g_newa (GValue, n_values);
  memset (real_instance_and_params, 0, sizeof (GValue));
  g_value_init (&real_instance_and_params[0], G_OBJECT_TYPE (instance));
  g_value_set_object (&real_instance_and_params[0], instance);
  memcpy (real_instance_and_params + 1, instance_and_params + 1,
          sizeof (GValue) * (n_values - 1));

  g_signal_emitv (real_instance_and_params, invocation_hint->signal_id,
                  invocation_hint->detail, return_value);

  g_value_unset (&real_instance_and_params[0]);
}

static void
peas_extension_python_init (PeasExtensionPython *pyexten)
{
}

static gboolean
peas_extension_python_emit_signal (PeasExtensionWrapper  *exten,
                                   GSignalInvocationHint *invocation_hint,
                                   guint                  n_values,
                                   const GValue          *instance_and_params,
                                   GValue                *return_value)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (exten);
  PyGILState_STATE state;
  GObject *instance;

  state = pyg_gil_state_ensure ();

  instance = pygobject_get (pyexten->instance);

  call_g_signal_emitv (instance, invocation_hint, n_values,
                       instance_and_params, return_value);

  pyg_gil_state_release (state);
  return TRUE;
}

static gboolean
peas_extension_python_call (PeasExtensionWrapper *exten,
                            const gchar          *method_name,
                            GIArgument           *args,
                            GIArgument           *retval)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (exten);
  GType gtype;
  PyGILState_STATE state;
  GObject *instance;
  gboolean success;

  gtype = peas_extension_wrapper_get_extension_type (exten);

  state = pyg_gil_state_ensure ();

  instance = pygobject_get (pyexten->instance);
  success = peas_method_apply (instance, gtype, method_name, args, retval);

  pyg_gil_state_release (state);
  return success;
}

static void
peas_extension_python_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (object);
  PyGILState_STATE state;
  GObject *instance;

  /* Don't add properties as they could shadow the instance's */

  state = pyg_gil_state_ensure ();

  instance = pygobject_get (pyexten->instance);
  g_object_set_property (instance, pspec->name, value);

  pyg_gil_state_release (state);
}

static void
peas_extension_python_get_property (GObject      *object,
                                    guint         prop_id,
                                    GValue       *value,
                                    GParamSpec   *pspec)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (object);
  PyGILState_STATE state;
  GObject *instance;

  /* Don't add properties as they could shadow the instance's */

  state = pyg_gil_state_ensure ();

  instance = pygobject_get (pyexten->instance);
  g_object_get_property (instance, pspec->name, value);

  pyg_gil_state_release (state);
}

static void
peas_extension_python_dispose (GObject *object)
{
  PeasExtensionPython *pyexten = PEAS_EXTENSION_PYTHON (object);

  if (pyexten->instance)
    {
      PyGILState_STATE state = pyg_gil_state_ensure ();

      Py_DECREF (pyexten->instance);
      pyexten->instance = NULL;

      pyg_gil_state_release (state);
    }

  G_OBJECT_CLASS (peas_extension_python_parent_class)->dispose (object);
}

static void
peas_extension_python_class_init (PeasExtensionPythonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasExtensionWrapperClass *extension_class = PEAS_EXTENSION_WRAPPER_CLASS (klass);

  object_class->dispose = peas_extension_python_dispose;
  object_class->get_property = peas_extension_python_get_property;
  object_class->set_property = peas_extension_python_set_property;

  extension_class->call = peas_extension_python_call;
  extension_class->emit_signal = peas_extension_python_emit_signal;
}

static void
signal_closure_marshal (GClosure              *closure,
                        GValue                *return_value,
                        guint                  n_values,
                        const GValue          *instance_and_params,
                        GSignalInvocationHint *invocation_hint)
{
  GObject *object = G_OBJECT (closure->data);
  PyGILState_STATE state;
  const GValue *saved_instance_and_params;
  static const GValue *current_instance_and_params = NULL;

  /* Because signals to the extension follow the pattern
   * below we have to guard against recursive signal emissions.
   *
   * App->Subclass->Extension->emit(object)->Extension->Subclass
   */
  if (current_instance_and_params != NULL &&
      memcmp (current_instance_and_params, instance_and_params,
              sizeof (GValue) * n_values) == 0)
    {
      GSignalQuery signal_query;

      g_signal_stop_emission (object, invocation_hint->signal_id,
                              invocation_hint->detail);

      g_signal_query (invocation_hint->signal_id, &signal_query);
      g_debug ("Blocked recursive emission of '%s::%s'",
               G_OBJECT_TYPE_NAME (object), signal_query.signal_name);
      return;
    }

  saved_instance_and_params = current_instance_and_params;
  current_instance_and_params = instance_and_params;

  state = pyg_gil_state_ensure ();

  call_g_signal_emitv (object, invocation_hint, n_values,
                       instance_and_params, return_value);

  pyg_gil_state_release (state);

  current_instance_and_params = saved_instance_and_params;
}

GObject *
peas_extension_python_new (GType     gtype,
                           PyObject *instance)
{
  PeasExtensionPython *pyexten;
  GType real_type;
  guint n_signals, i;
  guint *signals;
  GClosure *signal_closure;
  GObject *gobject_instance;

  real_type = peas_extension_register_subclass (PEAS_TYPE_EXTENSION_PYTHON, gtype);
  pyexten = PEAS_EXTENSION_PYTHON (g_object_new (real_type, NULL));

  pyexten->instance = instance;
  PEAS_EXTENSION_WRAPPER (pyexten)->exten_type = gtype;
  Py_INCREF (instance);

  signals = g_signal_list_ids (gtype, &n_signals);

  if (n_signals > 0)
    {
      signal_closure = g_closure_new_object (sizeof (GClosure),
                                             G_OBJECT (pyexten));
      g_closure_set_marshal (signal_closure,
                             (GClosureMarshal) signal_closure_marshal);

      gobject_instance = pygobject_get (pyexten->instance);
    }

  for (i = 0; i < n_signals; ++i)
    {
      GSignalQuery signal_query;

      g_signal_query (signals[i], &signal_query);

      g_debug ("Overrided '%s::%s' for '%s' proxy",
               g_type_name (gtype), signal_query.signal_name,
               G_OBJECT_TYPE_NAME (gobject_instance));

      g_signal_connect_closure_by_id (gobject_instance, signals[i], 0,
                                      signal_closure, FALSE);
    }

  g_free (signals);

  return G_OBJECT (pyexten);
}
