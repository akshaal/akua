import tensorflow as tf
import tensorflow_addons as tfa
import json
import numpy as np

from datetime import datetime
from typing import Literal

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
        filters=10,
        maxpool_size=4,
        maxpool_strides=2
    )

    conv2_layer = mk_conv1d_layers(
        name="L2",
        input_layer=conv1_layer,
        kernel_size=3,
        strides=1,
        filters=20,
        maxpool_size=3,
        maxpool_strides=3,
    )

    flatten_layer = tf.keras.layers.Flatten(
        name="Flatten"
    )(conv2_layer)

    fc1 = mk_fc_layers(
        name="L3",
        input_layer=flatten_layer,
        units=4,
    )

    output_layer = tf.keras.layers.Dense(
        name="Output",
        units=1,
        activation="selu",
        kernel_initializer='lecun_normal'
    )(fc1)

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
          first_decay_epochs: int,
          validation_freq: int,
          tensorboard: bool,
          early_stop_epoch_patience: float,
          weight_decay_lr_multiplier: float,
          opt: Literal["adam", "sgd"]):
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

    # scheduler for learning rate
    lr_scheduler = tf.keras.experimental.CosineDecayRestarts(
        initial_learning_rate=learning_rate,
        first_decay_steps=first_decay_epochs,
        alpha=0.05
    )

    weight_decay = tf.Variable(
        0.0,
        name='weight_decay',
        trainable=False,
        dtype="float32",
        shape=()
    )

    if opt == "sgd":
        print("Using SGDW Optimizer")
        optimizer = tfa.optimizers.SGDW(
            learning_rate=lr_scheduler,
            weight_decay=lambda: weight_decay
        )
    else:
        print("Using AdamW Optimizer")
        optimizer = tfa.optimizers.AdamW(
            learning_rate=lr_scheduler,
            weight_decay=lambda: weight_decay
        )

    model.compile(
        loss=tf.keras.losses.mean_squared_error,
        optimizer=optimizer
    )

    model.summary()

    def on_epoch_begin(epoch, logs):
        current_lr = lr_scheduler(epoch)
        weight_decay.assign(weight_decay_lr_multiplier * current_lr)
        print("LR=%0.4e, WD=%0.4e" %
              (current_lr.numpy(), weight_decay.numpy()))

    callbacks = [
        tf.keras.callbacks.LambdaCallback(on_epoch_begin=on_epoch_begin)
    ]

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
        patience = int(early_stop_epoch_patience / validation_freq)

        # Best weights are the initial ones
        initial_best = model.evaluate(x=valid_data)
        initial_best_weights = model.get_weights()

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
                print((",  epochs since the best valid_loss (%0.4e" % (self.best,)) + "): " +
                      str(self.wait*validation_freq) + "/" + str(early_stop_epoch_patience))
                return monitor_value

            def on_train_begin(self, logs):
                super().on_train_begin(logs)
                self.best = initial_best
                self.best_weights = initial_best_weights

            def on_train_end(self, logs=None):
                super().on_train_end()
                if self.stopped_epoch == 0 and self.best_weights:
                    print(
                        'AK: Not early stop. But still restoring model weights from the end of the best validation-epoch.')
                    self.model.set_weights(self.best_weights)

        callbacks.append(CustomEarlyStopping(
            monitor='val_loss',
            # Because we only use validation loss which appears 1 / validation_freq cases
            patience=patience,
            restore_best_weights=True,
            verbose=1
        ))

    model.fit(
        x=train_data,
        validation_data=valid_data,
        epochs=epochs,
        validation_freq=validation_freq,
        callbacks=callbacks
    )

    model.save(MODEL_FNAME, save_format='h5', include_optimizer=False)


# ###############################################################################
# ###############################################################################
# ###############################################################################
# ###############################################################################
# MAIN

if __name__ == '__main__':
    train(
        retrain=False,
        learning_rate=1e-2,
        weight_decay_lr_multiplier=0.01 * np.random.random(),
        epochs=600_000,
        first_decay_epochs=30_000,
        validation_freq=20,
        tensorboard=False,
        early_stop_epoch_patience=100_000,
        opt="adam"
    )
