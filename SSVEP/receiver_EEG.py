# -*- coding: utf-8 -*-

import time
import numpy as np
from API.neuracle_lib.dataServer import DataServerThread


class NeuracleEEG:
    def __init__(
        self,
        host="127.0.0.1",
        port=8712,
        device="Neuracle",
        channels=64,
        fs=1000,
        buffer_seconds=5
    ):
        self.host = host
        self.port = port
        self.device = device
        self.channels = channels
        self.fs = fs
        self.buffer_seconds = buffer_seconds

        self.server = DataServerThread(
            device=self.device,
            n_chan=self.channels,
            srate=self.fs,
            t_buffer=self.buffer_seconds
        )

        self.connected = False

    def start(self):
        notconnect = self.server.connect(
            hostname=self.host,
            port=self.port
        )

        if notconnect:
            raise RuntimeError(
                "Cannot connect to Neuracle DataServer. "
                "请确认 Neusen W 已打开数据转发，端口是否为 8712。"
            )

        self.server.Daemon = True
        self.server.start()
        self.connected = True

        print("Neuracle DataServer connected.")
        print(f"device={self.device}, channels={self.channels}, fs={self.fs}")

    def get_data(self, window=1.0):
        if not self.connected:
            return np.empty((self.channels, 0))

        data = self.server.GetBufferData()

        n_samples = int(window * self.fs)

        if data.shape[1] < n_samples:
            return data.copy()

        return data[:, -n_samples:].copy()

    def get_update_count(self):
        return self.server.GetDataLenCount()

    def reset_update_count(self):
        self.server.ResetDataLenCount()

    def stop(self):
        try:
            self.server.stop()
        except Exception:
            pass

        self.connected = False
        print("Neuracle DataServer stopped.")