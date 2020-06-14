import tensorflow as tf
import json

# TODO: Cleanup

# tf.debugging.set_log_device_placement(True)

# TODO: Same as in JS
PH_PREDICTION_WINDOW_LENGTH = 60

# TODO: Move
learning_rate = 5e-5

unroll_gru = True

model_fname = "data/co2-model.h5"

# ###############################################################################


def create_model():
    input_layer = tf.keras.Input(
        name="Input",
        shape=[PH_PREDICTION_WINDOW_LENGTH, 4]
    )

    gru1_layer = tf.keras.layers.Bidirectional(
        name="Gru1",
        layer=tf.keras.layers.GRU(
            units=4,
            activation="selu",
            kernel_initializer='lecun_normal',
            return_sequences=True,
            unroll=unroll_gru
        )
    )(input_layer)

    gru2_layer = tf.keras.layers.Bidirectional(
        name="Gru2",
        layer=tf.keras.layers.GRU(
            units=4,
            activation=None,
            kernel_initializer='lecun_normal',
            return_sequences=True,
            unroll=unroll_gru
        )
    )(gru1_layer)

    gru1n2_layer = tf.keras.layers.Add(
        name="Gru1n2"
    )([gru1_layer, gru2_layer])

    gru1n2_act_layer = tf.keras.layers.Activation(
        name="Gru1n2Act",
        activation="selu"
    )(gru1n2_layer)

    # ---- GRU3 ----

    gru3_layer = tf.keras.layers.Bidirectional(
        name="Gru3",
        layer=tf.keras.layers.GRU(
            units=4,
            activation="selu",
            kernel_initializer='lecun_normal',
            unroll=unroll_gru
        )
    )(gru1n2_act_layer)

    fc1_layer = tf.keras.layers.Dense(
        name="FullyConnected1",
        units=3,
        activation="selu",
        kernel_initializer='lecun_normal'
    )(gru3_layer)

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


def dataset_from_json(json):
    features = []
    labels = []

    for item in json:
        features.append(item['xs'])
        labels.append(item['ys'])

    dataset = tf.data.Dataset.from_tensor_slices((features, labels))
    dataset = dataset.batch(len(features))
    dataset = dataset.prefetch(tf.data.experimental.AUTOTUNE)

    return dataset


with open('temp/co2-train-data.json') as f:
    train_data = dataset_from_json(json.load(f))

with open('temp/co2-valid-data.json') as f:
    valid_data = dataset_from_json(json.load(f))

optimizer = tf.keras.optimizers.Adam(learning_rate)

model = tf.keras.models.load_model(model_fname, compile=False)
model.compile(loss="mean_squared_error", optimizer=optimizer)
model.summary()

print("Num GPUs Available: ", len(
    tf.config.experimental.list_physical_devices('GPU')))

model.fit(
    x=train_data,
    validation_data=valid_data,
    epochs=50_000,  # TODO: Move
)

model.save(model_fname, save_format='h5')
