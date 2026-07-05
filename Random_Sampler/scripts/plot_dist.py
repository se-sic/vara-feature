# from plot_bell import plot_bell
# from plot_bell2 import plot_bell_saeborn
# from plot_bell3 import plot_bell_numpy
from plot_config import plot_config_frequency
import sys

if __name__ == "__main__":
    csv_path = sys.argv[1]
    plot_config_frequency(csv_path)