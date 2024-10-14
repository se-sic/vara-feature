import itertools
import os
import unittest

import numpy as np
import pandas as pd
import statsmodels.api as sm

from bindings.python.ml.machine_learning import learning

class TestLearning(unittest.TestCase):
    def setUp(self):
        # Common random seed for reproducibility
        np.random.seed(42)
        self.default_random_seed = 42

        # Initialize common feature names
        self.feature_groups = {'features_5': ['Feature1', 'Feature2', 'Feature3', 'Feature4', 'Feature5'],
                               'features_4': ['A', 'B', 'C', 'D'],
                               'interaction_features': ['A$$B', 'A$$C', 'A$$D', 'B$$C', 'B$$D', 'C$$D'],
                               'extended_interactions': ['A$$B$$C', 'A$$B$$D', 'A$$C$$D', 'B$$C$$D']}

    # Helper Methods
    def generate_binary_features(self, feature_names, size=100, seed=None):
        if seed is not None:
            np.random.seed(seed)
        data = {feature: np.random.randint(0, 2, size=size) for feature in feature_names}
        return pd.DataFrame(data)

    def generate_target_variable(self, X, coef_dict, noise_std=0.5, seed=None):
        if seed is not None:
            np.random.seed(seed)
        y = sum(coef * X[feature] for feature, coef in coef_dict.items()) + np.random.randn(len(X)) * noise_std
        return y

    def add_interaction_terms(self, df, interactions):
        for interaction in interactions:
            features = interaction.split('$$')
            df[interaction] = df[features].prod(axis=1)
        return df

    def get_expected_interactions(self, selected_features, forbidden_features):
        # This method can be expanded based on specific logic for expected interactions
        pass

    # Test Methods
    def test_fit_ols_model(self):
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
        y_true = np.array([100, 200, 300])
        y_pred = np.array([110, 190, 310])
        mape = learning.calculate_mape(y_true, y_pred)
        expected_mape = (10 / 100 + 10 / 200 + 10 / 300) / 3 * 100
        self.assertAlmostEqual(mape, expected_mape, msg="MAPE calculation is incorrect.")

    def test_fit_and_evaluate(self):
        X = self.generate_binary_features(self.feature_groups['features_5'], seed=self.default_random_seed)
        coef_dict = {'Feature1': 4.5, 'Feature3': -3.2}
        y = self.generate_target_variable(X, coef_dict, seed=self.default_random_seed)

        best_features = {'Feature2'}
        feature_to_add = 'Feature1'

        feature, error = learning.fit_and_evaluate(X, y, best_features, feature_to_add)
        selected_features = sorted(best_features.union({feature_to_add}))

        model = learning.fit_ols_model(X, y, selected_features)
        predicted_error = learning.calculate_mape(y, model.predict(X[selected_features]))

        self.assertEqual(feature, feature_to_add, "The returned feature does not match the input feature.")
        self.assertEqual(error, predicted_error, "The returned error does not match the calculated MAPE.")

    def test_create_interaction_terms(self):
        df = self.generate_binary_features(['A', 'B', 'C', 'D', 'E'], seed=0)
        # All E are zeros
        df['E'] = 0

        selected_features = {'A', 'B', 'C', 'A$$B', 'B$$C', 'E'}
        initial_features = df.columns.tolist()
        forbidden_features = {'C$$D'}

        new_interactions, updated_df, updated_forbidden = learning.create_interaction_terms(df, selected_features,
                                                                                            initial_features,
                                                                                            forbidden_features)

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
        df = self.generate_binary_features(self.feature_groups['features_4'], seed=self.default_random_seed)
        # Create interaction terms
        interactions = ['AB', 'AC', 'ABC', 'ABCD', 'BCD']
        df['AB'] = df['A'] * df['B']
        df['AC'] = df['A'] * df['C']
        df['ABC'] = df['A'] * df['B'] * df['C']
        df['ABCD'] = df['A'] * df['B'] * df['C'] * df['D']
        df['BCD'] = df['B'] * df['C'] * df['D']

        y = 8 * df['A'] - 3 * df['B'] + 5 * df['AB'] + 7 * df['AC'] + np.random.randn(100) * 0.5
        features = ['A', 'B', 'AB', 'BCD', 'ABC', 'D', 'C', 'ABCD', 'AC']

        refined_features = learning.backward_selection(set(features), df, y)
        refined_refined_features = learning.backward_selection(set(refined_features), df, y)

        # Fit and evaluate models
        _, error_all = learning.fit_and_evaluate(df, y, set(features))
        _, error_refined = learning.fit_and_evaluate(df, y, set(refined_features))

        self.assertGreater(error_all, error_refined, "Error did not decrease after removing redundant features.")
        self.assertGreater(len(features), len(refined_features), "No features were deleted after backward selection.")
        self.assertEqual(set(refined_refined_features), set(refined_features),
                         "Refined features changed after a second backward selection.")

        # Exhaustive search for best subset
        min_error = float('inf')
        best_subset = None
        for r in range(1, len(features) + 1):
            for subset in itertools.combinations(features, r):
                _, error_subset = learning.fit_and_evaluate(df, y, set(subset))
                if error_subset < min_error:
                    min_error = error_subset
                    best_subset = subset

        self.assertAlmostEqual(error_refined, min_error, places=5,
                               msg="Refined features do not have the smallest error among all possible feature subsets.")
        self.assertSetEqual(set(refined_features), set(best_subset),
                            msg="Refined features do not match the best subset.")

    def test_forward_selection(self):
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

    def test_stepwise(self):
        df = self.generate_binary_features(self.feature_groups['features_4'], seed=self.default_random_seed)
        # Create interaction terms
        interaction = df['A'] * df['B']
        y = 8 * df['A'] - 3 * df['B'] + 9 * interaction - 3 * df['C'] + 10
        df['Performance'] = y

        final_model, refined_features = learning.Stepwise(df, margin=1e-2, threshold=1e-2,
                                                          random_seed=self.default_random_seed)

        expected_features = {'A', 'B', 'C', 'A$$B'}
        self.assertSetEqual(set(refined_features), expected_features,
                            msg="Stepwise did not select the correct features.")

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
        df = self.generate_binary_features(self.feature_groups['features_4'], seed=self.default_random_seed)
        # Create interaction terms
        interaction = df['A'] * df['B']
        y = 8 * df['A'] - 3 * df['B'] + 9 * interaction - 3 * df['C'] + 10
        df['Performance'] = y

        final_model, refined_features = learning.Stepwise(df, margin=1e-2, threshold=1e-2,
                                                          random_seed=self.default_random_seed)

        # Prepare interaction DataFrame
        interaction_df = pd.DataFrame({'A$$B': (df['A'] & df['B']).astype(int)})
        selected_features = ['A', 'A$$B', 'B', 'C']
        new_df = pd.concat([df, interaction_df], axis=1)

        mape = learning.validate_model(df, final_model, selected_features)

        expected_mape = learning.calculate_mape(y, final_model.predict(new_df[selected_features]))

        self.assertAlmostEqual(mape, expected_mape, places=5, msg="MAPE is not as expected.")

        # Test model export and load
        learning.export_model(final_model, selected_features, 'test-model.pkl')
        self.assertTrue(os.path.exists('test-model.pkl'))

        new_model, new_selected_features = learning.load_model('test-model.pkl')
        os.remove('test-model.pkl')

        self.assertEqual(new_selected_features, selected_features,
                         "Selected features do not match after loading the model.")

        self.assertAlmostEqual(final_model.intercept_, new_model.intercept_,
                               msg="Intercepts do not match after loading the model.")
        for new_coef, expected_coef in zip(new_model.coef_, final_model.coef_):
            self.assertAlmostEqual(new_coef, expected_coef, msg="Coefficients do not match after loading the model.")
