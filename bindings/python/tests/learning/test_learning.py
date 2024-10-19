import itertools
import os
import unittest
from typing import List

import numpy as np
import pandas as pd
import statsmodels.api as sm

from bindings.python.ml.machine_learning import learning


class TestLearning(unittest.TestCase):
    """
    Unit tests for the learning module.
    """

    def setUp(self):
        """
        Set up common variables and configurations for all tests.
        """
        # Common random seed for reproducibility
        np.random.seed(42)
        self.default_random_seed = 42

        # Initialize common feature names
        self.feature_groups = {'features_5': ['Feature1', 'Feature2', 'Feature3', 'Feature4', 'Feature5'],
            'features_4': ['A', 'B', 'C', 'D'],
            'interaction_features': ['A$$B', 'A$$C', 'A$$D', 'B$$C', 'B$$D', 'C$$D'],
            'extended_interactions': ['A$$B$$C', 'A$$B$$D', 'A$$C$$D', 'B$$C$$D']}

    # Helper Methods
    def generate_binary_features(self, feature_names: List[str], size: int = 100, seed: int = None) -> pd.DataFrame:
        """
        Generates a DataFrame with binary features.

        Args:
            feature_names (List[str]): List of feature names.
            size (int, optional): Number of samples. Defaults to 100.
            seed (int, optional): Random seed for reproducibility. Defaults to None.

        Returns:
            pd.DataFrame: DataFrame containing binary features.
        """
        if seed is not None:
            np.random.seed(seed)
        data = {feature: np.random.randint(0, 2, size=size) for feature in feature_names}
        return pd.DataFrame(data)

    def generate_target_variable(self, X: pd.DataFrame, coef_dict: dict, noise_std: float = 0.5,
            seed: int = None) -> pd.Series:
        """
        Generates a target variable based on linear combination of features and added noise.

        Args:
            X (pd.DataFrame): Feature DataFrame.
            coef_dict (dict): Dictionary mapping feature names to their coefficients.
            noise_std (float, optional): Standard deviation of the noise. Defaults to 0.5.
            seed (int, optional): Random seed for reproducibility. Defaults to None.

        Returns:
            pd.Series: Generated target variable.
        """
        if seed is not None:
            np.random.seed(seed)
        y = sum(coef * X[feature] for feature, coef in coef_dict.items()) + pd.Series(
            np.random.randn(len(X)) * noise_std)
        return y

    def add_interaction_terms(self, df: pd.DataFrame, interactions: List[str]) -> pd.DataFrame:
        """
        Adds interaction terms to the DataFrame.

        Args:
            df (pd.DataFrame): Original DataFrame.
            interactions (List[str]): List of interaction terms to add.

        Returns:
            pd.DataFrame: DataFrame with added interaction terms.
        """
        for interaction in interactions:
            features = interaction.split('$$')
            df[interaction] = df[features].prod(axis=1)
        return df

    # Test Methods
    def test_fit_ols_model(self):
        """
        Test the fit_ols_model function to ensure it correctly fits a linear regression model.
        """
        X = self.generate_binary_features(self.feature_groups['features_5'], seed=self.default_random_seed)
        coef_dict = {'Feature1': 4.5, 'Feature3': -3.2}
        y = self.generate_target_variable(X, coef_dict, seed=self.default_random_seed)
        selected_features = ['Feature1', 'Feature3']

        model = learning.fit_ols_model(X, y, selected_features)

        # Using statsmodels for expected coefficients
        X_selected = sm.add_constant(X[selected_features])
        ols = sm.OLS(y, X_selected).fit()

        self.assertAlmostEqual(ols.params['const'], model.intercept_, places=4,
                               msg="Intercepts do not match between custom OLS and statsmodels OLS.")
        for feature in selected_features:
            custom_coef = ols.params[feature]
            model_coef = model.coef_[selected_features.index(feature)]
            self.assertAlmostEqual(custom_coef, model_coef, places=4,
                                   msg=f"Coefficient for {feature} does not match between custom OLS and statsmodels OLS.")

    def test_calculate_mape(self):
        """
        Test the calculate_mape function to ensure it correctly computes the Mean Absolute Percentage Error.
        """
        y_true = np.array([100, 200, 300])
        y_pred = np.array([110, 190, 310])
        mape = learning.calculate_mape(y_true, y_pred)
        expected_mape = (10 / 100 + 10 / 200 + 10 / 300) / 3 * 100
        self.assertEqual(mape, expected_mape, msg="MAPE calculation is incorrect.")

    def test_fit_and_evaluate(self):
        """
        Test the fit_and_evaluate function to ensure it correctly fits a model and evaluates its error.
        """
        X = self.generate_binary_features(self.feature_groups['features_5'], seed=self.default_random_seed)
        coef_dict = {'Feature1': 4.5, 'Feature3': -3.2}
        y = self.generate_target_variable(X, coef_dict, seed=self.default_random_seed)

        best_features = {'Feature2'}
        feature_to_add = 'Feature1'

        feature, error = learning.fit_and_evaluate(X, y, best_features, feature_to_add)
        selected_features = sorted(best_features | {feature})

        model = learning.fit_ols_model(X, y, selected_features)
        predicted_error = learning.calculate_mape(y, model.predict(X[selected_features]))

        self.assertEqual(feature, feature_to_add, "The returned feature does not match the input feature.")
        self.assertAlmostEqual(error, predicted_error, places=5,
                               msg="The returned error does not match the calculated MAPE.")

    def test_create_interaction_terms(self):
        """
        Test the create_interaction_terms function to ensure it correctly adds interaction terms to the DataFrame.
        """
        df = self.generate_binary_features(['A', 'B', 'C', 'D'], seed=0)
        df['E'] = 0  # All E are zeros

        selected_features = {'A', 'B', 'C', 'A$$B', 'B$$C', 'E'}
        initial_features = set(df.columns.tolist())
        forbidden_features = {'C$$D'}

        new_interactions, updated_df, updated_forbidden = learning.create_interaction_terms(df, selected_features,
            initial_features, forbidden_features)

        expected_new_interactions = ['A$$C', 'A$$D', 'B$$D', 'A$$B$$C']
        self.assertListEqual(sorted(new_interactions), sorted(expected_new_interactions),
                             msg="New interactions do not match the expected interactions.")

        for interaction in expected_new_interactions:
            self.assertIn(interaction, updated_df.columns,
                          msg=f"Interaction column {interaction} not added to DataFrame.")
            features = interaction.split('$$')
            expected_series = updated_df[features[0]].astype(int)
            for feature in features[1:]:
                expected_series &= updated_df[feature].astype(int)
            pd.testing.assert_series_equal(updated_df[interaction], expected_series, check_names=False)

        expected_forbidden_features = forbidden_features.union(set(expected_new_interactions)).union(
            {'A$$E', 'B$$E', 'C$$E', 'D$$E'})
        self.assertSetEqual(updated_forbidden, expected_forbidden_features,
                            msg="Forbidden features not updated correctly.")

    def test_backward_selection(self):
        """
        Test the backward_selection function to ensure it correctly removes redundant features.
        """
        df = self.generate_binary_features(self.feature_groups['features_4'], seed=self.default_random_seed)
        # Create interaction terms
        interactions = ['A$$B', 'A$$C', 'A$$B$$C', 'A$$B$$C$$D', 'B$$C$$D']
        df = self.add_interaction_terms(df, interactions)

        y = 8 * df['A'] - 3 * df['B'] + 5 * df['A$$B'] + 7 * df['A$$C'] + np.random.randn(100) * 0.5
        features = ['A', 'B', 'A$$B', 'B$$C$$D', 'A$$B$$C', 'D', 'C', 'A$$B$$C$$D', 'A$$C']

        refined_features = learning.backward_selection(set(features), df, y)
        refined_refined_features = learning.backward_selection(set(refined_features), df, y)

        # Fit and evaluate models
        model_all = learning.fit_ols_model(df, y, sorted(features))
        error_all = learning.calculate_mape(y, model_all.predict(df[sorted(features)]))

        model_refined = learning.fit_ols_model(df, y, sorted(refined_features))
        error_refined = learning.calculate_mape(y, model_refined.predict(df[sorted(refined_features)]))

        self.assertGreater(error_all, error_refined, "Error did not decrease after removing redundant features.")
        self.assertGreater(len(features), len(refined_features), "No features were deleted after backward selection.")
        self.assertEqual(refined_refined_features, refined_features,
                         "Refined features changed after a second backward selection.")

        # Exhaustive search for best subset
        min_error = float('inf')
        best_subset = None
        for r in range(1, len(features) + 1):
            for subset in itertools.combinations(features, r):
                model_subset = learning.fit_ols_model(df, y, sorted(subset))
                error_subset = learning.calculate_mape(y, model_subset.predict(df[list(sorted(subset))]))
                if error_subset < min_error:
                    min_error = error_subset
                    best_subset = subset

        self.assertAlmostEqual(error_refined, min_error, places=5,
                               msg="Refined features do not have the smallest error among all possible feature subsets.")
        self.assertSetEqual(set(refined_features), set(best_subset),
                            msg="Refined features do not match the best subset.")

    def test_forward_selection(self):
        """
        Test the forward_selection function to ensure it correctly selects the best set of features.
        """
        df = self.generate_binary_features(self.feature_groups['features_4'], seed=self.default_random_seed)
        # Create interaction terms
        interaction_terms = ['A$$B', 'A$$C', 'A$$D', 'B$$C', 'B$$D', 'C$$D', 'A$$B$$C', 'A$$B$$D', 'A$$C$$D', 'B$$C$$D']
        df = self.add_interaction_terms(df, interaction_terms)

        y = 8 * df['A'] - 3 * df['B'] + 9 * df['A$$B'] - 3 * df['C'] + 10
        original_features = ['A', 'B', 'C', 'D']

        forward_features, X_forward = learning.forward_selection(df[original_features], y, margin=1e-5, threshold=1e-5)
        expected_features = {'A', 'B', 'C', 'A$$B'}

        self.assertSetEqual(set(forward_features), expected_features,
                            msg="Forward selection did not select the expected features.")

    def test_stepwise_learning(self):
        """
        Test the stepwise_selection function to ensure it correctly performs forward and backward feature selection.
        """
        df = self.generate_binary_features(self.feature_groups['features_4'], seed=self.default_random_seed)
        # Create interaction terms
        interaction = df['A'] * df['B']
        y = 8 * df['A'] - 3 * df['B'] + 9 * interaction - 3 * df['C'] + 10
        df['Performance'] = y

        final_model, refined_features = learning.stepwise_learning(df, margin=1e-2, threshold=1e-2,
            random_seed=self.default_random_seed)

        expected_features = {'A', 'B', 'C', 'A$$B'}
        self.assertSetEqual(set(refined_features), expected_features,
                            msg="Stepwise selection did not select the correct features.")

        # Check coefficients
        expected_coefficients = {'intercept': 10.0, 'A': 8.0, 'B': -3.0, 'A$$B': 9.0, 'C': -3.0}

        self.assertAlmostEqual(final_model.intercept_, expected_coefficients['intercept'], places=5,
                               msg="Intercept is not as expected.")

        coef_mapping = dict(zip(refined_features, final_model.coef_))

        for feature, expected_coef in expected_coefficients.items():
            if feature == 'intercept':
                continue
            self.assertAlmostEqual(coef_mapping.get(feature, 0), expected_coef, places=4,
                                   msg=f"Coefficient for {feature} is incorrect.")

    def test_validate_model(self):
        """
        Test the validate_model function to ensure it correctly calculates the validation error.
        """
        df = self.generate_binary_features(self.feature_groups['features_4'], seed=self.default_random_seed)
        # Create interaction terms
        interaction = df['A'] * df['B']
        y = 8 * df['A'] - 3 * df['B'] + 9 * interaction - 3 * df['C'] + 10
        df['Performance'] = y

        final_model, refined_features = learning.stepwise_learning(df, margin=1e-2, threshold=1e-2,
            random_seed=self.default_random_seed)

        selected_features = ['A', 'A$$B', 'B', 'C']
        validation_error = learning.validate_model(df, final_model, selected_features)

        # Calculate expected MAPE
        df['A$$B'] = interaction
        predictions = final_model.predict(df[selected_features])
        expected_mape = learning.calculate_mape(df['Performance'], predictions)

        self.assertAlmostEqual(validation_error, expected_mape, places=5, msg="MAPE is not as expected.")

    def test_export_and_load_model(self):
        """
        Test the export_model and load_model functions to ensure models are correctly saved and loaded.
        """
        df = self.generate_binary_features(self.feature_groups['features_4'], seed=self.default_random_seed)
        # Create interaction terms
        interaction = df['A'] * df['B']
        y = 8 * df['A'] - 3 * df['B'] + 9 * interaction - 3 * df['C'] + 10
        df['Performance'] = y

        final_model, refined_features = learning.stepwise_learning(df, margin=1e-2, threshold=1e-2,
            random_seed=self.default_random_seed)

        # Test model export and load
        model_file = 'test-model.pkl'
        learning.export_model(final_model, refined_features, model_file)
        self.assertTrue(os.path.exists(model_file), "Model file was not created.")

        loaded_model, loaded_selected_features = learning.load_model(model_file)
        os.remove(model_file)  # Clean up the model file after test

        self.assertEqual(loaded_selected_features, refined_features,
                         "Selected features do not match after loading the model.")

        self.assertAlmostEqual(final_model.intercept_, loaded_model.intercept_,
                               msg="Intercepts do not match after loading the model.")
        for new_coef, expected_coef in zip(loaded_model.coef_, final_model.coef_):
            self.assertAlmostEqual(new_coef, expected_coef, msg="Coefficients do not match after loading the model.")
