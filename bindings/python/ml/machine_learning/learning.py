import itertools
import pickle
import random
from concurrent.futures import ProcessPoolExecutor, as_completed
from typing import List, Set, Tuple, Dict

import numpy as np
import pandas as pd
from sklearn.linear_model import LinearRegression


def create_interaction_terms(df: pd.DataFrame, selected_features: Set[str], initial_features: Set[str],
        forbidden_features: Set[str], interaction_threshold: int = 3) -> Tuple[List[str], pd.DataFrame, Set[str]]:
    """
    Efficiently creates interaction terms up to a specified order. Using '$$' as a separator to indicator interactions between features.

    For:
        - Two-way interactions: Create between selected_features and initial_features, excluding self-interactions.
        - Three-way interactions: Only generate if at least two of the two-way interactions exist.
        - In general, for k-way interactions: Only generate if at least (k-1)-way interactions exist.

    Args:
        df (pd.DataFrame): The DataFrame containing feature data.
        selected_features (Set[str]): Set of currently selected feature names.
        initial_features (Set[str]): Set of initial feature names.
        forbidden_features (Set[str]): Set of feature names that are forbidden to be added.
        interaction_threshold (int): Maximum order of interactions to consider (default: 3).

    Returns:
        Tuple containing:
            - new_interactions (List[str]): List of newly created interaction names.
            - df (pd.DataFrame): Updated DataFrame with new interaction columns.
            - forbidden_features (Set[str]): Updated set of forbidden features.
    """
    new_interactions = []
    if interaction_threshold <= 1:
        return new_interactions, df, forbidden_features

    non_interaction_features = {f for f in selected_features if "$$" not in f}

    # ----- Part 1: Create Two-Way Interactions -----
    for feature in non_interaction_features:
        for init_feature in initial_features:
            if init_feature == feature:
                continue  # Skip self-interactions

            # Create a sorted interaction term name
            interaction_terms = sorted([feature, init_feature])
            interaction = "$$".join(interaction_terms)

            # Skip if interaction is already selected or forbidden
            if interaction in selected_features or interaction in forbidden_features:
                continue

            # Generate the interaction series by multiplying binary features
            interaction_series = df[interaction_terms].prod(axis=1)

            # Check if the interaction has variability (i.e., not constant)
            if interaction_series.nunique() > 1:
                # Assign the new interaction term to the DataFrame
                df = df.assign(**{interaction: interaction_series.astype(int)})
                new_interactions.append(interaction)

            forbidden_features.add(interaction)

    # ----- Part 2: Create Three-Way and higher Interactions -----
    # Iterate through interaction orders from 3 up to interaction_threshold
    for order in range(3, interaction_threshold + 1):
        # Generate all possible combinations of selected_features of the current order
        for interaction_terms in itertools.combinations(non_interaction_features, order):
            # Create a sorted interaction name using '$$' as a separator
            interaction = "$$".join(sorted(interaction_terms))

            # Skip if the interaction is already in the model or forbidden
            if interaction in selected_features or interaction in forbidden_features:
                continue

            # Hierarchical Interactions
            required_sub_interactions = itertools.combinations(interaction_terms, order - 1)
            existing_sub_interactions = 0
            for subset in required_sub_interactions:
                subset_interaction = "$$".join(sorted(subset))
                if subset_interaction in selected_features:
                    existing_sub_interactions += 1
                if existing_sub_interactions >= (order - 1):
                    break

            # If fewer than (order - 1) sub-interactions exist, skip this interaction
            if existing_sub_interactions < (order - 1):
                continue

            interaction_series = df[list(interaction_terms)].prod(axis=1)

            # Check if the interaction has variability (i.e., not constant)
            if interaction_series.nunique() > 1:
                df = df.assign(**{interaction: interaction_series.astype(int)})
                new_interactions.append(interaction)

            forbidden_features.add(interaction)

    return new_interactions, df, forbidden_features


