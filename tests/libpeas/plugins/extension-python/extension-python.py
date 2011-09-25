# -*- coding: utf-8 -*-
# ex:set ts=4 et sw=4 ai:

from gi.repository import GObject, Introspection, Peas
import traceback

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

        self.connect("a-signal", self.on_a_signal)
        self.connect("emit-a-signal", self.on_emit_a_signal)

    def on_a_signal(self, junk, number):
        print("on_a_signal: %i" % (number))
        return number

    def on_emit_a_signal(self, junk, number):
        print("on_emit_a_signal: %i" % (number))
        self.emit("a-signal", number)

class ActivatablePythonExtension(GObject.Object, Peas.Activatable):
    object = GObject.property(type=GObject.Object)

    def do_activate(self):
        pass

    def do_deactivate(self):
        pass

    def do_update_state(self):
        pass
