# -*- coding: utf-8 -*-

import numpy as np
from collections import deque, Counter
from scipy.signal import butter, filtfilt, iirnotch
from sklearn.cross_decomposition import CCA


class FBCCAClassifier:
    def __init__(
        self,
        freqs,
        fs=1000,
        window_size=1.5,
        n_harmonics=3,
        low=6,
        high=40,
        notch=50
    ):
        self.freqs = freqs
        self.fs = fs
        self.window_size = window_size
        self.n_harmonics = n_harmonics
        self.window_samples = int(window_size * fs)

        self.bp_b, self.bp_a = self._build_bandpass(low, high)
        self.notch_b, self.notch_a = self._build_notch(notch)

        self.refs = {
            f: self._make_reference(f)
            for f in self.freqs
        }

    def _build_bandpass(self, low, high, order=4):
        nyq = 0.5 * self.fs

        b, a = butter(
            order,
            [low / nyq, high / nyq],
            btype="bandpass"
        )

        return b, a

    def _build_notch(self, freq=50, q=30):
        nyq = 0.5 * self.fs

        b, a = iirnotch(
            freq / nyq,
            q
        )

        return b, a

    def _make_reference(self, freq):
        t = np.arange(self.window_samples) / self.fs

        refs = []

        for h in range(1, self.n_harmonics + 1):
            refs.append(np.sin(2 * np.pi * h * freq * t))
            refs.append(np.cos(2 * np.pi * h * freq * t))

        return np.array(refs, dtype=np.float32)

    def _preprocess(self, eeg):
        eeg = np.asarray(eeg, dtype=np.float32)

        eeg = eeg - np.mean(eeg, axis=1, keepdims=True)

        eeg = filtfilt(
            self.bp_b,
            self.bp_a,
            eeg,
            axis=1
        )

        eeg = filtfilt(
            self.notch_b,
            self.notch_a,
            eeg,
            axis=1
        )

        eeg = eeg - np.mean(eeg, axis=1, keepdims=True)

        std = np.std(eeg, axis=1, keepdims=True)
        std[std < 1e-6] = 1.0

        eeg = eeg / std

        return eeg

    def _cca_score(self, eeg, ref):
        try:
            cca = CCA(
                n_components=1,
                max_iter=1000
            )

            x = eeg.T
            y = ref.T

            cca.fit(x, y)

            u, v = cca.transform(x, y)

            r = np.corrcoef(
                u[:, 0],
                v[:, 0]
            )[0, 1]

            if np.isnan(r):
                return 0.0

            return float(abs(r))

        except Exception:
            return 0.0

    def predict(self, eeg):
        """
        eeg:
            shape = channels × samples
        """

        if eeg is None:
            return None, 0.0, {}

        eeg = np.asarray(eeg)

        if eeg.ndim != 2:
            return None, 0.0, {}

        if eeg.shape[1] < self.window_samples:
            return None, 0.0, {}

        eeg = eeg[:, -self.window_samples:]

        if not np.isfinite(eeg).all():
            return None, 0.0, {}

        eeg = self._preprocess(eeg)

        scores = {}

        for f in self.freqs:
            scores[f] = self._cca_score(
                eeg,
                self.refs[f]
            )

        best_freq = max(
            scores,
            key=scores.get
        )

        best_score = scores[best_freq]

        return best_freq, best_score, scores


class DecisionSmoother:
    def __init__(
        self,
        window_size=5,
        min_count=3
    ):
        self.window_size = window_size
        self.min_count = min_count
        self.history = deque(maxlen=window_size)

    def update(
        self,
        result,
        score,
        threshold=0.25
    ):
        if result is None:
            return None

        if score < threshold:
            return None

        self.history.append(result)

        if len(self.history) < self.window_size:
            return None

        counter = Counter(self.history)

        decision, count = counter.most_common(1)[0]

        if count >= self.min_count:
            return decision

        return None

    def reset(self):
        self.history.clear()