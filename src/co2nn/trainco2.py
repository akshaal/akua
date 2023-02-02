import tensorflow as tf
import json

# tf.debugging.set_log_device_placement(True)

from train_loop import train_loop
from model_builder import ModelBuilder

PH_PREDICTION_WINDOW_LENGTH = 60
PH_PREDICTION_FEATURES = 5

def create_model(name):
    mb = ModelBuilder(name)

    input = tf.keras.Input(
        name="Input",
        shape=[PH_PREDICTION_WINDOW_LENGTH, PH_PREDICTION_FEATURES]
    )

    conv0a = mb.mk_conv1d(name="L0a", kernel_size=1, strides=1, filters=6)(input)
    conv0b = mb.mk_conv1d(name="L0b", kernel_size=1, strides=1, filters=6)(conv0a)
    conv0c = mb.mk_conv1d(name="L0c", kernel_size=1, strides=1, filters=2)(conv0b)
    conv1 = mb.mk_conv1d(name="L1", kernel_size=4, strides=4, filters=15)(conv0c)
    conv2 = mb.mk_conv1d(name="L2", kernel_size=3, strides=3, filters=30)(conv1)
    flatten = mb.mk_flatten(name="Flatten")(conv2)
    fc1 = mb.mk_fc(name="L3", units=15)(flatten)
    fc2 = mb.mk_fc(name="L4", units=4)(fc1)
    output = mb.mk_fc(name="Output", units=1, output=True)(fc2)

    return tf.keras.Model(name=name, inputs=input, outputs=output)


def dataset_from_json(json):
    features = []
    labels = []

    for item in json:
        features.append(item['xs'])
        labels.append(item['ys'])

    return tf.data.Dataset.from_tensor_slices((features, labels)), len(features)


if __name__ == '__main__':
    def create_ds(train: bool):
        filename = 'temp/co2-train-data.json' if train else 'temp/co2-valid-data.json'
        with open(filename) as f:
            return dataset_from_json(json.load(f))

    train_loop(name="m1",
               create_model=create_model,
               create_ds=create_ds,
               loss={ "Output": tf.keras.losses.mean_squared_error },
               max_lr=0.01,
               loss_weights=None)
