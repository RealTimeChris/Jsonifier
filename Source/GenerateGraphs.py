import argparse
import json
import os
import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd

def parse_args():
    parser = argparse.ArgumentParser(description='Process benchmark results from a JSON file.')
    parser.add_argument('input_file', type=str, help='Path to the input JSON file')
    parser.add_argument('output_directory', type=str, help='Path to store the graphs')
    return parser.parse_args()

def get_raw_speeds(df):
    raw_speeds = {"Read": [], "Write": []}
    libraries = df["libraryName"].unique()

    for result_type in raw_speeds.keys():
        result_type_df = df[df["resultType"] == result_type].sort_values(by="resultSpeed")

        if result_type_df.empty:
            raw_speeds[result_type] = [0] * len(libraries)
            continue

        speed_map = dict(zip(result_type_df["libraryName"], result_type_df["resultSpeed"]))

        raw_speeds[result_type] = [
            speed_map.get(library, 0) for library in libraries
        ]

    return raw_speeds

def calculate_cumulative_speedup(df):
    cumulative_speedups = {"Read": [], "Write": []}
    libraries = df["libraryName"].unique()

    for result_type in cumulative_speedups.keys():
        result_type_df = df[df["resultType"] == result_type].sort_values(by="resultSpeed")

        if result_type_df.empty:
            cumulative_speedups[result_type] = [0] * len(libraries)
            continue

        slowest_speed = result_type_df.iloc[0]["resultSpeed"]
        result_type_speedups = [100]

        for i in range(1, len(result_type_df)):
            current_speed = result_type_df.iloc[i]["resultSpeed"]
            speedup = ((current_speed / slowest_speed) - 1) * 100 + 100
            result_type_speedups.append(speedup)

        speedup_map = dict(zip(result_type_df["libraryName"], result_type_speedups))
        cumulative_speedups[result_type] = [
            speedup_map.get(library, 0) for library in libraries
        ]

    return cumulative_speedups

def plot_cumulative_speedup(df, cumulative_speedups, output_folder, test_name):
    sns.set_style("dark")
    sns.set_style(rc={'axes.facecolor': '#0d1117'})
    plt.figure(figsize=(10, 6))
    ax = plt.gca()
    
    sns.set_theme(style="whitegrid", rc={"axes.edgecolor": "#0d1117", "xtick.color": "#0d1117", "ytick.color": "#0d1117"})
    plt.gcf().set_facecolor("#0d1117")
    ax = plt.gca()

    sorted_df = df.sort_values(by="resultSpeed", ascending=False)

    library_colors = {}
    for _, row in sorted_df.iterrows():
        library_colors[(row['libraryName'], row['resultType'])] = row['color']

    libraries = sorted_df["libraryName"].unique()

    cumulative_speedup_read = cumulative_speedups.get("Read", [0] * len(libraries))
    cumulative_speedup_write = cumulative_speedups.get("Write", [0] * len(libraries))

    num_libraries = len(libraries)
    max_libraries = max(2, num_libraries)
    width = 0.8 / max_libraries

    for i, library in enumerate(libraries):
        read_speedup = cumulative_speedup_read[i] if i < len(cumulative_speedup_read) else 0
        write_speedup = cumulative_speedup_write[i] if i < len(cumulative_speedup_write) else 0

        read_color = library_colors.get((library, 'Read'), 'gray')
        write_color = library_colors.get((library, 'Write'), 'gray')

        font_size = max(8, width * 30) 
        if read_speedup != 0:
            read_bar = ax.bar(i - width / 2, read_speedup, label=f"{library} Read", color=read_color, width=width)
            ax.text(i - width / 2, read_speedup - read_speedup * 0.05,
                    f"{read_speedup:.2f}%", ha='center', va='top', color='black', fontsize=font_size, fontweight='bold')

        if write_speedup != 0:
            write_bar = ax.bar(i + width / 2, write_speedup, label=f"{library} Write", color=write_color, width=width)
            ax.text(i + width / 2, write_speedup - write_speedup * 0.05,
                    f"{write_speedup:.2f}%", ha='center', va='top', color='black', fontsize=font_size, fontweight='bold')

    ax.set_xticks(range(len(libraries)))
    ax.set_xticklabels(libraries)
    ax.set_title(f'{test_name} Cumulative Speedup (Relative to Slowest Library)', color='white')
    ax.set_xlabel('Library Name', color='white')
    ax.set_ylabel('Cumulative Speedup (%)', color='white')

    handles, labels = ax.get_legend_handles_labels()
    for text in ax.get_xticklabels() + ax.get_yticklabels():
        text.set_color('lightgray')

    ax.legend(title='Library and Result Type', loc='best')

    output_file_path_speedup = os.path.join(output_folder, f'{test_name}_Cumulative_Speedup.png')
    plt.savefig(output_file_path_speedup)
    plt.close()

