/*
 * peas-extension-gjs.c
 * This file is part of libpeas
 *
 * Copyright (C) 2011 - Garrett Regier, Steve Frécinaux
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

#include <girepository.h>
#include <gi/arg.h>
#include <gi/value.h>

#include <libpeas/peas-introspection.h>
#include <libpeas/peas-extension-subclasses.h>

#include "peas-extension-gjs.h"

G_DEFINE_TYPE (PeasExtensionGjs, peas_extension_gjs, PEAS_TYPE_EXTENSION_WRAPPER);

typedef struct {
  GIArgInfo arg_info;
  GITypeInfo type_info;

  /* Only used by out arguments */
  gpointer ptr;
} CachedArg;

static void
peas_extension_gjs_init (PeasExtensionGjs *gexten)
{
}

static gchar *
convert_property_name (const gchar *pname)
{
  gint i;
  gchar *prop_name;

  prop_name = g_strdup (pname);

  for (i = 0; prop_name[i] != '\0'; ++i)
    {
      if (prop_name[i] == '-')
        prop_name[i] = '_';
    }

  return prop_name;
}

static void
peas_extension_gjs_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PeasExtensionGjs *gexten = PEAS_EXTENSION_GJS (object);
  gchar *prop_name;
  jsval js_value;

  prop_name = convert_property_name (g_param_spec_get_name (pspec));

  if (!gjs_value_from_g_value (gexten->js_context, &js_value, value))
    {
      g_warning ("Error: failed to convert GValue to "
                 "jsval for property '%s'", prop_name);
    }
  else if (!JS_SetProperty (gexten->js_context, gexten->js_object,
                            prop_name, &js_value))
    {
      g_warning ("Error: failed to set property '%s'", prop_name);
    }

  g_free (prop_name);
}

static void
peas_extension_gjs_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PeasExtensionGjs *gexten = PEAS_EXTENSION_GJS (object);
  gchar *prop_name;
  jsval js_value;

  prop_name = convert_property_name (g_param_spec_get_name (pspec));

  if (!JS_GetProperty (gexten->js_context, gexten->js_object,
                       prop_name, &js_value))
    {
      g_warning ("Error: failed to get property '%s'", prop_name);
    }
  else if (!gjs_value_to_g_value (gexten->js_context, js_value, value))
    {
      g_warning ("Error: failed to convert jsval to "
                 "GValue for property '%s'", prop_name);
    }

  g_free (prop_name);
}

static void
peas_extension_gjs_dispose (GObject *object)
{
  PeasExtensionGjs *gexten = PEAS_EXTENSION_GJS (object);

  if (gexten->js_context != NULL)
    {
      JS_RemoveObjectRoot (gexten->js_context, &gexten->js_object);
      gexten->js_context = NULL;
      gexten->js_object = NULL;
    }
}

static gboolean
set_out_arg (JSContext      *js_context,
             GIFunctionInfo *func_info,
             gboolean        is_return_value,
             GIArgInfo      *arg_info,
             GITypeInfo     *type_info,
             gpointer        ptr,
             jsval           js_value)
{
  gboolean nullable;
  GITransfer transfer;
  GIArgument argument;
  GjsArgumentType arg_type;

  if (is_return_value)
    {
      arg_type = GJS_ARGUMENT_RETURN_VALUE;
      nullable = g_callable_info_may_return_null (func_info);
      transfer = g_callable_info_get_caller_owns (func_info);
    }
  else
    {
      arg_type = GJS_ARGUMENT_ARGUMENT;
      nullable = g_arg_info_may_be_null (arg_info);
      transfer = g_arg_info_get_ownership_transfer (arg_info);
    }

  if (!gjs_value_to_g_argument (js_context, js_value, type_info, NULL,
                                arg_type, transfer, nullable, &argument))
    {
      if (is_return_value)
        {
          g_warning ("Error failed to convert return value to GIArgument");
        }
      else
        {
          g_warning ("Error failed to convert OUT argument '%s' from "
                     "jsval to GIArgument", g_base_info_get_name (arg_info));
        }

      return FALSE;
    }

  peas_gi_argument_to_pointer (type_info, &argument, ptr);

  return TRUE;
}

