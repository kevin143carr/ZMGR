import os
import tkinter as tk
from tkinter import filedialog
from tkinter import messagebox

def write_directory_contents():
    filename = filename_entry.get()
    directory = directory_entry.get()
    
    if not filename or not directory:
        messagebox.showerror("Input Error", "Please enter both filename and directory.")
        return
    
    if not os.path.isdir(directory):
        messagebox.showerror("Input Error", "The specified directory does not exist.")
        return
    
    try:
        full_path = os.path.join(directory, filename)
        with open(full_path, 'w') as file:
            for item in os.listdir(directory):
                file.write(f"{item}\n")
        messagebox.showinfo("Success", f"Contents of the directory written to {full_path}")
    except Exception as e:
        messagebox.showerror("Error", f"An error occurred: {e}")

def browse_directory():
    directory = filedialog.askdirectory()
    directory_entry.delete(0, tk.END)
    directory_entry.insert(0, directory)

def close_app():
    root.destroy()    

# Set up the main application window
root = tk.Tk()
root.title("Directory Scanner")

# Create and place the widgets
tk.Label(root, text="Filename:").grid(row=0, column=0, padx=10, pady=10)
filename_entry = tk.Entry(root, width=40)
filename_entry.grid(row=0, column=1, padx=10, pady=10)

tk.Label(root, text="Directory:").grid(row=1, column=0, padx=10, pady=10)
directory_entry = tk.Entry(root, width=40)
directory_entry.grid(row=1, column=1, padx=10, pady=10)
tk.Button(root, text="Browse", command=browse_directory).grid(row=1, column=2, padx=10, pady=10)

tk.Button(root, text="OK", command=write_directory_contents).grid(row=2, column=0, columnspan=3, pady=20)
tk.Button(root, text="Close", command=close_app).grid(row=3, column=0, columnspan=3, pady=5)


# Start the main loop
root.mainloop()
