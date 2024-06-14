import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","48c1b84f2245324e90871924e4f606f846197818")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","bca2b9f34ffad6b8cf36a01fcf4165049817d05c")
get_submodule("pr_curl","https://github.com/Silverlan/pr_curl.git","4f2a574a67439104290388193cce699b64f1364d")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","550d2c0117aa6e9dc2c6a51f9ba2561b8b4d6fad")
get_submodule("pr_prosper_vulkan","https://github.com/Silverlan/pr_prosper_vulkan.git","1601be00175b02b7e6986b2b7358285f845fb4d0")

os.chdir(curDir)