static gboolean
peas_extension_gjs_call (PeasExtensionWrapper *exten,
                         const gchar          *method_name,
                         GIArgument           *args,
                         GIArgument           *retval)
{
  PeasExtensionGjs *gexten = PEAS_EXTENSION_GJS (exten);
  GType exten_type;
  gboolean success = FALSE;
  jsval js_method, js_retval;
  GICallableInfo *func_info;
  jsval *js_args;
  CachedArg *arg_cache;
  gint i, n_args, nth_out_arg;
  gint n_in_args = 0;
  gint n_out_args = 0;
  gint cached_args = 0;

  exten_type = peas_extension_wrapper_get_extension_type (exten);

  /* Fetch the JS method we want to call */
  if (!JS_GetProperty (gexten->js_context, gexten->js_object,
                       method_name, &js_method) ||
      JSVAL_IS_VOID (js_method))
    {
      g_warning ("Method '%s.%s' was not found",
                 g_type_name (exten_type), method_name);
      return FALSE;
    }

  if (JSVAL_IS_NULL (js_method) || !JSVAL_IS_OBJECT (js_method) ||
      !JS_ObjectIsFunction (gexten->js_context, JSVAL_TO_OBJECT (js_method)))
    {
      g_warning ("Method '%s.%s' in not a function",
                 g_type_name (exten_type), method_name);
      return FALSE;
    }

  /* Prepare the arguments */
  func_info = peas_gi_get_method_info (exten_type, method_name);
  if (func_info == NULL)
    return FALSE;

  n_args = g_callable_info_get_n_args (func_info);
  if (n_args < 0)
    {
      g_warn_if_fail (n_args >= 0);
      goto out;
    }

  js_args = g_newa (jsval, n_args);
  arg_cache = g_newa (CachedArg, n_args + 1);

  /* Return value is an out arg */
  g_callable_info_load_return_type (func_info, &arg_cache[0].type_info);
  if (g_type_info_get_tag (&arg_cache[0].type_info) != GI_TYPE_TAG_VOID)
    {
      ++n_out_args;
      arg_cache[cached_args++].ptr = &retval->v_pointer;
    }

  /* Handle the arguments */
  for (i = 0; i < n_args; ++i, ++cached_args)
    {
      GIDirection direction;

      g_callable_info_load_arg (func_info, i, &arg_cache[cached_args].arg_info);
      direction = g_arg_info_get_direction (&arg_cache[cached_args].arg_info);
      g_arg_info_load_type (&arg_cache[cached_args].arg_info,
                            &arg_cache[cached_args].type_info);

      if (direction == GI_DIRECTION_IN &&
          !gjs_value_from_g_argument (gexten->js_context, &js_args[n_in_args++],
                                      &arg_cache[cached_args].type_info,
                                      &args[i]))
        {
          g_warning ("Error failed to convert argument '%s'",
                     g_base_info_get_name (&arg_cache[cached_args].arg_info));
          goto out;
        }

      if (direction == GI_DIRECTION_INOUT)
        {
          GIArgument arg;
          
          peas_gi_pointer_to_argument (&arg_cache[cached_args].type_info,
                                       args[i].v_pointer, &arg);

          if (!gjs_value_from_g_argument (gexten->js_context, &js_args[n_in_args++],
                                          &arg_cache[cached_args].type_info, &arg))
            {
              g_warning ("Error failed to convert argument '%s'",
                         g_base_info_get_name (&arg_cache[cached_args].arg_info));
              goto out;
            }
        }

      if (direction == GI_DIRECTION_OUT || direction == GI_DIRECTION_INOUT)
        {
          ++n_out_args;
          arg_cache[cached_args].ptr = args[i].v_pointer;
        }
    }

  success = JS_CallFunctionValue (gexten->js_context, gexten->js_object,
                                  js_method, n_in_args, js_args, &js_retval);

  if (!success)
    {
      g_warning ("Error while calling '%s.%s'",
                 g_type_name (exten_type), method_name);
      goto out;
    }

  /* First we need to release in argument */
  for (i = 0; i < cached_args; ++i)
    {
      GIDirection direction;

      /* First cached argument may be the return value */
      if (i == 0 && cached_args > n_args)
        continue;

      direction = g_arg_info_get_direction (&arg_cache[i].arg_info);

      if (direction == GI_DIRECTION_IN || direction == GI_DIRECTION_INOUT)
        {
          GITransfer transfer;

          transfer = g_arg_info_get_ownership_transfer (&arg_cache[i].arg_info);

          if (!gjs_g_argument_release_in_arg (gexten->js_context, transfer,
                                              &arg_cache[i].type_info,
                                              &args[i]))
            {
              g_warning ("Error failed to release IN argument '%s'",
                         g_base_info_get_name (&arg_cache[i].arg_info));
            }
        }
    }

  /* Check that we have a valid return value */
  if (n_out_args > 1)
    {
      if (!JSVAL_IS_OBJECT (js_retval) ||
          !JS_IsArrayObject (gexten->js_context, JSVAL_TO_OBJECT (js_retval)))
        {
          g_warning ("Error return value is not an array");
          success = FALSE;
          goto out;
        }
    }

  /* Set out arguments */
  for (i = 0, nth_out_arg = 0; i < cached_args && success; ++i)
    {
      gboolean is_return_value;

      is_return_value = i == 0 && cached_args > n_args;

      /* Return value does not have a GIArgInfo and is always out */
      if (!is_return_value)
        {
          GIDirection direction;

          direction = g_arg_info_get_direction (&arg_cache[i].arg_info);

          if (direction == GI_DIRECTION_IN)
            continue;
        }

      if (n_out_args == 1)
        {
          success = set_out_arg (gexten->js_context, func_info, is_return_value,
                                 &arg_cache[i].arg_info, &arg_cache[i].type_info,
                                 arg_cache[i].ptr, js_retval);
          break;
        }
      else if (n_out_args > 1)
        {
          jsval js_value;

          if (!JS_GetElement (gexten->js_context, JSVAL_TO_OBJECT (js_retval),
                              nth_out_arg++, &js_value) ||
              js_value == JSVAL_VOID)
            {
              g_warning ("Error failed to get out argument %i", nth_out_arg);
              success = FALSE;
            }
          else
            {
              success = set_out_arg (gexten->js_context, func_info,
                                     is_return_value, &arg_cache[i].arg_info,
                                     &arg_cache[i].type_info, arg_cache[i].ptr,
                                     js_value);
            }
        }
    }

out:

  g_base_info_unref (func_info);

  return success;
}

