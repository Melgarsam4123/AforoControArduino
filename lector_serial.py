import threading
import serial
import time
import re

class AforoReader:
    def __init__(self, puerto="COM3", baudrate=9600):
        self.serial_port = serial.Serial(puerto, baudrate, timeout=1)
        self.in_count = 0
        self.out_count = 0
        self.aforo = 0
        self.lock = threading.Lock()

        self.thread = threading.Thread(target=self.leer_serial)
        self.thread.daemon = True
        self.thread.start()

    def leer_serial(self):
        while True:
            if self.serial_port.in_waiting > 0:
                try:
                    linea = self.serial_port.readline().decode("utf-8").strip()
                    match = re.match(r"IN:(\d+),OUT:(\d+),AFORO:(\d+)", linea)
                    if match:
                        with self.lock:
                            self.in_count = int(match.group(1))
                            self.out_count = int(match.group(2))
                            self.aforo = int(match.group(3))
                except Exception as e:
                    print("Error:", e)
            time.sleep(0.1)

    def obtener_datos(self):
        with self.lock:
            return {
                "entradas": self.in_count,
                "salidas": self.out_count,
                "aforo": self.aforo
            }
