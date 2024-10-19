from dataclasses import dataclass
from typing import Optional, Union, Dict, Iterator, Tuple


@dataclass(frozen=True)
class ConfigurationOption:
    """
    Represents a configuration option with a name and a value.

    Attributes:
        name (str): The name of the configuration option.
        value (Union[str, bool, int]): The value of the configuration option.
    """
    name: str
    value: Union[str, bool, int]

    def as_string(self) -> str:
        """
        Converts the option's value to its string representation.

        Returns:
            str: The string representation of the value.
        """
        if isinstance(self.value, bool):
            return "true" if self.value else "false"
        return str(self.value)

    def __str__(self) -> str:
        """
        Provides a string representation of the configuration option.

        Returns:
            str: A string in the format "name: value".
        """
        return f"{self.name}: {self.as_string()}"

    def is_bool(self) -> bool:
        """
        Checks if the option's value is a boolean.

        Returns:
            bool: True if the value is a boolean, False otherwise.
        """
        return type(self.value) is bool

    def is_string(self) -> bool:
        """
        Checks if the option's value is a string.

        Returns:
            bool: True if the value is a string, False otherwise.
        """
        return type(self.value) is str

    def is_int(self) -> bool:
        """
        Checks if the option's value is an integer.

        Returns:
            bool: True if the value is an integer, False otherwise.
        """
        return type(self.value) is int


class Configuration:
    """
    Manages a collection of configuration options.

    Attributes:
        option_mappings (Dict[str, ConfigurationOption]):
            A dictionary mapping option names to ConfigurationOption instances.
    """

    def __init__(self):
        """
        Initializes a new Configuration instance with an empty option mapping.
        """
        self.option_mappings: Dict[str, ConfigurationOption] = {}

    def add_option(self, option: ConfigurationOption):
        """
        Adds or updates a configuration option.

        Args:
            option (ConfigurationOption): The option to add or update.
        """
        self.option_mappings[option.name] = option

    def set_option(self, name: str, value: Union[str, bool, int]):
        """
        Sets the value for a given option name.

        Args:
            name (str): The name of the option.
            value (Union[str, bool, int]): The value to set.
        """
        self.add_option(ConfigurationOption(name, value))

    def get_option_value(self, name: str) -> Optional[Union[str, bool, int]]:
        """
        Retrieves the value of a given option.

        Args:
            name (str): The name of the option.

        Returns:
            Optional[Union[str, bool, int]]: The value of the option or None if not set.
        """
        option = self.option_mappings.get(name)
        return option.value if option else None

    def __iter__(self) -> Iterator[Tuple[str, ConfigurationOption]]:
        """
        Allows iteration over configuration options sorted by name.

        Returns:
            Iterator[Tuple[str, ConfigurationOption]]: An iterator over sorted configuration options.
        """
        return iter(sorted(self.option_mappings.items(), key=lambda item: item[0]))

    def __str__(self) -> str:
        """
        Provides a string representation of the configuration with options sorted alphabetically.

        Returns:
            str: String representation with each option on a new line.
        """
        sorted_options = sorted(self.option_mappings.values(), key=lambda option: option.name)
        return "\n".join(str(option) for option in sorted_options)

    def __eq__(self, other: object) -> bool:
        """
        Checks equality with another Configuration instance.

        Args:
            other (object): The other object to compare with.

        Returns:
            bool: True if configurations are equal, False otherwise.
        """
        if not isinstance(other, Configuration):
            return False
        return self.option_mappings == other.option_mappings

    def __hash__(self) -> int:
        """
        Generates a hash based on the configuration's options.

        Returns:
            int: The hash value of the configuration.
        """
        # Since ConfigurationOption is frozen and hashable, create a frozenset of items
        return hash(frozenset(self.option_mappings.items()))