static void
peas_extension_gjs_class_init (PeasExtensionGjsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasExtensionWrapperClass *extension_class = PEAS_EXTENSION_WRAPPER_CLASS (klass);

  object_class->set_property = peas_extension_gjs_set_property;
  object_class->get_property = peas_extension_gjs_get_property;
  object_class->dispose = peas_extension_gjs_dispose;

  extension_class->call = peas_extension_gjs_call;
}

static void
signal_emitted_cb (GClosure              *closure,
                   GValue                *return_value,
                   guint                  n_values,
                   const GValue          *instance_and_params,
                   GSignalInvocationHint *invocation_hint)
{
  PeasExtensionGjs *gexten = PEAS_EXTENSION_GJS (closure->data);

  
}

static JSBool
gjs_peas_signal_connect (JSContext *js_context,
                         uintN      argc,
                         jsval     *vp)
{
  jsval *argv = JS_ARGV (context, vp);
  PeasExtensionGjs *gexten;
  gchar *signal_name;
  jsval callback;
  JSBool success = JS_FALSE;

  JS_BeginRequest (js_context);

  if (argc != 2)
    {
      gjs_throw (js_context, "Must pass at 2 arguments to connect()");
      goto out;
    }

  if (!JSVAL_IS_OBJECT (argv[1]) || !JS_ObjectIsFunction (argv[1]))
    {
      gjs_throw (js_context, "Must pass an object as the argument 2 "
                             "of connect()");
      goto out;
    }

  if (!JSVAL_IS_STRING (argv[0]) ||
      !gjs_string_to_utf8 (js_context, argv[0], &signal_name))
    {
      gjs_throw (js_context, "Must pass a string as argument 1 of connect()");
      goto out;
    }

  gexten = g_object_get_data (G_OBJECT (JS_GetContextPrivate (js_context)),
                             "PeasExtensionGjs");
  g_assert (gexten != NULL);

  callback = argv[1];

  exten = PEAS_EXTENSION_WRAPPER (gexten);
  exten_type = peas_extension_wrapper_get_extension_type (exten);
  exten_info = g_irepository_find_by_gtype (NULL, exten_type);
  n_signals = g_interface_info_get_n_signals (exten_info);

  for (i = 0; i < n_signal; ++i)
    {
      signal_info = g_interface_info_get_signal (exten_info, i);

      if (g_strcmp0 (g_base_info_get_name (signal_info), signal_name) == 0)
        break;
    }

  if (i == n_signals)
    goto out;

  closure = g_closure_new_object (sizeof (ConnectClosure), G_OBJECT (gexten));
  g_closure_set_marshal (closure, (GClosureMarshal) signal_emitted_cb);
  /*g_closure_set_finalize_notifier (closure, NULL, notify_func);*/

  connect_closure = (ConnectClosure *) closure;
  connect_closure->callback = callaback;
  connect_closure->signal_info = signal_info;
  JS_AddObjectRoot (gexten->js_context, &connect_closure->callback);

  /* Let GObject's signals do the works for us */
  g_signal_connect_closure_by_id (gexten, signal_name, detail, closure, FALSE);

  success = JS_TRUE;

out:

  JS_EndRequest (js_context);

  JS_SET_RVAL (js_context, vp, JSVAL_VOID);
  return success;
}

