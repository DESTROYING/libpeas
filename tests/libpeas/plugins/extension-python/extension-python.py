# -*- coding: utf-8 -*-
# ex:set ts=4 et sw=4 ai:

from gi.repository import GObject, Introspection, Peas

class CallablePythonPlugin(GObject.Object, Introspection.Callable):
    def do_call_with_return(self):
        return "Hello, World!";

    def do_call_no_args(self):
        pass

    def do_call_single_arg(self):
        return True

    def do_call_multi_args(self, in_, inout):
        return (inout, in_)

class PropertiesPythonPlugin(GObject.Object, Introspection.Properties):
    construct_only = GObject.property(type=str)

    read_only = GObject.property(type=str, default="read-only")

    write_only = GObject.property(type=str)

    readwrite = GObject.property(type=str, default="readwrite")

class SignalsPythonPlugin(GObject.Object, Introspection.Signals):
    def __init__(self):
        GObject.Object.__init__(self)

        self.no_args_called = False

        self.connect("no-args", self.on_no_args)
        self.connect("with-return", self.on_with_return)
        self.connect("single-arg", self.on_single_arg)
        self.connect("multi-args", self.on_multi_args)

    def on_no_args(self, junk):
        self.no_args_called = True

    def on_with_return(self, junk):
        return self.no_args_called

    def on_single_arg(self, junk, number):
        return number

    def on_multi_args(self, junk, number_1, number_2, number_3):
        return number_1 + number_2 + number_3


    def do_emit_no_args(self):
        self.emit("no-args")

class ActivatablePythonExtension(GObject.Object, Peas.Activatable):
    object = GObject.property(type=GObject.Object)

    def do_activate(self):
        pass

    def do_deactivate(self):
        pass

    def do_update_state(self):
        pass
