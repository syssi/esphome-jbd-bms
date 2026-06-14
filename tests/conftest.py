import importlib.util
import os
import sys

project_root = os.path.join(os.path.dirname(__file__), "..")
sys.path.insert(0, project_root)

# jbd_bms_up is a custom component, not part of the installed esphome package.
# Inject it so that jbd_bms_up_pack can resolve "from esphome.components import jbd_bms_up".
_init_path = os.path.join(project_root, "components", "jbd_bms_up", "__init__.py")
_spec = importlib.util.spec_from_file_location(
    "esphome.components.jbd_bms_up", _init_path
)
_mod = importlib.util.module_from_spec(_spec)
sys.modules["esphome.components.jbd_bms_up"] = _mod
_spec.loader.exec_module(_mod)