GObject *
peas_extension_gjs_new (GType      exten_type,
                        JSContext *js_context,
                        JSObject  *js_object)
{
  PeasExtensionGjs *gexten;
  GType real_type;

  g_return_val_if_fail (js_context != NULL, NULL);
  g_return_val_if_fail (js_object != NULL, NULL);

  real_type = peas_extension_register_subclass (PEAS_TYPE_EXTENSION_GJS, exten_type);
  gexten = PEAS_EXTENSION_GJS (g_object_new (real_type, NULL));

  gexten->js_context = js_context;
  gexten->js_object = js_object;
  PEAS_EXTENSION_WRAPPER (gexten)->exten_type = exten_type;
  JS_AddObjectRoot (gexten->js_context, &gexten->js_object);

  if (!JS_DefineFunction (js_context, js_object, "connect",
                          (JSNative) gjs_peas_signal_connect,
                          2, GJS_MODULE_PROP_FLAGS | JSFUN_FAST_NATIVE))
   {
      g_warning ("Failed to define connect");
      g_object_unref (gexten);
      return NULL;
   }

  /*if (!JS_DefineFunction (js_context, js_object, "emit",
                          (JSNative) gjs_peas_signal_emit,
                          2, GJS_MODULE_PROP_FLAGS | JSFUN_FAST_NATIVE))
     {
      g_warning ("Failed to define emit");
      g_object_unref (gexten);
      return NULL;
     }*/

  JS_CallFunctionName (js_context, extension, "_init", 0, NULL, NULL);

  return G_OBJECT (gexten);
}
