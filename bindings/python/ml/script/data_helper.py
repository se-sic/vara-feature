import os
import xml.etree.ElementTree as ET
from typing import List, Tuple

import pandas as pd
import vara_feature as vf
import vara_feature.feature_model as FM


def load_feature_model_and_extract_names(feature_model_path: str) -> Tuple[
    FM.FeatureModel, List[vf.feature.Feature], List[str]]:
    """
    Loads a feature model from the specified XML file and extracts feature names excluding the root feature.

    Args:
        feature_model_path (str): Path to the feature model XML file.

    Returns:
        Tuple containing:
            - feature_model (FM.FeatureModel): The loaded feature model.
            - features (List[vf.feature.Feature]): List of features excluding the root.
            - feature_names (List[str]): Names of the extracted features.

    Raises:
        FileNotFoundError: If the feature model file does not exist.
    """
    if not os.path.isfile(feature_model_path):
        raise FileNotFoundError(f"The file '{feature_model_path}' does not exist.")

    feature_model = FM.loadFeatureModel(feature_model_path)
    # Exclude the root feature from the list
    features = [feature for feature in feature_model if not isinstance(feature, vf.feature.RootFeature)]
    # Extract feature names as strings
    feature_names = [feature.name.str() for feature in features]
    return feature_model, features, feature_names


def parse_xml_to_csv(xml_file_path: str, csv_file_path: str, features: List[str]) -> pd.DataFrame:
    """
    Parses an XML file containing measurements data and converts it into a CSV file.

    Args:
        xml_file_path (str): Path to the input XML file.
        csv_file_path (str): Path where the output CSV file will be saved.
        features (List[str]): List of feature names to be included as columns.

    Returns:
        pd.DataFrame: DataFrame containing the parsed data.

    Raises:
        ET.ParseError: If the XML file cannot be parsed.
        AttributeError: If expected XML elements are missing.
    """
    tree = ET.parse(xml_file_path)
    root = tree.getroot()

    key = None
    # Determine the correct key by inspecting the first 5 rows
    for row in root.findall('row')[:5]:
        if row.find('data[@column="Configuration"]') is not None:
            key = 'column'
            break
        elif row.find('data[@columname="Configuration"]') is not None:
            key = 'columname'
            break

    if key is None:
        raise KeyError("Configuration key not found in the first 5 rows.")

    column_names = features + ["Performance"]
    data = []

    for row in root.findall('row'):
        config_element = row.find(f'data[@{key}="Configuration"]')
        performance_element = row.find(f'data[@{key}="Performance"]')

        if config_element is None or performance_element is None:
            continue  # Skip rows with missing data

        config = config_element.text.strip()
        performance = float(performance_element.text.strip())
        row_data = [0] * (len(column_names) - 1)  # Initialize Configuration with all zeros

        for feature in config.split(','):
            feature = feature.strip()
            if feature in features:
                index = column_names.index(feature)
                row_data[index] = 1
        row_data.append(performance)
        data.append(row_data)

    df = pd.DataFrame(data, columns=column_names)
    df.to_csv(csv_file_path, index=False)
    return df


def find_performance_for_configurations(measurements_csv_path: str, sampling_csv_path: str) -> pd.DataFrame:
    """
    Matches configurations from the sampling CSV with measurements and retrieves their performance metrics.

    Args:
        measurements_csv_path (str): Path to the measurements CSV file.
        sampling_csv_path (str): Path to the sampling configurations CSV file.

    Returns:
        pd.DataFrame: DataFrame containing matched configurations with their performance.

    Raises:
        ValueError: If column structures do not match or if a matching performance is not found.
    """
    df_measurements = pd.read_csv(measurements_csv_path)
    df_sampling = pd.read_csv(sampling_csv_path)

    # Ensure that the feature columns (excluding 'Performance') match
    if df_measurements.columns[:-1].tolist() != df_sampling.columns.tolist():
        raise ValueError("Columns in tests CSV do not match columns in output CSV.")

    matched_data = []
    for _, config_row in df_sampling.iterrows():
        # Check for rows in measurements that match the current configuration
        match = df_measurements[df_measurements.iloc[:, :-1].eq(config_row).all(axis=1)]
        if not match.empty:
            matched_data.append(match.iloc[0].values)
        else:
            raise ValueError("No matching performance found", {"Configuration": config_row.tolist()})

    df_matched = pd.DataFrame(matched_data, columns=df_measurements.columns)
    df_matched = df_matched.astype(df_measurements.dtypes)
    return df_matched