def plot_raw_comparisons(df, raw_speeds, output_folder, test_name):
    sns.set_style("dark")
    sns.set_style(rc={'axes.facecolor': '#0d1117'})
    plt.figure(figsize=(10, 6))
    ax = plt.gca()

    sns.set_theme(style="whitegrid", rc={"axes.edgecolor": "#0d1117", "xtick.color": "#0d1117", "ytick.color": "#0d1117"})
    plt.gcf().set_facecolor("#0d1117")
    ax = plt.gca()

    sorted_df = df.sort_values(by="resultSpeed", ascending=False)

    library_colors = {}
    for _, row in sorted_df.iterrows():
        library_colors[(row['libraryName'], row['resultType'])] = row['color']

    libraries = sorted_df["libraryName"].unique()

    cumulative_speedup_read = raw_speeds.get("Read", [0] * len(libraries))
    cumulative_speedup_write = raw_speeds.get("Write", [0] * len(libraries))

    num_libraries = len(libraries)
    max_libraries = max(2, num_libraries)
    width = 0.8 / max_libraries

    for i, library in enumerate(libraries):
        read_speedup = cumulative_speedup_read[i] if i < len(cumulative_speedup_read) else 0
        write_speedup = cumulative_speedup_write[i] if i < len(cumulative_speedup_write) else 0

        read_color = library_colors.get((library, 'Read'), 'gray')
        write_color = library_colors.get((library, 'Write'), 'gray')
        font_size = max(8, width * 30)
    
        if read_speedup != 0:
            read_bar = ax.bar(i - width / 2, read_speedup, label=f"{library} Read", color=read_color, width=width)
            ax.text(i - width / 2, read_speedup - read_speedup * 0.05,
                    f"{read_speedup:.2f}MB/s", ha='center', va='top', color='black', fontsize=font_size, fontweight='bold')

        if write_speedup != 0:
            write_bar = ax.bar(i + width / 2, write_speedup, label=f"{library} Write", color=write_color, width=width)
            ax.text(i + width / 2, write_speedup - write_speedup * 0.05,
                    f"{write_speedup:.2f}MB/s", ha='center', va='top', color='black', fontsize=font_size, fontweight='bold')


    ax.set_xticks(range(len(libraries)))
    ax.set_xticklabels(libraries)
    ax.set_title(f'{test_name} Result Speed Comparison', color='white')
    ax.set_xlabel('Library Name', color='white')
    ax.set_ylabel('Result Speed (MB/s)', color='white')

    handles, labels = ax.get_legend_handles_labels()
    for text in ax.get_xticklabels() + ax.get_yticklabels():
        text.set_color('lightgray')

    ax.legend(title='Library and Result Type', loc='best')

    output_file_path_speedup = os.path.join(output_folder, f'{test_name}_Results.png')
    plt.savefig(output_file_path_speedup)
    plt.close()

def main():
    args = parse_args()
    input_file = args.input_file
    output_folder = args.output_directory

    with open(input_file, 'r') as file:
        data = json.load(file)

    os.makedirs(output_folder, exist_ok=True)

    for test in data:
        df = pd.DataFrame(test["results"])

        raw_speed = get_raw_speeds(df)

        cumulative_speedups = calculate_cumulative_speedup(df)

        plot_raw_comparisons(df, raw_speed , output_folder, test["testName"])

        plot_cumulative_speedup(df, cumulative_speedups, output_folder, test["testName"])

        print(f'Graphs saved successfully for {test["testName"]}!')

if __name__ == "__main__":
    main()