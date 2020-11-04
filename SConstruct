import os, sys
import glob

import platform

env = Environment(ENV={'PATH': os.environ['PATH']})
#env.Append(CPPPATH = "#")
#env.Append(CPPPATH = "#")
#env.Append(CPPPATH = "./X86")
# env.Append(CPPPATH = "/home/lmy/myfile/tlb/gem5-tlb5")
env.Append(CPPFLAGS = "-ggdb")
env.Append( CPPPATH=['/home/lmy/myfile/tlb/gem5-tlb5'] )


print(platform.python_version())
# allSrcs = [f for dir, subdirs, files in os.walk(".") for f in Glob(dir + "/*.cc")]
allSrcs = []
allSrcs.append("base/bitfield.hh")
allSrcs.append("base/compiler.hh")
allSrcs.append("base/intmath.hh")
allSrcs.append("faults.hh")
allSrcs.append("faults.cc")
allSrcs.append("base/logging.hh")
# allSrcs.append("mmu.cc")
# allSrcs.append("mmu.hh")
allSrcs.append("page_table.hh")
allSrcs.append("page_table.cc")
allSrcs.append("process.hh")
allSrcs.append("process.cc")
allSrcs.append("request.hh")
allSrcs.append("system.hh")
allSrcs.append("tlb.cc")
allSrcs.append("tlb.hh")
allSrcs.append("thread_context.hh")
allSrcs.append("base/trie.hh")
allSrcs.append("types.hh")
allSrcs.append("test.cc")

# source = []
# base_path = "."
# for entry in os.listdir(base_path):
#     if os.path.isdir(os.path.join(base_path, entry)):
#         print("{}/*.cc".format(entry))
#         source += glob.glob("{}.cc".format(entry))

env.Program("gem5_tlb_test", allSrcs)
