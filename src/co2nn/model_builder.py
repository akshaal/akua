import tensorflow as tf

DEFAULT_ACTIVATION = 'selu'
DEFAULT_KERNEL_CONSTRAINT = tf.keras.constraints.max_norm(3)
DEFAULT_KERNEL_INITIALIZER = 'lecun_normal'
DEFAULT_DROPOUT_IMPL=tf.keras.layers.AlphaDropout


class ModelBuilder():
    def __init__(self, mname: str):
        self._mname = mname
        self._mname_pref = mname + "_"

    def mk_conv1d(self,
                  strides: int,
                  kernel_size: int,
                  filters: int,
                  name: str,
                  avgpool_strides: int = 0,
                  avgpool_size: int = 0,
                  maxpool_strides: int = 0,
                  maxpool_size: int = 0,
                  dropout: float = 0,
                  kernel_constraint=DEFAULT_KERNEL_CONSTRAINT,
                  dropout_impl=DEFAULT_DROPOUT_IMPL):
        conv_layer = tf.keras.layers.Conv1D(
            name=self._mname_pref + name + "_conv",
            kernel_size=kernel_size,
            strides=strides,
            filters=filters,
            activation=DEFAULT_ACTIVATION,
            kernel_initializer=DEFAULT_KERNEL_INITIALIZER,
            kernel_constraint=kernel_constraint
        )

        if dropout > 0:
            drop_layer = dropout_impl(name=self._mname_pref + name + "_dropout", rate=dropout)
        else:
            drop_layer = None

        if avgpool_size > 0:
            avg_pool_layer = tf.keras.layers.AvgPool1D(
                name=self._mname_pref + name + "_avg_pool",
                pool_size=avgpool_size,
                strides=avgpool_strides
            )
        else:
            avg_pool_layer = None

        if maxpool_size > 0:
            max_pool_layer = tf.keras.layers.MaxPool1D(
                name=self._mname_pref + name + "_max_pool",
                pool_size=maxpool_size,
                strides=maxpool_strides
            )
        else:
            max_pool_layer = None

        def apply(input):
            layer = conv_layer(input)
            if drop_layer is not None:
                layer = drop_layer(layer)
            if avg_pool_layer is not None:
                layer = avg_pool_layer(layer)
            if max_pool_layer is not None:
                layer = max_pool_layer(layer)
            return layer

        return apply

    def mk_lstm(self, units, name, unroll: bool, dropout: float = False, dropout_impl=DEFAULT_DROPOUT_IMPL):
        lstm_layer = tf.keras.layers.LSTM(units, name=name+"_lstm", unroll=unroll)

        if dropout > 0:
            drop_layer = dropout_impl(name=self._mname_pref + name + "_dropout", rate=dropout)
        else:
            drop_layer = None

        def apply(input):
            layer = lstm_layer(input)
            if drop_layer is not None:
                layer = drop_layer(layer)
            return layer

        return apply

    def mk_flatten(self, name):
        return tf.keras.layers.Flatten(name=self._mname_pref + name + "_flatten")

    def mk_add(self, name):
        return tf.keras.layers.Add(name=self._mname_pref + name + "_add")

    def mk_multiply(self, name):
        return tf.keras.layers.Multiply(name=self._mname_pref + name + "_mult")

    def mk_concat(self, name):
        return tf.keras.layers.Concatenate(name=self._mname_pref + name + "_concat")

    def mk_skip_fc(self,
                   units: int,
                   name: str,
                   levels: int,
                   dropout: float = 0,
                   dropout_impl=DEFAULT_DROPOUT_IMPL,
                   kernel_constraint=DEFAULT_KERNEL_CONSTRAINT):
        assert levels > 0

        def ret_layer_struct(level, br_name):
            fc = self.mk_fc(units = units, name = br_name, dropout=dropout, dropout_impl=dropout_impl, kernel_constraint=kernel_constraint)
            add = self.mk_add(name = br_name)

            if level == 1:
                sub_structs = []
            else:
                br_a = ret_layer_struct(level - 1, br_name=br_name + "a_l" + str(level))
                br_b = ret_layer_struct(level - 1, br_name=br_name + "b_l" + str(level))
                sub_structs = [br_a, br_b]

            return add, fc, sub_structs

        struct = ret_layer_struct(levels, name)

        def apply(input):
            def apply_struct(input, struct):
                add, fc, sub_structs = struct

                fc_output = fc(input)
                if len(sub_structs) == 0:
                    result = add([input, fc_output])
                else:
                    sub1, sub2 = sub_structs
                    result1 = apply_struct(input, sub1)
                    result2 = apply_struct(result1, sub2)
                    result = add([input, fc_output, result2])

                return result

            return apply_struct(input, struct)

        return apply        

    def mk_fc(self,
              units: int,
              name: str,
              dropout: float = 0,
              dropout_impl=DEFAULT_DROPOUT_IMPL,
              kernel_constraint=DEFAULT_KERNEL_CONSTRAINT,
              output: bool = False):
        if output:
            dense_name = name
        else:
            dense_name = self._mname_pref + name + "_fc"

        dense_layer = tf.keras.layers.Dense(
            name=dense_name,
            units=units,
            activation=DEFAULT_ACTIVATION,
            kernel_initializer=DEFAULT_KERNEL_INITIALIZER,
            kernel_constraint=kernel_constraint
        )

        if dropout > 0:
            drop_layer = dropout_impl(
                name=self._mname_pref + name + "_dropout",
                rate=dropout
            )
        else:
            drop_layer = None

        def apply(input):
            layer = dense_layer(input)
            if drop_layer is not None:
                layer = drop_layer(layer)
            return layer

        return apply
