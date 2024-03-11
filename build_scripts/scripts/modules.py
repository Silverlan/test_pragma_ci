import scripts.shared
from sys import platform

scripts.shared.init_global_vars()
from scripts.shared import *

curDir = os.getcwd()

modules_dir = curDir +"/modules"
os.chdir(modules_dir)
get_submodule("interfaces","https://github.com/Silverlan/pragma_interfaces.git","874295b")
get_submodule("pr_audio_dummy","https://github.com/Silverlan/pr_audio_dummy.git","f944362b82ef8d1c80c1ae2fe18785eb92ad32fe")
get_submodule("pr_curl","https://github.com/Slaweknowy/pr_curl.git","19780e3ae9222b6202c37a222303deb6f49dd1d1")
get_submodule("pr_prosper_opengl","https://github.com/Silverlan/pr_prosper_opengl.git","a268347")
get_submodule("pr_prosper_vulkan","https://github.com/Slaweknowy/pr_prosper_vulkan.git","8dd3c34cb4bb9ea22ab93f420886c334b167724d")

os.chdir(curDir)
