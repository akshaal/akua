from dataclasses import dataclass
from typing import Literal
import numpy as np

from print_utils import STRESS_COLOR, ff, print_info

MIN_BATCH_SIZE = 4096
DEFAULT_EARLY_STOP_EPOCH_PATIENCE = 5000
DEFAULT_NEW_MODEL_BATCH_SIZE = MIN_BATCH_SIZE
DEFAULT_NEW_MODEL_WEIGHT_DECAY_LR_MULT = 0.00001
DEFAULT_NEW_MODEL_OPT = "adm"

@dataclass
class TrainingSettings:
    learning_rate: float
    epochs: int
    first_decay_epochs: int
    validation_freq: int
    tensorboard: bool
    early_stop_epoch_patience: float
    weight_decay_lr_multiplier: float
    batch: int
    opt: Literal["adam", "sgd"]

    def print(self):
        print_info("Training settings:")
        print_info("    Initial learning rate: ",
                   STRESS_COLOR, ff(self.learning_rate))
        print_info("    Epochs: ", STRESS_COLOR, ff(self.epochs))
        print_info("    First decay epochs: ", STRESS_COLOR,
                   ff(self.first_decay_epochs))
        print_info("    Validation frequency: ",
                   STRESS_COLOR, ff(self.validation_freq))
        print_info("    Early stop epochs: ", STRESS_COLOR,
                   ff(self.early_stop_epoch_patience))
        print_info("    Weight decay multiplier: ", STRESS_COLOR,
                   ff(self.weight_decay_lr_multiplier))
        print_info("    Batch: ", STRESS_COLOR, str(self.batch))
        print_info("    Optimizer: ", STRESS_COLOR, self.opt)


def mk_new_model_training_settings(examples: int, early_stop_epoch_patience: int, max_lr: float):
    epochs = early_stop_epoch_patience * 3
    ts = TrainingSettings(
        learning_rate=max_lr,
        epochs=epochs,
        first_decay_epochs=epochs,
        validation_freq=10,
        batch=min(DEFAULT_NEW_MODEL_BATCH_SIZE, examples),
        tensorboard=False,
        early_stop_epoch_patience=early_stop_epoch_patience,
        weight_decay_lr_multiplier=DEFAULT_NEW_MODEL_WEIGHT_DECAY_LR_MULT,
        opt=DEFAULT_NEW_MODEL_OPT,
    )

    return ts


def mk_retrain_model_training_settings(examples: int, early_stop_epoch_patience: int, max_lr: float):
    ts = mk_new_model_training_settings(early_stop_epoch_patience=early_stop_epoch_patience,
                                        examples=examples,
                                        max_lr=max_lr)

    ts.learning_rate = min(float(np.random.choice(
        [1, 1e-1, 1e-2, 1e-3, 1e-4, 1e-4, 1e-5, 1e-5, 1e-6, 1e-6, 1e-6])) * float(np.random.random()), max_lr)
        
    ts.weight_decay_lr_multiplier = min(float(np.random.choice(
        [100, 10, 1, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 0])) * float(np.random.random()), 10)

    b = MIN_BATCH_SIZE
    batch_cands = [examples, b, b*2, b*4, b*8, b*16, b*32, b*64, b*128, b *
                   256, b*512, b*1024, b*2048, b*4096, b*8192, b*16384, b*32768, b*65536]
    ts.batch = int(np.random.choice([x for x in batch_cands if x <= examples]))

    return ts
