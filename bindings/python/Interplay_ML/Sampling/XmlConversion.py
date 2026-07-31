import os
import glob
import csv
import pandas as pd
from pathlib import Path
import xml.etree.ElementTree as Et
from .Config import Workspace
from ...ml.script.data_helper import parse_xml_to_csv, load_feature_model_and_extract_names

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
            parse_xml_to_csv(xml_file_path=xml_path, csv_file_path=out, features=featNames)

if __name__ == "__main__":
    run()
