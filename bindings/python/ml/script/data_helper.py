#data helper.py
import xml.etree.ElementTree as ET
import os
import pandas as pd
import vara_feature as vf
import vara_feature.feature_model as FM

import os


def load_feature_model_and_extract_names(feature_model_path):
    if not os.path.isfile(feature_model_path):
        raise FileNotFoundError(f"The file '{feature_model_path}' does not exist.")

    feature_model = FM.loadFeatureModel(feature_model_path)
    features = [feature for feature in feature_model if not isinstance(feature, vf.feature.RootFeature)]
    feature_names = [feature.name.str() for feature in features]
    return feature_model, features, feature_names


def parse_xml_to_csv(xml_file_path, csv_file_path, features):
    tree = ET.parse(xml_file_path)
    root = tree.getroot()

    for row in root.findall('row')[:5]:  # Check the first 5 rows
        if row.find('data[@column="Configuration"]') is not None:
            key = 'column'
        elif row.find('data[@columname="Configuration"]') is not None:
            key = 'columname'

    column_names = features + ["Performance"]
    data = []

    for row in root.findall('row'):
        config = row.find(f'data[@{key}="Configuration"]').text.strip()
        performance = float(row.find(f'data[@{key}="Performance"]').text.strip())
        row_data = [0] * (len(column_names) - 1)
        for feature in config.split(','):
            feature = feature.strip()
            if feature in column_names:
                index = column_names.index(feature)
                row_data[index] = 1
        row_data.append(performance)
        data.append(row_data)
    df = pd.DataFrame(data, columns=column_names)
    df.to_csv(csv_file_path, index=False)
    return df


def find_performance_for_configurations(measurements_csv_path, sampling_csv_path):
    df_measurements = pd.read_csv(measurements_csv_path)
    df_sampling = pd.read_csv(sampling_csv_path)

    if df_measurements.columns[:-1].tolist() != df_sampling.columns.tolist():
        raise ValueError("Columns in tests CSV do not match columns in output CSV")
    matched_data = []
    for _, config_row in df_sampling.iterrows():
        match = df_measurements[df_measurements.iloc[:, :-1].eq(config_row).all(axis=1)]
        if not match.empty:
            matched_data.append(match.values[0])
        else:
            raise ValueError("No matching performance found", {"Configuration": config_row.tolist()})

    df_matched = pd.DataFrame(matched_data, columns=df_measurements.columns)
    df_matched = df_matched.astype(df_measurements.dtypes)
    return df_matched
