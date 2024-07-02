# configuration.py
import json
from typing import Optional, Union, Dict, Iterator


class ConfigurationOption:
    def __init__(self, name: str, value: Union[str, bool, int]):
        self.name = name
        self.value = value

    def as_string(self) -> str:
        if isinstance(self.value, bool):
            return "true" if self.value else "false"
        return str(self.value)

    def __str__(self) -> str:
        return f"{self.name}: {self.as_string()}"

    def is_bool(self) -> bool:
        return isinstance(self.value, bool)

    def is_string(self) -> bool:
        return isinstance(self.value, str)

    def is_int(self) -> bool:
        return isinstance(self.value, int)


class Configuration:
    def __init__(self):
        self.option_mappings: Dict[str, ConfigurationOption] = {}

    @staticmethod
    def from_string(configuration_string: str) -> Optional['Configuration']:
        config = Configuration()
        try:
            parsed_config = json.loads(configuration_string)
            if not isinstance(parsed_config, dict):
                raise ValueError("The provided JSON must be a dictionary.")
            for name, value in parsed_config.items():
                config.add_option(ConfigurationOption(name, value))
        except (json.JSONDecodeError, ValueError) as e:
            print(e)
            return None
        return config

    def add_option(self, option: ConfigurationOption):
        self.option_mappings[option.name] = option

    def set_option(self, name: str, value: Union[str, bool, int]):
        self.add_option(ConfigurationOption(name, value))

    def get_option_value(self, name: str) -> Optional[Union[str, bool, int]]:
        option = self.option_mappings.get(name)
        return option.value if option else None

    def dump_to_string(self) -> str:
        config_dict = {name: option.as_string() for name, option in self.option_mappings.items()}
        return json.dumps(config_dict, indent=2)

    def __iter__(self) -> Iterator:
        return iter(self.option_mappings.items())

    def __str__(self) -> str:
        return "\n".join(str(option) for option in self.option_mappings.values())
