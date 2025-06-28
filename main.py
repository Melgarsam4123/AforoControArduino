import tkinter as tk
from serial import Serial
import threading
import time
import re

class LCDSimulador:
    def __init__(self, master):
        self.master = master
        self.master.title("LCD 16x2 - Aforo")
        self.line1 = tk.StringVar()
        self.line2 = tk.StringVar()

        self.label1 = tk.Label(master, textvariable=self.line1, font=("Courier", 16), bg="black", fg="green", width=16)
        self.label1.pack()
        self.label2 = tk.Label(master, textvariable=self.line2, font=("Courier", 16), bg="black", fg="green", width=16)
        self.label2.pack()

        self.line1.set("Esperando datos")
        self.line2.set("...")

    def actualizar_lineas(self, linea1, linea2):
        self.line1.set(linea1.ljust(16)[:16])
        self.line2.set(linea2.ljust(16)[:16])

class MonitorSerial:
    def __init__(self, puerto, baudrate, interfaz):
        self.serial_port = Serial(puerto, baudrate, timeout=1)
        self.ui = interfaz
        self.thread = threading.Thread(target=self.escuchar_serial)
        self.thread.daemon = True
        self.thread.start()

    def escuchar_serial(self):
        while True:
            if self.serial_port.in_waiting > 0:
                try:
                    linea = self.serial_port.readline().decode('utf-8').strip()
                    print(">>", linea)  # Para depuración

                    # Extrae datos del tipo IN:2,OUT:1,AFORO:5
                    match = re.match(r"IN:(\d+),OUT:(\d+),AFORO:(\d+)", linea)
                    if match:
                        entrada = int(match.group(1))
                        salida = int(match.group(2))
                        aforo = int(match.group(3))
                        linea1 = f"Aforo: {aforo}"
                        linea2 = f"In:{entrada} Out:{salida}"
                        self.ui.actualizar_lineas(linea1, linea2)
                except Exception as e:
                    print("Error:", e)
            time.sleep(0.1)

def main():
    puerto = "COM3"  # ⬅️ CAMBIA esto por el puerto que use tu Arduino (ej: /dev/ttyUSB0 en Linux)
    baudrate = 9600

    root = tk.Tk()
    lcd_ui = LCDSimulador(root)
    MonitorSerial(puerto, baudrate, lcd_ui)
    root.mainloop()

if __name__ == "__main__":
    main()
