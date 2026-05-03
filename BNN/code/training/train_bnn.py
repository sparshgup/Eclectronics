#!/usr/bin/env python3
"""Train a tiny BNN on Iris and emit binarized weights + thresholds.

Pipeline:
  1. Load Iris (4 features, 3 classes, 150 samples).
  2. Binarize each feature against its training-set median  -> 4 input bits.
  3. Train a 1-hidden-layer real-valued MLP (4 -> 16 -> 3) with sklearn.
  4. Sign-binarize the layer-1 weights.
  5. Pick a per-neuron threshold for layer 1 by scanning candidates and
     keeping the one that best matches the real-valued activation sign.
  6. Sign-binarize the layer-2 weights.
  7. Save everything as a JSON blob for export_weights.py.

The result is intentionally simple — there's no STE/binarization-aware
training loop; we accept a few % accuracy loss in exchange for a script
that runs in a couple of seconds with no GPU/PyTorch dependency.
"""

import json
import pathlib

import numpy as np
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split
from sklearn.neural_network import MLPClassifier

OUT_PATH = pathlib.Path(__file__).parent / "model.json"
HIDDEN   = 16
SEED     = 0


def binarize_features(X, medians):
    """Per-feature median split -> {0, 1}. Uses precomputed medians."""
    return (X > medians).astype(np.int8)


def main():
    iris = load_iris()
    X, y = iris.data, iris.target
    Xtr, Xte, ytr, yte = train_test_split(X, y, test_size=0.3, random_state=SEED, stratify=y)

    # ---- 1. Binarize inputs ----
    medians = np.median(Xtr, axis=0)
    Xtr_b = binarize_features(Xtr, medians)
    Xte_b = binarize_features(Xte, medians)

    # ---- 2. Train real-valued MLP on binary inputs ----
    mlp = MLPClassifier(
        hidden_layer_sizes=(HIDDEN,),
        activation="tanh",
        solver="lbfgs",
        max_iter=2000,
        random_state=SEED,
    )
    mlp.fit(Xtr_b, ytr)
    print(f"real-valued MLP test accuracy: {mlp.score(Xte_b, yte):.3f}")

    W1_real = mlp.coefs_[0]   # (4, 16)
    b1_real = mlp.intercepts_[0]
    W2_real = mlp.coefs_[1]   # (16, 3)
    b2_real = mlp.intercepts_[1]

    # ---- 3. Binarize layer 1 weights (sign -> {0, 1} representing {-1, +1}) ----
    W1_bin = (W1_real > 0).astype(np.int8)   # (4, 16)

    # ---- 4. Pick layer-1 thresholds by sweeping ----
    # For each hidden neuron, try thresholds 0..4 on popcount(XNOR) and pick
    # the one whose binary activation best matches sign(real activation) on Xtr.
    real_act = np.tanh(Xtr_b @ W1_real + b1_real)            # (n, 16)
    target   = (real_act > 0).astype(np.int8)                 # (n, 16)

    Xtr_pm = 2 * Xtr_b - 1                                    # {-1, +1}
    W1_pm  = 2 * W1_bin - 1                                   # {-1, +1}
    # popcount(XNOR(a, b)) = number of agreeing bits
    # = (4 + dot_pm) / 2  where dot_pm in {-4,-2,0,2,4}
    pop = ((Xtr_pm @ W1_pm) + 4) // 2                         # (n, 16) int in [0, 4]

    T1 = np.zeros(HIDDEN, dtype=np.int8)
    for i in range(HIDDEN):
        best_t, best_score = 2, -1
        for t in range(0, 5):
            pred = (pop[:, i] >= t).astype(np.int8)
            score = (pred == target[:, i]).sum()
            if score > best_score:
                best_score, best_t = score, t
        T1[i] = best_t

    # ---- 5. Compute binary hidden activations and binarize layer 2 ----
    hidden_bin = (pop >= T1).astype(np.int8)                  # (n, 16)
    W2_bin = (W2_real > 0).astype(np.int8)                    # (16, 3)

    # ---- 6. Evaluate the fully-binary pipeline ----
    def predict(Xb):
        Xpm = 2 * Xb - 1
        pop1 = ((Xpm @ W1_pm) + 4) // 2
        h    = (pop1 >= T1).astype(np.int8)
        hpm  = 2 * h - 1
        W2pm = 2 * W2_bin - 1
        # layer-2 score = popcount(XNOR(h, W2)) = (16 + dot)/2
        pop2 = ((hpm @ W2pm) + 16) // 2
        return np.argmax(pop2, axis=1)

    acc_tr = (predict(Xtr_b) == ytr).mean()
    acc_te = (predict(Xte_b) == yte).mean()
    print(f"binary BNN train acc: {acc_tr:.3f}   test acc: {acc_te:.3f}")

    # ---- 7. Save ----
    blob = {
        "feature_names": list(iris.feature_names),
        "class_names":   list(iris.target_names),
        "medians":       medians.tolist(),
        "W1": W1_bin.tolist(),    # shape (4, 16) — input feature x hidden neuron
        "T1": T1.tolist(),        # shape (16,)
        "W2": W2_bin.tolist(),    # shape (16, 3) — hidden neuron x class
    }
    OUT_PATH.write_text(json.dumps(blob, indent=2))
    print(f"wrote {OUT_PATH}")


if __name__ == "__main__":
    main()
