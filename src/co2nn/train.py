from datetime import datetime
import tensorflow as tf
import tensorflow_addons as tfa
from print_utils import INFO_COLOR, STRESS_COLOR, ff, print_error, print_info

from training_settings import TrainingSettings


# Returns if new model is better than original one
def train(ts: TrainingSettings,
          model: tf.keras.Model,
          train_ds: tf.data.Dataset,
          valid_ds: tf.data.Dataset,
          loss_weights: dict,
          loss) -> str:
    def wrap_ds(ds, no_config_batch = False):
        if no_config_batch:
            ds = ds.batch(ts.batch) # TODO: !!!!!!!!!!!! must use all items
        else:
            ds = ds.batch(ts.batch)
        return ds.prefetch(tf.data.experimental.AUTOTUNE)

    train_ds = wrap_ds(train_ds)
    valid_ds = wrap_ds(valid_ds, no_config_batch = True)

    lr_scheduler = tf.keras.experimental.CosineDecayRestarts(
        initial_learning_rate=ts.learning_rate,
        first_decay_steps=ts.first_decay_epochs,
        alpha=0.05
    )

    weight_decay = tf.Variable(
        0.0,
        name='weight_decay',
        trainable=False,
        dtype="float32",
        shape=()
    )

    if ts.opt == "sgd":
        optimizer = tfa.optimizers.SGDW(
            learning_rate=lr_scheduler,
            weight_decay=lambda: weight_decay,
            momentum=0.9
        )
    else:
        optimizer = tfa.optimizers.AdamW(
            learning_rate=lr_scheduler,
            weight_decay=lambda: weight_decay,
            # Taken from PDF for SELU activation function
            epsilon=0.01,
            beta_2=0.99
        )

    model.compile(
        loss=loss,
        optimizer=optimizer,
        loss_weights=loss_weights
    )

    def on_epoch_begin(epoch, _):
        current_lr = lr_scheduler(epoch)
        weight_decay.assign(ts.weight_decay_lr_multiplier * current_lr)
        print("LR=%0.4e, WD=%0.4e" % (current_lr.numpy(), weight_decay.numpy()))

    callbacks = [
        tf.keras.callbacks.LambdaCallback(on_epoch_begin=on_epoch_begin)
    ]

    if ts.tensorboard:
        print_error("ENABLING TENSORBOARD MAKES TRAINING SLOWER!!!!!!!!!")
        # TODO: !!!!!!!!!!!!
        logdir = "temp/logs/co2-fit/" + datetime.now().strftime("%Y%m%d-%H%M%S")
        callbacks.append(
            tf.keras.callbacks.TensorBoard(
                log_dir=logdir,
                write_images=True,
                write_graph=True,
            )
        )

    def eval_val_loss():
        result = model.evaluate(x=valid_ds)

        # Check that type of result is float
        if isinstance(result, float):
            return result
        else:
            return dict(zip(model.metrics_names, result))['loss']

    initial_val_lost = eval_val_loss()
    print_info("Initial validation loss: ", STRESS_COLOR, ff(initial_val_lost))

    early = False

    if ts.early_stop_epoch_patience > 0:
        patience = int(ts.early_stop_epoch_patience / ts.validation_freq)

        # Best weights are the initial ones
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
                print("")
                print_info("Best validation loss so far: ", STRESS_COLOR, ff(self.best), INFO_COLOR,
                           ". It was ", STRESS_COLOR, str((self.wait+1)*ts.validation_freq), INFO_COLOR,
                           " epochs ago (out of ", STRESS_COLOR, str(ts.early_stop_epoch_patience), INFO_COLOR, " allowed).")
                print("")
                return monitor_value

            def on_train_begin(self, logs):
                super().on_train_begin(logs)
                self.best = initial_val_lost
                self.best_weights = initial_best_weights

            def on_train_end(self, _=None):
                super().on_train_end()
                if self.stopped_epoch == 0 and self.best_weights:
                    print_info('Not early stop. But still restoring model weights from the end of the best validation-epoch.')
                    self.model.set_weights(self.best_weights)
                else:
                    nonlocal early
                    early = True

        callbacks.append(CustomEarlyStopping(
            monitor='val_loss',
            # Because we only use validation loss which appears 1 / validation_freq cases
            patience=patience,
            restore_best_weights=True,
            verbose=1
        ))

    ts.print()

    model.fit(
        x=train_ds,
        validation_data=valid_ds,
        epochs=ts.epochs,
        validation_freq=ts.validation_freq,
        callbacks=callbacks
    )

    trained_val_lost = eval_val_loss()
    print_info("Trained validation loss: ", STRESS_COLOR, ff(trained_val_lost))
    print_info("Initial validation loss: ", STRESS_COLOR, ff(initial_val_lost))

    if trained_val_lost < initial_val_lost:
        ctx = "b=" + str(ts.batch) + ", lr=" + ff(ts.learning_rate) + ", wd=" + ff(ts.weight_decay_lr_multiplier)
        return early, ff(initial_val_lost) + " => " + ff(trained_val_lost) + "(" + ctx + ")"
    else:
        return early, None
        