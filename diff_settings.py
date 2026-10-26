# asm-differ settings (https://github.com/simonlindholm/asm-differ)
def apply(config, args):
    config["baseimg"] = "baserom.us.z64"
    config["myimg"] = "build/jfg.us.z64"
    config["mapfile"] = "build/jfg.us.map"
    config["source_directories"] = ["src", "include"]
    config["arch"] = "mips"
    config["objdump_executable"] = "mips-linux-gnu-objdump"
    # runlink module functions (func_mNNN_XXXXX): tools/runlink_diff.py
    import sys
    sys.path.insert(0, "tools")
    import runlink_diff
    runlink_diff.apply(config, args)
