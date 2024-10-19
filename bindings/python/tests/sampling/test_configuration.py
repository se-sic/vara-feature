import unittest

from bindings.python.ml.sampling.configuration import Configuration, ConfigurationOption


class TestConfiguration(unittest.TestCase):
    """
    Unit tests for the Configuration and ConfigurationOption classes.
    """

    def test_str_representation_sorted(self):
        """
        Test that the string representation of Configuration has options sorted alphabetically.
        """
        config = Configuration()
        config.set_option("compressionZpaq", False)
        config.set_option("compression", True)
        config.set_option("encryption", True)
        config.set_option("compressionGzip", False)

        expected_str = "\n".join(
            ["compression: true", "compressionGzip: false", "compressionZpaq: false", "encryption: true"])

        self.assertEqual(str(config), expected_str, "String representation is not sorted alphabetically.")

    def test_configuration_equality_order_independent(self):
        """
        Test that configurations with the same options but added in different orders are equal.
        """
        config1 = Configuration()
        config1.set_option("B", True)
        config1.set_option("A", False)

        config2 = Configuration()
        config2.set_option("A", False)
        config2.set_option("B", True)

        self.assertEqual(config1, config2, "Configurations with the same options but different order are not equal.")

    def test_configuration_hash_equality(self):
        """
        Test that configurations with the same options have the same hash, regardless of insertion order.
        """
        config1 = Configuration()
        config1.set_option("A", True)
        config1.set_option("B", False)

        config2 = Configuration()
        config2.set_option("B", False)
        config2.set_option("A", True)

        self.assertEqual(hash(config1), hash(config2),
                         "Configurations with the same options should hash to the same value.")

    def test_configuration_inequality_different_options(self):
        """
        Test that configurations with different options are not equal.
        """
        config1 = Configuration()
        config1.set_option("A", True)
        config1.set_option("B", False)

        config2 = Configuration()
        config2.set_option("A", False)
        config2.set_option("B", False)

        self.assertNotEqual(config1, config2, "Configurations with different options are incorrectly considered equal.")

    def test_configuration_hash_uniqueness(self):
        """
        Test that configurations with different options have different hash values.
        """
        config1 = Configuration()
        config1.set_option("A", True)
        config1.set_option("B", False)

        config2 = Configuration()
        config2.set_option("A", False)
        config2.set_option("B", True)

        self.assertNotEqual(hash(config1), hash(config2),
                            "Configurations with different options should have different hash values.")

    def test_configuration_hash_in_set(self):
        """
        Test that configurations are correctly identified as unique within a set based on their hash.
        """
        config1 = Configuration()
        config1.set_option("A", True)
        config1.set_option("B", False)

        config2 = Configuration()
        config2.set_option("B", False)
        config2.set_option("A", True)

        config_set = {config1, config2}
        self.assertEqual(len(config_set), 1, "Identical configurations should only appear once in a set.")

    def test_partial_option_set(self):
        """
        Test that configurations can have a subset of possible options set and still behave correctly.
        """
        config_with_A = Configuration()
        config_with_A.set_option("A", True)

        config_with_B = Configuration()
        config_with_B.set_option("B", False)

        self.assertNotEqual(config_with_A, config_with_B,
                            "Configurations with different subsets of options are incorrectly considered equal.")

        self.assertNotEqual(hash(config_with_A), hash(config_with_B),
                            "Configurations with different subsets of options should have different hashes.")

    def test_configuration_option_hash_equality(self):
        """
        Test that ConfigurationOption instances with the same data hash identically.
        """
        option_true_A = ConfigurationOption(name="A", value=True)
        option_true_A_duplicate = ConfigurationOption(name="A", value=True)
        option_false_A = ConfigurationOption(name="A", value=False)

        self.assertEqual(hash(option_true_A), hash(option_true_A_duplicate),
                         "ConfigurationOption instances with identical data should have the same hash.")
        self.assertNotEqual(hash(option_true_A), hash(option_false_A),
                            "ConfigurationOption instances with different data should have different hashes.")

    def test_configuration_option_equality(self):
        """
        Test that ConfigurationOption instances with the same data are equal.
        """
        option_true_A = ConfigurationOption(name="A", value=True)
        option_true_A_duplicate = ConfigurationOption(name="A", value=True)
        option_false_A = ConfigurationOption(name="A", value=False)

        self.assertEqual(option_true_A, option_true_A_duplicate,
                         "ConfigurationOption instances with identical data should be equal.")
        self.assertNotEqual(option_true_A, option_false_A,
                            "ConfigurationOption instances with different data should not be equal.")

    def test_configuration_with_multiple_options(self):
        """
        Test a configuration with multiple options of different types.
        """
        config = Configuration()
        config.set_option("A", True)
        config.set_option("B", False)
        config.set_option("C", "enabled")
        config.set_option("D", 42)

        expected_str = "\n".join(["A: true", "B: false", "C: enabled", "D: 42"])

        self.assertEqual(str(config), expected_str, "String representation with multiple options is incorrect.")

    def test_configuration_option_types(self):
        """
        Test that ConfigurationOption correctly identifies its type.
        """
        option_bool = ConfigurationOption(name="A", value=True)
        option_str = ConfigurationOption(name="B", value="active")
        option_int = ConfigurationOption(name="C", value=10)

        self.assertTrue(option_bool.is_bool(), "Option A should be identified as boolean.")
        self.assertFalse(option_bool.is_string(), "Option A should not be identified as string.")
        self.assertFalse(option_bool.is_int(), "Option A should not be identified as integer.")

        self.assertFalse(option_str.is_bool(), "Option B should not be identified as boolean.")
        self.assertTrue(option_str.is_string(), "Option B should be identified as string.")
        self.assertFalse(option_str.is_int(), "Option B should not be identified as integer.")

        self.assertFalse(option_int.is_bool(), "Option C should not be identified as boolean.")
        self.assertFalse(option_int.is_string(), "Option C should not be identified as string.")
        self.assertTrue(option_int.is_int(), "Option C should be identified as integer.")
