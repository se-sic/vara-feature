"""
Module to parse and import configurations from the SPL Conqueror CSV format
"""
import pandas as pd

import  vara_feature as vf
import vara_feature.feature_model as FM

class ParseConfigurationsFromCSV():

    def __init__(self, FM_path, CSV_path):
        self.FM_path = FM_path
        self.CSV_path = CSV_path

    def __parse_feature_model(self):
        self.fm = FM.loadFeatureModel(self.FM_path)

    def __parse_csv(self):
        # TODO: How can we give the user the oportunity to specify (the) sep(arator in the csv)?
        self.csv = pd.from_csv(self.CSV_path, sep=',')

    def parse_configurations(self):
        self.__parse_feature_model()
        self.__parse_csv()

        feature_list = list(self.csv.columns)
        feature_objects = []
        for feature in feature_list:
            feature_objects.append((feature, self.fm.get_feature(feature)))

        for index, config in self.csv.iterrows():
            for f_tupple in feature_objects:
                # TODO: How to check if feature is binary or numeric?
                if f_tuple[1].to_string == 'BinaryFeature':
                    # TODO: We have to construct configurations now -> pybinds?
                    pass





if __name__ == '__main__':
    main()
