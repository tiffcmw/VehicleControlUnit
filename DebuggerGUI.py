import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox, filedialog
import serial
import serial.tools.list_ports
import threading
import queue
import re
from datetime import datetime
import json
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np
from collections import deque, defaultdict

class STM32StructuredMonitor:
    def __init__(self, root):
        self.root = root
        self.root.title("STM32 Vehicle Control Unit - Structured Monitor")
        self.root.geometry("1400x900")  # Increased size for plots
        
        # Serial connection
        self.serial_port = None
        self.is_connected = False
        self.data_queue = queue.Queue()
        
        # Data storage
        self.sensor_values = {}
        self.system_status = {}
        self.message_counts = {
            'SENSOR_VALUE': 0,
            'SYSTEM_STATUS': 0,
            'WARNING': 0,
            'ERROR': 0,
            'DEBUG': 0,
            'CAN_TX': 0,
            'CAN_RX': 0,
            'TIMER_STATS': 0
        }
        
        # Plotting data storage
        self.plot_data = defaultdict(lambda: {'times': deque(maxlen=1000), 'values': deque(maxlen=1000)})
        self.plot_groups = {}  # Group name -> list of signals
        self.selected_signals = set()
        self.is_plotting = False
        
        # Telemetry configuration storage
        self.telemetry_config = {}
        self.config_received = False
        
        self.setup_ui()
        self.start_data_processor()
        
    def setup_ui(self):
        # Main container with notebook for tabs
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(1, weight=1)
        
        # Connection Frame
        self.setup_connection_frame(main_frame)
        
        # Notebook for tabbed interface
        self.notebook = ttk.Notebook(main_frame)
        self.notebook.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(10, 0))
        
        # All existing tabs
        self.setup_vehicle_tab()
        self.setup_status_tab()
        self.setup_messages_tab()
        self.setup_plotting_tab()  # New plotting tab
        self.setup_raw_tab()
        
        # Initial port refresh
        self.refresh_ports()

    def setup_connection_frame(self, parent):
        """Setup the connection control frame"""
        conn_frame = ttk.LabelFrame(parent, text="Connection", padding="5")
        conn_frame.grid(row=0, column=0, sticky=(tk.W, tk.E), pady=(0, 10))
        
        ttk.Label(conn_frame, text="Port:").grid(row=0, column=0, padx=(0, 5))
        
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(conn_frame, textvariable=self.port_var, width=15)
        self.port_combo.grid(row=0, column=1, padx=(0, 10))
        
        ttk.Label(conn_frame, text="Baud:").grid(row=0, column=2, padx=(10, 5))
        
        self.baud_var = tk.StringVar(value="115200")
        baud_combo = ttk.Combobox(conn_frame, textvariable=self.baud_var, width=10)
        baud_combo['values'] = ('115200', '230400', '460800', '921600')
        baud_combo.grid(row=0, column=3, padx=(0, 10))
        
        self.refresh_btn = ttk.Button(conn_frame, text="Refresh", command=self.refresh_ports)
        self.refresh_btn.grid(row=0, column=4, padx=(0, 10))
        
        self.connect_btn = ttk.Button(conn_frame, text="Connect", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=5)
        
        self.status_label = ttk.Label(conn_frame, text="Disconnected", foreground="red")
        self.status_label.grid(row=0, column=6, padx=(10, 0))

        # Command buttons row
        cmd_frame = ttk.Frame(conn_frame)
        cmd_frame.grid(row=1, column=0, columnspan=7, pady=(10, 0), sticky=(tk.W, tk.E))
        
        ttk.Button(cmd_frame, text="Request Config", 
                command=self.request_telemetry_config).pack(side=tk.LEFT, padx=(0, 5))
        ttk.Button(cmd_frame, text="Health Check", 
                command=self.request_health_check).pack(side=tk.LEFT)
        

    def setup_vehicle_tab(self):
        """Setup the vehicle data tab with hierarchical sensor grouping"""
        vehicle_frame = ttk.Frame(self.notebook, padding="10")
        self.notebook.add(vehicle_frame, text="Vehicle Data")
        
        # Sensor values tree
        sensor_label_frame = ttk.LabelFrame(vehicle_frame, text="Sensor Values", padding="5")
        sensor_label_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        
        # Treeview for sensor data - now with tree structure
        columns = ('Value', 'Unit', 'Last Updated')
        self.sensor_tree = ttk.Treeview(sensor_label_frame, columns=columns, show='tree headings', height=15)
        
        # Set up column headings
        self.sensor_tree.heading('#0', text='Sensor', anchor=tk.W)
        self.sensor_tree.heading('Value', text='Value')
        self.sensor_tree.heading('Unit', text='Unit')
        self.sensor_tree.heading('Last Updated', text='Last Updated')
        
        # Set column widths
        self.sensor_tree.column('#0', width=200, minwidth=150)
        self.sensor_tree.column('Value', width=100, minwidth=80)
        self.sensor_tree.column('Unit', width=80, minwidth=60)
        self.sensor_tree.column('Last Updated', width=120, minwidth=100)
        
        scrollbar_sensors = ttk.Scrollbar(sensor_label_frame, orient=tk.VERTICAL, command=self.sensor_tree.yview)
        self.sensor_tree.configure(yscrollcommand=scrollbar_sensors.set)
        
        self.sensor_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar_sensors.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Configure tree styling for better visual grouping
        self.sensor_tree.tag_configure('sensor_group', background='#f0f0f0', font=('Arial', 9, 'bold'))
        self.sensor_tree.tag_configure('sensor_value', background='white')
        self.sensor_tree.tag_configure('warning_value', background='#fff3cd')
        self.sensor_tree.tag_configure('error_value', background='#f8d7da')

    def setup_status_tab(self):
        """Setup the system status tab"""
        status_frame = ttk.Frame(self.notebook, padding="10")
        self.notebook.add(status_frame, text="System Status")
        
        # System status tree
        status_label_frame = ttk.LabelFrame(status_frame, text="System Status", padding="5")
        status_label_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        
        columns = ('System', 'Status', 'Last Updated')
        self.status_tree = ttk.Treeview(status_label_frame, columns=columns, show='headings', height=10)
        
        for col in columns:
            self.status_tree.heading(col, text=col)
            self.status_tree.column(col, width=200)
        
        scrollbar_status = ttk.Scrollbar(status_label_frame, orient=tk.VERTICAL, command=self.status_tree.yview)
        self.status_tree.configure(yscrollcommand=scrollbar_status.set)
        
        self.status_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar_status.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Message counts
        counts_frame = ttk.LabelFrame(status_frame, text="Message Counts", padding="5")
        counts_frame.pack(fill=tk.X)
        
        self.count_labels = {}
        row = 0
        col = 0
        for msg_type in self.message_counts.keys():
            ttk.Label(counts_frame, text=f"{msg_type}:", font=('Arial', 9, 'bold')).grid(
                row=row, column=col*2, sticky=tk.W, padx=(0, 5))
            
            self.count_labels[msg_type] = ttk.Label(counts_frame, text="0", foreground="blue")
            self.count_labels[msg_type].grid(row=row, column=col*2+1, sticky=tk.W, padx=(0, 20))
            
            col += 1
            if col >= 4:
                col = 0
                row += 1

    def setup_messages_tab(self):
        """Setup the messages tab"""
        messages_frame = ttk.Frame(self.notebook, padding="10")
        self.notebook.add(messages_frame, text="Messages")
        
        # Filter frame
        filter_frame = ttk.LabelFrame(messages_frame, text="Filters", padding="5")
        filter_frame.pack(fill=tk.X, pady=(0, 10))
        
        ttk.Label(filter_frame, text="Message Type:").grid(row=0, column=0, padx=(0, 5))
        
        self.filter_var = tk.StringVar(value="ALL")
        filter_combo = ttk.Combobox(filter_frame, textvariable=self.filter_var, width=15)
        filter_combo['values'] = ['ALL'] + list(self.message_counts.keys())
        filter_combo.grid(row=0, column=1, padx=(0, 10))
        
        ttk.Button(filter_frame, text="Clear Messages", command=self.clear_messages).grid(row=0, column=2)
        
        # Messages display
        msg_label_frame = ttk.LabelFrame(messages_frame, text="Structured Messages", padding="5")
        msg_label_frame.pack(fill=tk.BOTH, expand=True)
        
        columns = ('Time', 'Sender', 'Type', 'Content')
        self.message_tree = ttk.Treeview(msg_label_frame, columns=columns, show='headings', height=15)
        
        self.message_tree.heading('Time', text='Time')
        self.message_tree.heading('Sender', text='Sender')
        self.message_tree.heading('Type', text='Type')
        self.message_tree.heading('Content', text='Content')
        
        self.message_tree.column('Time', width=100)
        self.message_tree.column('Sender', width=120)
        self.message_tree.column('Type', width=120)
        self.message_tree.column('Content', width=400)
        
        scrollbar_messages = ttk.Scrollbar(msg_label_frame, orient=tk.VERTICAL, command=self.message_tree.yview)
        self.message_tree.configure(yscrollcommand=scrollbar_messages.set)
        
        self.message_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar_messages.pack(side=tk.RIGHT, fill=tk.Y)

    def setup_raw_tab(self):
        """Setup the raw data tab"""
        raw_frame = ttk.Frame(self.notebook, padding="10")
        self.notebook.add(raw_frame, text="Raw Data")
        
        self.raw_display = scrolledtext.ScrolledText(raw_frame, height=25, width=100)
        self.raw_display.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        
        # Control buttons
        btn_frame = ttk.Frame(raw_frame)
        btn_frame.pack(fill=tk.X)
        
        ttk.Button(btn_frame, text="Clear", command=self.clear_raw).pack(side=tk.LEFT, padx=(0, 10))
        ttk.Button(btn_frame, text="Save Log", command=self.save_log).pack(side=tk.LEFT)
        
        self.auto_scroll_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(btn_frame, text="Auto Scroll", variable=self.auto_scroll_var).pack(side=tk.RIGHT)
    
    def setup_plotting_tab(self):
        """Setup the plotting tab with signal selection and grouping"""
        plot_frame = ttk.Frame(self.notebook, padding="10")
        self.notebook.add(plot_frame, text="Real-time Plots")
        
        # Configure grid weights
        plot_frame.columnconfigure(1, weight=1)
        plot_frame.rowconfigure(1, weight=1)
        
        # Left panel for controls
        control_frame = ttk.Frame(plot_frame, width=300)
        control_frame.grid(row=0, column=0, rowspan=2, sticky=(tk.W, tk.N, tk.S), padx=(0, 10))
        control_frame.grid_propagate(False)
        
        # Signal selection frame
        signal_frame = ttk.LabelFrame(control_frame, text="Available Signals", padding="5")
        signal_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        
        # Available signals listbox with checkboxes
        self.signals_listbox = tk.Listbox(signal_frame, selectmode=tk.MULTIPLE, height=10)
        signals_scrollbar = ttk.Scrollbar(signal_frame, orient=tk.VERTICAL, command=self.signals_listbox.yview)
        self.signals_listbox.configure(yscrollcommand=signals_scrollbar.set)
        
        self.signals_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        signals_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Group management frame
        group_frame = ttk.LabelFrame(control_frame, text="Plot Groups", padding="5")
        group_frame.pack(fill=tk.X, pady=(0, 10))
        
        # Group creation
        ttk.Label(group_frame, text="Group Name:").pack(anchor=tk.W)
        self.group_name_var = tk.StringVar()
        group_entry = ttk.Entry(group_frame, textvariable=self.group_name_var, width=25)
        group_entry.pack(fill=tk.X, pady=(0, 5))
        
        ttk.Button(group_frame, text="Create Group from Selected", 
                  command=self.create_plot_group).pack(fill=tk.X, pady=(0, 5))
        
        # Existing groups listbox
        ttk.Label(group_frame, text="Existing Groups:").pack(anchor=tk.W)
        self.groups_listbox = tk.Listbox(group_frame, height=4)
        self.groups_listbox.pack(fill=tk.X, pady=(0, 5))
        self.groups_listbox.bind('<Double-Button-1>', self.plot_selected_group)
        
        ttk.Button(group_frame, text="Plot Selected Group", 
                  command=self.plot_selected_group).pack(fill=tk.X, pady=(0, 2))
        ttk.Button(group_frame, text="Delete Group", 
                  command=self.delete_selected_group).pack(fill=tk.X)
        
        # Plot controls frame
        controls_frame = ttk.LabelFrame(control_frame, text="Plot Controls", padding="5")
        controls_frame.pack(fill=tk.X, pady=(0, 10))
        
        self.plot_enabled_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(controls_frame, text="Enable Real-time Plotting", 
                       variable=self.plot_enabled_var, 
                       command=self.toggle_plotting).pack(anchor=tk.W, pady=(0, 5))
        
        ttk.Button(controls_frame, text="Plot Selected Signals", 
                  command=self.plot_selected_signals).pack(fill=tk.X, pady=(0, 5))
        ttk.Button(controls_frame, text="Clear All Plots", 
                  command=self.clear_plots).pack(fill=tk.X, pady=(0, 5))
        ttk.Button(controls_frame, text="Clear Data History", 
                  command=self.clear_plot_data).pack(fill=tk.X)
        
        # Plot settings frame
        settings_frame = ttk.LabelFrame(control_frame, text="Plot Settings", padding="5")
        settings_frame.pack(fill=tk.X)
        
        ttk.Label(settings_frame, text="Time Window (seconds):").pack(anchor=tk.W)
        self.time_window_var = tk.StringVar(value="30")
        time_window_entry = ttk.Entry(settings_frame, textvariable=self.time_window_var, width=10)
        time_window_entry.pack(anchor=tk.W, pady=(0, 5))
        
        ttk.Label(settings_frame, text="Update Rate (ms):").pack(anchor=tk.W)
        self.update_rate_var = tk.StringVar(value="100")
        update_rate_entry = ttk.Entry(settings_frame, textvariable=self.update_rate_var, width=10)
        update_rate_entry.pack(anchor=tk.W)
        
        # Top right - Plot area
        plot_controls_frame = ttk.Frame(plot_frame)
        plot_controls_frame.grid(row=0, column=1, sticky=(tk.W, tk.E), pady=(0, 5))
        
        ttk.Label(plot_controls_frame, text="Real-time Signal Plots", 
                 font=('Arial', 12, 'bold')).pack(side=tk.LEFT)
        
        ttk.Button(plot_controls_frame, text="Save Plot", 
                  command=self.save_plot).pack(side=tk.RIGHT)
        
        # Main plot area
        self.setup_plot_canvas(plot_frame)
    
    def setup_plot_canvas(self, parent):
        """Setup matplotlib canvas for plotting"""
        # Create matplotlib figure
        self.plot_figure = Figure(figsize=(10, 6), dpi=100)
        self.plot_canvas = FigureCanvasTkAgg(self.plot_figure, parent)
        self.plot_canvas.get_tk_widget().grid(row=1, column=1, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Create subplots - we'll dynamically adjust these
        self.plot_axes = []
        self.current_plots = {}
        
        # Initially create one subplot
        ax = self.plot_figure.add_subplot(111)
        ax.set_title("Select signals to plot")
        ax.set_xlabel("Time (seconds)")
        ax.set_ylabel("Value")
        ax.grid(True, alpha=0.3)
        self.plot_axes.append(ax)
        
        self.plot_canvas.draw()

    def refresh_ports(self):
        """Refresh the list of available serial ports"""
        ports = serial.tools.list_ports.comports()
        port_names = [port.device for port in ports]
        self.port_combo['values'] = port_names
        if port_names:
            self.port_combo.set(port_names[0])
    
    def toggle_connection(self):
        """Connect or disconnect from serial port"""
        if not self.is_connected:
            self.connect()
        else:
            self.disconnect()
    
    def connect(self):
        """Connect to the selected serial port"""
        try:
            port = self.port_var.get()
            baud = int(self.baud_var.get())
            
            if not port:
                messagebox.showerror("Error", "Please select a port")
                return
                
            self.serial_port = serial.Serial(port, baud, timeout=0.1)
            self.is_connected = True
            
            # Start reading thread
            self.read_thread = threading.Thread(target=self.read_serial_data, daemon=True)
            self.read_thread.start()
            
            # Update UI
            self.connect_btn.configure(text="Disconnect")
            self.status_label.configure(text="Connected", foreground="green")
            
            self.add_raw_message(f"Connected to {port} at {baud} baud\n")

             # Request telemetry configuration after connecting
            self.root.after(1, self.request_telemetry_config)
            
        except Exception as e:
            messagebox.showerror("Connection Error", f"Failed to connect: {str(e)}")
    
    def disconnect(self):
        """Disconnect from serial port"""
        try:
            self.is_connected = False
            if self.serial_port and self.serial_port.is_open:
                self.serial_port.close()
            
            # Update UI
            self.connect_btn.configure(text="Connect")
            self.status_label.configure(text="Disconnected", foreground="red")
            
            self.add_raw_message("Disconnected\n")
            
        except Exception as e:
            messagebox.showerror("Disconnection Error", f"Failed to disconnect: {str(e)}")
    
    def read_serial_data(self):
        """Read data from serial port in separate thread"""
        while self.is_connected and self.serial_port and self.serial_port.is_open:
            try:
                if self.serial_port.in_waiting > 0:
                    data = self.serial_port.readline().decode('utf-8', errors='ignore').strip()
                    if data:
                        self.data_queue.put(data)
            except Exception as e:
                print(f"Serial read error: {e}")
                break
    
    def start_data_processor(self):
        """Process incoming data from queue"""
        try:
            while True:
                data = self.data_queue.get_nowait()
                self.process_data(data)
                self.data_queue.task_done()
        except queue.Empty:
            pass
        
        # Schedule next processing
        self.root.after(10, self.start_data_processor)
    
    def parse_structured_message(self, data):
        """Parse structured message format: Sender:X;InfoType:Y;Content:Z"""
        try:
            # Split by semicolons
            parts = data.split(';')
            parsed = {}
            
            for part in parts:
                if ':' in part:
                    key, value = part.split(':', 1)
                    parsed[key] = value
            
            return parsed
        except:
            return None
    
    def process_data(self, data):
        """Process incoming structured data"""
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        
        # Add to raw display
        self.add_raw_message(f"[{timestamp}] {data}\n")
        
        # Try to parse structured message
        parsed = self.parse_structured_message(data)
        
        if parsed and 'Sender' in parsed and 'InfoType' in parsed:
            sender = parsed['Sender']
            info_type = parsed['InfoType']
            content = parsed.get('Content', '')
            # Handle telemetry configuration messages FIRST
            if sender == 'TelemetryConfig' and info_type == 'DEBUG':
                self.process_telemetry_config(content)
                return  # Don't process as regular message
            
            # Update message counts
            if info_type in self.message_counts:
                self.message_counts[info_type] += 1
                if info_type in self.count_labels:
                    self.count_labels[info_type].config(text=str(self.message_counts[info_type]))
            
            # Add to message tree
            if self.filter_var.get() == "ALL" or self.filter_var.get() == info_type:
                self.message_tree.insert('', 0, values=(timestamp, sender, info_type, content))
            
            # Process specific message types
            if info_type == 'SENSOR_VALUE':
                self.update_sensor_display(sender, parsed, timestamp)
            elif info_type == 'SYSTEM_STATUS':
                self.update_status_display(sender, parsed, timestamp)

    def update_sensor_display(self, sender, parsed, timestamp):
        """Update sensor values display - simplified without sorting"""
        if 'Value' in parsed and 'Unit' in parsed:
            value_str = parsed['Value']
            unit = parsed['Unit']
            
            # Try to parse numeric value for plotting
            try:
                value = float(value_str)
                signal_name = f"{sender} ({unit})"
                self.plot_data[signal_name]['times'].append(datetime.now())
                self.plot_data[signal_name]['values'].append(value)
                
                # Determine if this is a warning/error value
                is_warning = value in [-1, 2001, 101] or value < 0
                
            except ValueError:
                value = None
                is_warning = False
            
            # Find or create sensor group (simple version)
            sensor_group_id = self._find_or_create_sensor_group(sender)
            
            # Find or create unit entry under this sensor (simple version)
            self._update_unit_entry(sensor_group_id, unit, value_str, timestamp, is_warning)
            
            # Update the sensor group's timestamp
            self.sensor_tree.item(sensor_group_id, values=('', '', timestamp))
            
        elif 'Content' in parsed:
            content = parsed['Content']
            parts = content.split(';')
            unit = "batch"
            
            # Extract unit if present
            for part in parts:
                if ':' in part:
                    key, val = part.split(':', 1)
                    if key == 'Unit':
                        unit = val
                        break
            
            # Store plotting data
            for part in parts:
                if ':' in part and not part.startswith('Unit:'):
                    try:
                        key, val = part.split(':', 1)
                        value = float(val)
                        signal_name = f"{sender}_{key} ({unit})"
                        self.plot_data[signal_name]['times'].append(datetime.now())
                        self.plot_data[signal_name]['values'].append(value)
                    except ValueError:
                        pass
            
            # Find or create sensor group (simple version)
            sensor_group_id = self._find_or_create_sensor_group(sender)
            
            # Find or create unit entry under this sensor (simple version)
            self._update_unit_entry(sensor_group_id, unit, content, timestamp, False)
            
            # Update the sensor group's timestamp
            self.sensor_tree.item(sensor_group_id, values=('', '', timestamp))
        
        # Update available signals list (simple version)
        self.root.after_idle(self.update_signals_list)

    def _find_or_create_sensor_group(self, sender):
        """Find existing sensor group or create new one (no sorting)"""
        # Check if sensor group already exists
        for item in self.sensor_tree.get_children():
            if self.sensor_tree.item(item, 'text') == sender:
                return item
        
        # Create new sensor group at end
        return self.sensor_tree.insert('', 'end', text=sender,
                                    values=('', '', ''),
                                    tags=('sensor_group',), open=True)

    def _update_unit_entry(self, sensor_group_id, unit, value, timestamp, is_warning):
        """Update or create unit entry under sensor group (no sorting)"""
        unit_display = f"  {unit}"
        
        # Check if unit entry already exists
        for child in self.sensor_tree.get_children(sensor_group_id):
            child_text = self.sensor_tree.item(child, 'text')
            if child_text == unit or child_text == unit_display:
                # Update existing entry
                value_tag = 'error_value' if is_warning else 'sensor_value'
                self.sensor_tree.item(child, values=(value, unit, timestamp), tags=(value_tag,))
                return child
        
        # Create new unit entry at end
        value_tag = 'error_value' if is_warning else 'sensor_value'
        return self.sensor_tree.insert(sensor_group_id, 'end', text=unit_display,
                                    values=(value, unit, timestamp),
                                    tags=(value_tag,))
    
    def update_signals_list(self):
        """Update the available signals list (simplified)"""
        current_signals = set(self.plot_data.keys())
        listbox_signals = set(self.signals_listbox.get(0, tk.END))
        
        # Only update if signals changed
        if current_signals != listbox_signals:
            # Get current selection
            current_selection = [self.signals_listbox.get(i) for i in self.signals_listbox.curselection()]
            
            # Simple list - add new signals at end
            self.signals_listbox.delete(0, tk.END)
            for signal in current_signals:
                self.signals_listbox.insert(tk.END, signal)
            
            # Restore selection
            for i, signal in enumerate(self.signals_listbox.get(0, tk.END)):
                if signal in current_selection:
                    self.signals_listbox.selection_set(i)

    def update_status_display(self, sender, parsed, timestamp):
        """Update system status display"""
        if 'Status' in parsed:
            status = parsed['Status']
            
            # Update or insert in status tree
            for item in self.status_tree.get_children():
                if self.status_tree.item(item)['values'][0] == sender:
                    self.status_tree.item(item, values=(sender, status, timestamp))
                    return
            
            # Insert new status
            self.status_tree.insert('', 0, values=(sender, status, timestamp))
    
    def update_signals_list(self):
        """Update the available signals list"""
        current_signals = set(self.plot_data.keys())
        listbox_signals = set(self.signals_listbox.get(0, tk.END))
        
        # Add new signals
        for signal in current_signals - listbox_signals:
            self.signals_listbox.insert(tk.END, signal)
    
    def create_plot_group(self):
        """Create a new plot group from selected signals"""
        group_name = self.group_name_var.get().strip()
        if not group_name:
            messagebox.showerror("Error", "Please enter a group name")
            return
        
        selected_indices = self.signals_listbox.curselection()
        if not selected_indices:
            messagebox.showerror("Error", "Please select signals to group")
            return
        
        selected_signals = [self.signals_listbox.get(i) for i in selected_indices]
        
        self.plot_groups[group_name] = selected_signals
        
        # Update groups listbox
        self.groups_listbox.delete(0, tk.END)
        for group in self.plot_groups.keys():
            self.groups_listbox.insert(tk.END, group)
        
        # Clear group name entry
        self.group_name_var.set("")
        
        messagebox.showinfo("Success", f"Created group '{group_name}' with {len(selected_signals)} signals")
    
    def delete_selected_group(self):
        """Delete the selected group"""
        selected = self.groups_listbox.curselection()
        if not selected:
            messagebox.showerror("Error", "Please select a group to delete")
            return
        
        group_name = self.groups_listbox.get(selected[0])
        del self.plot_groups[group_name]
        
        # Update groups listbox
        self.groups_listbox.delete(0, tk.END)
        for group in self.plot_groups.keys():
            self.groups_listbox.insert(tk.END, group)
        
        messagebox.showinfo("Success", f"Deleted group '{group_name}'")
    
    def plot_selected_group(self, event=None):
        """Plot all signals in the selected group"""
        selected = self.groups_listbox.curselection()
        if not selected:
            return
        
        group_name = self.groups_listbox.get(selected[0])
        signals = self.plot_groups.get(group_name, [])
        
        if signals:
            self.plot_signals(signals, group_name)
    
    def plot_selected_signals(self):
        """Plot the currently selected individual signals"""
        selected_indices = self.signals_listbox.curselection()
        if not selected_indices:
            messagebox.showerror("Error", "Please select signals to plot")
            return
        
        signals = [self.signals_listbox.get(i) for i in selected_indices]
        self.plot_signals(signals, "Selected Signals")
    
    def plot_signals(self, signals, title="Signals"):
        """Plot the specified signals"""
        if not signals:
            return
        
        # Clear current plots
        self.plot_figure.clear()
        self.plot_axes.clear()
        self.current_plots.clear()
        
        # Create subplot
        ax = self.plot_figure.add_subplot(111)
        ax.set_title(f"{title} - Real-time Plot")
        ax.set_xlabel("Time (seconds)")
        ax.set_ylabel("Value")
        ax.grid(True, alpha=0.3)
        
        # Plot each signal
        colors = plt.cm.tab10(np.linspace(0, 1, len(signals)))
        
        current_time = datetime.now()
        
        for signal, color in zip(signals, colors):
            if signal in self.plot_data and len(self.plot_data[signal]['values']) > 0:
                times = self.plot_data[signal]['times']
                values = self.plot_data[signal]['values']
                
                # Convert times to seconds relative to current time
                time_seconds = [(current_time - t).total_seconds() for t in times]
                time_seconds = [-t for t in time_seconds]  # Make positive, most recent = 0
                
                line, = ax.plot(time_seconds, values, label=signal, color=color, linewidth=2)
                self.current_plots[signal] = line
        
        if self.current_plots:
            ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
            
        # Set time window
        try:
            time_window = float(self.time_window_var.get())
            ax.set_xlim(-time_window, 0)
        except ValueError:
            ax.set_xlim(-30, 0)  # Default 30 seconds
        
        self.plot_axes.append(ax)
        self.plot_figure.tight_layout()
        self.plot_canvas.draw()
    
    def toggle_plotting(self):
        """Toggle real-time plotting on/off"""
        if self.plot_enabled_var.get():
            self.is_plotting = True
            self.start_real_time_plotting()
        else:
            self.is_plotting = False
    
    def start_real_time_plotting(self):
        """Start real-time plot updates"""
        if self.is_plotting and self.current_plots:
            self.update_plots()
        
        # Schedule next update
        if self.is_plotting:
            try:
                update_rate = int(self.update_rate_var.get())
            except ValueError:
                update_rate = 100
            
            self.root.after(update_rate, self.start_real_time_plotting)
    
    def update_plots(self):
        """Update the current plots with new data"""
        if not self.current_plots:
            return
        
        current_time = datetime.now()
        
        try:
            time_window = float(self.time_window_var.get())
        except ValueError:
            time_window = 30
        
        updated = False
        
        for signal, line in self.current_plots.items():
            if signal in self.plot_data and len(self.plot_data[signal]['values']) > 0:
                times = list(self.plot_data[signal]['times'])
                values = list(self.plot_data[signal]['values'])
                
                if times:
                    # Convert times to seconds relative to current time
                    time_seconds = [-(current_time - t).total_seconds() for t in times]
                    
                    # Filter to time window
                    filtered_times = []
                    filtered_values = []
                    for t, v in zip(time_seconds, values):
                        if t >= -time_window:
                            filtered_times.append(t)
                            filtered_values.append(v)
                    
                    if filtered_times:
                        line.set_data(filtered_times, filtered_values)
                        updated = True
        
        if updated and self.plot_axes:
            # Update axis limits
            ax = self.plot_axes[0]
            ax.set_xlim(-time_window, 0)
            
            # Auto-scale y-axis
            ax.relim()
            ax.autoscale_view(scalex=False, scaley=True)
            
            self.plot_canvas.draw_idle()
    
    def clear_plots(self):
        """Clear all current plots"""
        self.plot_figure.clear()
        self.plot_axes.clear()
        self.current_plots.clear()
        
        # Reset to default
        ax = self.plot_figure.add_subplot(111)
        ax.set_title("Select signals to plot")
        ax.set_xlabel("Time (seconds)")
        ax.set_ylabel("Value")
        ax.grid(True, alpha=0.3)
        self.plot_axes.append(ax)
        
        self.plot_canvas.draw()
    
    def clear_plot_data(self):
        """Clear all stored plot data"""
        for signal_data in self.plot_data.values():
            signal_data['times'].clear()
            signal_data['values'].clear()
        
        messagebox.showinfo("Success", "Plot data history cleared")
    
    def save_plot(self):
        """Save the current plot to a file"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = filedialog.asksaveasfilename(
                defaultextension=".png",
                filetypes=[("PNG files", "*.png"), ("PDF files", "*.pdf"), ("All files", "*.*")],
                initialname=f"stm32_plot_{timestamp}.png"
            )
            
            if filename:
                self.plot_figure.savefig(filename, dpi=300, bbox_inches='tight')
                messagebox.showinfo("Success", f"Plot saved as {filename}")
                
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save plot: {str(e)}")

    def add_raw_message(self, message):
        """Add message to raw display"""
        self.raw_display.insert(tk.END, message)
        
        # Auto-scroll if enabled
        if self.auto_scroll_var.get():
            self.raw_display.see(tk.END)
        
        # Limit text length to prevent memory issues
        lines = self.raw_display.get("1.0", tk.END).count('\n')
        if lines > 1000:
            self.raw_display.delete("1.0", "100.0")
    
    def clear_messages(self):
        """Clear the message tree"""
        for item in self.message_tree.get_children():
            self.message_tree.delete(item)
    
    def clear_raw(self):
        """Clear the raw display"""
        self.raw_display.delete("1.0", tk.END)
    
    def save_log(self):
        """Save the current log to a file"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = filedialog.asksaveasfilename(
                defaultextension=".txt",
                filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
                initialname=f"stm32_log_{timestamp}.txt"
            )
            
            if filename:
                with open(filename, 'w') as f:
                    f.write(self.raw_display.get("1.0", tk.END))
                
                messagebox.showinfo("Success", f"Log saved as {filename}")
                
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save log: {str(e)}")
    
    def request_telemetry_config(self):
        """Request telemetry configuration from STM32"""
        if self.is_connected and self.serial_port:
            try:
                self.serial_port.write(b"CONFIG_REQUEST\n")
                self.add_raw_message("Sent: CONFIG_REQUEST\n")
            except Exception as e:
                self.add_raw_message(f"Failed to send CONFIG_REQUEST: {e}\n")

    def request_health_check(self):
        """Request telemetry health check"""
        if self.is_connected and self.serial_port:
            try:
                self.serial_port.write(b"HEALTH_CHECK\n")
                self.add_raw_message("Sent: HEALTH_CHECK\n")
            except Exception as e:
                self.add_raw_message(f"Failed to send HEALTH_CHECK: {e}\n")

    def process_telemetry_config(self, content):
        """Process telemetry configuration data"""
        try:
            parsed = {}
            for part in content.split(';'):
                if ':' in part:
                    key, value = part.split(':', 1)
                    parsed[key] = value
            
            if 'TotalSignals' in parsed:
                total_signals = int(parsed['TotalSignals'])
                self.add_raw_message(f"Expected {total_signals} telemetry signals\n")
                
            elif 'Signal' in parsed:
                # Store signal configuration
                signal_name = parsed['Signal']
                self.telemetry_config[signal_name] = {
                    'unit': parsed.get('Unit', ''),
                    'category': parsed.get('Category', 'Other'),
                    'rate_ms': int(parsed.get('Rate', 1000)),
                    'min_val': float(parsed.get('Min', 0)),
                    'max_val': float(parsed.get('Max', 100)),
                    'enabled': parsed.get('Enabled', '1') == '1'
                }
                
            elif 'ConfigComplete' in parsed:
                self.config_received = True
                self.add_raw_message(f"Telemetry configuration complete: {len(self.telemetry_config)} signals\n")
                
        except Exception as e:
            self.add_raw_message(f"Error processing config: {e}\n")

def main():
    root = tk.Tk()
    app = STM32StructuredMonitor(root)
    
    def on_closing():
        app.is_plotting = False  # Stop plotting
        if app.is_connected:
            app.disconnect()
        root.destroy()
    
    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()

if __name__ == "__main__":
    main()