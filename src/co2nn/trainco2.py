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

    rrn1_layer = tf.keras.layers.Bidirectional(
        name="Rrn1",
        layer=tf.keras.layers.LSTM(
            units=2,
            activation="selu",
            kernel_initializer='lecun_normal',
            return_sequences=True,
            unroll=unroll_rrn
        )
    )(input_layer)

    rrn2_layer = tf.keras.layers.Bidirectional(
        name="Rrn2",
        layer=tf.keras.layers.LSTM(
            units=2,
            activation=None,
            kernel_initializer='lecun_normal',
            return_sequences=True,
            unroll=unroll_rrn
        )
    )(rrn1_layer)

    rrn1n2_layer = tf.keras.layers.Add(
        name="Rrn1n2"
    )([rrn1_layer, rrn2_layer])

    rrn1n2_act_layer = tf.keras.layers.Activation(
        name="Rrn1n2Act",
        activation="selu"
    )(rrn1n2_layer)

    # ---- rrn3 ----

    rrn3_layer = tf.keras.layers.Bidirectional(
        name="Rrn3",
        layer=tf.keras.layers.LSTM(
            units=2,
            activation="selu",
            kernel_initializer='lecun_normal',
            unroll=unroll_rrn
        )
    )(rrn1n2_act_layer)

    fc1_layer = tf.keras.layers.Dense(
        name="FullyConnected1",
        units=3,
        activation="selu",
        kernel_initializer='lecun_normal'
    )(rrn3_layer)

    output_layer = tf.keras.layers.Dense(
        name="Output",
        units=1,
        activation="relu",
        kernel_initializer='lecun_normal'
    )(fc1_layer)

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
        epochs=5000,
        validation_freq=20
    )
