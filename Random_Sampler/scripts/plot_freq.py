from Diagram import plot_frequency
import sys

if __name__ == "__main__":
    csv_path = sys.argv[1]
    out_path = sys.argv[2] if len(sys.argv) > 2 else None
    plot_frequency(csv_path, out_path)