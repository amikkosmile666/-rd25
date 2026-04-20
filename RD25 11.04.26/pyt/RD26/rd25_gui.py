import sys
import serial
import serial.tools.list_ports
from PySide6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                               QHBoxLayout, QGridLayout, QLabel, QPushButton,
                               QLineEdit, QComboBox, QTextEdit, QGroupBox,
                               QSpinBox, QTabWidget, QFileDialog, QMessageBox,
                               QListWidget, QListWidgetItem)
from PySide6.QtCore import QTimer, QThread, Signal

class SerialThread(QThread):
    data_received = Signal(str)
    error_occurred = Signal(str)

    def __init__(self, port, baudrate=115200):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.serial = None
        self.running = False

    def run(self):
        try:
            self.serial = serial.Serial(self.port, self.baudrate, timeout=0.01)
            self.serial.reset_input_buffer()
            self.serial.reset_output_buffer()
            self.running = True
            while self.running:
                if self.serial.in_waiting:
                    line = self.serial.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        self.data_received.emit(line)
                self.msleep(5)
        except Exception as e:
            self.error_occurred.emit(str(e))

    def send_command(self, cmd):
        if self.serial and self.serial.is_open:
            self.serial.write((cmd + "\n").encode())

    def stop(self):
        self.running = False
        if self.serial and self.serial.is_open:
            self.serial.close()
        self.wait()

