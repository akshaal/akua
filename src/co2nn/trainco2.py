import tensorflow as tf
import json

from datetime import datetime

# tf.debugging.set_log_device_placement(True)

PH_PREDICTION_WINDOW_LENGTH = 60
PH_PREDICTION_FEATURES = 4
MODEL_FNAME = "data/co2-model.h5"

# ###############################################################################
# ###############################################################################
# ###############################################################################
# ###############################################################################
# MODEL CONSTRUCTION


def create_model():
    def mk_conv1d_layers(input_layer,
                         strides: int,
                         kernel_size: int,
                         filters: int,
                         name: str,
                         avgpool_strides: int = 0,
                         avgpool_size: int = 0,
                         maxpool_strides: int = 0,
                         maxpool_size: int = 0,
                         dropout: float = 0):
        layer = tf.keras.layers.Conv1D(
            name=name + "Conv",
            kernel_size=kernel_size,
            strides=strides,
            filters=filters,
            activation="selu",
            kernel_initializer='lecun_normal'
        )(input_layer)

        if dropout > 0:
            layer = tf.keras.layers.Dropout(
                name=name + "Dropout",
                rate=dropout
            )(layer)

        if avgpool_size > 0:
            layer = tf.keras.layers.AvgPool1D(
                name=name + "AvgPool",
                pool_size=avgpool_size,
                strides=avgpool_strides
            )(layer)

        if maxpool_size > 0:
            layer = tf.keras.layers.MaxPool1D(
                name=name + "MaxPool",
                pool_size=maxpool_size,
                strides=maxpool_strides
            )(layer)

        return layer

    def mk_fc_layers(input_layer, units: int, name: str, dropout: float = 0):
        layer = tf.keras.layers.Dense(
            name=name + "FC",
            units=units,
            activation="selu",
            kernel_initializer='lecun_normal'
        )(input_layer)

        if dropout > 0:
            layer = tf.keras.layers.Dropout(
                name=name + "Dropout",
                rate=dropout
            )(layer)

        return layer

    input_layer = tf.keras.Input(
        name="Input",
        shape=[PH_PREDICTION_WINDOW_LENGTH, PH_PREDICTION_FEATURES]
    )

    conv0_layer = mk_conv1d_layers(
        name="L0",
        input_layer=input_layer,
        kernel_size=1,
        strides=1,
        filters=2,
    )

    conv1_layer = mk_conv1d_layers(
        name="L1",
        input_layer=conv0_layer,
        kernel_size=4,
        strides=1,
        filters=4,
        dropout=0.75,
        maxpool_size=4,
        maxpool_strides=2
    )

    conv2_layer = mk_conv1d_layers(
        name="L2",
        input_layer=conv1_layer,
        kernel_size=3,
        strides=1,
        filters=6,
        dropout=0.75,
        maxpool_size=3,
        maxpool_strides=3,
    )

    flatten_layer = tf.keras.layers.Flatten(
        name="Flatten"
    )(conv2_layer)

    output_layer = tf.keras.layers.Dense(
        name="Output",
        units=1,
        activation="selu",
        kernel_initializer='lecun_normal'
    )(flatten_layer)

    model = tf.keras.Model(
        name="co2-predict2",
        inputs=input_layer,
        outputs=output_layer
    )

    return model

# ###############################################################################
# ###############################################################################
# ###############################################################################
# ###############################################################################
# DATASET


def dataset_from_json(json):
    features = []
    labels = []

    for item in json:
        features.append(item['xs'])
        labels.append(item['ys'])

    dataset = tf.data.Dataset.from_tensor_slices((features, labels))
    dataset = dataset.shuffle(10000)
    dataset = dataset.batch(len(features))
    dataset = dataset.prefetch(tf.data.experimental.AUTOTUNE)

    return dataset


# ###############################################################################
# ###############################################################################
# ###############################################################################
# ###############################################################################
# TRAINING

def train(retrain: bool,
          learning_rate: float,
          epochs: int,
          validation_freq: int,
          tensorboard: bool,
          early_stop_epoch_patience: float):
    with open('temp/co2-train-data.json') as f:
        train_data = dataset_from_json(json.load(f))

    with open('temp/co2-valid-data.json') as f:
        valid_data = dataset_from_json(json.load(f))

    if retrain:
        print("##### CREATING NEW NETWORK")
        model = create_model()
    else:
        print("##### LOADING EXISTING NETWORK")
        model = tf.keras.models.load_model(MODEL_FNAME, compile=False)

    optimizer = tf.keras.optimizers.Adam(learning_rate)

    def loss(y_true, y_pred):
        return tf.keras.backend.mean(
            tf.keras.backend.switch(
                tf.math.greater(y_pred, y_true),
                # penalty for optimistic prediction!
                tf.math.squared_difference(y_pred, y_true) * 3,
                tf.math.squared_difference(y_pred, y_true)
            ),
            axis=-1
        )

    model.compile(
        loss=loss,
        optimizer=optimizer,
        metrics=['mse']
    )

    model.summary()

    callbacks = []

    if tensorboard:
        print("ENABLING TENSORBOARD MAKES TRAINING SLOWER!!!!!!!!!")
        logdir = "temp/logs/co2-fit/" + datetime.now().strftime("%Y%m%d-%H%M%S")
        callbacks.append(
            tf.keras.callbacks.TensorBoard(
                log_dir=logdir,
                write_images=True,
                write_graph=True,
            )
        )

    if early_stop_epoch_patience > 0:
        # Custom class to avoid having errors because validation_freq
        class CustomEarlyStopping(tf.keras.callbacks.EarlyStopping):
            def __init__(self, *args, **kwargs):
                super().__init__(*args, **kwargs)
                self.__error_given = False

            def get_monitor_value(self, logs):
                logs = logs or {}
                monitor_value = logs.get(self.monitor)
                if monitor_value is None:
                    if not self.__error_given:
                        self.__error_given = True
                        return super().get_monitor_value(logs)
                    return None
                return monitor_value

        callbacks.append(
            CustomEarlyStopping(
                monitor='val_loss',
                patience=early_stop_epoch_patience,
                restore_best_weights=True,
                verbose=True
            )
        )

    model.fit(
        x=train_data,
        validation_data=valid_data,
        epochs=epochs,
        validation_freq=validation_freq,
        callbacks=callbacks
    )

    model.save(MODEL_FNAME, save_format='h5')


# ###############################################################################
# ###############################################################################
# ###############################################################################
# ###############################################################################
# MAIN

if __name__ == '__main__':
    train(
        retrain=True,
        learning_rate=1e-2,
        epochs=300_000,
        validation_freq=100,
        tensorboard=False,
        early_stop_epoch_patience=30_000
    )
