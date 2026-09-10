"""Check the generated macOS product metadata without requiring Xcode."""

import pathlib
import plistlib
import sys
import unittest

PROJECT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT.parent / "iPlug2/Scripts"))
from parse_config import parse_config


class MacIdentityTest(unittest.TestCase):
    def test_supported_products(self):
        config = parse_config(str(PROJECT))
        self.assertEqual(config["BUNDLE_NAME"], "BassNAM")
        self.assertEqual(config["PLUG_UNIQUE_ID"], "BsN1")
        self.assertEqual(config["PLUG_MFR_ID"], "Dkwk")
        for suffix, api in (("macOS", "app"), ("VST3", "vst3"), ("AU", "audiounit")):
            with self.subTest(api=api):
                with (PROJECT / f"resources/NeuralAmpModeler-{suffix}-Info.plist").open("rb") as stream:
                    info = plistlib.load(stream)
                self.assertEqual(info["CFBundleIdentifier"], f"com.daeunkwak.{api}.BassNAM")
                self.assertEqual(info["CFBundleExecutable"], config["BUNDLE_NAME"])
                self.assertEqual(info["CFBundleVersion"], config["FULL_VER_STR"])
                self.assertEqual(info["CFBundleSignature"], config["PLUG_UNIQUE_ID"])
                if api == "app":
                    self.assertEqual(info["CFBundlePackageType"], "APPL")
                    self.assertTrue(info["NSMicrophoneUsageDescription"])
                    self.assertTrue((PROJECT / "resources" / (info["NSMainNibFile"] + ".xib")).is_file())
                if api == "audiounit":
                    component = info["AudioComponents"][0]
                    self.assertEqual(component["subtype"], config["PLUG_UNIQUE_ID"])
                    self.assertEqual(component["manufacturer"], config["PLUG_MFR_ID"])
                    self.assertEqual(component["factoryFunction"], config["AUV2_FACTORY"])
                    self.assertEqual(info["NSPrincipalClass"], config["AUV2_VIEW_CLASS_STR"])


if __name__ == "__main__":
    unittest.main()
