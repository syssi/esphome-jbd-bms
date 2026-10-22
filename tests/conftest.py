import importlib.util
from pathlib import Path
import sys

project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

# jbd_bms_up is a custom component, not part of the installed esphome package.
# Inject it so that jbd_bms_up_pack can resolve "from esphome.components import jbd_bms_up".
_init_path = project_root / "components" / "jbd_bms_up" / "__init__.py"
_spec = importlib.util.spec_from_file_location(
    "esphome.components.jbd_bms_up", _init_path
)
_mod = importlib.util.module_from_spec(_spec)
sys.modules["esphome.components.jbd_bms_up"] = _mod
_spec.loader.exec_module(_mod)
