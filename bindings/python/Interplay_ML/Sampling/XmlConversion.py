import os
import glob
import csv
import pandas as pd
from pathlib import Path
import xml.etree.ElementTree as Et
from .Config import Workspace
from ...ml.script.data_helper import parse_xml_to_csv, load_feature_model_and_extract_names

PERF_NAMES = ["Performance", "GC Time", "UserTime"]

def parse_xml_to_csv_new(xml_file_path, csv_file_path, features):
    tree = Et.parse(xml_file_path)
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
        performance_element = None

        performance = None # type: ignore
        for perf in PERF_NAMES:
            prf = row.find(f'data[@{key}="{perf}"]')
            if prf is not None and prf.text and prf.text.strip():
                performance_element = prf
                break
        if config_element is None or performance_element is None:
            continue
        config = config_element.text.strip() # type: ignore
        performance = float(performance_element.text.strip()) # type: ignore
        row_data = [0] * (len(column_names) - 1)  # Initialize Configuration with all zeros

        for feature in config.split(','):
            feature = feature.strip()
            if feature in features:
                index = column_names.index(feature)
                row_data[index] = 1
        row_data.append(performance) # type: ignore
        data.append(row_data)

    df = pd.DataFrame(data, columns=column_names)
    df.to_csv(csv_file_path, index=False)
    return df


def parse_VP9(xml_path, csv_path, features):
    df = pd.read_csv(xml_path, sep=";")
    df = df.loc[:, df.columns != ""]
    df.columns = df.columns.str.strip()
    
    feat_col = [f for f in features if f in df.columns]
    out = df[feat_col].copy()
    out["Performance"] = df["UserTime"]
    out.to_csv(csv_path, index=False)
    return out
   
def run():
    out = "/scratch/mani00001/vara-feature-1/bindings/python/Interplay_ML/Data"
    xmls = sorted(glob.glob(os.path.join(out, "**", "measurements*.xml"), recursive=True))
    VP9csv = sorted(glob.glob(os.path.join(out, "**", "measurements_VP9.csv"), recursive=True))
    xmls.append(VP9csv[0])
    for xml_path in xmls:
        out = os.path.splitext(xml_path)[0] + ".csv"
        system = Path(xml_path).parent.name
        featPath = Workspace / f"Random_Sampler/examples/{system}.xml"
        _,_,featNames = load_feature_model_and_extract_names(feature_model_path=featPath) # type: ignore
        if (system == "VP9"):
            parse_VP9(xml_path=xml_path, csv_path=out, features=featNames)
        else:
            parse_xml_to_csv_new(xml_file_path=xml_path, csv_file_path=out, features=featNames)

if __name__ == "__main__":
    run()
