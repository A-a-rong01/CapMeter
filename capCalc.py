#I am creating this file to to see how many pulse I should get 
import numpy as np
import pandas as pd
import os


# Constants used for calculating the frequencies
Ra = 1000  # 1 kOhm
Rb = 400   # 100 Ohms
# RBOne = 90000  #Resistor assosciated with RelayOne for 10pF to 2nF range
# RBTwo = 400  #Resistor assosciated with RelayTwo for 22nF to 2.2uF range
# RBFour = 50 #Resistor assosciated with RelayFour for above 200uF range
# RBThree = 100  #Resistor assosciated with RelayThree for 3.3uF to 200uF range
tripleFiveTimerConstant = 1.44729


def calculate_frequencies(Ra, Rb, capacitances):
    """
    Calculate frequencies for a range of capacitance values using a 555 timer in astable mode.
    
    Parameters:
    Ra (float): Resistance Ra in ohms.
    Rb (float): Resistance Rb in ohms.
    capacitances (array): Array of capacitance values in farads.
    
    Returns:
    list: Frequencies corresponding to the capacitance values.
    """
    frequencies = tripleFiveTimerConstant / ((Ra + 2 * Rb) * capacitances)
    return frequencies

def format_data(capacitances, frequencies):
    """
    Format capacitances and frequencies with appropriate prefix labels.
    
    Parameters:
    capacitances (array): Array of capacitance values in farads.
    frequencies (array): Array of frequency values in hertz.
    
    Returns:
    tuple: Formatted capacitance and frequency lists.
    """
    formatted_capacitances = []
    formatted_frequencies = []

    # Format capacitances
    for c in capacitances:
        if c >= 1e-6:  # Microfarads
            formatted_capacitances.append(f"{c * 1e6:.1f} µF")
        elif c >= 1e-9:  # Nanofarads
            formatted_capacitances.append(f"{c * 1e9:.1f} nF")
        else:  # Picofarads
            formatted_capacitances.append(f"{c * 1e12:.1f} pF")
    
    # Format frequencies
    for f in frequencies:
        if f >= 1e3:  # Kilohertz
            formatted_frequencies.append(f"{f / 1e3:.2f} kHz")
        else:  # Hertz
            formatted_frequencies.append(f"{f:.2f} Hz")
    
    return formatted_capacitances, formatted_frequencies

# Generate capacitance values
picofarads = np.arange(20, 100, 5) * 1e-12  # 20 pF to 100 pF in steps of 5 pF
nanofarads = np.arange(5, 805, 10) * 1e-9    # 5 nF to 800 nF in steps of 5 nF
microfarads = np.arange(3, 205, 5) * 1e-6   # 5 µF to 200 µF in steps of 5 µF

# Combine all capacitances
capacitances = np.concatenate((picofarads, nanofarads, microfarads))

# Calculate frequencies
frequencies = calculate_frequencies(Ra, Rb, capacitances)

# Format the data with appropriate labels
formatted_capacitances, formatted_frequencies = format_data(capacitances, frequencies)

# Create a Pandas DataFrame
data = pd.DataFrame({
    "Capacitance": formatted_capacitances,
    "Frequency": formatted_frequencies
})

# Display the first few rows
print(data)

# Optional: Save results to a CSV file
data.to_csv("/Users/aaronguerrero/Documents/Arduino/capMeterPrac/CapMeter/formatted_frequencies.csv", index=False)
print("Current Working Directory:", os.getcwd())