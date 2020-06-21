import tensorflow as tf
import json

# tf.debugging.set_log_device_placement(True)

PH_PREDICTION_WINDOW_LENGTH = 60
PH_PREDICTION_FEATURES = 4
MODEL_FNAME = "data/co2-model.h5"

# ###############################################################################
# ###############################################################################
# ###############################################################################
# ###############################################################################
# MODEL CONSTRUCTION


def create_model(unroll_rrn: bool):
    input_layer = tf.keras.Input(
        name="Input",
        shape=[PH_PREDICTION_WINDOW_LENGTH, PH_PREDICTION_FEATURES]
    )

    conv0_layer = tf.keras.layers.Conv1D(
        name="Conv0",
        kernel_size=1,
        strides=1,
        filters=2,
        activation="selu",
        kernel_initializer='lecun_normal',
    )(input_layer)

    conv1_layer = tf.keras.layers.Conv1D(
        name="Conv1",
        kernel_size=4,
        strides=4,
        filters=4,
        activation="selu",
        kernel_initializer='lecun_normal',
    )(conv0_layer)

    conv2_layer = tf.keras.layers.Conv1D(
        name="Conv2",
        kernel_size=1,
        strides=1,
        filters=4,
        activation="selu",
        kernel_initializer='lecun_normal',
    )(conv1_layer)

    conv2n1_layer = tf.keras.layers.Add(
        name="Add2n1"
    )([conv2_layer, conv1_layer])

    act2n1_layer = tf.keras.layers.Activation(
        name='Act2n1',
        activation="selu"
    )(conv2n1_layer)

    conv3_layer = tf.keras.layers.Conv1D(
        name="Conv3",
        kernel_size=3,
        strides=3,
        filters=4,
        activation="selu",
        kernel_initializer='lecun_normal',
    )(act2n1_layer)

    flatten_layer = tf.keras.layers.Flatten(
        name="Flatten"
    )(conv3_layer)

    fc1_layer = tf.keras.layers.Dense(
        name="FullyConnected1",
        units=2,
        activation="selu",
        kernel_initializer='lecun_normal'
    )(flatten_layer)

    preoutput_layer = tf.keras.layers.Dense(
        name="PreOutput",
        units=1,
        activation="selu",
        kernel_initializer='lecun_normal'
    )(fc1_layer)

    output_layer = tf.keras.layers.Dense(
        name="Output",
        units=1,
        activation="selu",
        kernel_initializer='lecun_normal'
    )(preoutput_layer)

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
          unroll_rrn: bool,
          epochs: int,
          validation_freq: int):
    with open('temp/co2-train-data.json') as f:
        train_data = dataset_from_json(json.load(f))

    with open('temp/co2-valid-data.json') as f:
        valid_data = dataset_from_json(json.load(f))

    if retrain:
        print("##### CREATING NEW NETWORK")
        model = create_model(unroll_rrn=unroll_rrn)
    else:
        print("##### LOADING EXISTING NETWORK")
        model = tf.keras.models.load_model(MODEL_FNAME, compile=False)

    optimizer = tf.keras.optimizers.Adam(learning_rate)

    model.compile(loss="mean_squared_error", optimizer=optimizer)
    model.summary()

    model.fit(
        x=train_data,
        validation_data=valid_data,
        epochs=epochs,
        validation_freq=validation_freq,
    )

    model.save(MODEL_FNAME, save_format='h5')


# ###############################################################################
# ###############################################################################
# ###############################################################################
# ###############################################################################
# MAIN

if __name__ == '__main__':
    train(
        retrain=False,
        learning_rate=1e-4,
        unroll_rrn=True,
        epochs=50_000,
        validation_freq=40
    )
