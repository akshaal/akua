from datetime import datetime
import tensorflow as tf
import os
import os.path

from print_utils import STRESS_COLOR, print_info
from train import train
from training_settings import DEFAULT_EARLY_STOP_EPOCH_PATIENCE, mk_new_model_training_settings, mk_retrain_model_training_settings


def train_loop(name: str,
               create_model,
               create_ds,
               loss,
               max_lr: float = 0.01,
               loss_weights=None):
    model_dirname = "temp/model-" + name
    os.makedirs(model_dirname, exist_ok=True)

    model_filename = model_dirname + "/model"
    log_filename = model_dirname + "/log"

    next_reuse_ts = None

    while True:
        if loss_weights != None:
            print_info("Loss weights ", STRESS_COLOR, loss_weights)

        train_ds, train_examples_count = create_ds(True)
        valid_ds, valid_examples_count = create_ds(False)

        train_ds = train_ds.shuffle(train_examples_count, reshuffle_each_iteration=True)

        print_info("Training examples count ", STRESS_COLOR, train_examples_count)
        print_info("Validation examples count ", STRESS_COLOR, valid_examples_count)

        if os.path.exists(model_filename):
            print_info("Loading existing model.")
            ts = mk_retrain_model_training_settings(
                examples=train_examples_count,
                early_stop_epoch_patience=DEFAULT_EARLY_STOP_EPOCH_PATIENCE,
                max_lr=max_lr / 10.0
            )
            model = tf.keras.models.load_model(model_filename, compile=False)
        else:
            print_info("Creating new model.")
            ts = mk_new_model_training_settings(
                examples=train_examples_count,
                early_stop_epoch_patience=DEFAULT_EARLY_STOP_EPOCH_PATIENCE*20,
                max_lr=max_lr
            )
            model = create_model(name)

        model.summary()

        model_img_filename = model_filename + ".png"
        tf.keras.utils.plot_model(
            model,
            to_file=model_img_filename,
            show_shapes=True,
            show_dtype=False,
            show_layer_names=True,
            rankdir="TB",
            expand_nested=False,
            dpi=96,
            layer_range=None,
        )

        if next_reuse_ts is not None:
            print_info(
                "Reusing previous training settings with lr adjusted (previous training was not interrupted).")
            ts = next_reuse_ts
            ts.learning_rate = ts.learning_rate / 10.0

        early, improvement = train(ts=ts,
                                   model=model,
                                   train_ds=train_ds,
                                   valid_ds=valid_ds,
                                   loss_weights=loss_weights,
                                   loss=loss)

        if early:
            next_reuse_ts = None
        else:
            next_reuse_ts = ts

        if improvement is None:
            print_info("Nothing is improved.")
        else:
            print_info("Trained model is worth saving.")

            timestamp = datetime.now().strftime("%Y-%m-%d-%H-%M-%S")

            if os.path.exists(model_filename):
                bck_file = model_filename + "-" + timestamp
                os.rename(model_filename, bck_file)

            model.save(model_filename, save_format='h5',
                       include_optimizer=False)

            with open(log_filename, "a") as f:
                f.write(timestamp + ": " + improvement + "\n")
