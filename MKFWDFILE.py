import os
import tkinter as tk
from tkinter import simpledialog
from tkinter import messagebox

def list_directory_to_file(directory, output_file):
    try:
        with open(output_file, 'w+') as out_file:
            _list_directory_to_file_helper(directory, out_file)
    except IOError:
        print("Error opening output file!")

def _list_directory_to_file_helper(directory, out_file):
    for root, dirs, files in os.walk(directory):
        for name in files:
            full_path = os.path.join(root, name)
            out_file.write(full_path + '\n')
        for name in dirs:
            full_path = os.path.join(root, name)
            _list_directory_to_file_helper(full_path, out_file)

def main():
    directory = 'path/to/directory'

    # Create a simple GUI to ask for the output file name
    root = tk.Tk()
    root.withdraw()  # Hide the main window

    output_file = simpledialog.askstring("Input", "Enter the name of the output file (with extension):")

    if output_file:
        list_directory_to_file(directory, output_file)
        messagebox.showinfo("Success", "Directory listing saved to " + output_file)
    else:
        messagebox.showerror("Error", "Output file name cannot be empty")

if __name__ == "__main__":
    main()
