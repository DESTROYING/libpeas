function callable_extension() {
}

callable_extension.prototype = {
  call_with_return: function() {
    return "Hello, World!";
  },
  call_no_args: function() {
  },
  call_single_arg: function() {
    return true;
  },
  call_multi_args: function(in_, inout) {
    return [ inout, in_ ];
  }
};

function properties_extension() {
  this.read_only = "read-only";
  this.readwrite = "readwrite";
}

function signals_extension() {
}

signals_extension.prototype = {
  _init: function() {
    this.connect("a-signal", this.on_a_signal);
    this.connect("emit-a-signal", this.on_emit_a_signal);
  },
  on_a_signal: function(number) {
    return number;
  },
  on_emit_a_signal: function(number) {
    this.emit("a-signal", number);
  }
}

var extensions = {
  "IntrospectionCallable": callable_extension,
  "IntrospectionProperties": properties_extension,
  "IntrospectionSignals": signals_extension
};