def calculate_mape(y_true: pd.DataFrame, y_pred: np.ndarray) -> float:
    """
    Calculates the Mean Absolute Percentage Error (MAPE).

    Args:
        y_true (pd.DataFrame): True target values.
        y_pred (np.ndarray): Predicted target values.

    Returns:
        float: The MAPE value.
    """
    return np.mean(np.abs((np.array(y_true) - y_pred)) / np.array(y_true)) * 100


def fit_and_evaluate(X: pd.DataFrame, y: pd.DataFrame, features: Set[str], feature: str = None) -> Tuple[str, float]:
    """
    Fits an OLS model with the given features and evaluates its error.

    Args:
        X (pd.DataFrame): Feature DataFrame.
        y (pd.DataFrame): Target values.
        features (Set[str]): Currently selected feature names.
        feature (str, optional): Feature to add to tge features list. Defaults to None.

    Returns:
        Tuple[str, float]: The feature evaluated and its corresponding error.
    """
    if feature:
        selected_features = sorted(features | {feature})
    else:
        selected_features = sorted(features)
    model = fit_ols_model(X, y, selected_features)
    error = calculate_mape(y, model.predict(X[selected_features]))
    return feature, error


def forward_selection(X: pd.DataFrame, y: pd.DataFrame, margin: float = 1e-2, threshold: float = 1e-2,
        random_seed: int = 42, max_interaction_order: int = 3) -> Tuple[Set[str], pd.DataFrame]:
    """
    Performs forward feature selection to identify the best set of features.

    Args:
        X (pd.DataFrame): Feature DataFrame.
        y (pd.DataFrame): Target values.
        margin (float, optional): Minimum improvement margin. Defaults to 1e-2.
        threshold (float, optional): Error threshold to stop selection. Defaults to 1e-2.
        random_seed (int, optional): Seed for random operations. Defaults to 42.
        max_interaction_order (int, optional): Max order of interaction terms. Defaults to 3.

    Returns:
        Tuple containing:
            - best_features (Set[str]): The selected set of features.
            - X (pd.DataFrame): Updated DataFrame with interaction terms.
    """
    random.seed(random_seed)  # Set the random seed for reproducibility
    initial_features = sorted(X.columns.tolist())
    best_features: Set[str] = set()
    current_error = float('inf')
    forbidden_features: Set[str] = set()
    remaining_features: Set[str] = set(initial_features)

    while remaining_features:
        new_errors: Dict[str, float] = {}
        for feature in remaining_features:
            _, error = fit_and_evaluate(X, y, best_features, feature)
            new_errors[feature] = error

        min_error = min(new_errors.values())

        if min_error >= current_error:
            break  # No improvement

        # Identify all features with the minimal error
        best_candidates = [f for f, e in new_errors.items() if e == min_error]

        # Randomly select one feature among the best candidates
        best_feature = random.choice(best_candidates)

        best_features.add(best_feature)
        remaining_features.discard(best_feature)

        # Generate interaction terms with updated features
        interactions, X, forbidden_features = create_interaction_terms(X, best_features, set(initial_features),
            forbidden_features, max_interaction_order)
        remaining_features |= set(interactions)

        # Check termination conditions
        improvement = current_error - min_error
        if improvement < margin or min_error < threshold:
            break

        current_error = min_error  # Update the current error
    return best_features, X


def backward_selection(features: Set[str], X: pd.DataFrame, y: pd.DataFrame) -> List[str]:
    """
    Performs backward feature selection to remove less significant features.

    Args:
        features (Set[str]): Currently selected feature names.
        X (pd.DataFrame): Feature DataFrame.
        y (pd.DataFrame): Target values.

    Returns:
        List[str]: The refined list of features after backward selection.
    """
    features = sorted(features)
    current_model = fit_ols_model(X, y, features)
    current_error = calculate_mape(y, current_model.predict(X[features]))

    while len(features) > 1:
        errors: pd.Series = pd.Series(dtype=float)

        for feature in features:
            reduced_features = features.copy()  # Create a copy of the original list
            reduced_features.remove(feature)
            reduced_model = fit_ols_model(X, y, reduced_features)
            reduced_model_error = calculate_mape(y, reduced_model.predict(X[reduced_features]))

            if reduced_model_error <= current_error:
                errors[feature] = reduced_model_error

        if not errors.empty:  # Improvement in error
            min_error = errors.min()
            candidates = errors[errors == min_error].index.tolist()

            # Sort candidates by the number of interactions and lex order
            candidates_sorted = sorted(candidates, key=lambda x: (x.count("$$"), x))
            excluded_feature = candidates_sorted[-1]  # Select the highest order feature in the sorted list
            features.remove(excluded_feature)
            current_error = min_error
        else:
            break

    return features


