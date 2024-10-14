# learning.py
import itertools
import pickle
import random
from concurrent.futures import ProcessPoolExecutor, as_completed

import numpy as np
import pandas as pd
from sklearn.linear_model import LinearRegression

def create_interaction_terms(df, selected_features, initial_features, forbidden_features, interaction_threshold=3):
    """
    Efficiently creates interaction terms up to a specified order.
    For:
        - Two-way interactions: Create between selected_features and initial_features, excluding self-interactions.
        - Three-way interactions: Only generate if at least two of the two-way interactions exist.
        - In general, for k-way interactions: Only generate if at least (k-1)-way interactions exist.
    Args:
        df (pd.DataFrame): The DataFrame containing feature data.
        selected_features (set): Set of currently selected feature names.
        initial_features (set): Set of initial feature names.
        forbidden_features (set): Set of feature names that are forbidden to be added.
        interaction_threshold (int): Maximum order of interactions to consider (default: 3).
    Returns:
        tuple:
            - new_interactions (list): List of newly created interaction names.
            - df (pd.DataFrame): Updated DataFrame with new interaction columns.
            - forbidden_features (set): Updated set of forbidden features.
    """

    new_interactions = []
    if interaction_threshold < 2:
        return new_interactions, df, forbidden_features
    # Ensure selected_features do not contain existing interaction terms
    selected_features_set = set(f for f in selected_features if "$$" not in f)

    # ----- Part 1: Create Two-Way Interactions -----
    for feature in selected_features_set:
        for init_feature in initial_features:
            if init_feature == feature:
                continue  # Skip self-interactions

            # Create a sorted interaction term name
            interaction_terms = sorted([feature, init_feature])
            interaction = "$$".join(interaction_terms)

            # Skip if interaction is already selected or forbidden
            if interaction in selected_features or interaction in forbidden_features:
                continue

            interaction_series = df[interaction_terms].prod(axis=1)

            # Check if the interaction has variability (i.e., not constant)
            if interaction_series.nunique() > 1:
                # Assign the new interaction term to the DataFrame
                df = df.assign(**{interaction: interaction_series.astype(int)})
                new_interactions.append(interaction)

            forbidden_features.add(interaction)
    # Iterate through interaction orders from 3 up to interaction_threshold
    for order in range(3, interaction_threshold + 1):
        # Generate all possible combinations of selected_features of the current order
        for interaction_terms in itertools.combinations(selected_features_set, order):
            # Create a sorted interaction name using '$$' as a separator
            interaction = "$$".join(sorted(interaction_terms))

            # Skip if the interaction is already in the model or forbidden
            if interaction in selected_features or interaction in forbidden_features:
                continue

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

            # Generate the interaction series by multiplying binary features
            interaction_series = df[list(interaction_terms)].prod(axis=1)

            # Check if the interaction has variability (i.e., not constant)
            if interaction_series.nunique() > 1:
                df = df.assign(**{interaction: interaction_series.astype(int)})
                new_interactions.append(interaction)

            forbidden_features.add(interaction)
    return new_interactions, df, forbidden_features


def calculate_mape(y_true, y_pred):
    return np.mean(np.abs((y_true - y_pred)) / y_true) * 100


def fit_and_evaluate(X, y, features, feature=None):
    if feature:
        selected_features = sorted(features | {feature})
    else:
        selected_features = sorted(features)
    model = fit_ols_model(X, y, selected_features)
    error = calculate_mape(y, model.predict(X[selected_features]))
    return feature, error


def forward_selection(X, y, margin=1e-2, threshold=1e-2, random_seed=42, max_interaction_order=3):
    random.seed(random_seed)  # Set the random seed for reproducibility
    initial_features = sorted(X.columns.tolist())
    best_features = set()
    current_error = float('inf')
    forbidden_features = set()
    remaining_features = set(initial_features)

    with ProcessPoolExecutor() as executor:
        while remaining_features:
            # Submit all fit_and_evaluate tasks in parallel
            future_to_feature = {executor.submit(fit_and_evaluate, X, y, best_features, feature): feature for feature in
                                 remaining_features}

            new_errors = {}
            for future in as_completed(future_to_feature):
                feature, error = future.result()
                new_errors[feature] = error

            if not new_errors:
                break  # No features were evaluated

            min_error = min(new_errors.values())

            if min_error >= current_error:
                break

            # Identify all features with the minimal error
            best_candidates = [f for f, e in new_errors.items() if e == min_error]

            # Randomly select one feature among the best candidates
            best_feature = random.choice(best_candidates)

            best_features.add(best_feature)
            current_error = min_error
            remaining_features.discard(best_feature)

            # Generate interaction terms with updated features
            interactions, X, forbidden_features = create_interaction_terms(X, best_features, initial_features,
                                                                           forbidden_features, max_interaction_order)
            remaining_features |= set(interactions)

            # Check termination conditions
            improvement = last_error - current_error if 'last_error' in locals() else None
            if improvement is not None and (improvement < margin or current_error < threshold):
                break

            last_error = current_error

    return best_features, X


def backward_selection(features, X, y):
    features = sorted(features)
    current_model = fit_ols_model(X, y, features)
    current_error = calculate_mape(y, current_model.predict(X[features]))

    while len(features) > 1:
        errors = pd.Series(dtype=float)

        for feature in features:
            reduced_features = features.copy()  # Create a copy of the original list
            reduced_features.remove(feature)
            new_model = fit_ols_model(X, y, reduced_features)
            error = calculate_mape(y, new_model.predict(X[reduced_features]))

            if current_error >= error:
                errors[feature] = error

        if not errors.empty:
            min_error = errors.min()
            candidates = errors[errors == min_error].index.tolist()

            candidates_sorted = sorted(candidates, key=lambda x: (x.count("$$"), x))
            excluded_feature = candidates_sorted[-1]  # Select the highest order feature in the sorted list
            features.remove(excluded_feature)
            current_error = min_error

        else:
            break
    return features


def fit_ols_model(X, y, features):
    X = X[features]
    model = LinearRegression().fit(X, y)

    return model


def Stepwise(df, max_interaction_order=3, margin=1e-2, threshold=1e-2, random_seed=42):
    selected_features, df_extended = forward_selection(df.drop(columns=['Performance']), df['Performance'].tolist(),
                                                       margin=margin, threshold=threshold, random_seed=random_seed,
                                                       max_interaction_order=max_interaction_order)
    refined_features = backward_selection(selected_features, df_extended, df['Performance'])
    final_model = fit_ols_model(df_extended, df['Performance'], refined_features)

    return final_model, refined_features


def export_model(model, selected_features, model_file):
    with open(model_file, 'wb') as f:
        pickle.dump((model, selected_features), f)


def load_model(model_file):
    with open(model_file, 'rb') as f:
        return pickle.load(f)


def validate_model(df, model, selected_features):
    interaction_cols = {}
    for feature in selected_features:
        if '$$' in feature:
            feat = feature.split('$$')
            interaction_cols[feature] = df[feat].all(axis=1).astype(int)
    if interaction_cols:
        interaction_df = pd.DataFrame(interaction_cols)
        df = pd.concat([df, interaction_df], axis=1)

    predictions = model.predict(df[selected_features])
    validation_error = calculate_mape(df['Performance'], predictions)

    return validation_error
