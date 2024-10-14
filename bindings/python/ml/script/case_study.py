import argparse
import logging
import os
import sys

import pandas as pd

from bindings.python.ml.machine_learning.learning import (Stepwise, export_model, validate_model)
from bindings.python.ml.sampling.variant_generator import (generate_variants, export_configurations_to_csv)
# Importing custom modules
from data_helper import (load_feature_model_and_extract_names, find_performance_for_configurations)


def setup_logging(filename):
    """Set up logging for the experiment pipeline."""
    logger = logging.getLogger("single-experiment")
    logger.setLevel(logging.INFO)

    # Check if handlers are already added to avoid duplication
    if not logger.hasHandlers():
        # Create handlers
        c_handler = logging.StreamHandler()
        f_handler = logging.FileHandler(filename)
        c_handler.setLevel(logging.INFO)
        f_handler.setLevel(logging.INFO)

        # Create formatters and add to handlers
        c_format = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        f_format = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        c_handler.setFormatter(c_format)
        f_handler.setFormatter(f_format)

        # Add handlers to logger
        logger.addHandler(c_handler)
        logger.addHandler(f_handler)

    return logger


def parse_arguments():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(description='Experimental Pipeline for Configuration Performance Prediction')
    parser.add_argument('--measurements_csv', type=str, required=True,
                        help='Path to the measurements CSV file containing all configurations and their performance.')
    parser.add_argument('--feature_model_xml', type=str, required=True, help='Path to the feature model XML file.')
    parser.add_argument('--sample_seed', type=int, default=42, help='Random seed for sampling.')
    parser.add_argument('--sample_strategy', type=str, choices=['random', 'solver', 'distance', 'diversified-distance'],
                        required=True, help='Sampling strategy to use.')
    parser.add_argument('--sample_size', type=int, required=True, help='Number of configurations to sample.')

    parser.add_argument('--max_interaction_order', type=int, default=3,
                        help='Maximum order of interactions to consider (default: 3)')
    parser.add_argument('--margin', type=float, default=1e-2,
                        help='Margin for error improvement in feature selection (default: 1e-2)')
    parser.add_argument('--threshold', type=float, default=1e-2,
                        help='Threshold for model error to stop feature selection (default: 1e-2)')
    parser.add_argument('--learning_seed', type=int, default=42, help='Random seed for machine learning (default: 42)')
    parser.add_argument('--output_dir', type=str, default='output',
                        help='Directory to store intermediate and final outputs as well as logfiles.')

    return parser.parse_args()


def main():
    # Setup logging
    # Parse arguments
    args = parse_arguments()
    measurements_csv = args.measurements_csv
    feature_model_xml = args.feature_model_xml
    sample_seed = args.sample_seed
    sample_strategy = args.sample_strategy
    sample_size = args.sample_size
    max_interaction_order = args.max_interaction_order
    margin = args.margin
    threshold = args.threshold
    learning_seed = args.learning_seed
    output_dir = args.output_dir

    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)

    logger = setup_logging(os.path.join(output_dir, 'run.log'))

    # Check if the provided files exist
    if not os.path.isfile(measurements_csv):
        logger.error(f"Measurements CSV file does not exist: {measurements_csv}")
        sys.exit(1)  # Abort execution

    if not os.path.isfile(feature_model_xml):
        logger.error(f"Feature model XML file does not exist: {feature_model_xml}")
        sys.exit(1)  # Abort execution

    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)

    logger.info("Starting the experimental pipeline.")

    # Log the parameters
    logger.info(f"Parameters: measurements_csv={measurements_csv}, feature_model_xml={feature_model_xml}, "
                f"sample_seed={sample_seed}, sample_strategy={sample_strategy}, sample_size={sample_size}, "
                f"max_interaction_order={max_interaction_order}, margin={margin}, "
                f"threshold={threshold}, learning_seed={learning_seed}, output_dir={output_dir}")

    try:
        # Step 1: Load Feature Model and Extract Feature Names
        logger.info("Loading feature model and extracting feature names.")
        feature_model, features, feature_names = load_feature_model_and_extract_names(feature_model_xml)
        logger.info(f"Number of features extracted: {len(features)}")

        # Step 2: Generate Sampled Configurations
        logger.info("Generating sampled configurations using variant generator.")
        sampled_configurations = generate_variants(feature_model=feature_model, features_to_consider=features,
                                                   strategy=sample_strategy, sample_size=sample_size, seed=sample_seed,
                                                   distances=None
                                                   # Modify if distance-specific parameters are needed
                                                   )
        logger.info(f"Number of configurations sampled: {len(sampled_configurations)}")

        # Step 3: Export Sampled Configurations to CSV
        sampled_csv_path = os.path.join(output_dir, 'sampled_configurations.csv')
        export_configurations_to_csv(configurations=sampled_configurations, features=features,
                                     # Replace with actual feature objects if necessary
                                     file_path=sampled_csv_path)
        logger.info(f"Sampled configurations exported to {sampled_csv_path}")

        # Step 4: Match Sampled Configurations with Performance Values
        logger.info("Matching sampled configurations with their performance values.")
        df_matched = find_performance_for_configurations(measurements_csv_path=measurements_csv,
                                                         sampling_csv_path=sampled_csv_path)
        logger.info(f"Number of matched configurations: {len(df_matched)}")

        # Step 5: Train the Regression Model using Stepwise Selection
        logger.info("Training the regression model using stepwise selection.")
        model, selected_features = Stepwise(df=df_matched, max_interaction_order=max_interaction_order, margin=margin,
                                            threshold=threshold, random_seed=learning_seed)
        # Log the selected features
        logger.info(f"Selected features: {selected_features}")
        coefs = model.coef_  # Assuming model has a coef_ attribute
        intercept = model.intercept_  # Assuming model has an intercept_ attribute
        model_expression = " + ".join(f"{coef} * {feature}" for coef, feature in zip(coefs, selected_features))
        model_expression += f" + {intercept}"

        # Log the final model expression
        logger.info(f"Final model: {model_expression}")

        # Step 6: Export the Trained Model
        model_file_path = os.path.join(output_dir, 'trained_model.pkl')
        export_model(model, selected_features, model_file_path)
        logger.info(f"Trained model exported to {model_file_path}")

        # Step 7: Load All Configurations for Prediction
        logger.info("Loading all configurations for performance prediction.")
        df_all_configs = pd.read_csv(measurements_csv)
        logger.info(f"Total number of configurations: {len(df_all_configs)}")

        # Step 9: Predict Performance for All Configurations
        logger.info("Predicting performance for all configurations.")
        # Ensure all interaction terms are present
        validation_error = validate_model(df=df_all_configs, model=model,
                                          selected_features=selected_features)
        logger.info(f"Validation Error: {validation_error}")

        logger.info("Experimental pipeline completed successfully.")

    except Exception as e:
        logger.error(f"An error occurred during the experimental pipeline: {str(e)}", exc_info=True)
        sys.exit(1)


if __name__ == '__main__':
    main()