def fit_ols_model(X: pd.DataFrame, y: pd.DataFrame, features: List[str]) -> LinearRegression:
    """
    Fits an Ordinary Least Squares (OLS) linear regression model.

    Args:
        X (pd.DataFrame): Feature DataFrame.
        y (pd.DataFrame): Target values.
        features (List[str]): List of feature names to include in the model.

    Returns:
        LinearRegression: The fitted linear regression model.
    """
    model = LinearRegression().fit(X[features], y)
    return model


def stepwise_learning(df: pd.DataFrame, max_interaction_order: int = 3, margin: float = 1e-2, threshold: float = 1e-2,
        random_seed: int = 42) -> Tuple[LinearRegression, List[str]]:
    """
    Performs stepwise feature selection (forward and backward) to build the final model.

    Args:
        df (pd.DataFrame): The input DataFrame containing features and the target.
        max_interaction_order (int, optional): Maximum order of interaction terms. Defaults to 3.
        margin (float, optional): Minimum improvement margin for forward selection. Defaults to 1e-2.
        threshold (float, optional): Error threshold to stop selection. Defaults to 1e-2.
        random_seed (int, optional): Seed for random operations. Defaults to 42.

    Returns:
        Tuple containing:
            - final_model (LinearRegression): The final fitted linear regression model.
            - refined_features (List[str]): The set of selected feature names.
    """
    X = df.drop(columns=['Performance'])
    y = df['Performance']
    selected_features, X_extended = forward_selection(X, y, margin=margin, threshold=threshold, random_seed=random_seed,
        max_interaction_order=max_interaction_order)
    refined_features = backward_selection(selected_features, X_extended, y)
    final_model = fit_ols_model(X_extended, y, refined_features)
    return final_model, refined_features


def export_model(model: LinearRegression, selected_features: List[str], model_file: str) -> None:
    """
    Exports the trained model and selected features to a file using pickle.

    Args:
        model (LinearRegression): The trained linear regression model.
        selected_features (List[str]): The list of selected feature names.
        model_file (str): Path to the file where the model will be saved.
    """
    with open(model_file, 'wb') as f:
        pickle.dump((model, selected_features), f)


def load_model(model_file: str) -> Tuple[LinearRegression, List[str]]:
    """
    Loads a trained model and its selected features from a pickle file.

    Args:
        model_file (str): Path to the file from which the model will be loaded.

    Returns:
        Tuple containing:
            - model (LinearRegression): The loaded linear regression model.
            - selected_features (List[str]): The list of selected feature names.
    """
    with open(model_file, 'rb') as f:
        model, selected_features = pickle.load(f)
    return model, selected_features


def validate_model(df: pd.DataFrame, model: LinearRegression, selected_features: List[str]) -> float:
    """
    Validates the model by calculating the Mean Absolute Percentage Error (MAPE) on the given DataFrame.

    Args:
        df (pd.DataFrame): The DataFrame containing all measurements.
        model (LinearRegression): The trained linear regression model.
        selected_features (List[str]): The list of selected feature names.

    Returns:
        float: The validation error (MAPE).
    """
    interaction_columns: Dict[str, pd.Series] = {}
    for feature in selected_features:
        if '$$' in feature:
            feat = feature.split('$$')
            interaction_columns[feature] = df[feat].all(axis=1).astype(int)

    if interaction_columns:
        interaction_df = pd.DataFrame(interaction_columns)
        df = pd.concat([df, interaction_df], axis=1)

    predictions = model.predict(df[selected_features])
    validation_error = calculate_mape(df['Performance'], predictions)
    return validation_error