class RD25App(QMainWindow):
    def __init__(self):
        super().__init__()
        self.serial_thread = None
        self.sensors_data = [[None, None] for _ in range(8)]
        self.sensors_present = [False] * 8
        self.file_list = []
        self.downloading_file = None
        self.log_file_content = []
        self.remaining_seconds = 0
        self.initUI()

    def add_log(self, msg):
        MAX_LOG_LINES = 300
        self.log_text.append(msg)
        if self.log_text.document().blockCount() > MAX_LOG_LINES:
            cursor = self.log_text.textCursor()
            cursor.movePosition(cursor.Start)
            cursor.movePosition(cursor.Down, cursor.KeepAnchor, 50)
            cursor.removeSelectedText()

    def initUI(self):
        self.setWindowTitle("RD25 - Управление измерителем")
        self.setGeometry(100, 100, 1000, 700)

        central = QWidget()
        self.setCentralWidget(central)
        main_layout = QVBoxLayout(central)

        # Панель подключения
        port_layout = QHBoxLayout()
        port_layout.addWidget(QLabel("COM-порт:"))
        self.port_combo = QComboBox()
        self.refresh_ports()
        port_layout.addWidget(self.port_combo)
        self.refresh_btn = QPushButton("Обновить")
        self.refresh_btn.clicked.connect(self.refresh_ports)
        port_layout.addWidget(self.refresh_btn)
        self.connect_btn = QPushButton("Подключиться")
        self.connect_btn.clicked.connect(self.toggle_connection)
        port_layout.addWidget(self.connect_btn)
        self.status_label = QLabel("Не подключено")
        self.status_label.setStyleSheet("color: red;")
        port_layout.addWidget(self.status_label)
        port_layout.addStretch()
        main_layout.addLayout(port_layout)

        # Вкладки
        tabs = QTabWidget()
        main_layout.addWidget(tabs)

        # === Вкладка "Датчики" ===
        sensors_tab = QWidget()
        tabs.addTab(sensors_tab, "Датчики")
        sensors_layout = QGridLayout(sensors_tab)

        interval_group = QGroupBox("Настройка интервала опроса")
        interval_layout = QHBoxLayout(interval_group)
        interval_layout.addWidget(QLabel("Интервал (сек):"))
        self.poll_spin = QSpinBox()
        self.poll_spin.setRange(1, 86400)
        self.poll_spin.setValue(1)
        self.poll_spin.valueChanged.connect(self.on_poll_interval_changed)
        interval_layout.addWidget(self.poll_spin)
        self.poll_status = QLabel("")
        interval_layout.addWidget(self.poll_status)
        sensors_layout.addWidget(interval_group, 0, 0, 1, 4)

        self.sensor_labels = []
        for i in range(8):
            gb = QGroupBox(f"Канал {i+1}")
            vb = QVBoxLayout()
            t_lbl = QLabel("Темп: ---")
            h_lbl = QLabel("Влаж: ---")
            s_lbl = QLabel("Статус: ---")
            vb.addWidget(t_lbl)
            vb.addWidget(h_lbl)
            vb.addWidget(s_lbl)
            gb.setLayout(vb)
            sensors_layout.addWidget(gb, i//4+1, i%4)
            self.sensor_labels.append((t_lbl, h_lbl, s_lbl))

        btn_layout = QHBoxLayout()
        self.read_btn = QPushButton("Прочитать сейчас")
        self.read_btn.clicked.connect(lambda: self.send("READ_SENSORS_NOW"))
        self.scan_btn = QPushButton("Сканировать датчики")
        self.scan_btn.clicked.connect(lambda: self.send("SCAN_SENSORS"))
        btn_layout.addWidget(self.read_btn)
        btn_layout.addWidget(self.scan_btn)
        sensors_layout.addLayout(btn_layout, 3, 0, 1, 4)

        # === Вкладка "Запись данных" ===
        log_tab = QWidget()
        tabs.addTab(log_tab, "Запись данных")
        log_layout = QVBoxLayout(log_tab)

        log_settings = QGroupBox("Настройки записи")
        log_settings_layout = QGridLayout(log_settings)
        log_settings_layout.addWidget(QLabel("Интервал записи (сек):"), 0, 0)
        self.log_int_spin = QSpinBox()
        self.log_int_spin.setRange(1, 86400)
        self.log_int_spin.setValue(60)
        self.log_int_spin.valueChanged.connect(self.on_log_interval_changed)
        log_settings_layout.addWidget(self.log_int_spin, 0, 1)

        log_settings_layout.addWidget(QLabel("Длительность записи (сек, 0=беск.):"), 1, 0)
        self.log_dur_spin = QSpinBox()
        self.log_dur_spin.setRange(0, 86400*365)
        self.log_dur_spin.setValue(0)
        self.log_dur_spin.valueChanged.connect(self.on_log_duration_changed)
        log_settings_layout.addWidget(self.log_dur_spin, 1, 1)

        self.remaining_label = QLabel("Осталось: --")
        self.remaining_label.setStyleSheet("font-weight: bold; font-size: 14px;")
        log_settings_layout.addWidget(self.remaining_label, 2, 0, 1, 2)

        log_btn_layout = QHBoxLayout()
        self.start_log_btn = QPushButton("Запустить запись")
        self.start_log_btn.clicked.connect(lambda: self.send("START_LOGGING"))
        self.stop_log_btn = QPushButton("Остановить запись")
        self.stop_log_btn.clicked.connect(lambda: self.send("STOP_LOGGING"))
        log_btn_layout.addWidget(self.start_log_btn)
        log_btn_layout.addWidget(self.stop_log_btn)
        log_settings_layout.addLayout(log_btn_layout, 3, 0, 1, 2)

        log_layout.addWidget(log_settings)

        file_group = QGroupBox("Файлы данных на устройстве")
        file_layout = QVBoxLayout(file_group)
        file_buttons = QHBoxLayout()
        self.refresh_files_btn = QPushButton("Обновить список")
        self.refresh_files_btn.clicked.connect(self.refresh_file_list)
        self.download_selected_btn = QPushButton("Скачать выбранный")
        self.download_selected_btn.clicked.connect(self.download_selected_file)
        self.delete_all_btn = QPushButton("Удалить все файлы")
        self.delete_all_btn.clicked.connect(lambda: self.send("CLEAR_FILES"))
        file_buttons.addWidget(self.refresh_files_btn)
        file_buttons.addWidget(self.download_selected_btn)
        file_buttons.addWidget(self.delete_all_btn)
        file_layout.addLayout(file_buttons)

        self.file_list_widget = QListWidget()
        self.file_list_widget.itemDoubleClicked.connect(self.download_selected_file)
        file_layout.addWidget(self.file_list_widget)
        log_layout.addWidget(file_group)

        log_messages = QGroupBox("Системные сообщения")
        msg_layout = QVBoxLayout(log_messages)
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        msg_layout.addWidget(self.log_text)
        log_layout.addWidget(log_messages)

        # === Вкладка "WiFi" (упрощённо, без активного WiFi) ===
        wifi_tab = QWidget()
        tabs.addTab(wifi_tab, "WiFi (не используется)")
        wifi_layout = QVBoxLayout(wifi_tab)
        wifi_info = QLabel("В данной версии WiFi и веб-сервер отключены.\n"
                           "Управление только через USB.")
        wifi_info.setWordWrap(True)
        wifi_layout.addWidget(wifi_info)
        wifi_layout.addStretch()

        # Таймер для обновления оставшегося времени
        self.remaining_timer = QTimer()
        self.remaining_timer.timeout.connect(self.update_remaining_display)
        self.remaining_timer.start(1000)

    def refresh_ports(self):
        self.port_combo.clear()
        for port in serial.tools.list_ports.comports():
            self.port_combo.addItem(port.device)

    def toggle_connection(self):
        if self.serial_thread is None:
            port = self.port_combo.currentText()
            if not port:
                QMessageBox.warning(self, "Ошибка", "Выберите COM-порт")
                return
            self.serial_thread = SerialThread(port)
            self.serial_thread.data_received.connect(self.handle_data)
            self.serial_thread.error_occurred.connect(self.handle_error)
            self.serial_thread.start()
            self.connect_btn.setText("Отключиться")
            self.status_label.setText("Подключено")
            self.status_label.setStyleSheet("color: green;")
            self.add_log(f"Подключено к {port}")
            QTimer.singleShot(500, lambda: self.send("GET_SETTINGS"))
            self.refresh_file_list()
        else:
            self.serial_thread.stop()
            self.serial_thread = None
            self.connect_btn.setText("Подключиться")
            self.status_label.setText("Не подключено")
            self.status_label.setStyleSheet("color: red;")
            self.add_log("Отключено")
            self.file_list_widget.clear()
            self.file_list.clear()

    def send(self, cmd):
        if self.serial_thread:
            self.serial_thread.send_command(cmd)
            self.add_log(f">> {cmd}")

    def refresh_file_list(self):
        self.file_list_widget.clear()
        self.file_list.clear()
        self.send("LIST_FILES")

    def handle_data(self, line):
        self.add_log(f"<< {line}")
        if line.startswith("DATA:"):
            parts = line[5:].split(',')
            for i in range(8):
                if i*2 < len(parts):
                    t = parts[i*2]
                    h = parts[i*2+1] if i*2+1 < len(parts) else "---"
                    if t != "---" and h != "---":
                        try:
                            self.sensors_data[i] = [float(t), float(h)]
                            self.sensors_present[i] = True
                        except:
                            self.sensors_present[i] = False
                    else:
                        self.sensors_present[i] = False
            self.update_sensors()
        elif line.startswith("FILE:"):
            parts = line[5:].split(',')
            fname = parts[0]
            if fname not in self.file_list:
                self.file_list.append(fname)
                self.file_list_widget.addItem(fname)
        elif line == "END_OF_LIST":
            pass
        elif line.startswith("FILE_START:"):
            self.downloading_file = line[11:]
            self.log_file_content = []
        elif line == "FILE_END":
            if self.downloading_file and self.log_file_content:
                path, _ = QFileDialog.getSaveFileName(self, "Сохранить файл", self.downloading_file.replace('/', ''), "*.txt")
                if path:
                    with open(path, 'w', encoding='utf-8') as f:
                        f.write('\n'.join(self.log_file_content))
                    QMessageBox.information(self, "Скачивание", f"Файл сохранён: {path}")
            self.downloading_file = None
            self.log_file_content = []
        elif line.startswith("LOGGING_STARTED"):
            QMessageBox.information(self, "Запись данных", "Запись данных запущена")
            self.send("GET_LOG_REMAINING")
        elif line.startswith("LOGGING_STOPPED"):
            QMessageBox.information(self, "Запись данных", "Запись данных завершена!")
            self.refresh_file_list()
            self.remaining_label.setText("Осталось: --")
        elif line.startswith("LOG_REMAINING:"):
            self.remaining_seconds = int(line[14:])
            self.update_remaining_display()
        elif line.startswith("SETTINGS:"):
            parts = line[9:].split(',')
            for p in parts:
                if '=' in p:
                    k, v = p.split('=')
                    if k == 'polling':
                        self.poll_spin.setValue(int(v)//1000)
                    elif k == 'log_interval':
                        self.log_int_spin.setValue(int(v)//1000)
                    elif k == 'log_duration':
                        self.log_dur_spin.setValue(int(v)//1000)
        elif line.startswith("SCAN_COMPLETE"):
            QMessageBox.information(self, "Сканирование", "Сканирование завершено")
        elif line.startswith("CLEAR_COMPLETE"):
            QMessageBox.information(self, "Очистка", "Все файлы удалены")
            self.refresh_file_list()
        elif line.startswith("ERROR:"):
            QMessageBox.warning(self, "Ошибка", line[6:])
        elif line.startswith("OK:"):
            self.poll_status.setText(line)
            QTimer.singleShot(2000, lambda: self.poll_status.setText(""))
        else:
            if self.downloading_file is not None:
                self.log_file_content.append(line)

    def update_remaining_display(self):
        if self.remaining_seconds > 0:
            self.remaining_label.setText(f"Осталось: {self.remaining_seconds} сек")
            self.remaining_seconds -= 1
        else:
            self.remaining_label.setText("Осталось: --")

    def handle_error(self, err):
        self.add_log(f"Ошибка: {err}")
        self.toggle_connection()

    def update_sensors(self):
        for i in range(8):
            t_lbl, h_lbl, s_lbl = self.sensor_labels[i]
            if self.sensors_present[i]:
                temp, hum = self.sensors_data[i]
                t_lbl.setText(f"Темп: {temp:.1f} °C")
                h_lbl.setText(f"Влаж: {hum:.1f} %")
                s_lbl.setText("Статус: OK")
                s_lbl.setStyleSheet("color: green;")
            else:
                t_lbl.setText("Темп: ---")
                h_lbl.setText("Влаж: ---")
                s_lbl.setText("Статус: нет")
                s_lbl.setStyleSheet("color: red;")

    def on_poll_interval_changed(self, value):
        if self.serial_thread:
            self.send(f"SET_INTERVAL:{value*1000}")

    def on_log_interval_changed(self, value):
        if self.serial_thread:
            self.send(f"SET_LOG_INTERVAL:{value}")

    def on_log_duration_changed(self, value):
        if self.serial_thread:
            self.send(f"SET_LOG_DURATION:{value}")

    def download_selected_file(self):
        item = self.file_list_widget.currentItem()
        if item:
            self.send(f"DOWNLOAD_FILE:{item.text()}")

    def closeEvent(self, event):
        if self.serial_thread:
            self.serial_thread.stop()
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = RD25App()
    window.show()
    sys.exit(app.exec())